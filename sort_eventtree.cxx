#include <cstdint>
#include <iostream>
#include <memory>
#include <queue>
#include <string>
#include <utility>
#include <vector>

#include "TFile.h"
#include "TTree.h"

namespace {

struct EventRow {
  UChar_t channel = 0;
  ULong64_t timestamp = 0;
  UShort_t fine_timestamp = 0;
  UShort_t energy = 0;
  UShort_t energy_short = 0;
  UShort_t flags_low_priority = 0;
  UShort_t flags_high_priority = 0;
  UInt_t event_size = 0;
  Double_t cal_energy = 0.0;
};

void PrintUsage(const char *prog) {
  std::cerr << "사용법: " << prog << " <입력1.root> [입력2.root ...] [-o <출력.root>] [-t <트리이름>]\n"
            << "  기본 트리이름: EventTree\n"
            << "  기본 출력파일: sorted.root\n"
            << "  동작: 각 입력 파일의 트리가 이미 정렬돼 있다고 가정하고 K-way merge로 병합/정렬합니다.\n"
            << "\n"
            << "예시:\n"
            << "  " << prog << " run1.root -o run1_sorted.root\n"
            << "  " << prog << " run*.root -o merged_sorted.root\n";
}

}  // namespace

int main(int argc, char **argv) {
  if (argc < 2) {
    PrintUsage(argv[0]);
    return 1;
  }

  std::string outName = "sorted.root";
  std::string treeName = "EventTree";
  std::vector<std::string> inputs;

  for (int i = 1; i < argc; ++i) {
    const std::string a = argv[i];
    if (a == "-o") {
      if (i + 1 >= argc) {
        std::cerr << "에러: -o 뒤에 출력 파일명이 필요합니다.\n";
        return 1;
      }
      outName = argv[++i];
    } else if (a == "-t") {
      if (i + 1 >= argc) {
        std::cerr << "에러: -t 뒤에 트리 이름이 필요합니다.\n";
        return 1;
      }
      treeName = argv[++i];
    } else if (!a.empty() && a[0] == '-') {
      std::cerr << "알 수 없는 옵션: " << a << "\n";
      PrintUsage(argv[0]);
      return 1;
    } else {
      inputs.push_back(a);
    }
  }

  if (inputs.empty()) {
    std::cerr << "에러: 입력 ROOT 파일이 없습니다.\n";
    PrintUsage(argv[0]);
    return 1;
  }

  struct InputState {
    std::string filename;
    std::unique_ptr<TFile> file;
    TTree *tree = nullptr;
    Long64_t nEntries = 0;
    Long64_t entry = 0;
    bool hasCalEnergy = false;
    std::unique_ptr<EventRow> row;
  };

  auto RequireBranch = [&](TTree *t, const char *name) -> bool {
    if (!t->GetBranch(name)) {
      std::cerr << "에러: 필수 브랜치가 없습니다 (" << name << ")\n";
      return false;
    }
    return true;
  };

  std::vector<InputState> ins;
  ins.reserve(inputs.size());
  bool anyCalEnergy = false;
  std::string firstTreeTitle = "";

  for (const auto &fn : inputs) {
    InputState st;
    st.filename = fn;
    st.file.reset(TFile::Open(fn.c_str(), "READ"));
    if (!st.file || st.file->IsZombie()) {
      std::cerr << "경고: 파일을 열 수 없습니다: " << fn << "\n";
      continue;
    }

    st.tree = dynamic_cast<TTree *>(st.file->Get(treeName.c_str()));
    if (!st.tree) {
      std::cerr << "경고: 트리(" << treeName << ")를 찾지 못했습니다: " << fn << "\n";
      continue;
    }

    if (!RequireBranch(st.tree, "channel") || !RequireBranch(st.tree, "timestamp") ||
        !RequireBranch(st.tree, "fine_timestamp") || !RequireBranch(st.tree, "energy") ||
        !RequireBranch(st.tree, "energy_short") || !RequireBranch(st.tree, "flags_low_priority") ||
        !RequireBranch(st.tree, "flags_high_priority") || !RequireBranch(st.tree, "event_size")) {
      return 1;
    }

    // 브랜치 주소는 "이동되지 않는" 메모리에 걸어야 함.
    // InputState는 vector에 move될 수 있으므로, row는 힙에 두고 주소를 고정한다.
    st.row = std::make_unique<EventRow>();
    st.tree->SetBranchAddress("channel", &st.row->channel);
    st.tree->SetBranchAddress("timestamp", &st.row->timestamp);
    st.tree->SetBranchAddress("fine_timestamp", &st.row->fine_timestamp);
    st.tree->SetBranchAddress("energy", &st.row->energy);
    st.tree->SetBranchAddress("energy_short", &st.row->energy_short);
    st.tree->SetBranchAddress("flags_low_priority", &st.row->flags_low_priority);
    st.tree->SetBranchAddress("flags_high_priority", &st.row->flags_high_priority);
    st.tree->SetBranchAddress("event_size", &st.row->event_size);
    st.hasCalEnergy = (st.tree->GetBranch("cal_energy") != nullptr);
    if (st.hasCalEnergy) {
      st.tree->SetBranchAddress("cal_energy", &st.row->cal_energy);
      anyCalEnergy = true;
    }

    st.nEntries = st.tree->GetEntries();
    if (st.nEntries <= 0) {
      std::cerr << "경고: 엔트리가 0개입니다: " << fn << "\n";
      continue;
    }

    if (firstTreeTitle.empty()) {
      firstTreeTitle = st.tree->GetTitle() ? st.tree->GetTitle() : "";
    }

    st.entry = 0;
    st.tree->GetEntry(st.entry);
    ins.push_back(std::move(st));
  }

  if (ins.empty()) {
    std::cerr << "에러: 유효한 입력 트리가 없습니다. 트리 이름(" << treeName << ")과 입력 파일을 확인하세요.\n";
    return 1;
  }

  struct Node {
    size_t idx = 0;  // index into ins
    ULong64_t timestamp = 0;
    UShort_t fine = 0;
  };

  struct NodeGreater {
    bool operator()(const Node &a, const Node &b) const {
      if (a.timestamp != b.timestamp) return a.timestamp > b.timestamp;  // min-heap behavior
      return a.fine > b.fine;
    }
  };

  std::priority_queue<Node, std::vector<Node>, NodeGreater> pq;
  for (size_t i = 0; i < ins.size(); ++i) {
    pq.push(Node{i, ins[i].row->timestamp, ins[i].row->fine_timestamp});
  }

  TFile out(outName.c_str(), "RECREATE");
  if (out.IsZombie()) {
    std::cerr << "에러: 출력 ROOT 파일을 만들 수 없습니다: " << outName << "\n";
    return 1;
  }

  EventRow outRow;
  Int_t source_id = -1;
  TTree outTree(treeName.c_str(), firstTreeTitle.c_str());
  outTree.Branch("channel", &outRow.channel, "channel/b");
  outTree.Branch("timestamp", &outRow.timestamp, "timestamp/l");
  outTree.Branch("fine_timestamp", &outRow.fine_timestamp, "fine_timestamp/s");
  outTree.Branch("energy", &outRow.energy, "energy/s");
  outTree.Branch("energy_short", &outRow.energy_short, "energy_short/s");
  if (anyCalEnergy) outTree.Branch("cal_energy", &outRow.cal_energy, "cal_energy/D");
  outTree.Branch("flags_low_priority", &outRow.flags_low_priority, "flags_low_priority/s");
  outTree.Branch("flags_high_priority", &outRow.flags_high_priority, "flags_high_priority/s");
  outTree.Branch("event_size", &outRow.event_size, "event_size/i");
  outTree.Branch("source_id", &source_id, "source_id/I");

  Long64_t written = 0;
  while (!pq.empty()) {
    const Node nd = pq.top();
    pq.pop();
    InputState &st = ins[nd.idx];

    // st.row 는 st.entry 엔트리를 이미 읽어둔 상태
    source_id = static_cast<Int_t>(nd.idx);
    outRow.channel = st.row->channel;
    outRow.timestamp = st.row->timestamp;
    outRow.fine_timestamp = st.row->fine_timestamp;
    outRow.energy = st.row->energy;
    outRow.energy_short = st.row->energy_short;
    outRow.flags_low_priority = st.row->flags_low_priority;
    outRow.flags_high_priority = st.row->flags_high_priority;
    outRow.event_size = st.row->event_size;
    if (anyCalEnergy) {
      outRow.cal_energy = st.hasCalEnergy ? st.row->cal_energy : 0.0;
    }

    outTree.Fill();
    written++;

    st.entry++;
    if (st.entry < st.nEntries) {
      st.tree->GetEntry(st.entry);
      pq.push(Node{nd.idx, st.row->timestamp, st.row->fine_timestamp});
    }

    if (written % 500000 == 0) {
      std::cout << "\r병합 기록 중: " << written << " (활성 입력: " << pq.size() << ")" << std::flush;
    }
  }
  std::cout << "\r병합 기록 완료: " << written << " (활성 입력: 0)\n";

  outTree.Write();
  out.Close();

  std::cout << "완료: " << outName << " 에 정렬된 트리(" << treeName << ")를 저장했습니다.\n";
  std::cout << "출력 엔트리 수: " << written << "\n";
  return 0;
}
