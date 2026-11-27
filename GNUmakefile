# Nuke built-in rules.
.SUFFIXES:

# Delete the target of a failed recipe.
.DELETE_ON_ERROR:

# This is the name that our final executable will have.
# Change as needed.
override OUTPUT := efi-template

# Target architecture to build for. Default to x86_64.
ARCH := x86_64

# Install prefix; /usr/local is a good, standard default pick.
PREFIX := /usr/local

# Check if the architecture is supported, unless only running distclean,
# which does not use it.
ifneq ($(filter-out distclean,$(or $(MAKECMDGOALS),all)),)
    ifeq ($(filter $(ARCH),ia32 aarch64 loongarch64 riscv64 x86_64),)
        $(error Architecture $(ARCH) not supported)
    endif
endif

# Default user QEMU flags. These are appended to the QEMU command calls.
QEMUFLAGS := -m 2G

# Per-architecture user controllable QEMU flags.
ifneq ($(filter $(ARCH),ia32 x86_64),)
    QEMU_MACHINE := -M q35
    # Uncomment this line and comment out/remove the xHCI/USB keyboard/mouse
    # block below if the program does not support xHCI/USB and wants legacy
    # PS/2 keyboard and mouse instead.
    #QEMU_DEVICES :=
    QEMU_DEVICES := \
        -device qemu-xhci \
        -device usb-kbd \
        -device usb-mouse
    QEMU_CPU :=
else
    QEMU_MACHINE := -M virt
    ifeq ($(ARCH),aarch64)
        QEMU_CPU := -cpu cortex-a72
    endif
    ifeq ($(ARCH),riscv64)
        QEMU_CPU := -cpu rv64
    endif
    ifeq ($(ARCH),loongarch64)
        QEMU_CPU := -cpu la464
    endif
    QEMU_DEVICES := \
        -device ramfb \
        -device qemu-xhci \
        -device usb-kbd \
        -device usb-mouse
endif

override QEMUFLAGS := \
    $(QEMU_MACHINE) \
    $(QEMU_CPU) \
    $(QEMU_DEVICES) \
    $(QEMUFLAGS)
override QEMU_UEFI_FW := \
    -drive if=pflash,unit=0,format=raw,file=edk2-ovmf-bins/ovmf-code-$(ARCH).fd,readonly=on

# Internal architecture specific variables that should not be changed by the
# user. QEMU calls ia32 i386.
override QEMU_ARCH := $(subst ia32,i386,$(ARCH))
ifeq ($(ARCH),ia32)
    override EFI_BOOT_FILE := BOOTIA32.EFI
endif
ifeq ($(ARCH),x86_64)
    override EFI_BOOT_FILE := BOOTX64.EFI
endif
ifeq ($(ARCH),aarch64)
    override EFI_BOOT_FILE := BOOTAA64.EFI
endif
ifeq ($(ARCH),riscv64)
    override EFI_BOOT_FILE := BOOTRISCV64.EFI
endif
ifeq ($(ARCH),loongarch64)
    override EFI_BOOT_FILE := BOOTLOONGARCH64.EFI
endif

# User controllable toolchain and toolchain prefix.
TOOLCHAIN :=
TOOLCHAIN_PREFIX :=
ifneq ($(TOOLCHAIN),)
    ifeq ($(TOOLCHAIN_PREFIX),)
        TOOLCHAIN_PREFIX := $(TOOLCHAIN)-
    endif
endif

# User controllable C and C++ compiler commands.
ifneq ($(TOOLCHAIN_PREFIX),)
    CC := $(TOOLCHAIN_PREFIX)gcc
    CXX := $(TOOLCHAIN_PREFIX)g++
else
    CC := cc
    CXX := c++
endif

# User controllable objcopy command.
OBJCOPY := $(TOOLCHAIN_PREFIX)objcopy

# Defaults overrides for variables if using "llvm" as toolchain.
ifeq ($(TOOLCHAIN),llvm)
    CC := clang
    CXX := clang++
endif

# User controllable C flags.
CFLAGS := -g -O2 -pipe

