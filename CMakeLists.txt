cmake_minimum_required(VERSION 3.1.0)

project(Cubes3D VERSION 1.0.0 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

if(CMAKE_VERSION VERSION_LESS "3.7.0")
set(CMAKE_INCLUDE_CURRENT_DIR ON)
endif()

find_package(Qt5 COMPONENTS Widgets REQUIRED)


add_executable(Cubes3D
    aabb.cpp
    main.cpp
    slm/mat4.cpp
    slm/float_util.cpp
    slm/quat.cpp
    slm/vec4.cpp
    slm/vec3.cpp
    slm/vec2.cpp
    cubes3d.ui
    cubes3d.h
    cubes3d.cpp
    fewrap.h
    fewrap.cpp
    geometry.h
    geometry.cpp
    camera.cpp
    util.h
    view3d.h
    view3d.cpp
    plyimport.cpp
    ray.cpp
    displayobject.cpp
    renderobject.cpp
    fesyntaxhighlighter.h
    fesyntaxhighlighter.cpp
    FeEdit.h
    FeEdit.cpp
    fe/fe.c
    rply/rply.c
    shader.qrc
)

target_include_directories(Cubes3D PRIVATE .)

target_link_libraries(Cubes3D Qt5::Widgets)
