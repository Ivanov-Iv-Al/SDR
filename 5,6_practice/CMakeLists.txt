cmake_minimum_required(VERSION 3.16)
project(PlutoSDR CXX)

set(CMAKE_BUILD_TYPE Debug)

# Ищем библиотеку SoapySDR
find_package(SoapySDR REQUIRED)

set(MAIN_SOURCE_FILES
    src/main.cpp
)

set(MODULATION_SOURCE_FILES
    modulation/main.cpp
    modulation/BPSK_modulator.cpp
    modulation/QPSK_modulator.cpp
)

# Добавляем исполняемый файл
add_executable(main.out ${MAIN_SOURCE_FILES})
add_executable(modulation.out ${MODULATION_SOURCE_FILES})

# Линкуем библиотеки к исполняемому файлу
target_link_libraries(main.out ${SoapySDR_LIBRARIES})
