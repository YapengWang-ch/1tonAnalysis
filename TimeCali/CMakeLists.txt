cmake_minimum_required(VERSION 3.10)
project(TimeCali)

# 设置C++标准
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# 查找ROOT库
find_package(ROOT REQUIRED COMPONENTS RIO Net Hist Tree Graf Graf3d Gpad)

# 包含ROOT头文件
list(APPEND CMAKE_PREFIX_PATH $ENV{ROOTSYS})
find_package(ROOT REQUIRED Minuit)
include(${ROOT_USE_FILE})
include_directories(${ROOT_INCLUDE_DIR})

# 添加include目录
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories($ENV{JSAPSYS}/Analysis)
include_directories($ENV{JSAPSYS}/Simulation/DataType/include)
include_directories($ENV{JSAPSYS}/Reconstruction/src)
include_directories($ENV{JSAPSYS})
include_directories($ENV{JSAPSYS} ${CMAKE_CURRENT_SOURCE_DIR}/include)
# 生成字典文件（修正头文件路径）
ROOT_GENERATE_DICTIONARY(G__PeakInfo ctools.h LINKDEF include/LinkDef.h)

# 添加可执行文件
add_executable(getPeaks src/GetPeaks.cpp src/ctools.cpp G__PeakInfo.cxx)
add_executable(TimeCali src/TimeCali.cpp src/ctools.cpp G__PeakInfo.cxx)
add_executable(TimeCali2 src/TimeCali2.cpp src/ctools.cpp G__PeakInfo.cxx)
add_executable(TimeCali2w src/TimeCali2w.cpp src/ctools.cpp G__PeakInfo.cxx)
add_executable(TimeCali2w_high src/TimeCali2w_high.cpp src/ctools.cpp G__PeakInfo.cxx)
add_executable(plotDirection src/plotuniform.cpp src/ctools.cpp G__PeakInfo.cxx)


add_executable(getPeaksMC src/GetPeaks_MC.cpp src/ctools.cpp G__PeakInfo.cxx)
add_executable(getPeaksMC_ReconR src/GetPeaks_MC_ReconR.cpp src/ctools.cpp $ENV{JSAPSYS}/Utils/JPUtils.cpp G__PeakInfo.cxx)
add_executable(TimeCaliMC src/TimeCali_MC.cpp src/ctools.cpp G__PeakInfo.cxx)


add_executable(getPeaks_z src/GetPeaks_z.cpp src/ctools.cpp G__PeakInfo.cxx)
add_executable(TimeCali_z src/TimeCali_z.cpp src/ctools.cpp G__PeakInfo.cxx)

# add_executable(TQmap src/TQmap.cpp src/ctools.cpp G__PeakInfo.cxx)
add_executable(TQmapW src/TQmapWalkpro.cpp src/ctools.cpp G__PeakInfo.cxx)
# add_executable(TWfit src/TWfit.cpp src/ctools.cpp G__PeakInfo.cxx)
# add_executable(TWfitL src/TWfitL.cpp src/ctools.cpp G__PeakInfo.cxx)

# 链接库
target_link_libraries(getPeaks ${ROOT_LIBRARIES})
target_link_libraries(TimeCali ${ROOT_LIBRARIES})
target_link_libraries(TimeCali2 ${ROOT_LIBRARIES})
target_link_libraries(TimeCali2w ${ROOT_LIBRARIES})
target_link_libraries(TimeCali2w_high ${ROOT_LIBRARIES})
target_link_libraries(plotDirection ${ROOT_LIBRARIES})

target_link_libraries(getPeaksMC ${ROOT_LIBRARIES})
target_link_libraries(getPeaksMC_ReconR ${ROOT_LIBRARIES})
target_link_libraries(TimeCaliMC ${ROOT_LIBRARIES})

target_link_libraries(getPeaks_z ${ROOT_LIBRARIES})
target_link_libraries(TimeCali_z ${ROOT_LIBRARIES})

# target_link_libraries(TQmap ${ROOT_LIBRARIES})
target_link_libraries(TQmapW ${ROOT_LIBRARIES})
# target_link_libraries(TWfit ${ROOT_LIBRARIES})
# target_link_libraries(TWfitL ${ROOT_LIBRARIES})
