CMAKE_MINIMUM_REQUIRED (VERSION 2.6)
SET(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/CMake)

# use, i.e. don't skip the full RPATH for the build tree
SET(CMAKE_SKIP_BUILD_RPATH  FALSE)

# when building, don't use the install RPATH already
# (but later on when installing)
SET(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE) 

SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")

# add the automatically determined parts of the RPATH
# which point to directories outside the build tree to the install RPATH
SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)


# the RPATH to be used when installing, but only if it's not a system directory
LIST(FIND CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES "${CMAKE_INSTALL_PREFIX}/lib" isSystemDir)
IF("${isSystemDir}" STREQUAL "-1")
   SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
ENDIF("${isSystemDir}" STREQUAL "-1")

FIND_PACKAGE(Boost 1.46 COMPONENTS serialization regex iostreams REQUIRED)
FIND_PACKAGE(ICU 49 REQUIRED)

INCLUDE_DIRECTORIES(${ICU_INCLUDE_DIRS})

SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -pthread -fPIC -Wall -Wpedantic -Wextra") 
SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DENABLE_PAPI") 
FIND_LIBRARY(PAPI_LIBRARY papi $ENV{HOME}/libraries/lib)
FIND_LIBRARY(PUGI_LIBRARY pugixml $ENV{HOME}/libraries/lib)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")

IF("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-inline")
ELSE()
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DNDEBUG")
ENDIF()

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/Transducers/include)

ADD_SUBDIRECTORY(Transducers)

SET (TEST_LIBS
  TransducersTest
)

ADD_SUBDIRECTORY(TestRunner)


