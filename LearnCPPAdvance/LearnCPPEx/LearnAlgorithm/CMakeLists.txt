
FILE (GLOB ALL_SOURCES "*/*.cpp" "*/*.c"  )
FILE (GLOB ALL_INCLUDES "*/*.hpp" "*/*.h" )

SET (ALL_SRCS 
	 ${ALL_SOURCES}
	 ${ALL_INCLUDES}
)

# file(GLOB MY_INCLUDE_DIRECTORIES LIST_DIRECTORIES TRUE "*/")
# message("${MY_INCLUDE_DIRECTORIES}")
# set(INCLUDE_DIRECTORIES )

add_executable(LearnAlgorithm ${ALL_SRCS})
# target_include_directories(LearnAlgorithm PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/includes)
target_include_directories(LearnAlgorithm PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_compile_features(LearnAlgorithm PRIVATE cxx_std_17)