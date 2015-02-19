cmake_minimum_required (VERSION 2.8)
project (morph)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/../cmake_modules")

############################################
# find dependencies

find_package(Boost REQUIRED COMPONENTS filesystem iostreams system serialization regex program_options)
include_directories(${Boost_INCLUDE_DIRS})

find_package(GSL REQUIRED COMPONENTS)
include_directories(${GSL_INCLUDE_DIR})

find_package(YAMLCpp REQUIRED COMPONENTS)
include_directories(${YAMLCPP_INCLUDE_DIR})

set(PC_LIBODB_INCLUDE_DIRS "/usr/include/odb")
set(PC_LIBODB_LIBRARY_DIRS "/usr/lib/odb")
find_package(ODB REQUIRED COMPONENTS mysql)

include(${ODB_USE_FILE})

############################################

include_directories(BEFORE ${CMAKE_SOURCE_DIR})

set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O3 -g -DNDEBUG")
set(CMAKE_CXX_FLAGS "-Wall -Wno-sign-compare -Wno-unknown-pragmas -std=c++11 ${SDL_CFLAGS}") # TODO consider turning on sign-compare again

#############
# common.so 
file(GLOB_RECURSE common_sources deep_blue_genome/common/*.cpp)
file(GLOB_RECURSE common_headers deep_blue_genome/common/*.h)

# Note: compiles some headers into more C++ code; adds generated sources to common_sources
get_property(include_dirs DIRECTORY PROPERTY INCLUDE_DIRECTORIES)
odb_compile(common_sources FILES ${common_headers} INCLUDE ${include_dirs} DB mysql STANDARD c++11 SCHEMA_FORMAT sql GENERATE_SCHEMA GENERATE_QUERY GENERATE_SESSION GENERATE_PREPARED)
	
add_library(common SHARED ${common_sources})
	
target_include_directories(common
	PRIVATE
	${ODB_INCLUDE_DIRS}
	${ODB_COMPILE_OUTPUT_DIR}
)

target_link_libraries(common
	${GSL_LIBRARIES}
	${Boost_LIBRARIES}
	${YAMLCPP_LIBRARY}
    ${ODB_LIBRARIES}
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
