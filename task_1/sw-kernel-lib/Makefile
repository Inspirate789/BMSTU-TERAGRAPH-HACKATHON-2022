#set target binary name, memory size and CFLAGS
#the makefile will build all c files in the src directory

#Output binary name
TARGET = $(PWD)/sw_kernel_main

#Platform Details
BASE_MEMORY_ADDR = 0x80000000
AXI4BRAM_BASE_ADDR = 0xA0000000
MEM_SIZE_KB = 64
STACK_SIZE_KB = 2

#Sizes coverted into bytes
MEM_SIZE_B = $$(($(MEM_SIZE_KB)*1024))
STACK_SIZE_B = $$(($(STACK_SIZE_KB)*1024))

BOARD_CFLAGS = -DAXI4BRAM_BASE_ADDR=$(AXI4BRAM_BASE_ADDR) -Xlinker -Map=$(TARGET).map -Xlinker --defsym=__executable_start=$(BASE_MEMORY_ADDR) -Xlinker --defsym=__mem_addr=$(BASE_MEMORY_ADDR) -Xlinker --defsym=__mem_size=$(MEM_SIZE_B) -Xlinker  --defsym=__stack_size=$(STACK_SIZE_B)

#picolibc.specs includes -ffunction-sections and linker garbage collection
CFLAGS = $(BOARD_CFLAGS) -march=rv32im -mabi=ilp32 -O2 -fno-inline -DDHRYSTONE_ITERATIONS=100000 -ffunction-sections -fdata-sections --specs=picolibc.specs 
# -Ttext=$(BASE_MEMORY_ADDR)
#
LDFLAGS = -Wl,--print-memory-usage,-gc-sections 

###############################
# Work directories and files
###############################
SRC_DIR   := $(PWD)
LIB_DIR   := lib
OBJ_DIR   := $(PWD)/obj
LINK_FILE := link/link.ld

###############################
#Don't Change past this point
###############################
CC = riscv32-unknown-elf-g++
ELF_TO_HW_INIT = python3 $(TAIGA_DIR)/tools/taiga_binary_converter.py riscv32-unknown-elf- $(BASE_MEMORY_ADDR) $(MEM_SIZE_B)
OBJDUMP = riscv32-unknown-elf-objdump
SRCS := $(wildcard $(SRC_DIR)/*.c) $(wildcard $(LIB_DIR)/*.c)
DEPDIR := $(OBJ_DIR)/.deps
DEPFLAGS = -MT $@ -MMD -MP -MF $(DEPDIR)/$*.d

SRCS_NO_PATH := $(notdir $(SRCS))
OBJECTS := $(SRCS_NO_PATH:%.c=$(OBJ_DIR)/%.o)

all: $(TARGET).hw_init 
	
#Produce ram init files
$(TARGET).hw_init : $(TARGET)
	$(OBJDUMP) -d $< > $<.dump
	$(ELF_TO_HW_INIT) $< $@ $<.sim_init

#Link
$(TARGET): $(OBJECTS)
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS) 
	
#Compile src
$(OBJ_DIR)/%.o : $(SRC_DIR)/%.c $(DEPDIR)/%.d | $(DEPDIR)
	$(CC) $(DEPFLAGS) $(CFLAGS) -I$(LIB_DIR) -c -o $@ $<

#Compile lib
$(OBJ_DIR)/%.o : $(LIB_DIR)/%.c $(DEPDIR)/%.d | $(DEPDIR)
	$(CC) $(DEPFLAGS) $(CFLAGS) -I$(LIB_DIR) -c -o $@ $<

$(DEPDIR): ; @mkdir -p $@

DEPFILES := $(SRCS_NO_PATH:%.c=$(DEPDIR)/%.d)
$(DEPFILES):
include $(wildcard $(DEPFILES))

.PHONY: clean all
clean:
	rm -rf $(OBJ_DIR) $(TARGET) $(TARGET).dissasembled $(TARGET).dump $(TARGET).hw_init $(TARGET).map $(TARGET).rawbinary $(TARGET).sim_init $(TARGET).log
