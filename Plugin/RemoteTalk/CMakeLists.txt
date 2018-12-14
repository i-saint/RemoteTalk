file(GLOB sources *.cpp *.h)
add_library(RemoteTalk STATIC ${sources})
target_include_directories(RemoteTalk PUBLIC "${CMAKE_SOURCE_DIR}" "${CMAKE_SOURCE_DIR}/RemoteTalk" ${RT_ADDITIONAL_INCLUDES})
