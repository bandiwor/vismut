CC = gcc
CFLAGS = -Wall -Wextra -pedantic # -flto
LIBS = -lm

# Базовое имя проекта
TARGET_NAME = vismut

SRCS = main.c $(shell find Vismut -type f -name '*.c')

# Директории для сборки
BUILD_DIR = build
DEBUG_DIR = $(BUILD_DIR)/debug
RELEASE_DIR = $(BUILD_DIR)/release

# Генерируем пути к объектным файлам для каждой версии
DEBUG_OBJS = $(SRCS:%.c=$(DEBUG_DIR)/%.o)
RELEASE_OBJS = $(SRCS:%.c=$(RELEASE_DIR)/%.o)

.PHONY: all clean debug

# --- Release Build ---
all: $(RELEASE_DIR)/$(TARGET_NAME)

$(RELEASE_DIR)/$(TARGET_NAME): CFLAGS += -O3 -march=native
$(RELEASE_DIR)/$(TARGET_NAME): $(RELEASE_OBJS)
	$(CC) $(RELEASE_OBJS) $(CFLAGS) -o $@ $(LIBS)

$(RELEASE_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -O3 -march=native -c $< -o $@

# --- Debug Build ---
debug: $(DEBUG_DIR)/$(TARGET_NAME)-debug

$(DEBUG_DIR)/$(TARGET_NAME)-debug: CFLAGS += -g3 -DDEBUG -DVISMUT_DEBUG
$(DEBUG_DIR)/$(TARGET_NAME)-debug: $(DEBUG_OBJS)
	$(CC) $(DEBUG_OBJS) $(CFLAGS) -o $@ $(LIBS)

$(DEBUG_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -g3 -DDEBUG -c $< -o $@

clean:
	rm -rf $(BUILD_DIR)
