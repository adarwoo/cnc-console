TOP:=.
ARCH:=attiny3224
BIN:=cnc_console
INCLUDE_DIRS:=conf src
ASX_USE:=i2c_master pca9555

# Project own files
SRCS = \
   src/main.cpp \
	src/mux.cpp

# Inlude the actual build rules
include asx/make/rules.mak

# Add dependency to generate the datagram from the config
src/main.cpp : conf/datagram.hpp
