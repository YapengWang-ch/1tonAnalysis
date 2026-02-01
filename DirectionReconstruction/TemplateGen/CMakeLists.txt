cmake_minimum_required(VERSION 3.10 FATAL_ERROR)
project(PreAnalysis)

#set(CMAKE_INSTALL_PREFIX $ENV{JSAPSYS}/)

list(APPEND CMAKE_PREFIX_PATH $ENV{ROOTSYS})
find_package(ROOT REQUIRED Minuit)
include(${ROOT_USE_FILE})
include_directories(${ROOT_INCLUDE_DIR})

include_directories($ENV{JSAPSYS}/Analysis)
include_directories($ENV{JSAPSYS}/Simulation/DataType/include)
include_directories($ENV{JSAPSYS}/Reconstruction/src)
include_directories($ENV{JSAPSYS})
include_directories($ENV{JSAPSYS} ${CMAKE_CURRENT_SOURCE_DIR}/include)

set(HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/include/ChannelInfo.h)
set(LINKDEF ${CMAKE_CURRENT_SOURCE_DIR}/include/LinkDef.h)
ROOT_GENERATE_DICTIONARY(G__ChannelInfo ${HEADERS} LINKDEF ${LINKDEF})
add_library(ChannelInfoLib SHARED G__ChannelInfo.cxx)

# add_executable(PreAnalysisMC 
#     PreAnalysisMC.cpp
#     $ENV{JSAPSYS}/Utils/JPUtils.cpp
#     )
# target_link_libraries(PreAnalysisMC ${ROOT_LIBRARIES})

# add_executable(PreAnalysisMC_60Ch 
#     PreAnalysisMC_60Ch.cpp
#     $ENV{JSAPSYS}/Utils/JPUtils.cpp
#     )
# target_link_libraries(PreAnalysisMC_60Ch ${ROOT_LIBRARIES})

add_executable(mapAnalysis
    src/mapAnalysis.cpp 
    $ENV{JSAPSYS}/Utils/JPUtils.cpp
    )
target_link_libraries(mapAnalysis ${ROOT_LIBRARIES} ChannelInfoLib)

add_executable(plotDistance
    src/plotDistance.cpp
    $ENV{JSAPSYS}/Utils/JPUtils.cpp
    )
target_link_libraries(plotDistance ${ROOT_LIBRARIES} ChannelInfoLib)

add_executable(MapGenLarge_60Ch 
    src/MapGen_60Ch.cpp
    $ENV{JSAPSYS}/Utils/JPUtils.cpp
    )
target_link_libraries(MapGenLarge_60Ch ${ROOT_LIBRARIES})

# cmake_minimum_required(VERSION 3.10 FATAL_ERROR)
# project(PreAnalysis)

# # 首先设置 ROOT 环境
# if(NOT DEFINED ENV{ROOTSYS})
#     message(FATAL_ERROR "ROOTSYS environment variable is not set! Please source thisroot.sh first.")
# endif()

# # 设置 ROOT
# list(APPEND CMAKE_PREFIX_PATH $ENV{ROOTSYS})
# find_package(ROOT REQUIRED COMPONENTS Core Minuit Physics Tree Matrix)
# if(ROOT_FOUND)
#     include(${ROOT_USE_FILE})
#     message(STATUS "Found ROOT: $ENV{ROOTSYS}")
#     message(STATUS "ROOT libraries: ${ROOT_LIBRARIES}")
#     message(STATUS "ROOT include dir: ${ROOT_INCLUDE_DIRS}")
# else()
#     message(FATAL_ERROR "ROOT package not found!")
# endif()

# # 设置编译器标志
# set(CMAKE_CXX_STANDARD 17)
# set(CMAKE_CXX_STANDARD_REQUIRED ON)

# # 包含目录
# include_directories(
#     ${ROOT_INCLUDE_DIRS}
#     $ENV{JSAPSYS}/Analysis
#     $ENV{JSAPSYS}/Simulation/DataType/include
#     $ENV{JSAPSYS}/Reconstruction/src
#     $ENV{JSAPSYS}
#     ${CMAKE_CURRENT_SOURCE_DIR}/include
# )

# # 设置库路径
# link_directories(
#     $ENV{ROOTSYS}/lib
#     $ENV{JSAPSYS}/Simulation/DataType/lib
# )

# # ChannelInfo 库
# set(HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/include/ChannelInfo.h)
# set(LINKDEF ${CMAKE_CURRENT_SOURCE_DIR}/include/LinkDef.h)
# ROOT_GENERATE_DICTIONARY(G__ChannelInfo ${HEADERS} LINKDEF ${LINKDEF})
# add_library(ChannelInfoLib SHARED G__ChannelInfo.cxx)

# # mapAnalysis 可执行文件
# add_executable(mapAnalysis
#     src/mapAnalysis.cpp 
#     $ENV{JSAPSYS}/Utils/JPUtils.cpp
# )

# target_link_libraries(mapAnalysis 
#     ${ROOT_LIBRARIES} 
#     ChannelInfoLib
#     JPSIMOUTPUT  # 确保这个库存在
# )

# # MapGenLarge_60Ch 可执行文件
# add_executable(MapGenLarge_60Ch 
#     src/MapGen_60Ch.cpp
#     $ENV{JSAPSYS}/Utils/JPUtils.cpp
# )

# target_link_libraries(MapGenLarge_60Ch 
#     ${ROOT_LIBRARIES}
# )

# # 设置运行时库路径
# if(UNIX AND NOT APPLE)
#     set_target_properties(mapAnalysis MapGenLarge_60Ch PROPERTIES
#         INSTALL_RPATH "$ENV{ROOTSYS}/lib;$ENV{JSAPSYS}/Simulation/DataType/lib"
#         BUILD_WITH_INSTALL_RPATH TRUE
#     )
# endif()
