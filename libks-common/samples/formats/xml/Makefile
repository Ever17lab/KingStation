TARGET := rxml

LIBKS_XML_DIR  := ../../../formats/xml
LIBKS_COMM_DIR := ../../../
LIBKS_DEPS_DIR := ../../../../deps

SOURCES := \
	rxml_test.c \
	$(LIBKS_XML_DIR)/rxml.c \
	$(LIBKS_DEPS_DIR)/yxml/yxml.c \
	$(LIBKS_COMM_DIR)/compat/fopen_utf8.c \
	$(LIBKS_COMM_DIR)/compat/compat_strl.c \
	$(LIBKS_COMM_DIR)/encodings/encoding_utf.c \
	$(LIBKS_COMM_DIR)/streams/file_stream.c \
	$(LIBKS_COMM_DIR)/vfs/vfs_implementation.c

OBJS := $(SOURCES:.c=.o)

CFLAGS += -DRXML_TEST -Wall -pedantic -std=gnu99 -g -I$(LIBKS_COMM_DIR)/include

all: $(TARGET)

%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)

$(TARGET): $(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

clean:
	rm -f $(TARGET) $(OBJS)

.PHONY: clean
