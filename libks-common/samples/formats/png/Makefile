TARGET := rpng

CORE_DIR          := .
LIBKS_PNG_DIR  := ../../../formats/png
LIBKS_COMM_DIR := ../../..

HAVE_IMLIB2=0

LDFLAGS +=  -lz

ifeq ($(HAVE_IMLIB2),1)
CFLAGS += -DHAVE_IMLIB2
LDFLAGS += -lImlib2
endif

SOURCES_C := 	\
	$(CORE_DIR)/rpng_test.c \
	$(LIBKS_PNG_DIR)/rpng.c \
	$(LIBKS_PNG_DIR)/rpng_encode.c \
	$(LIBKS_COMM_DIR)/encodings/encoding_crc32.c \
	$(LIBKS_COMM_DIR)/encodings/encoding_utf.c \
	$(LIBKS_COMM_DIR)/string/stdstring.c \
	$(LIBKS_COMM_DIR)/compat/fopen_utf8.c \
	$(LIBKS_COMM_DIR)/compat/compat_strl.c \
	$(LIBKS_COMM_DIR)/compat/compat_posix_string.c \
	$(LIBKS_COMM_DIR)/compat/compat_strcasestr.c \
	$(LIBKS_COMM_DIR)/file/nbio/nbio_intf.c \
	$(LIBKS_COMM_DIR)/file/nbio/nbio_stdio.c \
	$(LIBKS_COMM_DIR)/file/nbio/nbio_linux.c \
	$(LIBKS_COMM_DIR)/file/nbio/nbio_unixmmap.c \
	$(LIBKS_COMM_DIR)/file/nbio/nbio_windowsmmap.c \
	$(LIBKS_COMM_DIR)/file/archive_file.c \
	$(LIBKS_COMM_DIR)/file/archive_file_zlib.c \
	$(LIBKS_COMM_DIR)/file/file_path.c \
	$(LIBKS_COMM_DIR)/file/file_path_io.c \
	$(LIBKS_COMM_DIR)/streams/file_stream.c \
	$(LIBKS_COMM_DIR)/vfs/vfs_implementation.c \
	$(LIBKS_COMM_DIR)/streams/interface_stream.c \
	$(LIBKS_COMM_DIR)/streams/memory_stream.c \
	$(LIBKS_COMM_DIR)/streams/trans_stream.c \
	$(LIBKS_COMM_DIR)/streams/trans_stream_zlib.c \
	$(LIBKS_COMM_DIR)/streams/trans_stream_pipe.c \
	$(LIBKS_COMM_DIR)/lists/string_list.c

OBJS := $(SOURCES_C:.c=.o)

CFLAGS += -Wall -pedantic -std=gnu99 -O0 -g -DHAVE_ZLIB -DRPNG_TEST -I$(LIBKS_COMM_DIR)/include

all: $(TARGET)

%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)

$(TARGET): $(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

clean:
	rm -f $(TARGET) $(OBJS)

.PHONY: clean
