#include <iostream>
#include <stdio.h>
#include <stdexcept>
#include <iomanip>
#ifdef _WINDOWS
#include <io.h>
#else
#include <unistd.h>
#include <sys/time.h>
#endif


#include "experimental/xrt_device.h"
#include "experimental/xrt_kernel.h"
#include "experimental/xrt_bo.h"
#include "experimental/xrt_ini.h"

#include "gpc_defs.h"
#include "leonhardx64_xrt.h"
#include "gpc_handlers.h"

//Grid size
#define GRAPH_SIZE_X 20
#define GRAPH_SIZE_Y 20
#define BIFFER_SIZE ( (GRAPH_SIZE_X-2)*(GRAPH_SIZE_Y-2)*8 + (2*(GRAPH_SIZE_X-2)+2*(GRAPH_SIZE_Y-2))*5 + 4*3 )*3*sizeof(int)
static int offs = 0;
#define EDGE(u,v,w) \
	host2gpc_ext_buffer[group][core][offs]=u; offs++; \
	host2gpc_ext_buffer[group][core][offs]=v; offs++; \
	host2gpc_ext_buffer[group][core][offs]=w; offs++;



extern "C" {
	leonhardx64* Leonhard(			char* 			xclbin )
	{
		return new leonhardx64(0,xclbin);
	}
	void del_Leonhard(	leonhardx64* 	Leonhard)
	{
		Leonhard->~leonhardx64();
	}
	void Leonhard_load_sw_kernel(	leonhardx64* 	Leonhard,
									char* 			sw_kernel,
									unsigned int 	group,
									unsigned int 	core )
	{
		Leonhard->load_sw_kernel(sw_kernel, group, core);
	}
	void Leonhard_start_async(		leonhardx64* 	Leonhard,
									unsigned int 	group,
									unsigned int 	core,
									unsigned int handler )
	{
		Leonhard->gpc[group][core]->start_async(handler);
	}
	void Leonhard_start_sync(		leonhardx64* 	Leonhard,
									unsigned int 	group,
									unsigned int 	core,
									unsigned int handler )
	{
		Leonhard->gpc[group][core]->start_sync(handler);
	}
	void Leonhard_sync_with_gpc(	leonhardx64* 	Leonhard,
									unsigned int 	group,
									unsigned int 	core )
	{
		Leonhard->gpc[group][core]->sync_with_gpc();
	}
	unsigned int  Leonhard_mq_receive(	leonhardx64* 	Leonhard,
									unsigned int 	group,
									unsigned int 	core )
	{
		return Leonhard->gpc[group][core]->mq_receive();
	}
	void Leonhard_mq_send(			leonhardx64* 	Leonhard,
									unsigned int 	group,
									unsigned int 	core,
									unsigned int 	message
									)
	{
		Leonhard->gpc[group][core]->mq_send(message);
	}

	void Leonhard_finish(			leonhardx64* 	Leonhard,
									unsigned int 	group,
									unsigned int 	core )
	{
		Leonhard->gpc[group][core]->finish();
	}

	unsigned int* Leonhard_external_memory_create_buffer(
									leonhardx64* 	Leonhard,
									unsigned int 	group,
									unsigned int 	core,
									unsigned int 	size )
	{
		return (unsigned int *)Leonhard->gpc[group][core]->external_memory_create_buffer(size);
	}

	void Leonhard_external_memory_sync_to_device(
									leonhardx64* 	Leonhard,
									unsigned int 	group,
									unsigned int 	core,
									unsigned int offset,
									unsigned int size )
	{
		Leonhard->gpc[group][core]->external_memory_sync_to_device(offset,size);
	}

	void Leonhard_external_memory_sync_from_device(
									leonhardx64* 	Leonhard,
									unsigned int 	group,
									unsigned int 	core,
									unsigned int offset,
									unsigned int size )
	{
		Leonhard->gpc[group][core]->external_memory_sync_from_device(offset,size);
	}
}
