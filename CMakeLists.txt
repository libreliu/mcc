cmake_minimum_required (VERSION 2.8)

project (GeoPad)

set( CMAKE_EXPORT_COMPILE_COMMANDS ON )


aux_source_directory (src DIR_SRCS)

include_directories (include)

add_executable (mcc.out ${DIR_SRCS})

# target_link_libraries (mcc.out m)

set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -g")

