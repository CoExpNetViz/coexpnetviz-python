cmake_minimum_required (VERSION 2.8)
project (deep_blue_genome)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/../cmake_modules")

############################################
# find dependencies

find_package(Boost REQUIRED COMPONENTS filesystem iostreams system serialization regex program_options)
include_directories(${Boost_INCLUDE_DIRS})

# TODO still using gsl?
find_package(GSL REQUIRED COMPONENTS)
include_directories(${GSL_INCLUDE_DIR})

find_package(YAMLCpp REQUIRED COMPONENTS)
include_directories(${YAMLCPP_INCLUDE_DIR})

############################################

include_directories(BEFORE ${CMAKE_SOURCE_DIR})

set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g -DNDEBUG") #TODO for truly verifying it, go -O3
set(CMAKE_CXX_FLAGS "-Wall -Wno-sign-compare -Wno-unknown-pragmas -std=c++1y -ftemplate-depth-1024 ${SDL_CFLAGS}") # TODO consider turning on sign-compare again

#############
# common.so 
file(GLOB_RECURSE common_sources deep_blue_genome/common/*.cpp)
	
add_library(common SHARED ${common_sources})

target_link_libraries(common
	${GSL_LIBRARIES}
	${Boost_LIBRARIES}
	${YAMLCPP_LIBRARY}
)
		
#############
# database
file(GLOB_RECURSE database_sources deep_blue_genome/database/*.cpp)
add_executable(database ${database_sources})

target_link_libraries(database
	common
	ncurses #TODO we're not using this atm
)

#############
# morph
#file(GLOB_RECURSE morph_sources deep_blue_genome/morph/*.cpp)
#add_executable(morph ${morph_sources})

#target_link_libraries(morph
#	common
#)

#############
# coexpr
file(GLOB_RECURSE coexpr_sources deep_blue_genome/coexpr/*.cpp)
add_executable(coexpr ${coexpr_sources})

target_link_libraries(coexpr
	common
)
