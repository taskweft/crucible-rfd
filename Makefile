CC      ?= gcc
CFLAGS  ?= -std=c17 -Wall -Wextra -Wpedantic -O2 -g -pthread -I/usr/include/foundationdb
LDFLAGS ?= -pthread
LIBS    ?= -lh2o -lfdb_c -ldl -lm

SRC     := src/bench
BUILD   := build
TARGET  := $(BUILD)/crucible-demo

SRCS    := $(wildcard $(SRC)/*.c)
OBJS    := $(SRCS:%.c=$(BUILD)/%.o)
DEPS    := $(OBJS:.o=.d)

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) $(LIBS)

$(BUILD)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -MMD -MP -c -o $@ $<

clean:
	rm -rf $(BUILD)

-include $(DEPS)
