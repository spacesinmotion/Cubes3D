#-------------------------------------------------
#
# Project created by QtCreator 2016-07-02T12:38:32
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Cubes3D
TEMPLATE = app

CONFIG += c++17

SOURCES += main.cpp\
        cubes3d.cpp \
  fe/fe.c \
  fesyntaxhighlighter.cpp \
  fewrap.cpp \
    view3d.cpp \
    camera.cpp \
    aabb.cpp \
    ray.cpp \
    slm/float_util.cpp \
    slm/float_util.inl \
    slm/intersect_util.cpp \
    slm/mat4.cpp \
    slm/mat4.inl \
    slm/quat.cpp \
    slm/quat.inl \
    slm/random.cpp \
    slm/random.inl \
    slm/random_util.cpp \
    slm/runtime_checks.cpp \
    slm/runtime_checks.inl \
    slm/vec2.cpp \
    slm/vec2.inl \
    slm/vec3.cpp \
    slm/vec3.inl \
    slm/vec4.cpp \
    slm/vec4.inl \
    rply/rply.c \
    plyimport.cpp \
    geometry.cpp \
    renderobject.cpp \
    displayobject.cpp \
    overlaypainter.cpp \
    overlay/overlayobject.cpp \
    overlay/mainmenubutton.cpp

HEADERS  += cubes3d.h \
    SceneHandler.h \
    fe/fe.h \
    fesyntaxhighlighter.h \
    fewrap.h \
    view3d.h \
    camera.h \
    aabb.h \
    ray.h \
    slm/float_util.h \
    slm/intersect_util.h \
    slm/mat4.h \
    slm/mtrnd.h \
    slm/no_simd.h \
    slm/quat.h \
    slm/random.h \
    slm/runtime_checks.h \
    slm/simd.h \
    slm/slmath.h \
    slm/slmath_configure.h \
    slm/slmath_pp.h \
    slm/vec_impl.h \
    slm/vec2.h \
    slm/vec3.h \
    slm/vec4.h \
    slm/vector_simd.h \
    rply/rply.h \
    plyimport.h \
    geometry.h \
    renderobject.h \
    displayobject.h \
    overlaypainter.h \
    overlay/overlayobject.h \
    overlay/mainmenubutton.h

FORMS    += cubes3d.ui

DISTFILES += \
    assets/crystal.ply \
    assets/cube.ply \
    assets/example.fe \
    shader/vshader.glsl \
    shader/fshader.glsl \
    .gitignore \
    shader/overlay.vert \
    shader/overlay.frag
