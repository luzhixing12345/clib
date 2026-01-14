# ------------------------- #
#          VERSION          #
# ------------------------- #
VERSION 		:= 0
PATCHLEVEL 		:= 0
SUBLEVEL 		:= 1

rwildcard = $(foreach d, $(filter-out .., $(wildcard $1*)), \
             $(call rwildcard,$d/,$2) $(filter $2, $d))
# default: [all, lib, each, multi-exe]
# ------------------------- #
# all: compile all the files under SRC_PATH and generate TARGET
#      $ gcc $(SRC_PATH)/*.$(SRC_EXT) -o $(TARGET)
# lib: compile all the files under SRC_PATH and generate LIBNAME(a)
#      $ gcc $(SRC_PATH)/*.$(SRC_EXT) -o *.o
#      $ ar rcs lib$(LIBNAME).a *.o
# each: compile each file under SRC_PATH and generate each file
#      $ gcc $(SRC_PATH)/file1.$(SRC_EXT) -o file1
#      $ gcc $(SRC_PATH)/file2.$(SRC_EXT) -o file2
#      ...
# multi-exe: compile multiple executables from different source files
#      $ gcc $(SRC_PATH)/file1.$(SRC_EXT) -o file1
#      $ gcc $(SRC_PATH)/file2.$(SRC_EXT) -o file2
# ------------------------- #
default: lib

# ------------------------- #
#          PROJECT          #
# ------------------------- #
CC          	:= gcc
SRC_PATH    	:= clib
SRC_EXT     	:= c
BUILD_PATH 		:= ./build
EXCLUDE_SRC 	:= # src/func.c
TARGET      	:= clib

# ------------------------- #
#            LIB            #
# ------------------------- #
# lib: compile all the files under SRC_PATH and generate LIBNAME(a/so)
LIBNAME     	:= $(TARGET)

# ------------------------- #
#          MULTI-EXE        #
# ------------------------- #
# SRC += $(call rwildcard, <src>, %.$(SRC_EXT))
# SRC := $(filter-out $(EXCLUDE_SRC),$(SRC))
T1		   			:= exe1
T2		   			:= exe2
T3		   			:= exe3
SRC_T1	 			:= src/main.c
SRC_T2	 			:= t/t2_main.c
SRC_T3	 			:= t/t3_main.c
OBJS-T1 			:= $(SRC_T1:.$(SRC_EXT)=.o)
OBJS-T2 			:= $(SRC_T2:.$(SRC_EXT)=.o)
OBJS-T3 			:= $(SRC_T3:.$(SRC_EXT)=.o)
MULTI_EXE_TARGETS 	:= $(T1) $(T2) $(T3)
MULTI_EXE_OBJS 	  	:= $(OBJS-T1) $(OBJS-T2) $(OBJS-T3)
# ------------------------- #
#          FLAGS            #
# ------------------------- #
CFLAGS 			:= -O2
INCLUDE_PATH 	:= # -I./include
LDFLAGS 		:= # -L./lib
DEFINES     	:= # -DDEBUG
CFLAGS          += $(INCLUDE_PATH)
# ------------------------- #

# ------------------------- #
#          BINARIES         #
# ------------------------- #
AR          := ar
LD          := ld
MV          := mv
CP          := cp
RM          := rm -f
INSTALL     := install

# ------------------------- #
#          WARNING          #
# ------------------------- #
WARNING += -Wall
WARNING += -Wunused
WARNING += -Wformat-security
WARNING += -Wshadow
WARNING += -Wpedantic
WARNING += -Wstrict-aliasing
WARNING += -Wuninitialized
WARNING += -Wnull-dereference
WARNING += -Wformat=2
WARNING += -fno-strict-aliasing
WARNING += -Winit-self
WARNING += -Wno-system-headers
WARNING += -Wredundant-decls
WARNING += -Wundef
WARNING += -Wvolatile-register-var
WARNING += -Wno-format-nonliteral
WARNING += -Wno-pedantic
WARNING += -Wno-unused-result
# if SRC_EXT is c
ifeq ($(SRC_EXT),c)
WARNING += -Wnested-externs
WARNING += -Wno-discarded-qualifiers
endif
CFLAGS	+= $(WARNING)

