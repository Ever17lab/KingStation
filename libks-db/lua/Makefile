DEBUG                = 0
LIBKSDB_DIR      := ../
LIBKS_COMMON_DIR := ../../libks-common
INCFLAGS             = -I. -I$(LIBKSDB_DIR) -I$(LIBKS_COMMON_DIR)/include

TARGETS              = rmsgpack_test libksdb_tool lua_converter

ifeq ($(DEBUG), 1)
CFLAGS               = -g -O0 -Wall
else
CFLAGS               = -g -O2 -Wall -DNDEBUG
endif

LUA_CONVERTER_C = \
			 $(LIBKSDB_DIR)/rmsgpack.c \
			 $(LIBKSDB_DIR)/rmsgpack_dom.c \
			 lua_common.c \
			 $(LIBKSDB_DIR)/libksdb.c \
			 $(LIBKSDB_DIR)/bintree.c \
			 $(LIBKSDB_DIR)/query.c \
			 lua_converter.c \
			 $(LIBKS_COMMON_DIR)/compat/compat_fnmatch.c \
			 $(LIBKS_COMMON_DIR)/streams/file_stream.c \
			 $(LIBKS_COMMON_DIR)/compat/compat_strl.c

LUA_CONVERTER_OBJS := $(LUA_CONVERTER_C:.c=.o)

KINGSNDB_TOOL_C = \
			 $(LIBKSDB_DIR)/rmsgpack.c \
			 $(LIBKSDB_DIR)/rmsgpack_dom.c \
			 $(LIBKSDB_DIR)/libksdb_tool.c \
			 $(LIBKSDB_DIR)/bintree.c \
			 $(LIBKSDB_DIR)/query.c \
			 ($LIBKSDB_DIR)/libksdb.c \
			 $(LIBKS_COMMON_DIR)/compat/compat_fnmatch.c \
			 $(LIBKS_COMMON_DIR)/streams/file_stream.c \
			 $(LIBKS_COMMON_DIR)/compat/compat_strl.c

KINGSNDB_TOOL_OBJS := $(KINGSNDB_TOOL_C:.c=.o)

TESTLIB_C = \
			 testlib.c \
			 $(LIBKSDB_DIR)/query.c \
			 ($LIBKSDB_DIR)/libksdb.c \
			 $(LIBKSDB_DIR)/bintree.c \
			 $(LIBKSDB_DIR)/rmsgpack.c \
			 $(LIBKSDB_DIR)/rmsgpack_dom.c \
			 $(LIBKS_COMMON_DIR)/compat/compat_fnmatch.c \
			 $(LIBKS_COMMON_DIR)/streams/file_stream.c \
			 $(LIBKS_COMMON_DIR)/compat/compat_strl.c

TESTLIB_OBJS := $(TESTLIB_C:.c=.o)

RMSGPACK_C = \
			$(LIBKSDB_DIR)/rmsgpack.c \
			$(LIBKSDB_DIR)/rmsgpack_test.c \
			$(LIBKS_COMMON_DIR)/streams/file_stream.c

RMSGPACK_OBJS := $(RMSGPACK_C:.c=.o)

LUA_FLAGS     = `pkg-config lua --libs`
TESTLIB_FLAGS = $(CFLAGS) $(LUA_FLAGS) -shared -fpic

.PHONY: all clean check

all: $(TARGETS)

%.o: %.c
	$(CC) $(INCFLAGS) $< -c $(CFLAGS) -o $@

lua_converter: $(LUA_CONVERTER_OBJS)
	$(CC) $(INCFLAGS) $(LUA_CONVERTER_OBJS) $(LUA_FLAGS) -o $@

libksdb_tool: $(KINGSNDB_TOOL_OBJS)
	$(CC) $(INCFLAGS) $(KINGSNDB_TOOL_OBJS) -o $@

rmsgpack_test: $(RMSGPACK_OBJS)
	$(CC) $(INCFLAGS) $(RMSGPACK_OBJS) -g -o $@

testlib.so: CFLAGS += -fPIC
testlib.so: clean $(TESTLIB_OBJS)
	$(CC) $(INCFLAGS) $(TESTLIB_FLAGS) $(TESTLIB_OBJS) -o $@

check: testlib.so lua/tests.lua
	lua ./lua/tests.lua

clean:
	rm -rf $(TARGETS) $(LUA_CONVERTER_OBJS) $(C_CONVERTER_OBJS) $(KINGSNDB_TOOL_OBJS) $(RMSGPACK_OBJS) $(TESTLIB_OBJS) testlib.so
