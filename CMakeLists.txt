cmake_minimum_required(VERSION 3.19)
project(rm_auto_aim)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

## 强制启用警告
add_definitions(-Wall -Werror)

## 导出编译命令以便使用 clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# libxr
set(XROBOT_MODULES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Modules)
set(LIBXR_LOG_OUTPUT_LEVEL 3)
set(LIBXR_PRINT_INTEGER_ENABLE_64BIT
    ON
    CACHE BOOL "" FORCE
)
set(LIBXR_PRINT_FLOAT_ENABLE_DOUBLE
    ON
    CACHE BOOL "" FORCE
)
set(LIBXR_PRINT_ENABLE_POINTER
    ON
    CACHE BOOL "" FORCE
)
add_subdirectory(libxr)

# Hardware entry: Hik camera and the DevC USB link. User/xrobot_main.hpp is
# generated for the selected product (xrobot gen -c <config>).
add_executable(${PROJECT_NAME} User/main.cpp)
target_link_libraries(${PROJECT_NAME} PUBLIC xr)
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/User)