# User controllable C++ flags. We default to same as C flags.
CXXFLAGS := $(CFLAGS)

# User controllable C/C++ preprocessor flags. We set none by default.
CPPFLAGS :=

ifneq ($(filter $(ARCH),ia32 x86_64),)
    # User controllable nasm flags.
    NASMFLAGS := -g
endif

# User controllable linker flags. We set none by default.
LDFLAGS :=

# The goals other than cleaning and uninstalling, which need neither the
# dependencies nor a toolchain.
override BUILD_GOALS := $(filter-out clean distclean uninstall,$(or $(MAKECMDGOALS),all))

# Ensure the dependencies have been obtained, and set up the toolchain
# helper, which records the flags as given, before the internal ones.
ifneq ($(BUILD_GOALS),)
    ifeq ($(wildcard .deps-obtained),)
        $(error Please run the ./get-deps script first)
    endif
    override TCH_DRIVERS := CC CXX
    override TCH_ARCH := $(ARCH)
    # Clang calls ia32 i686.
    override TCH_CLANG_TARGET := $(subst ia32,i686,$(ARCH))-unknown-none-elf
    override TCH_SELECTORS := TOOLCHAIN TOOLCHAIN_PREFIX
    override TCH_CROSS_HINT := use TOOLCHAIN=llvm or a cross compiler for $(ARCH) with TOOLCHAIN_PREFIX
    include gmake-toolchain-helper/init.mk
endif

# Internal flags shared by both C and C++ compilers.
override SHARED_FLAGS := \
    -ffreestanding \
    -fno-builtin \
    -fno-asynchronous-unwind-tables \
    -fno-unwind-tables \
    -fno-common \
    -fno-strict-aliasing \
    -fno-stack-protector \
    -fno-stack-check \
    -fshort-wchar \
    -fPIE \
    -fvisibility=hidden \
    -ffunction-sections \
    -fdata-sections

# Internal C flags that should not be changed by the user. The standard
# comes before the checks below, so that a compiler too old for it is
# caught there rather than when the first source file is built.
override CFLAGS += \
    -Wall \
    -Wextra \
    -std=gnu11 \
    -fno-exceptions

# Internal C++ flags that should not be changed by the user.
override CXXFLAGS += \
    -Wall \
    -Wextra \
    -std=gnu++20 \
    -fno-exceptions \
    -fno-rtti

# Internal C/C++ preprocessor flags that should not be changed by the user.
override CPPFLAGS_C := \
    -nostdinc \
    -include hidden.h \
    -I src \
    -I picoefi/include \
    -isystem freestanding-c-hdrs/include \
    $(CPPFLAGS) \
    -MD \
    -MP

# Internal C++ only preprocessor flags.
override CPPFLAGS_CXX := \
    $(CPPFLAGS_C) \
    -isystem freestanding-cxx-hdrs/include

ifneq ($(filter $(ARCH),ia32 x86_64),)
    # Internal nasm flags that should not be changed by the user.
    override NASMFLAGS := \
        $(patsubst -g,-g -F dwarf,$(NASMFLAGS)) \
        -i src/ \
        -Wall
endif

# Internal linker flags that should not be changed by the user.
override LDFLAGS += \
    -nostdlib \
    -Wl,-pie \
    -Wl,-z,text \
    -Wl,-z,max-page-size=0x1000 \
    -Wl,-z,noexecstack \
    -Wl,--gc-sections \
    -Wl,--build-id=none \
    -Wl,--hash-style=gnu \
    -Wl,-T,picoefi/src/$(ARCH)/link_script.lds

# Architecture specific internal flags, and the macro that compilers define
# for the architecture, which the toolchain helper checks for.
ifeq ($(ARCH),ia32)
    override SHARED_FLAGS += \
        -m32 \
        -march=i686 \
        -mno-mmx \
        -malign-double
    override LDFLAGS += \
        -Wl,-m,elf_i386
    override NASMFLAGS := \
        -f elf32 \
        $(NASMFLAGS)
    override TCH_TARGET_MACRO := __i386__
