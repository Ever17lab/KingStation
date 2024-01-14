TARGET := strl

CORE_DIR          := .
LIBKS_COMM_DIR := ../../..

SOURCES_C := 	\
	$(CORE_DIR)/strl_test.c \
	$(LIBKS_COMM_DIR)/compat/compat_strl.c

OBJS := $(SOURCES_C:.c=.o)

CFLAGS += -Wall -pedantic -std=gnu99 -I$(LIBKS_COMM_DIR)/include

all: $(TARGET)

%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)

$(TARGET): $(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

clean:
	rm -f $(TARGET) $(OBJS)

.PHONY: clean
