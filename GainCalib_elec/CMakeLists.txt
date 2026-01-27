cmake_minimum_required(VERSION 3.10)
project(GainCalib)

# 设置C++标准
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# 查找ROOT库
find_package(ROOT REQUIRED COMPONENTS RIO Net Hist Tree Graf Graf3d Gpad)

# 包含ROOT头文件
include(${ROOT_USE_FILE})

# 添加include目录
include_directories(include)

# 生成字典文件（修正头文件路径）
ROOT_GENERATE_DICTIONARY(G__PeakInfo ctools.h ChannelInfo.h LINKDEF include/LinkDef.h)

# 添加可执行文件
add_executable(GetPeaks src/GetPeaks_preAnalysis.C src/ctools.cpp G__PeakInfo.cxx)
add_executable(GetDN src/GetDN.C src/ctools.cpp G__PeakInfo.cxx)
add_executable(GetDN_FullInt src/GetDN_FullInt.C src/ctools.cpp G__PeakInfo.cxx)
add_executable(GetGain src/GetGain.C src/ctools.cpp include/SER.h include/MultiGauss.h G__PeakInfo.cxx)
add_executable(GetFullGain src/GetGain_full.C src/ctools.cpp include/SER.h include/MultiGauss.h G__PeakInfo.cxx)

add_executable(GetGain_PhaseI src/GetGain_PhaseI.C src/ctools.cpp include/SER.h include/MultiGauss.h G__PeakInfo.cxx)
add_executable(GetDN_pre src/GetDN_preAnalysis.C src/ctools.cpp G__PeakInfo.cxx)
add_executable(GetDNEvo src/GetDNEvo.C src/ctools.cpp G__PeakInfo.cxx)
add_executable(SERScan src/SERScan.C src/ctools.cpp include/SER.h include/MultiGauss.h G__PeakInfo.cxx)


# 链接ROOT库
target_link_libraries(GetPeaks ${ROOT_LIBRARIES})
target_link_libraries(GetDN ${ROOT_LIBRARIES})
target_link_libraries(GetDN_FullInt ${ROOT_LIBRARIES})
target_link_libraries(GetDNEvo ${ROOT_LIBRARIES})
target_link_libraries(GetGain ${ROOT_LIBRARIES})
target_link_libraries(GetFullGain ${ROOT_LIBRARIES})
target_link_libraries(GetGain_PhaseI ${ROOT_LIBRARIES})
target_link_libraries(GetDN_pre ${ROOT_LIBRARIES})
target_link_libraries(SERScan ${ROOT_LIBRARIES})

# file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/src/plot2D.py 
#     DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
# file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/EventDisp.sh 
#     DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
# file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/EventDispMC.sh 
#     DESTINATION ${CMAKE_CURRENT_BINARY_DIR})