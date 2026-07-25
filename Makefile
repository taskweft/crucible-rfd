CC      ?= gcc
CFLAGS  ?= -std=c17 -Wall -Wextra -Wpedantic -O2 -g
LDFLAGS ?=
LIBS    ?= -lfdb_c -ldl -lm

SRC     := src
NIF     := $(SRC)/nif
BUILD   := build
TARGET  := $(BUILD)/crucible

SRCS    := $(wildcard $(SRC)/*.c) $(wildcard $(NIF)/*.c)
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
