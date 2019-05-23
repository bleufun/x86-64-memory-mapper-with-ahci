arch ?= x86_64
kernel := build/kernel-$(arch).bin
iso := build/os-$(arch).iso

linker_script := src/arch/$(arch)/linker.ld
grub_cfg := src/arch/$(arch)/grub.cfg
assembly_source_files := $(wildcard src/arch/$(arch)/*.asm)
cpp_source_files := $(wildcard src/arch/$(arch)/*.cpp)
assembly_object_files := $(patsubst src/arch/$(arch)/%.asm, build/arch/$(arch)/%.o, $(assembly_source_files))
cpp_object_files := $(patsubst src/arch/$(arch)/%.cpp, build/arch/$(arch)/%.o, $(cpp_source_files))

#qemu_hdd := -hda hdd0
qemu_hdd := -drive file=hdd0,format=raw,media=disk,cache=none
machine := -machine type=q35

.PHONY: build clean run iso create-hdd

build: $(kernel)

clean:
	@rm -r build

run: $(iso)
	@qemu-system-x86_64 $(machine) $(qemu_hdd) -cdrom $(iso)

debug: $(iso)
	@qemu-system-x86_64 -vga std $(machine) $(qemu_hdd) -s -S -cdrom $(iso)
create-hdd: $(name)
	# @qemu-system-x86_64 $(name) 32K
	qemu-img create $(name) 32K
iso: $(iso)

$(iso): $(kernel) $(grub_cfg)
	@mkdir -p build/isofiles/boot/grub
	@cp $(kernel) build/isofiles/boot/kernel.bin
	@cp $(grub_cfg) build/isofiles/boot/grub/
	@grub-mkrescue -o $(iso) build/isofiles 2> /dev/null
	@rm -r build/isofiles

$(kernel): $(assembly_object_files) $(cpp_object_files) $(linker_script)
	@ld -n -T $(linker_script) -o $(kernel) $(assembly_object_files) $(cpp_object_files)

build/arch/$(arch)/%.o: src/arch/$(arch)/%.asm
	@echo "Compiling $<"
	@mkdir -p $(shell dirname $@)
	@nasm -felf64 $< -o $@

build/arch/$(arch)/%.o : src/arch/$(arch)/%.cpp
	@echo "Compiling $<"
	@mkdir -p $(shell dirname $@)
	@g++ -g -c -O2 -m64 -fno-asynchronous-unwind-tables -fno-stack-protector -fno-pic -fno-dwarf2-cfi-asm -masm=intel $< -o $@

build/arch/$(arch)/init.o: src/arch/$(arch)/init.cpp
	@echo "Compiling $<"
	@mkdir -p $(shell dirname $@)
	@g++ -g -S -O2 -m32 -fno-asynchronous-unwind-tables -fno-stack-protector -fno-dwarf2-cfi-asm -fno-pic -masm=intel $< -o build/arch/$(arch)/init.asm.64
	@as -O2 -g build/arch/$(arch)/init.asm.64 --64 -o $@

build/arch/$(arch)/interrupts.o: src/arch/$(arch)/interrupts.cpp
	@echo "Compiling $<"
	@mkdir -p $(shell dirname $@)
	@g++ -g -c -O2 -m64 -fno-asynchronous-unwind-tables -mgeneral-regs-only -fno-stack-protector -fno-dwarf2-cfi-asm -fno-pic -masm=intel $< -o build/arch/$(arch)/interrupts.o
