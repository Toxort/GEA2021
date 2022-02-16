QT          += core gui widgets opengl

TEMPLATE    = app
CONFIG      += c++17

TARGET      = GEA2021

win32 {
    INCLUDEPATH += $(OPENAL_HOME)\\include\\AL
    LIBS *= $(OPENAL_HOME)\\libs\\Win64\\libOpenAL32.dll.a
}

SOURCES += main.cpp \
    aisystem.cpp \
    camera.cpp \
    collisionsystem.cpp \
    components.cpp \
    gameengine.cpp \
    gameobject.cpp \
    matrix3x3.cpp \
    matrix4x4.cpp \
    meshhandler.cpp \
    particle.cpp \
    particlesystem.cpp \
    physicsballsystem.cpp \
    renderwindow.cpp \
    resourcemanager.cpp \
    shader.cpp \
    mainwindow.cpp \
    soundhandler.cpp \
    soundmanager.cpp \
    soundsource.cpp \
    texture.cpp \
    transformsystem.cpp \
    vector2d.cpp \
    vector3d.cpp \
    vertex.cpp

HEADERS += \
    aisystem.h \
    camera.h \
    collisionsystem.h \
    components.h \
    constants.h \
    gameengine.h \
    gameobject.h \
    gltypes.h \
    input.h \
    math_constants.h \
    matrix3x3.h \
    matrix4x4.h \
    meshhandler.h \
    particle.h \
    particlesystem.h \
    physicsballsystem.h \
    renderwindow.h \
    resourcemanager.h \
    shader.h \
    mainwindow.h \
    soundhandler.h \
    soundmanager.h \
    soundsource.h \
    texture.h \
    transformsystem.h \
    vector2d.h \
    vector3d.h \
    vertex.h

FORMS += \
    mainwindow.ui

DISTFILES += \
    Shaders/mousepickingfragment.frag \
    Shaders/mousepickingvertex.vert \
    Shaders/phongshader.frag \
    Shaders/phongshader.vert \
    Shaders/textureshader.frag \
    Shaders/textureshader.vert \
    Shaders\plainfragment.frag \
    Shaders\plainvertex.vert