ifeq ($(strip $(V)),)
	ifeq ($(findstring s,$(filter-out --%,$(firstword $(MAKEFLAGS)))),)
		E = @printf
	else
		E = @\#
	endif
	Q = @
else
	E = @\#
	Q =
endif
export E Q

# ------------------------- #
#        auto config        #
# ------------------------- #
# create build dirs
BUILD_BIN_PATH 	:= $(BUILD_PATH)/bin
BUILD_LIB_PATH 	:= $(BUILD_PATH)/lib
BUILD_DIRS := $(BUILD_BIN_PATH) $(BUILD_LIB_PATH)
$(shell mkdir -p $(BUILD_DIRS))
DESTDIR_SQ := $(shell echo "$(DESTDIR)" | sed "s/'/'\\\\''/g;1s/^/'/;\$$s/$$/'/")
bindir_SQ  := $(shell echo "$(bindir)" | sed "s/'/'\\\\''/g;1s/^/'/;\$$s/$$/'/")
# final output binary
PROGRAM = $(BUILD_BIN_PATH)/$(TARGET)
LIBFDT_STATIC 	:= $(BUILD_LIB_PATH)/lib$(LIBNAME).a
LIBFDT_DYNAMIC  := $(BUILD_LIB_PATH)/lib$(LIBNAME).so

# Translate uname -m into ARCH string
ARCH ?= $(shell uname -m | sed -e s/i.86/i386/ -e s/ppc.*/powerpc/ \
	  -e s/armv.*/arm/ -e s/aarch64.*/arm64/ -e s/mips64/mips/ \
	  -e s/riscv64/riscv/ -e s/riscv32/riscv/)

# ifneq ($(THIRD_LIB),)
# CFLAGS 	+= $(shell pkg-config --cflags $(THIRD_LIB))
# LDFLAGS += $(shell pkg-config --libs $(THIRD_LIB))
# endif

ifneq ($(strip $(SRC_PATH)),)
    SRC += $(call rwildcard, $(SRC_PATH), %.$(SRC_EXT))
endif

# exclude some source files
SRC := $(filter-out $(EXCLUDE_SRC),$(SRC))

OBJS = $(SRC:$(SRC_EXT)=o)

comma = ,
# The dependency file for the current target
depfile = $(subst $(comma),_,$(dir $@).$(notdir $@).d)

DEPS	:= $(foreach obj,$(OBJS) $(MULTI_EXE_OBJS),\
		$(subst $(comma),_,$(dir $(obj)).$(notdir $(obj)).d))

CFLAGS  += $(DEFINES)
c_flags	= -Wp,-MD,$(depfile) -Wp,-MT,$@ $(CFLAGS)

ifeq ($(MAKECMDGOALS),debug)
CFLAGS += -g
endif

all: $(PROGRAM)
.PHONY: all

debug: default
.PHONY: debug

# ------------------------- #
#       compile rules       #
# ------------------------- #

# compile program bin
$(PROGRAM): $(OBJS)
	$(E) "  LINK    \033[1;32m%s\033[0m\n" $@
	$(Q) $(CC) $(CFLAGS) $(OBJS) $(LDFLAGS) -o $@
	$(E) "  Binary program $(PROGRAM) is ready.\n"

# ------------------------- #
#            libs           #
# ------------------------- #

# compile static lib
$(LIBFDT_STATIC): $(OBJS)
	$(E) "  LINK    \033[1;32m%s\033[0m\n" $@
	$(Q) $(AR) rcs $@ $(OBJS)
	$(E) "  static  lib $@ is ready.\n"

# compile dynamic lib
# set LD_LIBRARY_PATH when using dynamic lib
$(LIBFDT_DYNAMIC): $(OBJS)
	$(E) "  LINK    \033[1;32m%s\033[0m\n" $@
	$(Q) $(CC) -fPIC -shared $(OBJS) -o $@
	$(E) "  dynamic lib $@ is ready.\n"
	$(E) "  use lib with \033[1m-L$(BUILD_LIB_PATH) -l$(TARGET)\033[0m\n"

