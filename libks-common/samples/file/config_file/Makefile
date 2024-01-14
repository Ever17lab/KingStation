TARGET := config_file_test

LIBKS_COMM_DIR := ../../..

SOURCES := \
	config_file_test.c \
	$(LIBKS_COMM_DIR)/compat/fopen_utf8.c \
	$(LIBKS_COMM_DIR)/compat/compat_strl.c \
	$(LIBKS_COMM_DIR)/compat/compat_strcasestr.c \
	$(LIBKS_COMM_DIR)/compat/compat_posix_string.c \
	$(LIBKS_COMM_DIR)/encodings/encoding_utf.c \
	$(LIBKS_COMM_DIR)/file/file_path.c \
	$(LIBKS_COMM_DIR)/file/file_path_io.c \
	$(LIBKS_COMM_DIR)/file/config_file.c \
	$(LIBKS_COMM_DIR)/lists/string_list.c \
	$(LIBKS_COMM_DIR)/string/stdstring.c \
	$(LIBKS_COMM_DIR)/streams/file_stream.c \
	$(LIBKS_COMM_DIR)/vfs/vfs_implementation.c \
	$(LIBKS_COMM_DIR)/time/rtime.c

OBJS := $(SOURCES:.c=.o)

CFLAGS += -Wall -pedantic -std=gnu99 -g -I$(LIBKS_COMM_DIR)/include

all: $(TARGET)

%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)

$(TARGET): $(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

clean:
	rm -f $(TARGET) $(OBJS)

.PHONY: clean
