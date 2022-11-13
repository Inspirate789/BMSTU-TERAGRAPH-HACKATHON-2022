#
# this file was not created by a computer. do not trust it.
#

.PHONY: all clean

all: sw_kernel host leonhard_2cores_267mhz.xclbin

sw_kernel:
	make -C sw-kernel-lib

host:
	make -C host-lib

leonhard_2cores_267mhz.xclbin:
	curl --output leonhard_2cores_267mhz.xclbin https://gitlab.com/leonhard-x64-xrt-v2/leonhard-2cores-267mhz/-/package_files/50993121/download

clean:
	make -C sw-kernel-lib clean
	make -C host-lib clean
	rm leonhard_2cores_267mhz.xclbin
