/*
 * leonhardx64.cpp
 *
 * host library (XRT runtime version)
 *
 *  Created on: April 23, 2021
 *      Author: A.Popov
 */
#include "leonhardx64_xrt.h"

// Constructor
leonhardx64::leonhardx64(
		unsigned int 		dev_index,
		char 				*xclbin
) {
	// Open device
	xrt_device = new xrt::device(dev_index);
	// Program or connect to the hw kernel
	xrt_uuid = new xrt::uuid(xrt_device->load_xclbin(xclbin));
	// Create GPC objects
	xrt::bo::flags flags = xrt::bo::flags::normal;

	__foreach_core(group, core)
	{
		gpc[group][core] = new gpc_io(xrt_device,xrt_uuid,group,core);

	}
	__foreach_group(group) {
		//Create buffers
		ddr4_memory[group] = new xrt::bo(*xrt_device, 4096, flags, gpc[group][LNH_CORES_LOW[0]]->kernel->group_id(3));
		global_memory[group] = new xrt::bo(*xrt_device, GLOBAL_MEMORY_MAX_LENGTH*sizeof(int), flags, gpc[group][LNH_CORES_LOW[0]]->kernel->group_id(4));
	}
}

void leonhardx64::load_sw_kernel(char *gpnbin, unsigned int group, unsigned int core) {
	// GPC queue initialization
	gpc[group][core]->load_sw_kernel(gpnbin,global_memory[group],ddr4_memory[group]);
}

// Destructor
leonhardx64::~leonhardx64() {
	device_reset();
	__foreach_core(group, core) {
		free(gpc[group][core]);
	}
	__foreach_group(group) {
		free(ddr4_memory[group]);
		free(global_memory[group]);
	}
	free(xrt_uuid);
	free(xrt_device);
}

//Reset device
void leonhardx64::device_reset() {

}
