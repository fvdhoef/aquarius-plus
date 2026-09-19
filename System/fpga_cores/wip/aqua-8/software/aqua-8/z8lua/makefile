# makefile for building Lua
# see INSTALL for installation instructions
# see ../Makefile and luaconf.h for further customization

# == CHANGE THE SETTINGS BELOW TO SUIT YOUR ENVIRONMENT =======================

CWARNS= -pedantic -Wpointer-arith -Wshadow \
        -Wsign-compare -Wundef -Wwrite-strings
# -Wcast-align
# -Wcast-qual


# -DEXTERNMEMCHECK -DHARDSTACKTESTS
# -g -DLUA_USER_H='"ltests.h"'
# -fomit-frame-pointer #-pg -malign-double
TESTS= -g -DLUA_USER_H='"ltests.h"'

LOCAL = $(CWARNS)


CC= g++
CFLAGS= -Wall -Wno-deprecated -std=c++17 $(MYCFLAGS) -O2
AR= ar rcu
RANLIB= ranlib
RM= rm -f

MYCFLAGS= $(LOCAL)
MYLDFLAGS=
MYLIBS=


# enable Linux goodies
ifneq ($(OS),Windows_NT)
MYCFLAGS= $(LOCAL) -DLUA_USE_LINUX
MYLDFLAGS= -Wl,-E
MYLIBS= -ldl
endif
MYLIBS= -lreadline -lhistory



# == END OF USER SETTINGS. NO NEED TO CHANGE ANYTHING BELOW THIS LINE =========


LIBS = -lm

CORE_T=	liblua.a
CORE_O=	lapi.o lcode.o ldebug.o ldo.o ldump.o lfunc.o lgc.o llex.o lmem.o \
	lobject.o lopcodes.o lparser.o lstate.o lstring.o ltable.o ltm.o  \
	lundump.o lvm.o lzio.o lctype.o
AUX_O=	lauxlib.o
LIB_O=	lbaselib.o lcorolib.o ldblib.o ltablib.o lstrlib.o lpico8lib.o linit.o

LUA_T=	z8lua
LUA_O=	lua.o

#LUAC_T=	luac
#LUAC_O=	luac.o print.o

ALL_T= $(CORE_T) $(LUA_T) $(LUAC_T)
ALL_O= $(CORE_O) $(LUA_O) $(LUAC_O) $(AUX_O) $(LIB_O)
ALL_A= $(CORE_T)

all:	$(ALL_T)

o:	$(ALL_O)

a:	$(ALL_A)

$(CORE_T): $(CORE_O) $(AUX_O) $(LIB_O)
	$(AR) $@ $?
	$(RANLIB) $@

$(LUA_T): $(LUA_O) $(CORE_T)
	$(CC) -o $@ $(MYLDFLAGS) $(LUA_O) $(CORE_T) $(LIBS) $(MYLIBS) $(DL)

$(LUAC_T): $(LUAC_O) $(CORE_T)
	$(CC) -o $@ $(MYLDFLAGS) $(LUAC_O) $(CORE_T) $(LIBS) $(MYLIBS)

clean:
	rcsclean -u || true
	$(RM) $(ALL_T) $(ALL_O)

depend:
	@$(CC) $(CFLAGS) -MM *.cpp

echo:
	@echo "CC = $(CC)"
	@echo "CFLAGS = $(CFLAGS)"
	@echo "AR = $(AR)"
	@echo "RANLIB = $(RANLIB)"
	@echo "RM = $(RM)"
	@echo "MYCFLAGS = $(MYCFLAGS)"
	@echo "MYLDFLAGS = $(MYLDFLAGS)"
	@echo "MYLIBS = $(MYLIBS)"
	@echo "DL = $(DL)"

# DO NOT DELETE

lapi.o: lapi.cpp lua.h luaconf.h fix32.h lapi.h llimits.h lstate.h \
 lobject.h ltm.h lzio.h lmem.h ldebug.h ldo.h lfunc.h lgc.h lstring.h \
 ltable.h lundump.h lvm.h
lauxlib.o: lauxlib.cpp lua.h luaconf.h fix32.h lauxlib.h
lbaselib.o: lbaselib.cpp lua.h luaconf.h fix32.h lauxlib.h lualib.h
lbitlib.o: lbitlib.cpp lua.h luaconf.h fix32.h lauxlib.h lualib.h
lcode.o: lcode.cpp lua.h luaconf.h fix32.h lcode.h llex.h lobject.h \
 llimits.h lzio.h lmem.h lopcodes.h lparser.h ldebug.h lstate.h ltm.h \
 ldo.h lgc.h lstring.h ltable.h lvm.h
lcorolib.o: lcorolib.cpp lua.h luaconf.h fix32.h lauxlib.h lualib.h
lctype.o: lctype.cpp lctype.h lua.h luaconf.h fix32.h llimits.h
ldblib.o: ldblib.cpp lua.h luaconf.h fix32.h lauxlib.h lualib.h
ldebug.o: ldebug.cpp lua.h luaconf.h fix32.h lapi.h llimits.h lstate.h \
 lobject.h ltm.h lzio.h lmem.h lcode.h llex.h lopcodes.h lparser.h \
 ldebug.h ldo.h lfunc.h lstring.h lgc.h ltable.h lvm.h
