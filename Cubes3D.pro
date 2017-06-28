#-------------------------------------------------
#
# Project created by QtCreator 2016-07-02T12:38:32
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Cubes3D
TEMPLATE = app


SOURCES += main.cpp\
        cubes3d.cpp \
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
    dispayobject.cpp

HEADERS  += cubes3d.h \
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
    dispayobject.h

FORMS    += cubes3d.ui

DISTFILES += \
    shader/vshader.glsl \
    shader/fshader.glsl
