cmake_minimum_required(VERSION 3.10)
project(NavigationSystem VERSION 1.0)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 添加 Qt6
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# 查找Qt组件
find_package(Qt6 COMPONENTS Widgets REQUIRED)

# 根据不同平台设置编译标志
if(MSVC)
    # Windows MSVC编译器标志
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
    
    # 为Windows应用程序设置属性
    set(CMAKE_WIN32_EXECUTABLE ON)
else()
    # GCC/Clang编译器标志（macOS/Linux）
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
endif()

# 设置输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# 查找所有源文件
file(GLOB_RECURSE APP_SOURCES "src/app/*.cpp")
file(GLOB_RECURSE CORE_SOURCES "src/core/*.cpp")
file(GLOB_RECURSE ALGORITHMS_SOURCES "src/algorithms/*.cpp")
file(GLOB_RECURSE UI_SOURCES "src/ui/*.cpp")
file(GLOB_RECURSE UI_HEADERS "src/ui/*.h")

# 创建可执行文件
add_executable(navigation_system
    MACOSX_BUNDLE # <--- 确保添加或存在这一行
    ${APP_SOURCES}
    ${CORE_SOURCES}
    ${ALGORITHMS_SOURCES}
    ${UI_SOURCES}
    ${UI_HEADERS}
)

# 添加头文件目录
target_include_directories(navigation_system PRIVATE src)

# 链接 Qt6 Widgets
target_link_libraries(navigation_system PRIVATE Qt6::Widgets)

# 添加线程库
find_package(Threads REQUIRED)
target_link_libraries(navigation_system PRIVATE Threads::Threads)

# 为Windows平台添加额外的库和设置
if(WIN32)
    # 添加Windows特定的库
    target_link_libraries(navigation_system PRIVATE ws2_32)
    
    # 增加链接器的堆栈大小
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /STACK:10000000")
    
    # 使用响应文件来处理长命令行
    set(CMAKE_C_USE_RESPONSE_FILE_FOR_OBJECTS 1)
    set(CMAKE_CXX_USE_RESPONSE_FILE_FOR_OBJECTS 1)
    set(CMAKE_C_USE_RESPONSE_FILE_FOR_INCLUDES 1)
    set(CMAKE_CXX_USE_RESPONSE_FILE_FOR_INCLUDES 1)
    set(CMAKE_C_RESPONSE_FILE_LINK_FLAG "@")
    set(CMAKE_CXX_RESPONSE_FILE_LINK_FLAG "@")
    
    # 确保正确部署Qt DLL
    # add_custom_command(TARGET navigation_system POST_BUILD
    #     COMMAND ${CMAKE_COMMAND} -E echo "正在部署Qt依赖..."
    #     COMMAND windeployqt --qmldir ${CMAKE_SOURCE_DIR}/src $<TARGET_FILE:navigation_system>
    # )
endif()
