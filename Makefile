
SRCS := malloc_challenge_shimazu.c simple_malloc.c
OBJS = $(SRCS:.c=.o)
DEPS = $(SRCS:.c=.d)

CFLAGS = -Wall -Wextra
LDFLAGS = -lm

TARGET := malloc_challenge.bin

.PHONY: all
all: $(TARGET)

$(TARGET): $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -MMD -c -o $@ $^

.PHONY: clean
clean:
	rm -f $(OBJS) $(TARGET) $(DEPS)

-include $(DEPS)