endif
ifeq ($(ARCH),x86_64)
    override SHARED_FLAGS += \
        -m64 \
        -march=x86-64 \
        -mno-mmx \
        -mno-sse \
        -mno-red-zone
    override LDFLAGS += \
        -Wl,-m,elf_x86_64
    override NASMFLAGS := \
        -f elf64 \
        $(NASMFLAGS)
    override TCH_TARGET_MACRO := __x86_64__
endif
ifeq ($(ARCH),aarch64)
    # The UEFI AArch64 binding reserves x18.
    override SHARED_FLAGS += \
        -mcpu=generic \
        -march=armv8-a+nofp+nosimd \
        -mlittle-endian \
        -mcmodel=small \
        -ffixed-x18
    override LDFLAGS += \
        -Wl,-m,aarch64elf
    override TCH_TARGET_MACRO := __aarch64__
endif
ifeq ($(ARCH),riscv64)
    # The ABI comes first, as the instruction set is checked against it.
    override SHARED_FLAGS += \
        -mabi=lp64
    override LDFLAGS += \
        -Wl,-m,elf64lriscv
    override TCH_TARGET_MACRO := __riscv_xlen=64
endif
ifeq ($(ARCH),loongarch64)
    # -msoft-float, unlike -mfpu=none, also overrides a -mdouble-float or
    # -msingle-float in the user flags, which take effect wherever they appear.
    override SHARED_FLAGS += \
        -mabi=lp64s \
        -march=loongarch64 \
        -msoft-float
    override LDFLAGS += \
        -Wl,-m,elf64loongarch
    override TCH_TARGET_MACRO := __loongarch_grlen=64
endif

# Add the shared flags to the C and C++ flags, now that they are complete.
override CFLAGS += \
    $(SHARED_FLAGS)
override CXXFLAGS += \
    $(SHARED_FLAGS)

# Check the toolchain with the internal flags, which also makes Clang target
# the architecture and link with LLD unless LDFLAGS picks another linker.
ifneq ($(BUILD_GOALS),)
    include gmake-toolchain-helper/check.mk
endif

# Flags that depend on the toolchain, for each compiler, as the two can be
# different toolchains. When nothing is to be built, the helper is not
# included: its macros expand to nothing, and its results are empty.
override CFLAGS += $(call TCH_DRIVER_HAS_COMPILE_FLAG,CC,-fno-stack-clash-protection)
override CXXFLAGS += $(call TCH_DRIVER_HAS_COMPILE_FLAG,CXX,-fno-stack-clash-protection)

# Architecture specific flags that depend on the toolchain.
ifeq ($(ARCH),ia32)
    override CFLAGS += $(call TCH_DRIVER_HAS_COMPILE_FLAG,CC,-fcf-protection=none)
    override CXXFLAGS += $(call TCH_DRIVER_HAS_COMPILE_FLAG,CXX,-fcf-protection=none)
    # GCC needs this flag for interrupt handlers and Clang below 3.9 has
    # no name for it, where only a long double could reach the x87.
    override CFLAGS += $(call TCH_DRIVER_HAS_COMPILE_FLAG,CC,-mno-80387)
    override CXXFLAGS += $(call TCH_DRIVER_HAS_COMPILE_FLAG,CXX,-mno-80387)
    # Clang below 9 takes -malign-double and ignores it, which lays out
    # every firmware structure with a 64 bit member wrong, so check the
    # alignment each compiler gives rather than the flag. G++ has no
    # _Static_assert, so C++ is given static_assert instead.
    ifneq ($(BUILD_GOALS),)
        override CC_ALIGNS_64BIT := $(shell ! printf 'struct s { char c; long long x; };\n_Static_assert(sizeof(struct s) == 16, "");\n' | $(CC) $(CFLAGS) -c -x c - -o /dev/null 2>/dev/null; echo $$?)
        ifneq ($(CC_ALIGNS_64BIT),1)
            $(error The C compiler does not align 64 bit types to 8 bytes)
        endif
        override CXX_ALIGNS_64BIT := $(shell ! printf 'struct s { char c; long long x; };\nstatic_assert(sizeof(struct s) == 16, "");\n' | $(CXX) $(CXXFLAGS) -c -x c++ - -o /dev/null 2>/dev/null; echo $$?)
        ifneq ($(CXX_ALIGNS_64BIT),1)
            $(error The C++ compiler does not align 64 bit types to 8 bytes)
        endif
    endif
