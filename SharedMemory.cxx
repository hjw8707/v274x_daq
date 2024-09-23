#include "SharedMemory.hxx"

#include <iostream>
// 생성자
SharedMemory::SharedMemory(const std::string& name, bool readOnly)
    : shmName(name), shmFd(-1), shmData(nullptr), readOnly(readOnly) {
    // 공유 메모리 생성
    if (shmName.empty()) shmName = "/v2740";
    if (shmName.front() != '/') shmName = "/" + shmName;

    Initialize();
}

bool SharedMemory::Initialize() {
    shmFd = shm_open(shmName.c_str(), readOnly ? O_RDWR : (O_CREAT | O_RDWR), 0666);
    if (shmFd == -1) {
        std::cerr << "Failed to create shared memory" << std::endl;
        return false;
    }

    // 크기 설정
    shmSize = sizeof(SharedMemoryData);
    if (!readOnly && ftruncate(shmFd, shmSize) == -1) {
        shm_unlink(shmName.c_str());
        std::cerr << "Failed to set size of shared memory" << std::endl;
        shmFd = -1;
        return false;
    }

    // 메모리 매핑
    shmData = static_cast<SharedMemoryData*>(mmap(0, shmSize, PROT_READ | PROT_WRITE, MAP_SHARED, shmFd, 0));
    if (shmData == MAP_FAILED) {
        shm_unlink(shmName.c_str());
        std::cerr << "Failed to map shared memory" << std::endl;
        shmFd = -1;
        return false;
    }

    // 세마포어 생성
    // shmSemaphore = sem_open("pSem", O_CREAT, 0666, 1);
    // if (shmSemaphore == SEM_FAILED) {
    //     throw std::runtime_error("Failed to create semaphore");
    // }
    if (readOnly) {
        std::cout << "Shared memory initialized (read only)" << std::endl;
    } else {
        std::cout << "Shared memory initialized (read/write)" << std::endl;
    }
    return true;
}

// 소멸자
SharedMemory::~SharedMemory() {
    // 매핑 해제
    if (shmData != nullptr) {
        munmap(shmData, shmSize);
    }
    // 공유 메모리 삭제
    shm_unlink(shmName.c_str());

    sem_unlink("pSem");
    // 세마포어 닫기
    sem_close(shmSemaphore);
}

// 데이터 쓰기
void SharedMemory::writeData(const void* data, size_t size) {
    if (readOnly) {
        throw std::runtime_error("Shared memory is read-only");
    }
    if (size > shmSize) {
        throw std::runtime_error("Data size exceeds shared memory size");
    }
    // sem_wait(shmSemaphore);
    shmData->newData = true;
    shmData->size = size;
    std::memcpy(shmData->data, data, size);
    // sem_post(shmSemaphore);
}

// 데이터 읽기
size_t SharedMemory::readData(void* buffer) {
    if (shmFd == -1) {
        std::cerr << "Shared memory is not opened" << std::endl;
        std::cerr << "Try to open shared memory" << std::endl;
        shmFd = shm_open(shmName.c_str(), O_RDWR, 0666);
        if (shmFd == -1) {
            std::cerr << "Failed to open shared memory" << std::endl;
            return 0;
        }
        shmData = static_cast<SharedMemoryData*>(mmap(0, shmSize, PROT_READ | PROT_WRITE, MAP_SHARED, shmFd, 0));
        if (shmData == MAP_FAILED) {
            std::cerr << "Failed to map shared memory" << std::endl;
            return 0;
        }
    }
    // sem_wait(shmSemaphore);
    std::memcpy(buffer, shmData->data, shmData->size);
    // sem_post(shmSemaphore);
    return shmData->size;
}

// 데이터 읽기
size_t SharedMemory::readData(void* buffer, size_t size) {
    if (size > shmData->size) {
        throw std::runtime_error("Buffer size exceeds shared memory size");
    }
    // sem_wait(shmSemaphore);
    std::memcpy(buffer, shmData->data, size);
    // sem_post(shmSemaphore);
    return size;
}

// 새로운 데이터 읽기
size_t SharedMemory::readNewData(void* buffer) {
    if (!shmData->newData) return 0;
    //  sem_wait(shmSemaphore);
    std::memcpy(buffer, shmData->data, shmData->size);
    shmData->newData = false;
    // sem_post(shmSemaphore);
    return shmData->size;
}

// 새로운 데이터 읽기
size_t SharedMemory::readNewData(void* buffer, size_t size) {
    if (!shmData->newData) return 0;
    if (size > shmData->size) {
        throw std::runtime_error("Buffer size exceeds shared memory size");
    }
    // sem_wait(shmSemaphore);
    std::memcpy(buffer, shmData->data, size);
    shmData->newData = false;
    // sem_post(shmSemaphore);
    return size;
}