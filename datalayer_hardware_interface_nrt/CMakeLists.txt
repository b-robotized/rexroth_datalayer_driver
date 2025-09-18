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

# Root of the ctrlx SDK
set (SDK_ROOT_DIR ${CMAKE_CURRENT_LIST_DIR}/../../../../..)
MESSAGE("SDK_ROOT_DIR: ${SDK_ROOT_DIR}")
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
ament_target_dependencies(${PROJECT_NAME} ${THIS_PACKAGE_INCLUDE_DEPENDS})

target_include_directories ( ${PROJECT_NAME}
  PRIVATE ${PRIVATE_INCLUDE_DIRS}
  PUBLIC
    $<INSTALL_INTERFACE:include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
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