endif
ifeq ($(ARCH),x86_64)
    override CFLAGS += $(call TCH_DRIVER_HAS_COMPILE_FLAG,CC,-fcf-protection=none)
    override CXXFLAGS += $(call TCH_DRIVER_HAS_COMPILE_FLAG,CXX,-fcf-protection=none)
    # GCC needs this flag for interrupt handlers and Clang below 3.9 has
    # no name for it, where only a long double could reach the x87.
    override CFLAGS += $(call TCH_DRIVER_HAS_COMPILE_FLAG,CC,-mno-80387)
    override CXXFLAGS += $(call TCH_DRIVER_HAS_COMPILE_FLAG,CXX,-mno-80387)
    # Older GCCs need this for ms_abi calls. Clang has no such flag.
    ifneq ($(TCH_CC_IS_CLANG),1)
        override CFLAGS += \
            -maccumulate-outgoing-args
    endif
    ifneq ($(TCH_CXX_IS_CLANG),1)
        override CXXFLAGS += \
            -maccumulate-outgoing-args
    endif
endif
ifeq ($(ARCH),aarch64)
    override CFLAGS += $(call TCH_DRIVER_HAS_COMPILE_FLAG,CC,-mbranch-protection=none)
    override CXXFLAGS += $(call TCH_DRIVER_HAS_COMPILE_FLAG,CXX,-mbranch-protection=none)
    # Clang does not take this ABI name.
    ifneq ($(TCH_CC_IS_CLANG),1)
        override CFLAGS += $(call TCH_DRIVER_HAS_COMPILE_FLAG,CC,-mabi=lp64)
    endif
    ifneq ($(TCH_CXX_IS_CLANG),1)
        override CXXFLAGS += $(call TCH_DRIVER_HAS_COMPILE_FLAG,CXX,-mabi=lp64)
    endif
    # Only the linker can work around Cortex-A53 erratum 843419, so
    # require one that can.
    override LDFLAGS += \
        $(call TCH_REQUIRE,$(call TCH_LINKER_HAS_FLAG,--fix-cortex-a53-843419),The linker cannot work around Cortex-A53 erratum 843419)
endif
ifeq ($(ARCH),riscv64)
    override CFLAGS += $(call TCH_DRIVER_HAS_COMPILE_FLAG,CC,-fcf-protection=none)
    override CXXFLAGS += $(call TCH_DRIVER_HAS_COMPILE_FLAG,CXX,-fcf-protection=none)
    # Name Zicsr and Zifencei only if the compilers know them, as older
    # ones have them in the base ISA. The whole compile is checked, as the
    # flag is not always named when an instruction set is rejected.
    override CFLAGS += \
        $(or $(call TCH_DRIVER_COMPILES_WITH,CC,-march=rv64imac_zicsr_zifencei),-march=rv64imac)
    override CXXFLAGS += \
        $(or $(call TCH_DRIVER_COMPILES_WITH,CXX,-march=rv64imac_zicsr_zifencei),-march=rv64imac)
    # Clang called this code model "small" before the ISA's own name.
    override CFLAGS += \
        $(or $(call TCH_DRIVER_HAS_COMPILE_FLAG,CC,-mcmodel=medlow,-mcode-model),$(call TCH_DRIVER_HAS_COMPILE_FLAG,CC,-mcmodel=small,-mcode-model))
    override CXXFLAGS += \
        $(or $(call TCH_DRIVER_HAS_COMPILE_FLAG,CXX,-mcmodel=medlow,-mcode-model),$(call TCH_DRIVER_HAS_COMPILE_FLAG,CXX,-mcmodel=small,-mcode-model))
    # The UEFI RISC-V binding forbids relying on unaligned accesses.
    # Compilers without the flag never make them.
    override CFLAGS += $(call TCH_DRIVER_HAS_COMPILE_FLAG,CC,-mstrict-align)
    override CXXFLAGS += $(call TCH_DRIVER_HAS_COMPILE_FLAG,CXX,-mstrict-align)
    # Keep linker relaxation away from gp, which nothing sets up. Linkers
    # without the flag only use gp if the script defines __global_pointer$.
    override LDFLAGS += \
        $(call TCH_LINKER_HAS_FLAG,--no-relax-gp)
