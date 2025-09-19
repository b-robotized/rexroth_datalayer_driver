cmake_minimum_required(VERSION 3.8)
project(datalayer_hardware_interface_nrt)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

set(THIS_PACKAGE_INCLUDE_DEPENDS
  hardware_interface
  pluginlib
  rclcpp
  rclcpp_lifecycle
)
find_package(ament_cmake REQUIRED)
foreach(Dependency IN ITEMS ${THIS_PACKAGE_INCLUDE_DEPENDS})
  find_package(${Dependency} REQUIRED)
endforeach()


set (SDK_ROOT_DIR ${CMAKE_CURRENT_LIST_DIR}/../../../../..)
MESSAGE("SDK_ROOT_DIR: ${SDK_ROOT_DIR}")
# Include directories
SET ( PRIVATE_INCLUDE_DIRS
  ${CMAKE_CURRENT_LIST_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_LIST_DIR}/include
  ${SDK_ROOT_DIR}/include
  ${SDK_ROOT_DIR}/include/comm.datalayer
)

########################################################
# DatalayerHardwareInterface_NRT
add_library(
  ${PROJECT_NAME}
  SHARED
  src/${PROJECT_NAME}.cpp
)
ament_target_dependencies(${PROJECT_NAME} 
  ${THIS_PACKAGE_INCLUDE_DEPENDS})

target_include_directories ( ${PROJECT_NAME}
  PRIVATE ${PRIVATE_INCLUDE_DIRS}
  PUBLIC
    $<INSTALL_INTERFACE:include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

# Link against libcomm_datalayer.so
find_library(COMM_DATALAYER_LIBRARY
  NAMES comm_datalayer
  HINTS ${SDK_ROOT_DIR}/bin/comm.datalayer/ubuntu22-gcc-x64
)
target_link_libraries(${PROJECT_NAME}
  ${COMM_DATALAYER_LIBRARY}
)

# When the resulting .so is copied into the bundle, make sure we know where to look for the libcomm_datalayer.so
# depending on where it is copied during bundle build: https://en.wikipedia.org/wiki/Rpath.
# default is $ORIGIN, the same dir as the datalayer_hardware_interface_nrt.so
set(BUNDLE_SDK_DEP_RPATH "$ORIGIN" CACHE STRING 
  "Path to ctrlX SDK dependencies within the final bundle.")

message(STATUS "Hardware Interface: INSTALL_RPATH will be set to '${BUNDLE_SDK_DEP_RPATH}'")
set_target_properties(datalayer_hardware_interface_nrt PROPERTIES
  INSTALL_RPATH "${BUNDLE_SDK_DEP_RPATH}"
)


########################################################
# Plugin export

pluginlib_export_plugin_description_file(hardware_interface datalayer_hardware_interface_nrt.xml)

install(
  DIRECTORY include/
  DESTINATION include
)

install(
  TARGETS ${PROJECT_NAME}
  EXPORT export_${PROJECT_NAME}
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
)

ament_export_targets(export_${PROJECT_NAME} HAS_LIBRARY_TARGET)
ament_export_dependencies(${THIS_PACKAGE_INCLUDE_DEPENDS})


ament_package()