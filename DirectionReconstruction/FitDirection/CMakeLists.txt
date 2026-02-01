cmake_minimum_required(VERSION 2.6 FATAL_ERROR)
project(FitDirection)

#set(CMAKE_INSTALL_PREFIX $ENV{JSAPSYS}/)

list(APPEND CMAKE_PREFIX_PATH $ENV{ROOTSYS})
find_package(ROOT REQUIRED Minuit)
include(${ROOT_USE_FILE})
include_directories(${ROOT_INCLUDE_DIR})

include_directories($ENV{JSAPSYS}/Analysis)
include_directories($ENV{JSAPSYS}/Simulation/DataType/include)
include_directories($ENV{JSAPSYS}/Reconstruction/src)
include_directories($ENV{JSAPSYS})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories($ENV{JSAPSYS}/Utils)

# include_directories(/home/xinshunzhang/jinping-1ton-analysis/MuonSimRecon/MuonAnalysis/DirectionRecon/DrawFigures/include)

set(HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/include/ChannelInfo.h ${CMAKE_CURRENT_SOURCE_DIR}/include/MyStructs.h)
set(LINKDEF ${CMAKE_CURRENT_SOURCE_DIR}/include/LinkDef.h)
ROOT_GENERATE_DICTIONARY(G__ChannelInfo ${HEADERS} LINKDEF ${LINKDEF})
add_library(ChannelInfoLib SHARED G__ChannelInfo.cxx)


# 进行初步数据处理
add_executable(PreAnalysisData 
    src/PreAnalysisData.cpp
    )
target_link_libraries(PreAnalysisData ${ROOT_LIBRARIES} ChannelInfoLib)
add_executable(PreAnalysisMC 
    src/PreAnalysisMC.cpp
    $ENV{JSAPSYS}/Utils/JPUtils.cpp
    )
target_link_libraries(PreAnalysisMC ${ROOT_LIBRARIES} ChannelInfoLib)

# 进行初步拟合，获得最近k个节点的信息
# add_executable(FitMC_WA # 电荷加权法 Wang
#     src/FitMCLarge_WA.cpp
#     $ENV{JSAPSYS}/Utils/JPUtils.cpp
#     )
# target_link_libraries(FitMC_WA ${ROOT_LIBRARIES})

add_executable(FitMCLarge # 双边加权法 Wang
    src/FitMCLarge_WCT.cpp
    $ENV{JSAPSYS}/Utils/JPUtils.cpp
    )
target_link_libraries(FitMCLarge ${ROOT_LIBRARIES})

add_executable(FitMC_AA # 算术平均法 Guo
    src/FitMCLarge_AA.cpp
    $ENV{JSAPSYS}/Utils/JPUtils.cpp
    )
target_link_libraries(FitMC_AA ${ROOT_LIBRARIES})

add_executable(FitMC_CT # 时间电荷法 Zhang
    src/FitMCLarge_CT.cpp
    $ENV{JSAPSYS}/Utils/JPUtils.cpp
    )
target_link_libraries(FitMC_CT ${ROOT_LIBRARIES})

add_executable(FitDataLarge  # 双边加权法 Wang
    src/FitDataLarge_WCT.cpp
    $ENV{JSAPSYS}/Utils/JPUtils.cpp
    )
target_link_libraries(FitDataLarge ${ROOT_LIBRARIES})

add_executable(FitData_AA  # 时间距离法 Guo
    src/FitDataLarge_AA.cpp
    $ENV{JSAPSYS}/Utils/JPUtils.cpp
    )
target_link_libraries(FitData_AA ${ROOT_LIBRARIES})

add_executable(FitData_CT  # 时间电荷法 Zhang
    src/FitDataLarge_CT.cpp
    $ENV{JSAPSYS}/Utils/JPUtils.cpp
    )
target_link_libraries(FitData_CT ${ROOT_LIBRARIES})

# 进行方向重建，得到粒子的方向与位置信息
add_executable(GetMCDirection 
    src/GetMCDirection.cpp
    $ENV{JSAPSYS}/Utils/JPUtils.cpp
    )
target_link_libraries(GetMCDirection ${ROOT_LIBRARIES})

add_executable(GetDataDirection 
    src/GetDataDirection.cpp
    $ENV{JSAPSYS}/Utils/JPUtils.cpp
    )
target_link_libraries(GetDataDirection ${ROOT_LIBRARIES})

# # 使用投影方法进行重建
# add_executable(FitMCLargeProj 
#     src/FitMCLargeProjection.cpp
#     $ENV{JSAPSYS}/Utils/JPUtils.cpp
#     )
# target_link_libraries(FitMCLargeProj ${ROOT_LIBRARIES} ChannelInfoLib)

# add_executable(GetMCDirectionProj 
#     src/GetMCDirectionProjection.cpp
#     $ENV{JSAPSYS}/Utils/JPUtils.cpp
#     )
# target_link_libraries(GetMCDirectionProj ${ROOT_LIBRARIES} ChannelInfoLib)

# 对重建效果进行评估
add_executable(AngularResolution 
    src/AngularResolution.cpp
    # $ENV{JSAPSYS}/Utils/JPUtils.cpp
    )
target_link_libraries(AngularResolution ${ROOT_LIBRARIES})

add_executable(plot_k_Resolution 
    src/plot_k_Resolution.cpp
    # $ENV{JSAPSYS}/Utils/JPUtils.cpp
    )
target_link_libraries(plot_k_Resolution ${ROOT_LIBRARIES})

# 计算Pvalue
# add_executable(Pvalue 
#     src/Pvalue.cpp
#     $ENV{JSAPSYS}/Utils/JPUtils.cpp
#     )
# target_link_libraries(Pvalue ${ROOT_LIBRARIES})
# 绘制最终分布图
# copy the script to the build directory 
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/PreAnalysis.sh 
    DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/runData.sh 
    DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/runMC.sh 
    DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/Resolution.sh 
    DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/runMCProj.sh 
    DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

