cmake_minimum_required(VERSION 3.10)
project(WavePlot)

# 设置C++标准
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# 查找ROOT库
find_package(ROOT REQUIRED COMPONENTS RIO Net Hist Tree Graf Graf3d Gpad)

# 包含ROOT头文件
include(${ROOT_USE_FILE})

# 添加 include 目录（使用源目录绝对路径以避免相对路径问题）
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# 生成字典文件（使用源目录下的头文件绝对路径以确保编译器可找到）
ROOT_GENERATE_DICTIONARY(G__PeakInfo ${CMAKE_CURRENT_SOURCE_DIR}/include/PeakInfo.h LINKDEF ${CMAKE_CURRENT_SOURCE_DIR}/include/LinkDef.h)

# 添加可执行文件
add_executable(plot src/WavePlot.C src/PeakInfo.cpp G__PeakInfo.cxx)
add_executable(plotMC src/WavePlotMC.C src/PeakInfo.cpp G__PeakInfo.cxx)
add_executable(plotS src/WavePlot_s.C src/PeakInfo.cpp G__PeakInfo.cxx)
add_executable(Eventdata src/Eventdata.C src/PeakInfo.cpp G__PeakInfo.cxx)
add_executable(EventMC src/EventMC.C src/PeakInfo.cpp G__PeakInfo.cxx)

# 链接ROOT库
target_link_libraries(plot ${ROOT_LIBRARIES})
target_link_libraries(plotMC ${ROOT_LIBRARIES})
target_link_libraries(plotS ${ROOT_LIBRARIES})
target_link_libraries(Eventdata ${ROOT_LIBRARIES})
target_link_libraries(EventMC ${ROOT_LIBRARIES})

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/src/plot2D.py 
    DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/src/EventDisp.sh 
    DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/src/EventDispMC.sh 
    DESTINATION ${CMAKE_CURRENT_BINARY_DIR})