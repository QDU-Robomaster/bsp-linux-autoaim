cmake_minimum_required(VERSION 3.10)
project(rm_auto_aim)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

## 强制启用警告
add_definitions(-Wall -Werror)

## 导出编译命令以便使用 clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# libxr
set(XROBOT_MODULES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Modules)
# Both generated entries are stamp-checked before building.
set(XROBOT_MAIN_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/User/xrobot_main.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/User/Replay/xrobot_main.hpp
)
set(LIBXR_LOG_OUTPUT_LEVEL 2)
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

# Hardware entry (Hik camera + DevC USB) and recorded-frame replay entry. Each
# includes the xrobot_main.hpp generated next to its own main source.
add_executable(${PROJECT_NAME} User/main.cpp)
add_executable(${PROJECT_NAME}_replay User/Replay/main.cpp)

foreach(_target ${PROJECT_NAME} ${PROJECT_NAME}_replay)
  target_link_libraries(${_target} PUBLIC xr)
  target_include_directories(${_target} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/User)
endforeach()
