# 최소 CMake 버전 설정
cmake_minimum_required(VERSION 3.10)

# 프로젝트 이름 및 버전
project(V274X_DAQ VERSION 1.0 LANGUAGES CXX)

# C++17 표준 사용
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# yaml-cpp, ROOT 라이브러리 찾기
find_package(ryml REQUIRED)
find_package(c4core REQUIRED)
find_package(ROOT REQUIRED)

# Qt 라이브러리 찾기
find_package(Qt6 COMPONENTS Core Gui Widgets REQUIRED)

# 실행 파일을 생성할 소스 파일 설정
add_executable(test test.cc)

# 라이브러리 디렉토리 추가
# (라이브러리 디렉토리를 프로젝트의 라이브러리 경로로 수정)
link_directories(/usr/local/lib /opt/homebrew/lib)

# 외부 라이브러리 링크
# (여기에 실제 라이브러리 이름을 추가)
find_library(CAEN_FELib_PATH NAMES CAEN_FELib PATHS /usr/local/lib)
target_link_libraries(test PRIVATE ${CAEN_FELib_PATH})

# 포함 디렉토리 설정
# (헤더 파일이 위치한 디렉토리를 추가)
target_include_directories(test PRIVATE /usr/local/include)

# v2740 관련 소스 파일 목록 설정
set(V2740_SOURCES
    CAENV2740.cxx
    CAENV2740Par.cxx
    CAENV2740Event.cxx
    QCAENV2740.cxx
    # 필요한 다른 소스 파일들을 여기에 추가하세요
)

# v2740 공유 라이브러리 생성
add_library(v2740 SHARED ${V2740_SOURCES})
#set_target_properties(v2740 PROPERTIES
#    AUTOMOC ON
#    AUTOUIC ON
#    AUTORCC ON
#)

# v2740 라이브러리에 대한 include 디렉토리 설정
target_include_directories(v2740 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include /usr/local/include /opt/homebrew/include)

# v2740 라이브러리 링크
target_link_libraries(v2740 PRIVATE ${CAEN_FELib_PATH} ryml c4core Qt6::Core Qt6::Gui Qt6::Widgets)

# test 실행 파일에 v2740 라이브러리 링크
target_link_libraries(test PRIVATE v2740)

# daq.cxx 컴파일 관련 내용 추가
add_executable(daq daq.cxx)
target_link_libraries(daq PRIVATE v2740)


# v274x_daq.cxx 컴파일 관련 내용 추가
add_executable(v274x_daq v274x_daq.cxx)
target_link_libraries(v274x_daq PRIVATE v2740 Qt6::Widgets)

# daq.cxx 컴파일 관련 내용 추가
add_executable(readRaw readRaw.cxx)
target_include_directories(readRaw PRIVATE /usr/local/include ${ROOT_INCLUDE_DIRS})
target_link_libraries(readRaw PRIVATE v2740 ${ROOT_LIBRARIES})


# 옵션으로 디버깅 정보 생성
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_definitions(-DDEBUG)
endif()