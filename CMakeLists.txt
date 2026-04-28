cmake_minimum_required(VERSION 3.10)
project(delorean)

set(CMAKE_CXX_STANDARD 17)

find_package(SDL2 REQUIRED)
find_package(SDL2_ttf REQUIRED)
find_package(SDL2_image REQUIRED)

include_directories(
    ${SDL2_INCLUDE_DIRS}
    ${SDL2_TTF_INCLUDE_DIRS}
    ${SDL2_IMAGE_INCLUDE_DIRS}
)

add_executable(delorean
    main.cpp
)

target_link_libraries(delorean
    ${SDL2_LIBRARIES}
    ${SDL2_TTF_LIBRARIES}
    ${SDL2_IMAGE_LIBRARIES}
    SDL2_ttf
    SDL2_image
    SDL2
)
