cmake_minimum_required(VERSION 3.4.1)

set(CMAKE_CXX_STANDARD 14)

SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")

set(libusb-dir ../../../3rdparty/libusb)

include_directories(
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${libusb-dir})

set(libusb-libs ${libusb-dir}/libs/${ANDROID_ABI})

link_directories(${libusb-libs})

aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR} SRC)

add_library(usb_accessory SHARED ${SRC})

target_link_libraries(usb_accessory log usb1.0)