# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "D:/phanmemm/Espressif/frameworks/esp-idf-v5.0.1/components/bootloader/subproject"
  "D:/phanmemm/Espressif/frameworks/esp-idf-v5.0.1/workspace/uart_async_rxtxtasks/build/bootloader"
  "D:/phanmemm/Espressif/frameworks/esp-idf-v5.0.1/workspace/uart_async_rxtxtasks/build/bootloader-prefix"
  "D:/phanmemm/Espressif/frameworks/esp-idf-v5.0.1/workspace/uart_async_rxtxtasks/build/bootloader-prefix/tmp"
  "D:/phanmemm/Espressif/frameworks/esp-idf-v5.0.1/workspace/uart_async_rxtxtasks/build/bootloader-prefix/src/bootloader-stamp"
  "D:/phanmemm/Espressif/frameworks/esp-idf-v5.0.1/workspace/uart_async_rxtxtasks/build/bootloader-prefix/src"
  "D:/phanmemm/Espressif/frameworks/esp-idf-v5.0.1/workspace/uart_async_rxtxtasks/build/bootloader-prefix/src/bootloader-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "D:/phanmemm/Espressif/frameworks/esp-idf-v5.0.1/workspace/uart_async_rxtxtasks/build/bootloader-prefix/src/bootloader-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "D:/phanmemm/Espressif/frameworks/esp-idf-v5.0.1/workspace/uart_async_rxtxtasks/build/bootloader-prefix/src/bootloader-stamp${cfgdir}") # cfgdir has leading slash
endif()
