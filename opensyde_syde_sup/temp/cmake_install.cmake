# Install script for directory: /home/marko/Grimme/openSYDE/opensyde_syde_sup/pjt

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
  set(CMAKE_CROSSCOMPILING "TRUE")
endif()

# Set default install directory permissions.
if(NOT DEFINED CMAKE_OBJDUMP)
  set(CMAKE_OBJDUMP "/usr/bin/objdump")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/marko/Grimme/openSYDE/opensyde_syde_sup/temp/opensyde_core/cmake_install.cmake")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/home/marko/Grimme/openSYDE/opensyde_syde_sup/pjt/../result/SYDEsup")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  file(INSTALL DESTINATION "/home/marko/Grimme/openSYDE/opensyde_syde_sup/pjt/../result" TYPE EXECUTABLE FILES "/home/marko/Grimme/openSYDE/opensyde_syde_sup/temp/CMakeFiles/CMakeRelink.dir/SYDEsup")
  if(EXISTS "$ENV{DESTDIR}/home/marko/Grimme/openSYDE/opensyde_syde_sup/pjt/../result/SYDEsup" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}/home/marko/Grimme/openSYDE/opensyde_syde_sup/pjt/../result/SYDEsup")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}/home/marko/Grimme/openSYDE/opensyde_syde_sup/pjt/../result/SYDEsup")
    endif()
  endif()
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/home/marko/Grimme/openSYDE/opensyde_syde_sup/pjt/../result/sydesup_lib/libsydesuplib.so")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  file(INSTALL DESTINATION "/home/marko/Grimme/openSYDE/opensyde_syde_sup/pjt/../result/sydesup_lib" TYPE SHARED_LIBRARY FILES "/home/marko/Grimme/openSYDE/opensyde_syde_sup/temp/CMakeFiles/CMakeRelink.dir/libsydesuplib.so")
  if(EXISTS "$ENV{DESTDIR}/home/marko/Grimme/openSYDE/opensyde_syde_sup/pjt/../result/sydesup_lib/libsydesuplib.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}/home/marko/Grimme/openSYDE/opensyde_syde_sup/pjt/../result/sydesup_lib/libsydesuplib.so")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}/home/marko/Grimme/openSYDE/opensyde_syde_sup/pjt/../result/sydesup_lib/libsydesuplib.so")
    endif()
  endif()
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/home/marko/Grimme/openSYDE/opensyde_syde_sup/pjt/../result/sydesup_lib/C_SclString.hpp;/home/marko/Grimme/openSYDE/opensyde_syde_sup/pjt/../result/sydesup_lib/C_SclDynamicArray.hpp;/home/marko/Grimme/openSYDE/opensyde_syde_sup/pjt/../result/sydesup_lib/C_SydeSup.hpp;/home/marko/Grimme/openSYDE/opensyde_syde_sup/pjt/../result/sydesup_lib/C_SydeSupLinux.hpp")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  file(INSTALL DESTINATION "/home/marko/Grimme/openSYDE/opensyde_syde_sup/pjt/../result/sydesup_lib" TYPE FILE FILES
    "/home/marko/Grimme/openSYDE/opensyde_syde_sup/pjt/../libs/opensyde_core/scl/C_SclString.hpp"
    "/home/marko/Grimme/openSYDE/opensyde_syde_sup/pjt/../libs/opensyde_core/scl/C_SclDynamicArray.hpp"
    "/home/marko/Grimme/openSYDE/opensyde_syde_sup/pjt/../src/C_SydeSup.hpp"
    "/home/marko/Grimme/openSYDE/opensyde_syde_sup/pjt/../src/C_SydeSupLinux.hpp"
    )
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/home/marko/Grimme/openSYDE/opensyde_syde_sup/pjt/../result/sydesup_lib/stwtypes/stwtypes.hpp")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  file(INSTALL DESTINATION "/home/marko/Grimme/openSYDE/opensyde_syde_sup/pjt/../result/sydesup_lib/stwtypes" TYPE FILE FILES "/home/marko/Grimme/openSYDE/opensyde_syde_sup/pjt/../libs/opensyde_core/stwtypes/stwtypes.hpp")
endif()

if(CMAKE_INSTALL_COMPONENT)
  set(CMAKE_INSTALL_MANIFEST "install_manifest_${CMAKE_INSTALL_COMPONENT}.txt")
else()
  set(CMAKE_INSTALL_MANIFEST "install_manifest.txt")
endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
file(WRITE "/home/marko/Grimme/openSYDE/opensyde_syde_sup/temp/${CMAKE_INSTALL_MANIFEST}"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
