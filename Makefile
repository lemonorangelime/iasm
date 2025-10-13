ifeq ($(PREFIX),)
        PREFIX := /usr/local
endif

BUILD_DIR := build

CC := gcc
ASM := nasm
ASMFLAGS := -f elf64 -I include/
CCFLAGS := -flto -s -Oz -funroll-all-loops -ftree-vectorize -fomit-frame-pointer -m64 -mhard-float -fno-stack-protector -Iinclude -Wno-address-of-packed-member -z noexecstack -z noseparate-code
LD := ld
LDFLAGS := --strip-all --discard-all --discard-locals --strip-debug

STRUCTURE := $(shell find src/ -type d)
FILES := $(addsuffix /*,$(STRUCTURE))
FILES := $(wildcard $(FILES))

SOURCES := $(filter %.c,$(FILES))
ASM_SOURCES := $(filter %.asm,$(FILES))

OBJS := $(subst src/,build/,$(SOURCES:%.c=%.c.o))
ASM_OBJS := $(subst src/,build/,$(ASM_SOURCES:%.asm=%.asm.o))

BUILD_STRUCTURE := $(subst src/,build/,$(STRUCTURE))

OUTPUT := iasm

default: mkdir $(OUTPUT)

mkdir:
	mkdir -p ${BUILD_DIR} ${BUILD_STRUCTURE}

clean:
	rm -rf ${BUILD_DIR} ${OUTPUT}

$(BUILD_DIR)/%.c.o: src/%.c
	$(CC) $(CCFLAGS) $^ -c -o $@

$(BUILD_DIR)/%.asm.o: src/%.asm
	$(ASM) $(ASMFLAGS) $^ -o $@

$(OUTPUT): $(OBJS) $(ASM_OBJS)
	$(CC) $(CCFLAGS) $^ -o $@
	objcopy --remove-section .note $@
	objcopy --remove-section .note.gnu.property $@
	objcopy --remove-section .note.ABI-tag $@
	objcopy --remove-section .hash $@
	objcopy --remove-section .gnu.hash $@
	objcopy --remove-section .gnu.version $@
	objcopy --remove-section .comment $@
	strip --strip-unneeded --strip-debug --strip-section-headers $@

# "iasm" seems to be a pretty common name
install: $(OUTPUT)
	if [ -f "$(PREFIX)/bin/$(OUTPUT)" ]; then \
		echo "\e[0;31mERROR\e[0m: File conflict. $(PREFIX)/bin/$(OUTPUT) already exists"; \
		exit; \
	else \
		install -m 775 $(OUTPUT) $(PREFIX)/bin/; \
	fi

uninstall:
	rm $(PREFIX)/bin/$(OUTPUT)
