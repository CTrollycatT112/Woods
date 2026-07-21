BUILD ?= chk

TOOLCHAINPATH ?= $(HOME)/opt/cross/bin
TARGET := x86_64-elf

CXX := $(TOOLCHAINPATH)/$(TARGET)-g++
CC := $(TOOLCHAINPATH)/$(TARGET)-gcc
LD := $(TOOLCHAINPATH)/$(TARGET)-ld
OBJCOPY := $(TOOLCHAINPATH)/$(TARGET)-objcopy
NASM := nasm

ROOTPATH := $(CURDIR)
OUTPATH := $(ROOTPATH)/out
BUILDPATH := $(OUTPATH)/$(BUILD)
OBJPATH := $(BUILDPATH)/obj
SYSTEM32PATH := $(BUILDPATH)/system32
DRIVERSPATH := $(SYSTEM32PATH)/drivers

LIMINEPATH := $(ROOTPATH)/vendor/limine
FLANTERMPATH := $(ROOTPATH)/vendor/flanterm
PRINTFPATH := $(ROOTPATH)/vendor/printf
INCPATH := $(ROOTPATH)/src/inc
SCRIPTSPATH := $(ROOTPATH)/tools/scripts

HTOSKRNLNAME := htoskrnl.exe
HTOSKRNLPATH := $(SYSTEM32PATH)/$(HTOSKRNLNAME)

LIMINECONF := $(ROOTPATH)/limine.conf
LINKSCRIPT := $(ROOTPATH)/src/boot/woods-x86_64.ld

COLOR_RESET := \033[0m
COLOR_CYAN := \033[1;36m
COLOR_MAGENTA := \033[1;35m
COLOR_YELLOW := \033[1;33m
COLOR_GREEN := \033[1;32m
COLOR_RED := \033[1;31m

LOGCXX = printf "  $(COLOR_CYAN)[ CXX ]$(COLOR_RESET)   %s\n" "$(1)"
LOGAS = printf "  $(COLOR_MAGENTA)[ AS ]$(COLOR_RESET)  %s\n" "$(1)"
LOGLD = printf "  $(COLOR_YELLOW)[ LD ]$(COLOR_RESET)   %s\n" "$(1)"
LOGGEN = printf "  $(COLOR_GREEN)[ GEN ]$(COLOR_RESET)  %s\n" "$(1)"
LOGCLEAN = printf "  $(COLOR_RED)[ CLN ]$(COLOR_RESET)  %s\n" "$(1)"

SRCS :=

include src/htos/local.mk
include src/boot/local.mk

CPPSRCS := $(filter %.cpp,$(SRCS))
ASMSRCS := $(filter %.asm,$(SRCS))
S_SRCS  := $(filter %.S,$(SRCS))

CPPOBJS := $(patsubst src/%.cpp,$(OBJPATH)/%.cpp.o,$(CPPSRCS))
ASMOBJS := $(patsubst src/%.asm,$(OBJPATH)/%.asm.o,$(ASMSRCS))
S_OBJS  := $(patsubst src/%.S,$(OBJPATH)/%.S.o,$(S_SRCS))

FLANTERM_CORE_OBJ := $(OBJPATH)/flanterm/flanterm.o
FLANTERM_FB_OBJ := $(OBJPATH)/flanterm/backends/fb.o
FLANTERMOBJS := $(FLANTERM_CORE_OBJ) $(FLANTERM_FB_OBJ)

OBJS := $(CPPOBJS)
OBJS += $(ASMOBJS)
OBJS += $(S_OBJS)
OBJS += $(FLANTERMOBJS)

DEPS := $(OBJS:.o=.d)

CXXSTD := -std=c++23

COMMONFLAGS := -ffreestanding -fno-builtin -fno-stack-protector -fno-stack-check \
    -fno-lto -fno-pic -fno-pie -m64 -march=x86-64 \
    -mno-80387 -mno-mmx -mno-3dnow -mno-sse -mno-sse2 \
    -mno-red-zone -mgeneral-regs-only -mcmodel=kernel

CFLAGS := $(COMMONFLAGS) -Wall -Wextra -Werror \
    -I$(FLANTERMPATH)/src -MMD -MP

CXXFLAGS := $(CXXSTD) $(COMMONFLAGS) -Wall -Wextra -Werror \
    -fno-exceptions -fno-rtti -fno-unwind-tables -fno-asynchronous-unwind-tables \
    -I$(INCPATH) -I$(LIMINEPATH) -I$(FLANTERMPATH)/src -MMD -MP

LDFLAGS := -nostdlib -static -m elf_x86_64 -z max-page-size=0x1000 -T $(LINKSCRIPT)

ifeq ($(BUILD),chk)
CFLAGS   += -O0 -g -DHTOS_CHK=1
CXXFLAGS += -O0 -g -DHTOS_CHK=1
endif

ifeq ($(BUILD),fre)
CFLAGS   += -O2 -DHTOS_CHK=0
CXXFLAGS += -O2 -DHTOS_CHK=0
endif

.PHONY: all chk fre kernel dirs disk run clean

all: chk

chk:
	@$(MAKE) BUILD=chk kernel

fre:
	@$(MAKE) BUILD=fre kernel

kernel: $(HTOSKRNLPATH)

dirs:
	@mkdir -p $(BUILDPATH)
	@mkdir -p $(OBJPATH)
	@mkdir -p $(SYSTEM32PATH)
	@mkdir -p $(DRIVERSPATH)

$(HTOSKRNLPATH): dirs $(OBJS)
	@$(call LOGLD,$(HTOSKRNLNAME))
	@$(LD) $(LDFLAGS) -o $@ $(OBJS)

$(OBJPATH)/%.cpp.o: src/%.cpp
	@mkdir -p $(dir $@)
	@$(call LOGCXX,$<)
	@$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJPATH)/flanterm/%.o: $(FLANTERMPATH)/src/%.c
	@mkdir -p $(dir $@)
	@printf "  $(COLOR_CYAN)[ CC ]$(COLOR_RESET)    %s\n" "$<"
	@$(CC) $(CFLAGS) -c $< -o $@

$(OBJPATH)/flanterm/backends/%.o: $(FLANTERMPATH)/src/flanterm_backends/%.c
	@mkdir -p $(dir $@)
	@printf "  $(COLOR_CYAN)[ CC ]$(COLOR_RESET)    %s\n" "$<"
	@$(CC) $(CFLAGS) -c $< -o $@

$(OBJPATH)/%.asm.o: src/%.asm
	@mkdir -p $(dir $@)
	@$(call LOGAS,$<)
	@$(NASM) -f elf64 $< -o $@

$(OBJPATH)/%.S.o: src/%.S
	@mkdir -p $(dir $@)
	@$(call LOGAS,$<)
	@$(CC) $(CXXFLAGS) -c $< -o $@

disk:
	@$(call LOGGEN,htos.img)
	@$(SCRIPTSPATH)/buildDisk.sh $(BUILD)

run: disk
	@$(SCRIPTSPATH)/runQemu.sh $(BUILD)

clean:
	@$(call LOGCLEAN,$(OUTPATH))
	@rm -rf $(OUTPATH)

-include $(DEPS)
