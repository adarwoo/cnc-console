TOP=..

# Select the architecture
ARCH=attiny3224

# Name of the binary to produce
BIN := blink

# Reference all from the solution
VPATH=$(TOP)

# Paths, local to src
LOGGER_DIR     := logger
ASX_DIR        := .
STDCXX_DIR     := libstdc++

# -I throughout (C and C++)
INCLUDE_DIRS = \
   . \
   $(TOP)/${LOGGER_DIR}/include \
   $(TOP)/$(STDCXX_DIR)/include \
   $(TOP)/${ASX_DIR}/include \

# Project own files
SRCS = \
   $(ASX_DIR)/src/builtin.cpp \
   $(ASX_DIR)/src/_ccp.s \
   $(ASX_DIR)/src/sysclk.c \
   $(ASX_DIR)/src/timer.c \
   $(ASX_DIR)/src/alert.c \
   $(ASX_DIR)/src/reactor.c \
   blink.cpp \

# Inlude the actual build rules
include $(TOP)/make/rules.mak
