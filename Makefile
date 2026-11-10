FUNC := gcc
copt := -c
OBJ_DIR := ./bin/
ARCH ?= $(shell $(FUNC) -march=x86-64-v3 -E -x c /dev/null >/dev/null 2>&1 && echo -march=x86-64-v3 || echo -mavx2 -mfma -mbmi -mbmi2 -mlzcnt -mmovbe -mf16c)
FLAGS := -O3 $(ARCH) -fopenmp -lm -g -Werror

C_FILES := $(wildcard src/*.c)
OBJ_FILES := $(addprefix $(OBJ_DIR),$(notdir $(C_FILES:.c=.obj)))

all:
	$(FUNC) ./sandpile.c -o ./sandpile.exe $(FLAGS)

clean:
	rm -f ./*.exe
