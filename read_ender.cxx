#include <QTextStream>
#include <algorithm>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "RawDataEnder.hxx"

constexpr size_t RAWDATAENDER_SIZE = 56;  // 1 agg word + 6 data words
constexpr size_t SEARCH_CHUNK_SIZE = 8192;

int main(int argc, char *argv[]) {
  if (argc < 2) {
    std::cerr << "사용법: " << argv[0] << " <raw 파일 경로>" << std::endl;
    std::cerr << "raw 데이터 파일의 엔더 정보를 읽어서 출력합니다. (파일 끝에서부터 검색)" << std::endl;
    return 1;
  }

  std::string filename = argv[1];
  std::ifstream inputFile(filename, std::ios::binary | std::ios::ate);

  if (!inputFile) {
    std::cerr << "파일을 열 수 없습니다: " << filename << std::endl;
    return 1;
  }

  std::streamsize fileSize = inputFile.tellg();
  if (fileSize < static_cast<std::streamsize>(RAWDATAENDER_SIZE)) {
    std::cerr << "파일 크기가 너무 작습니다. (최소 " << RAWDATAENDER_SIZE << " 바이트 필요)" << std::endl;
    return 1;
  }

  // 파일 끝에서부터 검색할 청크 크기
  size_t chunkSize = std::min(static_cast<size_t>(fileSize), SEARCH_CHUNK_SIZE);
  inputFile.seekg(fileSize - chunkSize, std::ios::beg);

  std::vector<char> buffer(chunkSize);
  inputFile.read(buffer.data(), chunkSize);
  if (!inputFile || static_cast<size_t>(inputFile.gcount()) < RAWDATAENDER_SIZE) {
    std::cerr << "파일 읽기 실패" << std::endl;
    return 1;
  }

  RawDataEnder *ender = nullptr;
  uint64_t data[8];
  QTextStream stream(stdout);

  // 8바이트 정렬로 역방향 스캔 (끝에서부터)
  for (size_t offset = 0; offset <= chunkSize - RAWDATAENDER_SIZE; offset += 8) {
    size_t pos = chunkSize - RAWDATAENDER_SIZE - offset;
    uint64_t aggregateWord;
    std::memcpy(&aggregateWord, buffer.data() + pos, sizeof(aggregateWord));
    aggregateWord = __builtin_bswap64(aggregateWord);

    int nWords = aggregateWord & 0xFFFFFFFF;
    int eventType = (aggregateWord >> 60) & 0xF;

    if (eventType == 5 && nWords == 6) {
      std::memcpy(data, buffer.data() + pos + 8, 6 * sizeof(uint64_t));
      ender = new RawDataEnder(data);
      break;
    }
  }

  if (!ender) {
    std::cerr << "RawDataEnder를 찾을 수 없습니다. (eventType=5인 이벤트가 없음)" << std::endl;
    return 1;
  }

  stream << "파일: " << filename.c_str() << "\n";
  ender->getEnderInfo(stream);
  stream.flush();

  delete ender;
  return 0;
}
