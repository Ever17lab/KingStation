TARGETS  = http_test http_parse_test net_ifinfo

LIBKS_COMM_DIR := ../..

INCFLAGS = -I$(LIBKS_COMM_DIR)/include

ifeq ($(platform),)
platform = unix
ifeq ($(shell uname -a),)
   platform = win
else ifneq ($(findstring Darwin,$(shell uname -a)),)
   platform = osx
	arch = intel
ifeq ($(shell uname -p),powerpc)
	arch = ppc
endif
else ifneq ($(findstring MINGW,$(shell uname -a)),)
   platform = win
endif
endif

ifeq ($(DEBUG),1)
CFLAGS += -O0 -g
else
CFLAGS += -O2
endif
CFLAGS += -Wall -pedantic -std=gnu99

HTTP_TEST_C = \
				  $(LIBKS_COMM_DIR)/net/net_http.c \
				  $(LIBKS_COMM_DIR)/net/net_compat.c \
				  $(LIBKS_COMM_DIR)/net/net_socket.c \
				  $(LIBKS_COMM_DIR)/compat/compat_strl.c \
				  $(LIBKS_COMM_DIR)/encodings/encoding_utf.c \
				  $(LIBKS_COMM_DIR)/string/stdstring.c \
				  net_http_test.c

HTTP_TEST_OBJS := $(HTTP_TEST_C:.c=.o)

HTTP_PARSE_TEST_C = \
				  $(LIBKS_COMM_DIR)/net/net_http.c \
				  $(LIBKS_COMM_DIR)/net/net_http_parse.c \
				  $(LIBKS_COMM_DIR)/net/net_compat.c \
				  $(LIBKS_COMM_DIR)/net/net_socket.c \
				  $(LIBKS_COMM_DIR)/compat/compat_strl.c \
				  $(LIBKS_COMM_DIR)/compat/compat_strcasestr.c \
				  $(LIBKS_COMM_DIR)/encodings/encoding_utf.c \
				  $(LIBKS_COMM_DIR)/string/stdstring.c \
				  net_http_parse_test.c

HTTP_PARSE_TEST_OBJS := $(HTTP_PARSE_TEST_C:.c=.o)

NET_IFINFO_C = \
					$(LIBKS_COMM_DIR)/net/net_ifinfo.c \
					net_ifinfo_test.c

ifeq ($(platform), win)
CFLAGS += -liphlpapi -lws2_32
endif

NET_IFINFO_OBJS := $(NET_IFINFO_C:.c=.o)

.PHONY: all clean

all: $(TARGETS)

%.o: %.c
	$(CC) $(INCFLAGS) $< -c $(CFLAGS) -o $@

http_parse_test: $(HTTP_PARSE_TEST_OBJS)
	$(CC) $(INCFLAGS) $(HTTP_PARSE_TEST_OBJS) $(CFLAGS) -o $@

http_test: $(HTTP_TEST_OBJS)
	$(CC) $(INCFLAGS) $(HTTP_TEST_OBJS) $(CFLAGS) -o $@

net_ifinfo: $(NET_IFINFO_OBJS)
	$(CC) $(INCFLAGS) $(NET_IFINFO_OBJS) $(CFLAGS) -o $@

clean:
	rm -rf $(TARGETS) $(HTTP_TEST_OBJS) $(HTTP_PARSE_TEST_OBJS) $(NET_IFINFO_OBJS)
