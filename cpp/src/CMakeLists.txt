cmake_minimum_required (VERSION 2.8)
project (deep_blue_genome)

cmake_policy(SET CMP0058 NEW)  # suppresses a warning

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/../cmake_modules")

include(cotire)  # Speeds up build process

add_definitions(-DBOOST_RESULT_OF_USE_DECLTYPE) # Needed to build boost 1.57 on gcc 4.8

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

set(CMAKE_CXX_FLAGS_RELEASE "-O3  -DNDEBUG") # -flto try with plugin
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-g -O3  -fno-omit-frame-pointer -DNDEBUG")
set(CMAKE_CXX_FLAGS "-Wall -Wextra -Wno-sign-compare -Wno-unknown-pragmas -Wno-unused-parameter -std=c++1y -ftemplate-depth-1024 ${SDL_CFLAGS}") # why the SDL flags?

#############
# common.so 
file(GLOB_RECURSE common_sources deep_blue_genome/common/*.cpp)
	
add_library(common SHARED ${common_sources})

target_link_libraries(common
	${GSL_LIBRARIES}
	${Boost_LIBRARIES}
	${YAMLCPP_LIBRARY}
)

set_target_properties(common PROPERTIES COTIRE_CXX_PREFIX_HEADER_INIT "deep_blue_genome/common/stdafx.h") # precompiled header file
cotire(common)

#############
# database
file(GLOB_RECURSE database_sources deep_blue_genome/database/*.cpp)
add_executable(database ${database_sources})

target_link_libraries(database
	common
)

set_target_properties(database PROPERTIES COTIRE_CXX_PREFIX_HEADER_INIT "deep_blue_genome/database/stdafx.h") # precompiled header file
cotire(database)

#############
# morph
file(GLOB_RECURSE morph_sources deep_blue_genome/morph/*.cpp)
add_executable(morph ${morph_sources})

target_link_libraries(morph
	common
)

set_target_properties(morph PROPERTIES COTIRE_CXX_PREFIX_HEADER_INIT "deep_blue_genome/morph/stdafx.h") # precompiled header file
cotire(morph)

#############
# coexpnetviz
file(GLOB_RECURSE coexpnetviz_sources deep_blue_genome/coexpr/*.cpp)
add_executable(coexpnetviz ${coexpnetviz_sources})

target_link_libraries(coexpnetviz
	common
)

set_target_properties(coexpnetviz PROPERTIES COTIRE_CXX_PREFIX_HEADER_INIT "deep_blue_genome/coexpr/stdafx.h") # precompiled header file
cotire(coexpnetviz)


#############
# ortholog_stats
file(GLOB_RECURSE ortholog_stats_sources deep_blue_genome/ortholog_stats/*.cpp)
add_executable(ortholog_stats ${ortholog_stats_sources})

target_link_libraries(ortholog_stats
	common
)

set_target_properties(ortholog_stats PROPERTIES COTIRE_CXX_PREFIX_HEADER_INIT "deep_blue_genome/ortholog_stats/stdafx.h") # precompiled header file
cotire(ortholog_stats)

##############
# Install instructions
install(TARGETS common database morph coexpnetviz ortholog_stats
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)
