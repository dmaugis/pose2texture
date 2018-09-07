cmake_minimum_required(VERSION 3.6)
project( pose2texture )
find_package(OpenCV REQUIRED )
find_package(Boost 1.55.0 COMPONENTS filesystem system REQUIRED)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(pose2texture_SOURCES	pose2texture.cpp)

include_directories(.)
include_directories(${OPENCV_INCLUDE_DIRS})	

add_executable( pose2texture ${pose2texture_SOURCES} )
target_link_libraries( pose2texture ${OpenCV_LIBS} ${Boost_LIBRARIES}   )
install(TARGETS pose2texture RUNTIME DESTINATION bin)	


