#include "QOnlineMonitor.hxx"

QOnlineMonitor::QOnlineMonitor(QObject* parent) : QObject(parent) {
    qDebug() << "QOnlineMonitor constructor";
    httpServer = new THttpServer("http:8080");
    buffer = new uint64_t[QSHM_SIZE];
    isRunning = false;

    thread = new QThread();
    moveToThread(thread);
    connect(thread, &QThread::started, this, &QOnlineMonitor::run);
}

QOnlineMonitor::~QOnlineMonitor() {
    qDebug() << "QOnlineMonitor destructor";
    if (httpServer) delete httpServer;
    if (thread) delete thread;
    for (auto& shm : sharedMemory) {
        if (shm && shm->isAttached()) shm->detach();
        if (shm) delete shm;
    }
    for (auto& lock : locks) {
        if (lock) delete lock;
    }
    if (buffer) delete[] buffer;
}

void QOnlineMonitor::attachSharedMemory(QString name) {
    qDebug() << "QOnlineMonitor::attachSharedMemory()" << name;
    if (isRunning) return;
    sharedMemory[name] = new QSharedMemory(name);
    if (sharedMemory[name]->isAttached()) return;
    if (!sharedMemory[name]->create(QSHM_SIZE)) {
        if (sharedMemory[name]->error() == QSharedMemory::AlreadyExists) {
            qDebug() << "QOnlineMonitor::attachSharedMemory(): already exists";
            sharedMemory[name]->attach();
        } else {
            qCritical() << "Unable to create or attach to shared memory segment: " << sharedMemory[name]->errorString();
            throw std::runtime_error("Unable to create or attach to shared memory segment: " +
                                     sharedMemory[name]->errorString().toStdString());
        }
    }
    qDebug() << "QOnlineMonitor::attachSharedMemory(): created with SHM_SIZE" << QSHM_SIZE;
    uint64_t size = 0;
    memcpy(sharedMemory[name]->data(), &size, sizeof(uint64_t));  // write size = 0

    locks[name] = new QReadWriteLock();
    createHistograms(name);
}

void QOnlineMonitor::createHistograms(QString name) {
    qDebug() << "QOnlineMonitor::createHistograms()" << name;
    if (isRunning) return;
    histograms[name] = HistVec();

    canvases[name] = new TCanvas(QString("canvas_" + name).toStdString().c_str(), "DAQ Histogram");
    canvases[name]->Divide(8, 8);
    httpServer->Register("/", canvases[name]);
    TDirectory* cdir = gDirectory;
    directories[name] =
        new TDirectory(QString("dir_" + name).toStdString().c_str(), QString("dir_" + name).toStdString().c_str());
    directories[name]->cd();
    for (int i = 0; i < nCh; i++) {
        TH1F* histogram =
            new TH1F(Form("h%s_%02d", name.toStdString().c_str(), i), Form("Channel %d Histogram", i), 1000, 0, 10000);
        histograms[name].push_back(histogram);
        canvases[name]->cd(i + 1);
        histogram->Draw();
    }
    cdir->cd();
}

void QOnlineMonitor::detachSharedMemory(QString name) {
    qDebug() << "QOnlineMonitor::detachSharedMemory()" << name;
    if (isRunning) return;
    if (!sharedMemory.contains(name)) return;
    if (sharedMemory[name]->isAttached()) sharedMemory[name]->detach();
    delete sharedMemory[name];
    sharedMemory.remove(name);
    delete locks[name];
    locks.remove(name);
    removeHistograms(name);
}

void QOnlineMonitor::removeHistograms(QString name) {
    qDebug() << "QOnlineMonitor::removeHistograms()" << name;
    if (isRunning) return;
    if (!histograms.contains(name)) return;

    delete canvases[name];
    delete directories[name];
    canvases.remove(name);
    directories.remove(name);

    // for (auto& hist : histograms[name]) {
    //     if (hist) hist->Print();
    //     // delete hist;
    // }
    histograms[name].clear();
    histograms.remove(name);
}

void QOnlineMonitor::clearHistogram() {
    for (auto& shm : sharedMemory) {
        clearHistogram(shm->key());
    }
}

void QOnlineMonitor::clearHistogram(QString name) {
    for (auto& hist : histograms[name]) {
        hist->Reset();
    }
}

uint64_t QOnlineMonitor::getSharedMemorySize(QString name) {
    QReadLocker locker(locks[name]);
    uint64_t size = 0;
    memcpy(&size, sharedMemory[name]->data(), sizeof(uint64_t));
    return size;
}

uint64_t* QOnlineMonitor::getSharedMemoryData(QString name, uint64_t size) {
    QReadLocker locker(locks[name]);
    memcpy(buffer, reinterpret_cast<uint64_t*>(sharedMemory[name]->data()) + 1, size * sizeof(uint64_t));
    currentShmName = name;
    return buffer;
}

void QOnlineMonitor::clearSharedMemory(QString name) {
    QWriteLocker locker(locks[name]);
    uint64_t size = 0;
    memcpy(reinterpret_cast<uint64_t*>(sharedMemory[name]->data()), &size, sizeof(uint64_t));
    currentShmName = "";
}

