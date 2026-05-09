cmake_minimum_required(VERSION 3.19)
project(OS_MemoryAllocation LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable Qt's automatic compilers
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets Gui)

qt_standard_project_setup()

# 1. Define the source files
set(PROJECT_SOURCES
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    MemoryCanvas.cpp
    MemoryCanvas.h
    MemoryManager.cpp
    MemoryManager.h
    MemoryModels.h
)

# 2. Create the executable (The Target)
qt_add_executable(OS_MemoryAllocation
    WIN32 MACOSX_BUNDLE
    ${PROJECT_SOURCES}
)

# 3. NOW specify include directories (After the target is created)
target_include_directories(OS_MemoryAllocation PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# 4. Link the necessary Qt modules
target_link_libraries(OS_MemoryAllocation
    PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::Gui
)

# Deployment settings (Optional for school projects but keeps CMake happy)
install(TARGETS OS_MemoryAllocation
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)