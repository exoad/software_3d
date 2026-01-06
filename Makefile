CC = gcc
ASAN_FLAGS =
ifeq ($(ASAN),1)
    ASAN_FLAGS = -fsanitize=address -fsanitize=undefined -fsanitize-trap=all
else ifeq ($(filter asan,$(MAKECMDGOALS)),asan)
    ASAN_FLAGS = -fsanitize=address -fsanitize=undefined -fsanitize-trap=all
endif

SRCDIR = src
INCDIR = include
ASSETDIR = assets
OBJDIR = obj
BIN = main
ENTRY= main.c
DEFINES ?=
C_FILES = $(ENTRY) $(wildcard $(SRCDIR)/*.c) tigr.c
OBJ_FILES = $(patsubst %.c,$(OBJDIR)/%.o,$(notdir $(C_FILES)))
OPT = -O3
DEF_FILES =

CFLAGS = -std=c17 -D_GNU_SOURCE -Wall -Wextra -Wpedantic -Wunused -Wcast-align -Wno-unused-function -fno-stack-protector -march=native -Wshadow -fopenmp $(OPT) $(ASAN_FLAGS) $(DEFINES) -I$(INCDIR) -I$(ASSETDIR)

UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Linux)
    PLATFORM_LDFLAGS = -lrt -lm
    PLATFORM_LDFLAGS += -lGLU -lGL -lX11
	BIN = game.o
else
    PLATFORM_LDFLAGS =
    PLATFORM_LDFLAGS += -lopengl32 -lgdi32 -fopenmp
endif

LDFLAGS = $(ASAN_FLAGS) $(PLATFORM_LDFLAGS)

all: $(BIN)
	@echo Linker Flags: $(PLATFORM_LDFLAGS)

$(OBJDIR):
	@mkdir -p $(OBJDIR)

$(BIN): $(OBJ_FILES) | $(OBJDIR)
	$(CC) $(OBJ_FILES) -o $@ $(LDFLAGS)

$(OBJDIR)/%.o: %.c $(DEF_FILES) | $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR)/%.o: $(SRCDIR)/%.c $(DEF_FILES) | $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ_FILES)
	rm -rf $(OBJDIR)

fclean: clean
	rm -f $(BIN)

re: fclean all

asan:
	@$(MAKE) all ASAN=1

.PHONY: all clean fclean re asan