
ifeq ($(strip $(V)),)
	ifeq ($(findstring s,$(filter-out --%,$(firstword $(MAKEFLAGS)))),)
		E = @echo
	else
		E = @\#
	endif
	Q = @
else
	E = @\#
	Q =
endif
export E Q

VERSION 	:= 0
PATCHLEVEL 	:= 0
SUBLEVEL 	:= 1

# Translate uname -m into ARCH string
ARCH ?= $(shell uname -m | sed -e s/i.86/i386/ -e s/ppc.*/powerpc/ \
	  -e s/armv.*/arm/ -e s/aarch64.*/arm64/ -e s/mips64/mips/ \
	  -e s/riscv64/riscv/ -e s/riscv32/riscv/)

# ------------------------- #
#          PROJECT          #
# ------------------------- #
CC          	:= gcc
TARGET      	:=
LIBNAME     	:= clib
SRC_PATH    	:= clib
SRC_EXT     	:= c

# ------------------------- #
STATIC_LIB 		:= $(SRC_PATH)/lib$(LIBNAME).a

# default: [all, lib, each]
default: lib

# ------------------------- #
#          BINARIES         #
# ------------------------- #
AR          := ar
LD          := ld
FIND	    := find
CSCOPE	    := cscope
TAGS	    := ctags
INSTALL     := install
CHECK       := check
OBJCOPY		:= objcopy

# ------------------------- #
#          FLAGS            #
# ------------------------- #
CFLAGS 			:= -DCONFIG_VERSION=\"$(VERSION).$(PATCHLEVEL).$(SUBLEVEL)\" -g
LDFLAGS 		:= 
LIBS			:= 
DEFINES     	:= 
THIRD_LIB   	:= 
INCLUDE_PATH 	:= 
# ------------------------- #

ifneq ($(strip $(INCLUDE_PATH)),)
    CFLAGS += $(foreach dir, $(INCLUDE_PATH), -I$(dir))
endif

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
WARNING += -Wnested-externs
WARNING += -Wno-system-headers
WARNING += -Wredundant-decls
WARNING += -Wsign-compare
WARNING += -Wundef
WARNING += -Wvolatile-register-var
WARNING += -Wno-format-nonliteral
WARNING += -Wno-pedantic -Wno-discarded-qualifiers
CFLAGS	+= $(WARNING)

# ------------------------- #


ifneq ($(THIRD_LIB),)
CFLAGS 	+= $(shell pkg-config --cflags $(THIRD_LIB))
LDFLAGS += $(shell pkg-config --libs $(THIRD_LIB))
endif

rwildcard = $(foreach d, $(filter-out .., $(wildcard $1*)), \
             $(call rwildcard,$d/,$2) $(filter $2, $d))

ifneq ($(strip $(SRC_PATH)),)
    SRC += $(call rwildcard, $(SRC_PATH), %.$(SRC_EXT))
endif

OBJS = $(SRC:$(SRC_EXT)=o)

comma = ,
# The dependency file for the current target
depfile = $(subst $(comma),_,$(dir $@).$(notdir $@).d)

DEPS	:= $(foreach obj,$(OBJS) $(OBJS_DYNOPT) $(OTHEROBJS) $(GUEST_OBJS),\
		$(subst $(comma),_,$(dir $(obj)).$(notdir $(obj)).d))

CFLAGS  += $(DEFINES)
c_flags	= -Wp,-MD,$(depfile) -Wp,-MT,$@ $(CFLAGS)

ifeq ($(MAKECMDGOALS),debug)
CFLAGS+=-g
endif

PROGRAM = $(TARGET)

all: $(PROGRAM)
.PHONY: all

debug: all
.PHONY: debug

# compile program bin
$(PROGRAM): $(OBJS) $(OBJS_DYNOPT) $(OTHEROBJS) $(GUEST_OBJS)
	$(E) -e "  LINK    \033[1;32m" $@ "\033[0m"
	$(Q) $(CC) $(CFLAGS) $(OBJS) $(OBJS_DYNOPT) $(OTHEROBJS) $(GUEST_OBJS) $(LDFLAGS) $(LIBS) $(LIBS_DYNOPT) $(LIBFDT_STATIC) -o $@
	$(E) "  binary program $(PROGRAM) is ready."

