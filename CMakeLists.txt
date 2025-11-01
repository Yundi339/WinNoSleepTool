cmake_minimum_required(VERSION 3.15)
project(WinNoSleepTool VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_CXX_FLAGS_RELEASE "-O2 -DNDEBUG -s")
set(CMAKE_CXX_FLAGS_DEBUG "-g -ggdb")

if(WIN32)
    add_definitions(-DWIN32 -D_WIN32)
    # 静态链接 C/C++
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libgcc -static-libstdc++")
    # 隐藏控制台窗口
    set(CMAKE_WIN32_EXECUTABLE TRUE)
endif()

# 源文件
set(SOURCES
    main.cpp
)
# 可执行文件
add_executable(${PROJECT_NAME} WIN32 ${SOURCES})

if(WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE psapi)
endif()

install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin
)

