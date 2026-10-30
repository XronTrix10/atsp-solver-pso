# Hybrid PSO solver - build file
#
#   make                 optimised build  -> ./pso
#   make debug           ASan/UBSan build -> ./pso_debug
#   make VELOCITY=1      enable the PSO velocity update (w, c1, c2 take effect)
#   make clean
#
# Overridable: CC, NATIVE=0 (drop -march=native), CFLAGS, LDFLAGS

CC      ?= gcc
TARGET  := pso
SRC_DIR := src
INC_DIR := include

BUILD    ?= release
VELOCITY ?= 0
NATIVE   ?= 1

# Separate object directories so switching modes never mixes objects
BUILD_DIR := build/$(BUILD)$(if $(filter 1,$(VELOCITY)),-velocity)

WARNFLAGS := -Wall -Wextra

ifeq ($(BUILD),debug)
  OPTFLAGS := -O0 -g3 -fsanitize=address,undefined -fno-omit-frame-pointer
else
  OPTFLAGS := -Ofast -flto
  ifeq ($(NATIVE),1)
    OPTFLAGS += -march=native
  endif
endif

CFLAGS  += -std=gnu11 $(WARNFLAGS) $(OPTFLAGS) -fopenmp -I$(INC_DIR) -MMD -MP
LDFLAGS += $(OPTFLAGS) -fopenmp
LDLIBS  += -lm

ifeq ($(VELOCITY),1)
  CFLAGS += -DPSO_ENABLE_VELOCITY
endif

SRCS := $(wildcard $(SRC_DIR)/*.c)
OBJS := $(SRCS:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)
DEPS := $(OBJS:.o=.d)

.PHONY: all debug clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $@ $(LDFLAGS) $(LDLIBS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR):
	mkdir -p $@

debug:
	$(MAKE) BUILD=debug TARGET=pso_debug pso_debug

clean:
	rm -rf build $(TARGET) pso_debug

-include $(DEPS)
