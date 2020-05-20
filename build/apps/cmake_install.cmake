# Install script for directory: /home/manuel/Data/Università/Magistrale/ComputerGraphics/homeworks/YoctoImageDenoiser/apps

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr/local")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Release")
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

# Install shared libraries without execute permission?
if(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  set(CMAKE_INSTALL_SO_NO_EXE "1")
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  include("/home/manuel/Data/Università/Magistrale/ComputerGraphics/homeworks/YoctoImageDenoiser/build/apps/yscenetrace/cmake_install.cmake")
  include("/home/manuel/Data/Università/Magistrale/ComputerGraphics/homeworks/YoctoImageDenoiser/build/apps/yimagedenoise/cmake_install.cmake")
  include("/home/manuel/Data/Università/Magistrale/ComputerGraphics/homeworks/YoctoImageDenoiser/build/apps/prova/cmake_install.cmake")
  include("/home/manuel/Data/Università/Magistrale/ComputerGraphics/homeworks/YoctoImageDenoiser/build/apps/ysceneitraces/cmake_install.cmake")

endif()