void QOnlineMonitor::parseData(uint64_t* datas, uint64_t size) {
    if (size <= 0) return;
    // endian 변환
    for (int i = 0; i < size; i++) datas[i] = __builtin_bswap64(datas[i]);

    // 헤더 파싱
    int eventType = parseHeader(datas[0]);
    int startIndex;
    // eventType에 따라 적절한 처리를 수행합니다.
    switch (eventType) {
        case 1:  // Common Trigger 이벤트 처리
            break;
        case 2:  // Individual Trigger 이벤트 처리
            startIndex = 1;
            for (int i = 1; i < size; i++) {
                if ((datas[i] >> 63) & 0x1) {  // 이벤트 끝 탐지
                    eventParser(&datas[startIndex], i - startIndex + 1);
                    startIndex = i + 1;
                }  // 이벤트 수 증가
            }
            break;
        case 3:                                     // Special 이벤트 처리
            if ((aggregate_counter & 0xFF) == 3) {  // runEvent
                startRunParser(&datas[1]);
            } else if ((aggregate_counter & 0xFF) == 2) {  // runEvent
                stopRunParser(&datas[1]);
            } else {
                std::cout << "Unknown event type: " << (aggregate_counter & 0xFF) << std::endl;
            }
            break;
        default:  // 알 수 없는 이벤트 타입 처리
            break;
    }
}

int QOnlineMonitor::parseHeader(uint64_t header) {
    nWords = header & 0xFFFFFFFF;                   // 0 ~ 31 비트 추출
    aggregate_counter = (header >> 32) & 0xFFFFFF;  // 32 ~ 47 비트 추출
    flush = (header >> 56) & 0x1;                   // 56 비트 추출
    board_good = (header >> 59) & 0x1;              // 59 비트 추출
    eventType = (header >> 60) & 0xF;               // 60 ~ 63 비트 추출

    if (verbose) {
        std::cout << "nWords: " << nWords << std::endl;
        std::cout << "aggregate_counter: " << aggregate_counter << std::endl;
        std::cout << "flush: " << flush << std::endl;
        std::cout << "board_good: " << board_good << std::endl;
        std::cout << "eventType: " << eventType << std::endl;
    }

    return eventType;
}

void QOnlineMonitor::startRunParser(uint64_t* datas) {
    uint32_t acquisition_width = datas[0] & 0x1FFFFFF;
    uint16_t n_traces = (datas[0] >> 25) & 0x3;
    uint16_t dec_factor_log2 = (datas[0] >> 27) & 0x1F;
    uint64_t channel_mask = datas[1] & 0xFFFFFFFF + ((datas[2] & 0xFFFFFFFF) << 32);

    if (verbose) {
        std::cout << "Start Run 이벤트 파싱" << std::endl;
        std::cout << "acquisition_width: " << acquisition_width << std::endl;
        std::cout << "n_traces: " << n_traces << std::endl;
        std::cout << "dec_factor_log2: " << dec_factor_log2 << std::endl;
        std::cout << "channel_mask: 0x" << std::uppercase << std::hex << std::setfill('0') << std::setw(16)
                  << channel_mask << std::endl;
        std::cout << std::dec;
    }
}

void QOnlineMonitor::stopRunParser(uint64_t* datas) {
    uint64_t timestamp = datas[0] & 0xFFFFFFFFFFFF;
    uint32_t deadtime = datas[1] & 0xFFFFFFFF;
    if (verbose) {
        std::cout << "Stop Run 이벤트 파싱" << std::endl;
        std::cout << "timestamp: " << timestamp << std::endl;
        std::cout << "deadtime: " << deadtime << std::endl;
    }
}

void QOnlineMonitor::eventParser(uint64_t* datas, int nWords) {
    int channel = (datas[0] >> 56) & 0x7F;
    bool special_event = (datas[0] >> 55) & 0x1;
    uint64_t timestamp = datas[0] & 0xFFFFFFFFFFFF;
    bool with_waveform = (datas[1] >> 62) & 0x1;
    uint16_t flags_low_priority = (datas[1] >> 50) & 0x3FF;
    uint16_t flags_high_priority = (datas[1] >> 42) & 0xFF;
    uint16_t energy_short = (datas[1] >> 26) & 0xFFFF;
    uint16_t fine_timestamp = (datas[1] >> 16) & 0xFFFF;
    uint16_t energy = datas[1] & 0xFFFF;

    try {
        histograms[currentShmName][channel]->Fill(energy);
    } catch (const std::exception& e) {
        qCritical() << "Error filling histogram: " << e.what();
    }

    if (verbose) {
        std::cout << "Event Parser" << std::endl;
        std::cout << "channel: " << channel << std::endl;
        std::cout << "special_event: " << special_event << std::endl;
        std::cout << "timestamp: " << timestamp << std::endl;
        std::cout << "with_waveform: " << with_waveform << std::endl;
        std::cout << "flags_high_priority: 0x" << std::uppercase << std::hex << std::setfill('0') << std::setw(3)
                  << flags_high_priority << std::endl;
        std::cout << "flags_low_priority: 0x" << std::uppercase << std::hex << std::setfill('0') << std::setw(2)
                  << flags_low_priority << std::endl;
        std::cout << std::dec;
        std::cout << "energy_short: " << energy_short << std::endl;
        std::cout << "fine_timestamp: " << fine_timestamp << std::endl;
        std::cout << "energy: " << energy << std::endl;
        // getchar();
    }
}

void QOnlineMonitor::start() {
    qDebug() << "QOnlineMonitor::start()";
    thread->start();
    isRunning = true;
}

void QOnlineMonitor::run() {
    qDebug() << "QOnlineMonitor::run()";
    while (isRunning) {
        for (auto& shm : sharedMemory) {
            if (shm->isAttached()) {
                uint64_t size = getSharedMemorySize(shm->key());
                histograms[shm->key()][0]->Fill(100);
                if (size > 0) {
                    uint64_t* data = getSharedMemoryData(shm->key(), size);
                    parseData(data, size);
                }
            }
        }
        QThread::msleep(t_sleep);
    }
}

void QOnlineMonitor::stop() {
    qDebug() << "QOnlineMonitor::stop()";
    isRunning = false;
    thread->quit();
    thread->wait();
}
