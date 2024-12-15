TOP:=.
ARCH:=attiny3224
BIN:=relay
INCLUDE_DIRS:=conf src
ASX_USE:=modbus_rtu

# Project own files
SRCS = \
   src/main.cpp \

# Inlude the actual build rules
include asx/make/rules.mak

# Add dependency to generate the datagram from the config
src/main.cpp : conf/datagram.hpp
