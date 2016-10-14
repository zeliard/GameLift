cmake_minimum_required(VERSION 2.8)

project(GameLiftLinuxServer)

add_definitions(-std=c++11)

include_directories(include)
link_directories(${CMAKE_SOURCE_DIR}/lib)

file(GLOB GLServerSRC
	"*.h"
	"*.cpp"
	"../PacketShare/PacketType.h"
)

add_executable(GameLiftLinuxServer ${GLServerSRC})

target_link_libraries(GameLiftLinuxServer aws-cpp-sdk-gamelift-server sioclient boost_date_time boost_random boost_system protobuf pthread)


