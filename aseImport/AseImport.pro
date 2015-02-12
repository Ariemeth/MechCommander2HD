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
    aseitem.cpp \
    ase_singleint.cpp \
    ase_singlefloat.cpp \
    ase_string.cpp

HEADERS  += mainwindow.h \
    aseitem.h \
    asemodel.h \
    ase_singlefloat.hpp \
    ase_threefloat.hpp \
    ase_singleint.hpp \
    ase_threeint.h \
    ase_string.h \
    ase_mesh_vertex.h \
    ase_mesh_face.h \
    ase_mesh_tface.h \
    ase_mesh_normal.h \
    ase_scene.h \
    ase_mapdiffuse.h \
    ase_material.h \
    ase_material_parent.h \
    ase_node_tm.h \
    tm_animation.h \
    ase_control_rot.h \
    ase_mesh.h \
    ase_helperobject.h \
    ase_geomobject.h

FORMS    += mainwindow.ui
