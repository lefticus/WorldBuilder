cmake_minimum_required(VERSION 2.6)

project(WorldBuilder)


find_package(SDL)
find_package(SDL_image)

IF(MSVC)
  ADD_DEFINITIONS(/W4)
  IF(CMAKE_CL_64)
    ADD_DEFINITIONS(/bigobj)
   ENDIF()
ELSE()
  ADD_DEFINITIONS(-Wall -Wextra -Wshadow -std=c++0x) 

  IF (APPLE)
    # -Wno-missing-field-initializers is for boost on macos
    ADD_DEFINITIONS(-Wno-missing-field-initializers -Wno-sign-compare)
  ENDIF()
ENDIF()

add_executable(worldbuilder main.cpp Map.cpp Point.cpp Region.cpp Shape.cpp World.cpp ChaiScript_Builder.cpp ChaiScript_Creator.cpp)

include_directories(/home/jason/Programming/ChaiScript/include)
target_link_libraries(worldbuilder ${SDL_LIBRARY} ${SDLIMAGE_LIBRARY} )
