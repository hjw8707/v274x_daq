# 최소 CMake 버전 설정
cmake_minimum_required(VERSION 3.10)

# 프로젝트 이름 및 버전
project(V274X_DAQ VERSION 1.0 LANGUAGES CXX)

# C++17 표준 사용
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# yaml-cpp 라이브러리 찾기
find_package(yaml-cpp REQUIRED)

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
    v2740.cxx
    v2740par.cxx
    # 필요한 다른 소스 파일들을 여기에 추가하세요
)

# v2740 공유 라이브러리 생성
add_library(v2740 SHARED ${V2740_SOURCES})

# v2740 라이브러리에 대한 include 디렉토리 설정
target_include_directories(v2740 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include /usr/local/include /opt/homebrew/include)

# v2740 라이브러리 링크
target_link_libraries(v2740 PRIVATE ${CAEN_FELib_PATH} yaml-cpp)

# test 실행 파일에 v2740 라이브러리 링크
target_link_libraries(test PRIVATE v2740)

# 옵션으로 디버깅 정보 생성
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_definitions(-DDEBUG)
endif()