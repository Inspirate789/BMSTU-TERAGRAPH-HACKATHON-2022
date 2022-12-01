#
# this file was not created by a computer. do not trust it.
#

.PHONY: all clean

all: gpc_test Leonhardx64_xrt leonhard_2cores_267mhz.xclbin

gpc_test: 
	make -C sw-kernel-lib
	make -C sw-kernel

Leonhardx64_xrt: 
	make -C host-lib
	make -C host

leonhard_2cores_267mhz.xclbin:
	curl --output leonhard_2cores_267mhz.xclbin https://gitlab.com/leonhard-x64-xrt-v2/leonhard-2cores-267mhz/-/package_files/50993121/download

clean:
	make -C sw-kernel clean
	make -C host clean
	make -C sw-kernel-lib clean
	make -C host-lib clean
	rm leonhard_2cores_267mhz.xclbin