endif
ifeq ($(ARCH),loongarch64)
    # Clang 16 takes the soft float flags but still records the double
    # float ABI, which no linker mixes with soft float objects, so read
    # the ABI out of the ELF header each compiler writes. LTO is turned
    # off for the check, as it would write bitcode with no such header.
    ifeq ($(TCH_CC_IS_CLANG),1)
        override CC_FLOAT_ABI := $(shell $(CC) $(CFLAGS) -fno-lto -c -x c /dev/null -o - 2>/dev/null | od -A n -t x1 -j 48 -N 1)
        ifeq ($(filter %1,$(CC_FLOAT_ABI)),)
            $(error The C compiler does not record the soft float ABI)
        endif
    endif
    ifeq ($(TCH_CXX_IS_CLANG),1)
        override CXX_FLOAT_ABI := $(shell $(CXX) $(CXXFLAGS) -fno-lto -c -x c++ /dev/null -o - 2>/dev/null | od -A n -t x1 -j 48 -N 1)
        ifeq ($(filter %1,$(CXX_FLOAT_ABI)),)
            $(error The C++ compiler does not record the soft float ABI)
        endif
    endif
    # Do not go through the GOT for external symbols. Only GCC is checked,
    # as Clang had its flag before the architecture existed.
    ifeq ($(TCH_CC_IS_CLANG),1)
        override CFLAGS += \
            -fdirect-access-external-data
    else
        override CFLAGS += \
            $(call TCH_REQUIRE,$(call TCH_DRIVER_HAS_COMPILE_FLAG,CC,-mdirect-extern-access),The C compiler cannot keep external data out of the GOT)
    endif
    ifeq ($(TCH_CXX_IS_CLANG),1)
        override CXXFLAGS += \
            -fdirect-access-external-data
    else
        override CXXFLAGS += \
            $(call TCH_REQUIRE,$(call TCH_DRIVER_HAS_COMPILE_FLAG,CXX,-mdirect-extern-access),The C++ compiler cannot keep external data out of the GOT)
    endif
    # Clang called this code model "small" before the ISA's own name.
    override CFLAGS += \
        $(or $(call TCH_DRIVER_HAS_COMPILE_FLAG,CC,-mcmodel=normal,-mcode-model),$(call TCH_DRIVER_HAS_COMPILE_FLAG,CC,-mcmodel=small,-mcode-model))
    override CXXFLAGS += \
        $(or $(call TCH_DRIVER_HAS_COMPILE_FLAG,CXX,-mcmodel=normal,-mcode-model),$(call TCH_DRIVER_HAS_COMPILE_FLAG,CXX,-mcmodel=small,-mcode-model))
    # Some Clangs do not record the LoongArch ABI in LTO objects, so pass
    # it to LTO if the IR of an empty file does not record it.
    override CC_LTO_HAS_ABI := $(if $(filter 11,$(TCH_CC_IS_CLANG)$(TCH_CC_LTO)),$(shell ! $(CC) $(CFLAGS) -S -emit-llvm -x c /dev/null -o - 2>/dev/null | grep -q target-abi; echo $$?),1)
    override CXX_LTO_HAS_ABI := $(if $(filter 11,$(TCH_CXX_IS_CLANG)$(TCH_CXX_LTO)),$(shell ! $(CXX) $(CXXFLAGS) -S -emit-llvm -x c++ /dev/null -o - 2>/dev/null | grep -q target-abi; echo $$?),1)

    ifneq ($(CC_LTO_HAS_ABI)$(CXX_LTO_HAS_ABI),11)
        override LDFLAGS += \
            -Wl,-plugin-opt=-target-abi=lp64s
    endif
