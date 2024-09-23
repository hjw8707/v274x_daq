#ifndef SHAREDMEMORY_H
#define SHAREDMEMORY_H

#define SHM_SIZE 2621440

#include <fcntl.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include <cstring>
#include <stdexcept>
#include <string>

struct SharedMemoryData {
    bool newData;
    size_t size;
    uint8_t data[SHM_SIZE];
};

class SharedMemory {
   public:
    SharedMemory(const std::string& name, bool readOnly = false);
    ~SharedMemory();

    bool Initialize();
    bool IsOpen() { return shmFd != -1; }

    void writeData(const void* data, size_t size);
    // @brief 공유 메모리에서 데이터를 읽는 함수
    // @param buffer 데이터를 저장할 버퍼
    // @return 읽은 데이터의 크기 (byte)
    size_t readData(void* buffer);
    // @brief 공유 메모리에서 데이터를 읽는 함수
    // @param buffer 데이터를 저장할 버퍼
    // @param size 읽을 데이터의 크기 (byte)
    // @return 읽은 데이터의 크기 (byte)
    size_t readData(void* buffer, size_t size);
    // @brief 공유 메모리에서 새로운 데이터를 읽는 함수
    // @param buffer 데이터를 저장할 버퍼
    // @return 읽은 데이터의 크기 (byte)
    size_t readNewData(void* buffer);
    // @brief 공유 메모리에서 새로운 데이터를 읽는 함수
    // @param buffer 데이터를 저장할 버퍼
    // @param size 읽을 데이터의 크기 (byte)
    // @return 읽은 데이터의 크기 (byte)
    size_t readNewData(void* buffer, size_t size);

   private:
    std::string shmName;        // 공유 메모리 이름
    size_t shmSize;             // 공유 메모리 크기
    bool readOnly;              // 읽기 전용 여부
    int shmFd;                  // 파일 디스크립터
    SharedMemoryData* shmData;  // 공유 메모리 데이터
    sem_t* shmSemaphore;        // 공유 메모리 접근을 위한 세마포어
};

#endif  // SHAREDMEMORY_H