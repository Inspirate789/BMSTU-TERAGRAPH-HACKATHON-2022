#
# this file was created by a computer. trust it.
#

# compiler tools
XILINX_VITIS ?= /data/Xilinx/Vitis/2020.2
XILINX_XRT ?= /opt/xilinx/xrt

HOST_CXX ?= g++
RM = rm -f
RMDIR = rm -rf

VITIS_PLATFORM = xilinx_u200_xdma_201830_2
VITIS_PLATFORM_PATH = $(VITIS_PLATFORM)
VITIS_INCLUDE_FILES = $(XILINX_XRT)/include/experimental/xrt_device.h $(XILINX_XRT)/include/experimental/xrt_kernel.h $(XILINX_XRT)/include/experimental/xrt_bo.h $(XILINX_XRT)/include/experimental/xrt_ini.h

# host compiler global settings
CXXFLAGS += -std=c++17 -DVITIS_PLATFORM=$(VITIS_PLATFORM) -D__USE_XOPEN2K8 -I$(XILINX_XRT)/include -I$(XILINX_XRT)/lib -I/data/Xilinx/Vivado/2020.2/include/ -O0 -g -Wall -c -fmessage-length=0 -std=c++14
LDFLAGS += -lxilinxopencl -lxrt_coreutil -lpthread -lrt -lstdc++ -L$(XILINX_XRT)/lib -L$(XILINX_XRT)/include/ -Wl,-rpath-link,$(XILINX_XRT)/lib

###############################
# Work directories and files
###############################
SRC_DIR   := $(PWD)
LIB_DIR   := src
OBJ_DIR   := $(PWD)/obj

#
# host files
#

SRCS := $(wildcard $(SRC_DIR)/*.cpp) $(wildcard $(LIB_DIR)/*.cpp)

SRCS_NO_PATH := $(notdir $(SRCS))
HOST_OBJECTS += $(SRCS_NO_PATH:%.cpp=$(OBJ_DIR)/%.o)

HOST_EXE = $(PWD)/host_main

BUILD_SUBDIRS += src/
BUILD_SUBDIRS += lib/

CXXFLAGS += -I$(SRC_DIR)
CXXFLAGS += -I$(LIB_DIR)

#
# primary build targets
#

.PHONY: all clean
all:  $(HOST_EXE)

clean:
	-$(RM) $(HOST_EXE) $(HOST_OBJECTS)

.PHONY: incremental
incremental: all

nothing:

test:
	echo $(HOST_OBJECTS)

#
# host rules
#

#Compile src
$(OBJ_DIR)/%.o : $(SRC_DIR)/%.cpp $(VITIS_INCLUDE_FILES) $(LIB_DIR)/gpc_defs.h $(LIB_DIR)/leonhardx64_xrt.h
	-@mkdir -p $(@D)
	$(HOST_CXX) $(CXXFLAGS) -o "$@" "$<"

#Compile lib
$(OBJ_DIR)/%.o : $(LIB_DIR)/%.cpp $(LIB_DIR)/%.h
	-@mkdir -p $(@D)
	$(HOST_CXX) $(CXXFLAGS) -o "$@" "$<"

#Compile exe
$(HOST_EXE): $(HOST_OBJECTS)
	$(HOST_CXX) -o "$@" $(+) $(LDFLAGS)