endif

# Pack relative relocations where the linker can, as RELR tables take a
# fraction of the room. LLD before 15 only knows --pack-dyn-relocs.
ifneq ($(call TCH_LINKER_HAS_Z_FLAG,pack-relative-relocs),)
    override LDFLAGS += \
        -Wl,-z,pack-relative-relocs
else
    ifneq ($(call TCH_LINKER_HAS_FLAG,--pack-dyn-relocs),)
        override LDFLAGS += \
            -Wl,--pack-dyn-relocs=relr
    endif
endif

# Tell the compiler as well if it takes the flag, as it otherwise passes
# its own default on to the linker.
override LDFLAGS += \
    $(call TCH_DRIVER_HAS_LINK_FLAG,-pie)

# Use "find" to glob all *.c, *.cpp, *.S, and *.asm files in the tree
# (except the src/arch/* directories, as those are gonna be added
# in the next step).
override SRCFILES := $(shell find -L src freestanding-lgrt/src picoefi/src/$(ARCH) -type f ! -path 'src/arch/*' 2>/dev/null | LC_ALL=C sort)
# Add architecture specific files, if they exist.
override SRCFILES += $(shell find -L src/arch/$(ARCH) -type f 2>/dev/null | LC_ALL=C sort)
# Obtain the object and header dependencies file names.
override CFILES := $(filter %.c,$(SRCFILES))
override CXXFILES := $(filter %.cpp,$(SRCFILES))
override ASFILES := $(filter %.S,$(SRCFILES))
override OBJ := $(addprefix obj-$(ARCH)/,$(CFILES:.c=.c.o) $(CXXFILES:.cpp=.cpp.o) $(ASFILES:.S=.S.o))
override HEADER_DEPS := $(addprefix obj-$(ARCH)/,$(CFILES:.c=.c.d) $(CXXFILES:.cpp=.cpp.d) $(ASFILES:.S=.S.d))
ifneq ($(filter $(ARCH),ia32 x86_64),)
override NASMFILES := $(filter %.asm,$(SRCFILES))
override OBJ += $(addprefix obj-$(ARCH)/,$(NASMFILES:.asm=.asm.o))
override HEADER_DEPS += $(addprefix obj-$(ARCH)/,$(NASMFILES:.asm=.asm.d))
endif

# Default target. This must come first, before header dependencies.
.PHONY: all
all: bin-$(ARCH)/$(OUTPUT).efi

# Include header dependencies.
-include $(HEADER_DEPS)

# Each kind of file depends on records of the variables it is built with,
# named after them, which the toolchain helper remakes when they change.
override TCH_RECORDS := $(addprefix obj-$(ARCH)/,CC CXX OBJCOPY CFLAGS CXXFLAGS CPPFLAGS LDFLAGS)
ifneq ($(filter $(ARCH),ia32 x86_64),)
override TCH_RECORDS += obj-$(ARCH)/NASMFLAGS
endif
ifneq ($(BUILD_GOALS),)
    include gmake-toolchain-helper/records.mk
endif

# Rule to convert the final ELF executable to a .EFI PE executable.
bin-$(ARCH)/$(OUTPUT).efi: bin-$(ARCH)/$(OUTPUT) GNUmakefile obj-$(ARCH)/OBJCOPY
	mkdir -p "$(dir $@)"
	$(OBJCOPY) -O binary $< $@

# Link rules for the final executable.
bin-$(ARCH)/$(OUTPUT): GNUmakefile obj-$(ARCH)/CXX obj-$(ARCH)/CXXFLAGS obj-$(ARCH)/LDFLAGS picoefi/src/$(ARCH)/link_script.lds $(OBJ)
	mkdir -p "$(dir $@)"
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(OBJ) -o $@

# The compilers may emit calls to the memory functions and the compiler
# runtime that LTO does not account for, so never build those with LTO.
obj-$(ARCH)/src/memory.cpp.o: override CXXFLAGS += -fno-lto
obj-$(ARCH)/freestanding-lgrt/src/freestanding-lgrt.c.o: override CFLAGS += -fno-lto

