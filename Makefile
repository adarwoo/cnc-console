TOP:=.
ARCH:=attiny3224
BIN:=cnc_console
INCLUDE_DIRS:=conf src

ASX_USE:=piezzo i2c_master pca9555 modbus_rtu

# Project own files
SRCS = \
   src/main.cpp \
   src/mux.cpp \
   src/console.cpp \

# Inlude the actual build rules
include asx/make/rules.mak

# Add dependency to generate the datagram from the config
src/main.cpp : conf/datagram.hpp

# CLEAN_FILES += conf/datagram.hpp