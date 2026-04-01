#include <QTextStream>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <iostream>
#include <string>

#include "RawDataHeader.hxx"

int main(int argc, char *argv[]) {
  if (argc < 2) {
    std::cerr << "사용법: " << argv[0] << " <raw 파일 경로>" << std::endl;
    std::cerr << "raw 데이터 파일의 헤더 정보를 읽어서 출력합니다." << std::endl;
    return 1;
  }

  std::string filename = argv[1];
  std::ifstream inputFile(filename, std::ios::binary);

  if (!inputFile) {
    std::cerr << "파일을 열 수 없습니다: " << filename << std::endl;
    return 1;
  }

  uint64_t aggregateWord;
  uint64_t data[8];
  int nWords;
  int eventType;
  RawDataHeader *header = nullptr;
  QTextStream stream(stdout);

  // 파일을 순회하며 RawDataHeader(eventType=4) 찾기
  while (inputFile.read(reinterpret_cast<char *>(&aggregateWord), sizeof(aggregateWord))) {
    aggregateWord = __builtin_bswap64(aggregateWord);

    nWords = aggregateWord & 0xFFFFFFFF;
    eventType = (aggregateWord >> 60) & 0xF;

    if (eventType == 4) {
      // RawDataHeader: 다음 8워드 읽기 (엔디안 변환 없음)
      if (nWords != 8) {
        std::cerr << "경고: RawDataHeader는 8워드이어야 하는데 " << nWords << "워드입니다." << std::endl;
      }
      for (int i = 0; i < nWords && i < 8; i++) {
        inputFile.read(reinterpret_cast<char *>(&data[i]), sizeof(data[i]));
      }
      if (!inputFile) {
        std::cerr << "헤더 데이터 읽기 실패" << std::endl;
        return 1;
      }
      header = new RawDataHeader(data);
      break;
    }

    // 다른 이벤트 타입: nWords만큼 건너뛰기
    for (int i = 0; i < nWords; i++) {
      inputFile.read(reinterpret_cast<char *>(&aggregateWord), sizeof(aggregateWord));
      if (!inputFile) break;
    }
  }

  if (!header) {
    std::cerr << "RawDataHeader를 찾을 수 없습니다. (eventType=4인 이벤트가 없음)" << std::endl;
    return 1;
  }

  stream << "파일: " << filename.c_str() << "\n";
  header->getHeaderInfo(stream);
  stream.flush();

  delete header;
  return 0;
}