# compile both static and dynamic lib
lib: $(LIBFDT_STATIC) $(LIBFDT_DYNAMIC)
.PHONY: lib

# ------------------------- #
#         each exe          #
# ------------------------- #

EXECUTABLES = $(OBJS:.o=)
%: %.o
	$(E) "  LINK    \033[1;32m%s\033[0m\n" $@
	$(Q) $(CC) $(CFLAGS) $< $(LDFLAGS) -o $@

each: $(EXECUTABLES)
.PHONY: each

# ------------------------- #
#        multi-exe          #
# ------------------------- #

$(T1): $(OBJS-T1)
	$(E) "  LINK    \033[1;32m%s\033[0m\n" $@
	$(Q) $(CC) $(CFLAGS) $< $(LDFLAGS) -o $@
$(T2): $(OBJS-T2)
	$(E) "  LINK    \033[1;32m%s\033[0m\n" $@
	$(Q) $(CC) $(CFLAGS) $< $(LDFLAGS) -o $@
$(T3): $(OBJS-T3)
	$(E) "  LINK    \033[1;32m%s\033[0m\n" $@
	$(Q) $(CC) $(CFLAGS) $< $(LDFLAGS) -o $@

multi-exe: $(MULTI_EXE_TARGETS)
.PHONY: multi-exe

CFLAGS_DYNOPT += -fPIC
$(OBJS):
%.o: %.$(SRC_EXT)
ifeq ($(C),1)
	$(E) "  CHECK   %s\n" $@
	$(Q) $(CHECK) -c $(CFLAGS) $(CFLAGS_DYNOPT) $< -o $@
endif
	$(E) "  CC      %s\n" $@
	$(Q) $(CC) -c $(c_flags) $(CFLAGS_DYNOPT) $< -o $@

# ------------------------- #
#          使用方法
# ------------------------- #
.PHONY: clean distclean lib release tar all test

install: all
	$(E) "  INSTALL\n"
	$(Q) $(INSTALL) -d -m 755 '$(DESTDIR_SQ)$(bindir_SQ)' 
	$(Q) $(INSTALL) $(PROGRAM) '$(DESTDIR_SQ)$(bindir_SQ)' 
.PHONY: install


clean:
	$(E) "  CLEAN\n"
	$(Q) rm -f $(DEPS) $(OBJS)
	$(Q) rm -f $(EXECUTABLES)
	$(Q) [ -f $(PROGRAM) ] && rm -f $(PROGRAM) || true
	$(Q) rm -rf $(BUILD_BIN_PATH) $(BUILD_LIB_PATH) $(BUILD_PATH) > /dev/null 2>&1 || true
	$(Q) rm -f $(LIBFDT_STATIC)
	$(Q) rm -f $(LIBFDT_DYNAMIC)
	$(Q) rm -f $(MULTI_EXE_TARGETS) $(MULTI_EXE_OBJS)
release:
	$(MAKE) -j4
	mkdir $(RELEASE)
	@cp $(EXE) $(RELEASE)/ 
	tar -cvf $(TARGET).tar $(RELEASE)/

# 输出配置信息, 包括 CFLAGS, LDFLAGS, LIBS
config:
	$(E) "CONFIG\n"
	$(E) "  [SRC FILES]: %s\n" "$(shell echo $(SRC) | tr '\n' ' ')"
	$(E) "  [CFLAGS]: %s\n" "$(CFLAGS)"
	$(E) "  [LDFLAGS]: %s\n" "$(LDFLAGS)"

help:
	$(E) "\n"
	$(E) "  [%s compile help]\n" $(TARGET)
	$(E) "\n"
	$(E) "    make              编译\n"
	$(E) "    make help         帮助信息\n"
	$(E) "    make clean        清除编译文件\n"
	$(E) "    make config       查看配置信息\n"
	$(E) "    make install      安装\n"
	$(E) "    make debug        调试模式\n"
	$(E) "\n"

ifneq ($(MAKECMDGOALS),clean)
-include $(DEPS)
endif

