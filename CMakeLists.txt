cmake_minimum_required(VERSION 3.16)
cmake_policy(SET CMP0076 NEW)

project(App C CXX)

add_executable(App WIN32 MACOSX_BUNDLE)
target_compile_features(App PRIVATE cxx_std_17)
target_include_directories(App PRIVATE include)
target_link_directories(App PRIVATE lib)

#target_link_libraries(App PRIVATE
#)

if (MSVC)
  set_target_properties(App PROPERTIES LINK_FLAGS "/ENTRY:mainCRTStartup")
  add_definitions(/bigobj)
endif()

add_subdirectory(src)

get_property("TARGET_SOURCE_FILES" TARGET App PROPERTY SOURCES)
source_group(TREE "${CMAKE_CURRENT_LIST_DIR}" FILES ${TARGET_SOURCE_FILES})