# compile static lib
$(STATIC_LIB): $(OBJS) $(OTHEROBJS) $(GUEST_OBJS)
	$(E) -e "  LINK    \033[1;32m" $@ "\033[0m"
	$(Q) $(AR) rcs $@ $(OBJS) $(OTHEROBJS) $(GUEST_OBJS)
	$(E) "  static library $(STATIC_LIB) is ready."

# compile dynamic lib
# $(DYNAMIC_LIB): $(OBJS) $(OTHEROBJS) $(GUEST_OBJS)
# 	$(E) -e "  LINK    \033[1;32m" $@ "\033[0m"
# 	$(Q) $(LD) -shared $(OBJS) $(OTHEROBJS) $(GUEST_OBJS) -o $@
# 	$(E) "  dynamic library $(DYNAMIC_LIB) is ready."

# compile both static and dynamic lib
lib: $(STATIC_LIB)
.PHONY: lib

EXECUTABLES = $(OBJS:.o=)
%: %.o
	$(E) -e "  LINK    \033[1;32m" $@ "\033[0m"
	$(Q) $(CC) $(CFLAGS) $< $(LDFLAGS) $(LIBS) -o $@

each: $(EXECUTABLES)
.PHONY: each


$(OBJS):
%.o: %.c
ifeq ($(C),1)
	$(E) "  CHECK   " $@
	$(Q) $(CHECK) -c $(CFLAGS) $(CFLAGS_DYNOPT) $< -o $@
endif
	$(E) "  CC      " $@
	$(Q) $(CC) -c $(c_flags) $(CFLAGS_DYNOPT) $< -o $@

# ------------------------- #
#          使用方法
# ------------------------- #
.PHONY: clean distclean lib release tar all test

install: all
	$(E) "  INSTALL"
	$(Q) $(INSTALL) -d -m 755 '$(DESTDIR_SQ)$(bindir_SQ)' 
	$(Q) $(INSTALL) $(PROGRAM) '$(DESTDIR_SQ)$(bindir_SQ)' 
.PHONY: install


clean:
	$(E) "  CLEAN"
	$(Q) rm -f $(DEPS) $(STATIC_DEPS) $(OBJS) $(OTHEROBJS) $(OBJS_DYNOPT) $(STATIC_OBJS) $(PROGRAM) $(PROGRAM_ALIAS) $(GUEST_INIT) $(GUEST_PRE_INIT) $(GUEST_OBJS)
	$(Q) rm -f $(PROGRAM) $(EXECUTABLES)
	$(Q) rm -f $(STATIC_LIB)
	$(Q) rm -f $(DYNAMIC_LIB)
release:
	$(MAKE) -j4
	mkdir $(RELEASE)
	@cp $(EXE) $(RELEASE)/ 
	tar -cvf $(TARGET).tar $(RELEASE)/

# 输出配置信息, 包括 CFLAGS, LDFLAGS, LIBS
config:
	$(E) "CONFIG"
	$(E) "  [SRC FILES]: $(shell echo $(SRC) | tr '\n' ' ')"
	$(E) "  [CFLAGS]: $(CFLAGS)"
	$(E) "  [LDFLAGS]: $(LDFLAGS)"
	$(E) "  [LIBS]: $(LIBS)"

help:
	$(E) ""
	$(E) "  [$(TARGET) compile help]"
	$(E) ""
	$(E) "    make              编译"
	$(E) "    make help         帮助信息"
	$(E) "    make clean        清除编译文件"
	$(E) "    make config       查看配置信息"
	$(E) "    make install      安装"
	$(E) "    make debug        调试模式"
	$(E) ""

ifneq ($(MAKECMDGOALS),clean)
-include $(DEPS)
-include $(STATIC_DEPS)
endif
