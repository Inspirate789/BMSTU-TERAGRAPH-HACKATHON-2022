/*
 * gpc_host.cpp
 *
 * host library (XRT runtime version)
 *
 *  Created on: April 23, 2021
 *      Author: A.Popov
 */

#include "gpc_xrt_host.h"


// Constructor
gpc::gpc(xrt::device *xrt_device,xrt::uuid *xrt_uuid,unsigned int group_number,unsigned int core_number)
{
	group=group_number;
	core=core_number;
	device=xrt_device;
	uuid=xrt_uuid;
	kernel = new xrt::kernel(*device, *uuid, LNH_CORE_DEFS.KERNEL_NAME[group][core], xrt::kernel::cu_access_mode::exclusive);
};

gpc::~gpc() {
	free(kernel);
}

unsigned int gpc::load_file_to_memory(const char *filename, char **result)
{
	unsigned int size = 0;
	FILE *f = fopen(filename, "rb");
	if (f == NULL)
	{
		*result = NULL;
		return -1; // -1 means file opening fail
	}
	fseek(f, 0, SEEK_END);
	size = ftell(f);
	fseek(f, 0, SEEK_SET);
	if (size != fread(*result, sizeof(char), size, f))
	{
		free(*result);
		return -2; // -2 means file reading fail
	}
	fclose(f);
	(*result)[size] = 0;
	return size;
}


// Load RV kernel binary to the GPN
void gpc::load_sw_kernel(char *gpcbin)
{
	//Set buffers
	global_memory_map = global_memory_pointer->map<char*>();
	external_memory_map = external_memory_pointer->map<char*>();
	//Write sw_kernel to global memory
	unsigned int n_gpc0 = load_file_to_memory(gpcbin, (char **)&global_memory_map);
	global_memory_pointer->sync(XCL_BO_SYNC_BO_TO_DEVICE, n_gpc0, 0);
	//Create RUN context, Reset CPE and start bootloader
	run = kernel->operator()(LNH_CORE_DEFS.LEONHARD_CONFIG[group][core],GPC_RESET_HIGH,n_gpc0,*ddr_memory_pointer,*global_memory_pointer,*external_memory_pointer);
	run.wait();
	// Free memory
	// Enable handlers execution
	run.set_arg(1,GPC_RESET_LOW);
}


void gpc::start_sync(const unsigned int event_handler)
{
	//Set args for RUN context
	run.set_arg(2,event_handler);
	run.start();
	run.wait();
}

void gpc::start_async(const unsigned int event_handler)
{
	//Check if previous run is finished
	run.wait();
	//Set args for RUN context
	run.set_arg(2,event_handler);
	run.start();
}

void gpc::finish()
{
	run.wait();
}



