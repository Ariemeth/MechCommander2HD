#-------------------------------------------------
#
# Project created by QtCreator 2015-02-05T13:40:47
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = AseImport
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    AseModel/ase_singlefloat.cpp \
    AseModel/ase_singleint.cpp \
    AseModel/ase_string.cpp \
    AseModel/aseitem.cpp \
    build-AseImport-Desktop_Qt_5_4_0_MSVC2010_OpenGL_32bit-Debug/debug/moc_mainwindow.cpp

HEADERS  += mainwindow.h \
    build-AseImport-Desktop_Qt_5_4_0_MSVC2010_OpenGL_32bit-Debug/ui_mainwindow.h \
    Include/AseModel/ase_control_rot.hpp \
    Include/AseModel/ase_geomobject.hpp \
    Include/AseModel/ase_helperobject.hpp \
    Include/AseModel/ase_mapdiffuse.hpp \
    Include/AseModel/ase_material.hpp \
    Include/AseModel/ase_material_parent.hpp \
    Include/AseModel/ase_mesh.hpp \
    Include/AseModel/ase_mesh_face.hpp \
    Include/AseModel/ase_mesh_normal.hpp \
    Include/AseModel/ase_mesh_tface.hpp \
    Include/AseModel/ase_mesh_vertex.hpp \
    Include/AseModel/ase_node_tm.hpp \
    Include/AseModel/ase_scene.hpp \
    Include/AseModel/ase_singlefloat.hpp \
    Include/AseModel/ase_singleint.hpp \
    Include/AseModel/ase_string.hpp \
    Include/AseModel/ase_threefloat.hpp \
    Include/AseModel/ase_threeint.hpp \
    Include/AseModel/aseitem.hpp \
    Include/AseModel/asemodel.hpp \
    Include/AseModel/tm_animation.hpp


FORMS    += mainwindow.ui
