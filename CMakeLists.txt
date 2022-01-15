# For more information about build system see
# https://docs.espressif.com/projects/esp-idf/en/latest/api-guides/build-system.html
# The following five lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.5)

set(HOMEKIT_PATH $ENV{IDF_PATH}/../esp-homekit-sdk/)

set(src ${HOMEKIT_PATH})
list(APPEND src ${HOMEKIT_PATH}/components)
list(APPEND src ${HOMEKIT_PATH}/components/homekit)
list(APPEND src ${HOMEKIT_PATH}/examples/common)

set(EXTRA_COMPONENT_DIRS "${src}")

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(cloud-bulb)