# Compilation rules for *.c files.
obj-$(ARCH)/%.c.o: %.c GNUmakefile obj-$(ARCH)/CC obj-$(ARCH)/CFLAGS obj-$(ARCH)/CPPFLAGS
	mkdir -p "$(dir $@)"
	$(CC) $(CFLAGS) $(CPPFLAGS_C) -c $< -o $@

# Compilation rules for *.cpp files.
obj-$(ARCH)/%.cpp.o: %.cpp GNUmakefile obj-$(ARCH)/CXX obj-$(ARCH)/CXXFLAGS obj-$(ARCH)/CPPFLAGS
	mkdir -p "$(dir $@)"
	$(CXX) $(CXXFLAGS) $(CPPFLAGS_CXX) -c $< -o $@

# Compilation rules for *.S files.
obj-$(ARCH)/%.S.o: %.S GNUmakefile obj-$(ARCH)/CC obj-$(ARCH)/CFLAGS obj-$(ARCH)/CPPFLAGS
	mkdir -p "$(dir $@)"
	$(CC) $(CFLAGS) $(CPPFLAGS_C) -c $< -o $@

ifneq ($(filter $(ARCH),ia32 x86_64),)
# Compilation rules for *.asm (nasm) files.
obj-$(ARCH)/%.asm.o: %.asm GNUmakefile obj-$(ARCH)/NASMFLAGS
	mkdir -p "$(dir $@)"
	nasm $(NASMFLAGS) -MD $(@:.o=.d) -MP $< -o $@
endif

# Rules to download the UEFI firmware per architecture for testing.
.INTERMEDIATE: edk2-ovmf-bins.tar.gz
edk2-ovmf-bins.tar.gz:
	curl -fL -o $@ https://github.com/osdev0/edk2-ovmf-stable-bins/releases/latest/download/edk2-ovmf-bins.tar.gz

edk2-ovmf-bins: edk2-ovmf-bins.tar.gz
	rm -rf edk2-ovmf-bins
	gunzip < edk2-ovmf-bins.tar.gz | tar -xf -

# Rules for running our executable in QEMU.
.PHONY: run
run: all edk2-ovmf-bins
	mkdir -p boot/EFI/BOOT
	cp bin-$(ARCH)/$(OUTPUT).efi boot/EFI/BOOT/$(EFI_BOOT_FILE)
	qemu-system-$(QEMU_ARCH) \
		-drive file=fat:rw:boot \
		$(QEMUFLAGS) \
		$(QEMU_UEFI_FW)
	rm -rf boot

# Remove object files and the final executable.
.PHONY: clean
clean:
	rm -rf bin-$(ARCH) obj-$(ARCH)

# Remove everything built and generated including downloaded dependencies.
.PHONY: distclean
distclean:
	rm -rf bin-* obj-* .deps-obtained .cache compile_commands.json freestanding-c-hdrs freestanding-cxx-hdrs freestanding-lgrt gmake-toolchain-helper picoefi edk2-ovmf-bins edk2-ovmf-bins.tar.gz

# Install the final built executable to its final on-root location. Not
# install(1), which takes other arguments on illumos and Solaris.
.PHONY: install
install: all
	mkdir -p "$(DESTDIR)$(PREFIX)/share/$(OUTPUT)"
	cp bin-$(ARCH)/$(OUTPUT).efi "$(DESTDIR)$(PREFIX)/share/$(OUTPUT)/$(OUTPUT)-$(ARCH).efi"
	chmod 644 "$(DESTDIR)$(PREFIX)/share/$(OUTPUT)/$(OUTPUT)-$(ARCH).efi"

# Try to undo whatever the "install" target did.
.PHONY: uninstall
uninstall:
	rm -f "$(DESTDIR)$(PREFIX)/share/$(OUTPUT)/$(OUTPUT)-$(ARCH).efi"
	rmdir "$(DESTDIR)$(PREFIX)/share/$(OUTPUT)" 2>/dev/null || :
