# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "C:/Espressif/frameworks/esp-idf-v5.1.3/components/bootloader/subproject"
  "D:/IoT182/led_light/build/bootloader"
  "D:/IoT182/led_light/build/bootloader-prefix"
  "D:/IoT182/led_light/build/bootloader-prefix/tmp"
  "D:/IoT182/led_light/build/bootloader-prefix/src/bootloader-stamp"
  "D:/IoT182/led_light/build/bootloader-prefix/src"
  "D:/IoT182/led_light/build/bootloader-prefix/src/bootloader-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "D:/IoT182/led_light/build/bootloader-prefix/src/bootloader-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "D:/IoT182/led_light/build/bootloader-prefix/src/bootloader-stamp${cfgdir}") # cfgdir has leading slash
endif()
