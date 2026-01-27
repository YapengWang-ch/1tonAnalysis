cmake_minimum_required(VERSION 3.0 FATAL_ERROR)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
project(PreAnalysis)
option(CMAKE_BUILD_TYPE "Release")
SET(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g -ggdb")
SET(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall")

list(APPEND CMAKE_PREFIX_PATH $ENV{ROOTSYS})
find_package(ROOT COMPONENTS Minuit XMLParser)

include_directories($ENV{JSAPSYS})
include_directories($ENV{JSAPSYS}/Simulation/DataType/include)
include_directories($ENV{JSAPSYS}/Reconstruction/ReconAlgorithm)
include(${ROOT_USE_FILE})
include_directories(${ROOT_INCLUDE_DIR})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp")

ROOT_GENERATE_DICTIONARY(G__XMLREADER $ENV{JSAPSYS}/Utils/PMTPosReader.h OPTIONS -noIncludePaths)
add_library(XMLREADER SHARED G__XMLREADER.cxx)
target_link_libraries(XMLREADER ${ROOT_LIBRARIES})

ROOT_GENERATE_DICTIONARY(G__CHANNELINFO ${PROJECT_SOURCE_DIR}/ChannelInfo.h LINKDEF ${PROJECT_SOURCE_DIR}/LinkDef.h OPTIONS -noIncludePaths)
add_library(CHANNELINFO SHARED G__CHANNELINFO.cxx)
target_link_libraries(CHANNELINFO ${ROOT_LIBRARIES})

if(NOT PreAlgorithm)
	set(PreAlgorithm WuYy)
endif()
if(NOT FindPeakAlgorithm)
	set(FindPeakAlgorithm FindPeaksSG)
endif()
message("Using PreAlgorithm : ${PreAlgorithm}")
message("Using FindPeak Algorithm : ${FindPeakAlgorithm}")
add_definitions(-D_${PreAlgorithm}_Algorithm_)
add_definitions(-D_FindPeakAlgorithm_=${FindPeakAlgorithm})

add_executable(PreAnalysisData 
    PreAnalysisData.cpp
    PreAnalysisTree.cpp
    $ENV{JSAPSYS}/Utils/JPUtils.cpp
	$ENV{JSAPSYS}/Reconstruction/ReconAlgorithm/Recon1.cpp
    )

add_executable(PreAnalysisMC 
	PreAnalysisMC.cpp
    PreAnalysisTree.cpp
    $ENV{JSAPSYS}/Utils/JPUtils.cpp
	$ENV{JSAPSYS}/Reconstruction/ReconAlgorithm/Recon1.cpp)

add_executable(check_preAna 
    check_preAna.cc
    )

target_link_libraries(PreAnalysisData ${ROOT_LIBRARIES})
target_link_libraries(PreAnalysisMC ${ROOT_LIBRARIES})
target_link_libraries(check_preAna ${ROOT_LIBRARIES})

# add_executable(test test.C)
# target_link_libraries(test ${ROOT_LIBRARIES})
