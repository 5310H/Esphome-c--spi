cmake_minimum_required(VERSION 3.10)
project(esphome_c_api C)

# Public headers
include_directories(include)

# Library sources (placeholders for now)
add_library(esphome_c_api
    src/noise/noise.c
    src/transport/transport.c
    src/protobuf/messages.c
    src/api/esphome_api.c
)

# Example programs (optional)
add_executable(linux_client examples/linux_client.c)
target_link_libraries(linux_client esphome_c_api)
