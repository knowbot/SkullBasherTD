# Install script for directory: C:/Users/kb/Documents/2020-22_MSC_GAMES/GAPRO/SRE_assignments/project

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "C:/Program Files (x86)/SimpleRenderEngineProject")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Debug")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  include("C:/Users/kb/Documents/2020-22_MSC_GAMES/GAPRO/SRE_assignments/project/assignment_1_asteroids/cmake_install.cmake")
  include("C:/Users/kb/Documents/2020-22_MSC_GAMES/GAPRO/SRE_assignments/project/assignment_2_wolf/cmake_install.cmake")
  include("C:/Users/kb/Documents/2020-22_MSC_GAMES/GAPRO/SRE_assignments/project/assignment_3_bird/cmake_install.cmake")
  include("C:/Users/kb/Documents/2020-22_MSC_GAMES/GAPRO/SRE_assignments/project/assignment_4_platformer/cmake_install.cmake")

endif()
