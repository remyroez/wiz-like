cmake_minimum_required(VERSION 3.16)
cmake_policy(SET CMP0076 NEW)

# vcpkg
if(DEFINED ENV{VCPKG_ROOT} AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  set(CMAKE_TOOLCHAIN_FILE $ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake CACHE STRING "")
endif()

project(wizlike C CXX)

add_executable(${PROJECT_NAME} WIN32 MACOSX_BUNDLE)
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)

find_package(SDL2 CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE SDL2::SDL2 SDL2::SDL2main)

find_package(imgui CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE imgui::imgui)

find_package(pugixml CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE pugixml pugixml::shared pugixml::pugixml)

find_package(Flatbuffers CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE flatbuffers::flatbuffers)

if (MSVC)
  set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "/ENTRY:mainCRTStartup")
  add_definitions(/bigobj)
endif()

add_subdirectory(src)
add_subdirectory(thirdparty)

get_property("TARGET_SOURCE_FILES" TARGET ${PROJECT_NAME} PROPERTY SOURCES)
source_group(TREE "${CMAKE_CURRENT_LIST_DIR}" FILES ${TARGET_SOURCE_FILES})
