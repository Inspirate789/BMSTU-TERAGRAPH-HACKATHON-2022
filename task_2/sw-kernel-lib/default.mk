ifndef SW_KERNEL_LIB_DIR
	$(error "В Makefile необходимо объявить переменную SW_KERNEL_LIB_DIR, содержащую путь к каталогу sw-kernel-lib")
endif

ifndef TARGET
	$(error "В Makefile необходимо объявить переменную TARGET, содержащую имя результирующего файла")
endif

include $(SW_KERNEL_LIB_DIR)/common.mk

TARGET_DIR = $(sort $(dir $(TARGET))) #Сортировка удаляет дубликаты
SRC_DIR   = src
OBJ_DIR   = obj
CXXFLAGS += -Iinclude -I$(SW_KERNEL_LIB_DIR)/include $(AUX_INCLUDE) 
CFLAGS += -Iinclude -I$(SW_KERNEL_LIB_DIR)/include $(AUX_INCLUDE) 
LDFLAGS += -L$(SW_KERNEL_LIB_DIR)/lib/ -lsw-kernel

SRCS_C = $(wildcard $(SRC_DIR)/*.c)
SRCS_CXX = $(wildcard $(SRC_DIR)/*.cpp)
SRCS_C_NO_PATH = $(notdir $(SRCS_C))
SRCS_CXX_NO_PATH = $(notdir $(SRCS_CXX))
OBJECTS = $(SRCS_CXX_NO_PATH:%.cpp=$(OBJ_DIR)/%.cpp.o) $(SRCS_C_NO_PATH:%.c=$(OBJ_DIR)/%.c.o)

all:  $(TARGET)
#Produce ram init files
%.rawbinary : %.elf 
	$(OBJCOPY) --gap-fill=0x00 -O binary $< $@
#Link
%.elf: $(OBJECTS) | $(TARGET_DIR)
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS) -Xlinker -Map=$@.map
%.dump : %.elf
	$(OBJDUMP) -d $< > $@
%.a: $(OBJECTS) | $(TARGET_DIR) 
	$(AR) rcs $@ $^

DEPDIR = $(OBJ_DIR)/.deps
C_DEPFLAGS = -MT $@ -MMD -MP -MF $(DEPDIR)/$*.c.d
CXX_DEPFLAGS = -MT $@ -MMD -MP -MF $(DEPDIR)/$*.cpp.d

#Compile C++ src
$(OBJ_DIR)/%.cpp.o : $(SRC_DIR)/%.cpp $(DEPDIR)/%.cpp.d | $(DEPDIR)
	$(CC) $(CXX_DEPFLAGS) $(CXXFLAGS) -c -o $@ $<

#Compile C src
$(OBJ_DIR)/%.c.o : $(SRC_DIR)/%.c $(DEPDIR)/%.c.d | $(DEPDIR)
	$(CC) $(C_DEPFLAGS) $(CFLAGS) -c -o $@ $<

$(DEPDIR): ; @mkdir -p $@
$(TARGET_DIR): ; @mkdir -p $@

DEPFILES = $(SRCS_CXX_NO_PATH:%.cpp=$(DEPDIR)/%.cpp.d) $(SRCS_C_NO_PATH:%.c=$(DEPDIR)/%.c.d)
$(DEPFILES):
include $(wildcard $(DEPFILES))

.PHONY: clean all
clean:
	$(RMDIR) $(OBJ_DIR)
	$(RM) $(TARGET) *.map
ifneq ($(strip $(TARGET_DIR)), ./)
	$(RMDIR) $(TARGET_DIR)
endif
