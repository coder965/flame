cmake_minimum_required(VERSION 3.4.1)

set (CMAKE_CXX_STANDARD 14)

add_library(native-lib SHARED src/main/cpp/native-lib.cpp
            C:/Users/Administrator/AppData/Local/Android/Sdk/ndk-bundle/sources/android/native_app_glue/android_native_app_glue.c)

find_library(log-lib log)

target_link_libraries(native-lib ${log-lib})

find_library(android-lib android)

target_link_libraries(native-lib ${android-lib})

target_include_directories(native-lib PUBLIC "C:/Users/Administrator/AppData/Local/Android/Sdk/ndk-bundle/sources/android/native_app_glue")
