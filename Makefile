.POSIX:
.SUFFIXES:

##
## GENERAL SETUP
#
SRC_DIRS  = ./src/fv1 ./src/vm ./src/misc ./src/fv1/debug
BUILD_DIR = ./build

INCLUDES = ./src/
DEFINES  += FV1_DELAY_I32

CPPFLAGS += -g -O3
CPPFLAGS += -Wall -Wextra -Wpedantic -Wshadow -Werror -Wno-missing-braces
CPPFLAGS += -Wdouble-promotion
CPPFLAGS += -Wconversion -Wno-sign-conversion
CPPFLAGS += -Wno-gnu-zero-variadic-macro-arguments
CPPFLAGS += -Wframe-larger-than=2048 # somewhat arbitrary
CPPFLAGS += $(addprefix -I, $(INCLUDES))
CPPFLAGS += $(addprefix -D, $(DEFINES))
CPPFLAGS += -MMD -MP
CPPFLAGS += -std=c++17
LIBS+=
LDFLAGS+=

###
## CORE
#
SRCS += $(wildcard $(patsubst %,%/*.cc,$(SRC_DIRS)))
OBJS += $(patsubst %,$(BUILD_DIR)/%,$(notdir $(SRCS:.cc=.o)))
DEPS += $(OBJS:.o=.d)
VPATH += $(SRC_DIRS)

ASFV1 := asfv1

###
## TESTS
#
GTEST_DIR = ./extern/googletest/googletest
GTEST_INCS = -isystem$(GTEST_DIR)/include
GTEST_OBJS = $(BUILD_DIR)/gtest-all.o
GTEST_SRCS =  $(GTEST_DIR)/src/*.cc $(GTEST_DIR)/src/*.h

TEST_SRC_DIRS = $(SRC_DIRS) ./test
TEST_SRCS += $(wildcard $(patsubst %,%/*.cc,$(TEST_SRC_DIRS)))
TEST_OBJS += $(patsubst %,$(BUILD_DIR)/%,$(notdir $(TEST_SRCS:.cc=.o)))
TESTS = $(BUILD_DIR)/fv1_tests
VPATH += ./test
DEPS += $(TEST_OBJS:.o=.d)

TEST_ASM_SRC = $(wildcard ./test/asm/*.asm)
TEST_ASM_BIN = $(patsubst %,$(BUILD_DIR)/tests/%,$(notdir $(TEST_ASM_SRC:.asm=.bin)))
TEST_ASM_DST = $(BUILD_DIR)/tests

###
## TOOLS
#
TOOLS = fv1_dump
TOOL_SRC_DIR = ./tools
ALL_TOOL_SRCS += $(wildcard $(patsubst %,%/*.cc,$(TOOL_SRC_DIR)))
ALL_TOOL_OBJS += $(patsubst %,$(BUILD_DIR)/%,$(notdir $(ALL_TOOL_SRCS:.cc=.o)))
VPATH += ./tools
DEPS += $(ALL_TOOL_OBJS:.o=.d)

# linkable bits, without the files that contain main
TOOL_SRCS = $(TOOL_SRC_DIR)/fv1_tools.cc
TOOL_OBJS += $(patsubst %,$(BUILD_DIR)/%,$(notdir $(TOOL_SRCS:.cc=.o)))

define tool-target
.PHONY: $1
$1: $$(addprefix $(BUILD_DIR)/, $1)
$$(addprefix $(BUILD_DIR)/, $1): $$(BUILD_DIR)
$$(addprefix $(BUILD_DIR)/, $1): $(OBJS) $(TOOL_OBJS) $(BUILD_DIR)/$(addsuffix .o, $1)
	$$(ECHO) "Linking $$@..."
	$$(Q)$(CXX) $(LDFLAGS) -o $$@ $(OBJS) $(TOOL_OBJS) $(BUILD_DIR)/$(addsuffix .o, $1)
endef

ifdef VERBOSE
Q :=
ECHO := @true
else
Q := @
ECHO := @echo
endif

.PHONY: all
all: runtests tools

.PHONY: clean
clean: clean_tests
	@$(RM) -f $(BUILD_DIR)/*

.PHONY: tests
tests: $(TESTS) test_asm

.PHONY: runtests
runtests: tests
	@$(TESTS)

.PHONY: test_asm
test_asm: $(TEST_ASM_BIN)

.PHONY: clean_tests
clean_tests:
	@$(RM) -rf $(TEST_ASM_DST)

$(TESTS): CPPFLAGS += $(GTEST_INCS)
$(TESTS): $(BUILD_DIR) $(TEST_OBJS) $(GTEST_OBJS)
	$(ECHO) "Linking $@..."
	$(Q)$(CXX) $(LDFLAGS) -o $@ $(TEST_OBJS) $(GTEST_OBJS) $(LIBS) -pthread

.PHONY: tools
tools: $(TOOLS)

$(foreach tool, $(TOOLS), $(eval $(call tool-target, $(tool))))

$(BUILD_DIR):
	@mkdir -p $@

$(TEST_ASM_DST):

$(BUILD_DIR)/gtest-all.o: $(GTEST_SRCS)
	$(ECHO) "CC $<..."
	$(Q)$(CXX) -I$(GTEST_DIR) -I$(GTEST_DIR)/include $(CPPFLAGS) \
		-Wframe-larger-than=32768 -Wno-conversion \
		-c $(GTEST_DIR)/src/gtest-all.cc \
		-o $@

.SUFFIXES:
$(BUILD_DIR)/%.o: %.cc
	$(ECHO) "CC $<..."
	$(Q)$(CXX) -c $(CPPFLAGS) $< -o $@

$(TEST_ASM_DST)/%.bin: ./test/asm/%.asm
	@mkdir -p $(TEST_ASM_DST)
	@echo "FV1 $<..."
	@$(ASFV1) -b -q $< $@

-include $(DEPS)