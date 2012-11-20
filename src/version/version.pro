TEMPLATE += app
SOURCES = ewa_versiondef.cpp
CONFIG += console
TARGET = ewa_vg

DESTDIR = ./

CONFIG(debug, debug|release) {
  OUTDIR = ./tmp/debug
 } else {
  OUTDIR = ./tmp/release
 }
 
OBJECTS_DIR =   $$OUTDIR/obj
MOC_DIR =       $$OUTDIR/moc