ldo.o: ldo.cpp lua.h luaconf.h fix32.h lapi.h llimits.h lstate.h lobject.h \
 ltm.h lzio.h lmem.h ldebug.h ldo.h lfunc.h lgc.h lopcodes.h lparser.h \
 lstring.h ltable.h lundump.h lvm.h
ldump.o: ldump.cpp lua.h luaconf.h fix32.h lobject.h llimits.h lstate.h \
 ltm.h lzio.h lmem.h lundump.h
lfunc.o: lfunc.cpp lua.h luaconf.h fix32.h lfunc.h lobject.h llimits.h \
 lgc.h lstate.h ltm.h lzio.h lmem.h
lgc.o: lgc.cpp lua.h luaconf.h fix32.h ldebug.h lstate.h lobject.h \
 llimits.h ltm.h lzio.h lmem.h ldo.h lfunc.h lgc.h lstring.h ltable.h
linit.o: linit.cpp lua.h luaconf.h fix32.h lualib.h lauxlib.h
liolib.o: liolib.cpp lua.h luaconf.h fix32.h lauxlib.h lualib.h
llex.o: llex.cpp lua.h luaconf.h fix32.h lctype.h llimits.h ldo.h lobject.h \
 lstate.h ltm.h lzio.h lmem.h llex.h lparser.h lstring.h lgc.h ltable.h
lmathlib.o: lmathlib.cpp lua.h luaconf.h fix32.h lauxlib.h lualib.h
lmem.o: lmem.cpp lua.h luaconf.h fix32.h ldebug.h lstate.h lobject.h \
 llimits.h ltm.h lzio.h lmem.h ldo.h lgc.h
loadlib.o: loadlib.cpp lua.h luaconf.h fix32.h lauxlib.h lualib.h
lobject.o: lobject.cpp lua.h luaconf.h fix32.h lctype.h llimits.h ldebug.h \
 lstate.h lobject.h ltm.h lzio.h lmem.h ldo.h lstring.h lgc.h lvm.h
lopcodes.o: lopcodes.cpp lopcodes.h llimits.h lua.h luaconf.h fix32.h
loslib.o: loslib.cpp lua.h luaconf.h fix32.h lauxlib.h lualib.h
lparser.o: lparser.cpp lua.h luaconf.h fix32.h lcode.h llex.h lobject.h \
 llimits.h lzio.h lmem.h lopcodes.h lparser.h ldebug.h lstate.h ltm.h \
 ldo.h lfunc.h lstring.h lgc.h ltable.h
lpico8lib.o: lpico8lib.cpp lua.h luaconf.h fix32.h lauxlib.h llimits.h \
 lobject.h lstate.h ltm.h lzio.h lmem.h
lstate.o: lstate.cpp lua.h luaconf.h fix32.h lapi.h llimits.h lstate.h \
 lobject.h ltm.h lzio.h lmem.h ldebug.h ldo.h lfunc.h lgc.h llex.h \
 lstring.h ltable.h
lstring.o: lstring.cpp lua.h luaconf.h fix32.h lmem.h llimits.h lobject.h \
 lstate.h ltm.h lzio.h lstring.h lgc.h
lstrlib.o: lstrlib.cpp lua.h luaconf.h fix32.h lauxlib.h lualib.h
ltable.o: ltable.cpp lua.h luaconf.h fix32.h ldebug.h lstate.h lobject.h \
 llimits.h ltm.h lzio.h lmem.h ldo.h lgc.h lstring.h ltable.h lvm.h
ltablib.o: ltablib.cpp lua.h luaconf.h fix32.h lauxlib.h lualib.h
ltests.o: ltests.cpp lua.h luaconf.h fix32.h lapi.h llimits.h lstate.h \
 lobject.h ltm.h lzio.h lmem.h lauxlib.h lcode.h llex.h lopcodes.h \
 lparser.h lctype.h ldebug.h ldo.h lfunc.h lstring.h lgc.h ltable.h \
 lualib.h
ltm.o: ltm.cpp lua.h luaconf.h fix32.h lobject.h llimits.h lstate.h ltm.h \
 lzio.h lmem.h lstring.h lgc.h ltable.h
lua.o: lua.cpp lua.h luaconf.h fix32.h lauxlib.h lualib.h
lundump.o: lundump.cpp lua.h luaconf.h fix32.h ldebug.h lstate.h lobject.h \
 llimits.h ltm.h lzio.h lmem.h ldo.h lfunc.h lstring.h lgc.h lundump.h
lvm.o: lvm.cpp lua.h luaconf.h fix32.h ldebug.h lstate.h lobject.h \
 llimits.h ltm.h lzio.h lmem.h ldo.h lfunc.h lgc.h lopcodes.h lstring.h \
 ltable.h lvm.h
lzio.o: lzio.cpp lua.h luaconf.h fix32.h llimits.h lmem.h lstate.h \
 lobject.h ltm.h lzio.h

# (end of Makefile)
