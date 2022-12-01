#Platform Details
BASE_MEMORY_ADDR = 0x80000000
AXI4BRAM_BASE_ADDR = 0xA0000000
MEM_SIZE_KB = 64
STACK_SIZE_KB = 2

#Sizes coverted into bytes
MEM_SIZE_B = $$(($(MEM_SIZE_KB)*1024))
STACK_SIZE_B = $$(($(STACK_SIZE_KB)*1024))

BOARD_CFLAGS = -DAXI4BRAM_BASE_ADDR=$(AXI4BRAM_BASE_ADDR) 

#picolibc.specs includes -ffunction-sections and linker garbage collection
CFLAGS += $(BOARD_CFLAGS) -march=rv32im -mabi=ilp32 -O2 -fno-inline -DDHRYSTONE_ITERATIONS=100000 -ffunction-sections -fdata-sections --specs=picolibc.specs 
CXXFLAGS += $(BOARD_CFLAGS) -march=rv32im -mabi=ilp32 -O2 -fno-inline -DDHRYSTONE_ITERATIONS=100000 -ffunction-sections -fdata-sections --specs=picolibc.specs -fno-rtti -fno-exceptions -std=gnu++20
LDFLAGS = -Wl,--print-memory-usage,-gc-sections -Xlinker --defsym=__executable_start=$(BASE_MEMORY_ADDR) -Xlinker --defsym=__mem_addr=$(BASE_MEMORY_ADDR) -Xlinker --defsym=__mem_size=$(MEM_SIZE_B) -Xlinker  --defsym=__stack_size=$(STACK_SIZE_B)

LINK_FILE := link/link.ld #???

CC = riscv32-unknown-elf-g++
AR = riscv32-unknown-elf-ar
ELF_TO_HW_INIT = python3 $(TAIGA_DIR)/tools/taiga_binary_converter.py riscv32-unknown-elf- $(BASE_MEMORY_ADDR) $(MEM_SIZE_B)
OBJDUMP = riscv32-unknown-elf-objdump
OBJCOPY = riscv32-unknown-elf-objcopy
RM = rm -f
RMDIR = rm -rf
