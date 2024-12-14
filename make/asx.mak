# Directories relative to TOP
STDCXX_DIR:=libstdc++

VPATH+=$(ASX_DIR)

ASX_PATH:=$(filter-out $(TOP), $(ASX_DIR))

# Append ASX code files
SRCS+=\
   $(ASX_PATH)src/builtin.cpp \
   $(ASX_PATH)src/_ccp.s \
   $(ASX_PATH)src/sysclk.c \
   $(ASX_PATH)src/alert.c \
   $(if $(filter timer,$(ASX_USE)), $(ASX_PATH)src/timer.c, ) \
   $(if $(filter reactor,$(ASX_USE)), $(ASX_PATH)src/reactor.c, ) \
   $(if $(filter mem,$(ASX_USE)), $(ASX_PATH)src/mem.c, )

INCLUDE_DIRS+=\
   $(TOP)/$(STDCXX_DIR)/include \
   $(ASX_DIR)/include \
