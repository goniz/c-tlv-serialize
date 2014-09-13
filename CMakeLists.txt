cmake_minimum_required(VERSION 2.8.4)
project(c_tlv_serialize)

set(SOURCE_FILES main.c lib/c-tlv.c lib/struct.c lib/enums.c)
include_directories("lib")
add_executable(c_tlv_serialize ${SOURCE_FILES})