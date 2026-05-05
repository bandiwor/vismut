CC = clang
CFLAGS = -Wall -Wextra -pedantic
DEPFLAGS = -MMD -MP
LIBS = -lm

TARGET_NAME = vismut
TEST_TARGET_NAME = vismut_test

ALL_VISMUT_SRCS = $(shell find Vismut -type f -name '*.c')
MAIN_SRC = main.c
TEST_SRCS = $(shell find Vismut/test -type f -name '*.c')

CORE_SRCS = $(filter-out $(TEST_SRCS), $(ALL_VISMUT_SRCS))

BUILD_DIR = build
RELEASE_DIR = $(BUILD_DIR)/release
DEBUG_DIR = $(BUILD_DIR)/debug
TEST_REL_DIR = $(BUILD_DIR)/test_release
TEST_DBG_DIR = $(BUILD_DIR)/test_debug

RELEASE_OBJS = $(CORE_SRCS:%.c=$(RELEASE_DIR)/%.o) $(RELEASE_DIR)/$(MAIN_SRC:.c=.o)
DEBUG_OBJS   = $(CORE_SRCS:%.c=$(DEBUG_DIR)/%.o) $(DEBUG_DIR)/$(MAIN_SRC:.c=.o)

TEST_REL_OBJS = $(CORE_SRCS:%.c=$(TEST_REL_DIR)/%.o) $(TEST_SRCS:%.c=$(TEST_REL_DIR)/%.o)
TEST_DBG_OBJS = $(CORE_SRCS:%.c=$(TEST_DBG_DIR)/%.o) $(TEST_SRCS:%.c=$(TEST_DBG_DIR)/%.o)

DEPS = $(RELEASE_OBJS:.o=.d) $(DEBUG_OBJS:.o=.d) $(TEST_REL_OBJS:.o=.d) $(TEST_DBG_OBJS:.o=.d)

.PHONY: all clean debug test test-dbg

all: $(RELEASE_DIR)/$(TARGET_NAME)

$(RELEASE_DIR)/$(TARGET_NAME): CFLAGS += -O3 -march=native -flto=auto 
$(RELEASE_DIR)/$(TARGET_NAME): $(RELEASE_OBJS)
	$(CC) $(CFLAGS) $^ -s -o $@ $(LIBS)

$(RELEASE_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(DEPFLAGS) -DNDEBUG -c $< -o $@

debug: $(DEBUG_DIR)/$(TARGET_NAME)-debug

$(DEBUG_DIR)/$(TARGET_NAME)-debug: CFLAGS += -g3 -O0 -DDEBUG -DVISMUT_DEBUG -fsanitize=address
$(DEBUG_DIR)/$(TARGET_NAME)-debug: $(DEBUG_OBJS)
	$(CC) $(CFLAGS) $^ -o $@ $(LIBS)

$(DEBUG_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(DEPFLAGS) -g3 -O0 -DDEBUG -c $< -o $@

test: $(TEST_REL_DIR)/$(TEST_TARGET_NAME)
	@./$(TEST_REL_DIR)/$(TEST_TARGET_NAME)

$(TEST_REL_DIR)/$(TEST_TARGET_NAME): CFLAGS += -O3 -march=native -flto=auto 
$(TEST_REL_DIR)/$(TEST_TARGET_NAME): $(TEST_REL_OBJS)
	$(CC) $(CFLAGS) $^ -s -o $@ $(LIBS)

$(TEST_REL_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(DEPFLAGS) -DNDEBUG -c $< -o $@

test-dbg: $(TEST_DBG_DIR)/$(TEST_TARGET_NAME)-debug
	@./$(TEST_DBG_DIR)/$(TEST_TARGET_NAME)-debug

$(TEST_DBG_DIR)/$(TEST_TARGET_NAME)-debug: CFLAGS += -g3 -O0 -DDEBUG -DVISMUT_DEBUG -fsanitize=address
$(TEST_DBG_DIR)/$(TEST_TARGET_NAME)-debug: $(TEST_DBG_OBJS)
	$(CC) $(CFLAGS) $^ -o $@ $(LIBS)

$(TEST_DBG_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(DEPFLAGS) -g3 -O0 -DDEBUG -c $< -o $@

clean:
	rm -rf $(BUILD_DIR)

-include $(DEPS)
