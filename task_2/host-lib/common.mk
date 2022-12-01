# compiler tools
XILINX_VITIS ?= /data/Xilinx/Vitis/2020.2
XILINX_XRT ?= /opt/xilinx/xrt

CC = g++
AR = ar
RM = rm -f
RMDIR = rm -rf

VITIS_PLATFORM = xilinx_u200_xdma_201830_2
VITIS_PLATFORM_PATH = $(VITIS_PLATFORM)
VITIS_INCLUDE_FILES = $(XILINX_XRT)/include/experimental/xrt_device.h $(XILINX_XRT)/include/experimental/xrt_kernel.h $(XILINX_XRT)/include/experimental/xrt_bo.h $(XILINX_XRT)/include/experimental/xrt_ini.h

# host compiler global settings
CXXFLAGS += -std=c++17 -DVITIS_PLATFORM=$(VITIS_PLATFORM) -D__USE_XOPEN2K8 -I$(XILINX_XRT)/include -I$(XILINX_XRT)/lib -I/data/Xilinx/Vivado/2020.2/include/ -O0 -g -Wall -c -fmessage-length=0 -std=c++14
CFLAGS += -DVITIS_PLATFORM=$(VITIS_PLATFORM) -D__USE_XOPEN2K8 -I$(XILINX_XRT)/include -I$(XILINX_XRT)/lib -I/data/Xilinx/Vivado/2020.2/include/ -O0 -g -Wall -fmessage-length=0 
LDFLAGS += -lxilinxopencl -lxrt_coreutil -lpthread -lrt -lstdc++ -L$(XILINX_XRT)/lib -L$(XILINX_XRT)/include/ -Wl,-rpath-link,$(XILINX_XRT)/lib
