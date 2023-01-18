
FILE (GLOB ALL_SOURCES "src/*.cpp" "src/*.c"  )
FILE (GLOB ALL_INCLUDES "includes/*.hpp" "includes/*.h" )

SET (ALL_SRCS 
	 ${ALL_SOURCES}
	 ${ALL_INCLUDES}
)

add_executable(LearnAllocator ${ALL_SRCS})
target_include_directories(LearnAllocator PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/includes)
target_compile_features(LearnAllocator PRIVATE cxx_std_17)
