DEBUG                = 0
LIBKSDB_DIR      := .
LIBKS_COMM_DIR   := ../libks-common
INCFLAGS             = -I. -I$(LIBKS_COMM_DIR)/include

TARGETS              = rmsgpack_test libksdb_tool c_converter

ifeq ($(DEBUG), 1)
CFLAGS               = -g -O0 -Wall
else
CFLAGS               = -g -O2 -Wall -DNDEBUG
endif

LIBKS_COMMON_C = \
			 $(LIBKS_COMM_DIR)/string/stdstring.c \
			 $(LIBKS_COMM_DIR)/streams/file_stream.c \
			 $(LIBKS_COMM_DIR)/compat/compat_strcasestr.c \
			 $(LIBKS_COMM_DIR)/time/rtime.c \
			 $(LIBKS_COMM_DIR)/file/file_path.c \
			 $(LIBKS_COMM_DIR)/file/file_path_io.c \
			 $(LIBKS_COMM_DIR)/vfs/vfs_implementation.c \
			 $(LIBKS_COMM_DIR)/encodings/encoding_utf.c \
			 $(LIBKS_COMM_DIR)/compat/compat_strl.c \
			 $(LIBKS_COMM_DIR)/compat/fopen_utf8.c

C_CONVERTER_C = \
			 $(LIBKSDB_DIR)/rmsgpack.c \
			 $(LIBKSDB_DIR)/rmsgpack_dom.c \
			 $(LIBKSDB_DIR)/libksdb.c \
			 $(LIBKSDB_DIR)/bintree.c \
			 $(LIBKSDB_DIR)/query.c \
			 $(LIBKSDB_DIR)/c_converter.c \
			 $(LIBKS_COMM_DIR)/hash/rhash.c \
			 $(LIBKS_COMM_DIR)/compat/compat_fnmatch.c \
			 $(LIBKS_COMMON_C)

C_CONVERTER_OBJS := $(C_CONVERTER_C:.c=.o)

KINGSNDB_TOOL_C = \
			 $(LIBKSDB_DIR)/rmsgpack.c \
			 $(LIBKSDB_DIR)/rmsgpack_dom.c \
			 $(LIBKSDB_DIR)/libksdb_tool.c \
			 $(LIBKSDB_DIR)/bintree.c \
			 $(LIBKSDB_DIR)/query.c \
			 $(LIBKSDB_DIR)/libksdb.c \
			 $(LIBKS_COMM_DIR)/compat/compat_fnmatch.c \
			 $(LIBKS_COMMON_C)

KINGSNDB_TOOL_OBJS := $(KINGSNDB_TOOL_C:.c=.o)

RMSGPACK_C = \
			$(LIBKSDB_DIR)/rmsgpack.c \
			$(LIBKSDB_DIR)/rmsgpack_test.c \
			 $(LIBKS_COMMON_C)

RMSGPACK_OBJS := $(RMSGPACK_C:.c=.o)

TESTLIB_FLAGS = $(CFLAGS) -shared -fpic

.PHONY: all clean

all: $(TARGETS)

%.o: %.c
	$(CC) $(INCFLAGS) $< -c $(CFLAGS) -o $@

c_converter: $(C_CONVERTER_OBJS)
	$(CC) $(INCFLAGS) $(C_CONVERTER_OBJS) $(CFLAGS) -o $@

libksdb_tool: $(KINGSNDB_TOOL_OBJS)
	$(CC) $(INCFLAGS) $(KINGSNDB_TOOL_OBJS) -o $@

rmsgpack_test: $(RMSGPACK_OBJS)
	$(CC) $(INCFLAGS) $(RMSGPACK_OBJS) -g -o $@

clean:
	rm -rf $(TARGETS) $(C_CONVERTER_OBJS) $(KINGSNDB_TOOL_OBJS) $(RMSGPACK_OBJS) $(TESTLIB_OBJS)
