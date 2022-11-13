/*
 * gpc_io_host.cpp
 *
 * host library (XRT runtime version)
 *
 *  Created on: April 23, 2022
 *      Author: A.Popov
 */
#include "gpc_io_xrt_host.h"

// Read queue status from global memory

// Constructor
gpc_io	::	gpc_io(xrt::device *xrt_device,xrt::uuid *xrt_uuid,unsigned int group_number,unsigned int core_number)
		: 	gpc (xrt_device,xrt_uuid,group_number,core_number)
{
	// Init queues credits
	host2gpc_credit = 0;
	// Init buffers offsets
	host2gpc_buffer = MSG_BLOCK_START + core * MSG_BLOCK_SIZE + HOST2GPC_BUFFER;
	gpc2host_buffer = MSG_BLOCK_START + core * MSG_BLOCK_SIZE + GPC2HOST_BUFFER;
	// Map IO user space buffers to global memory
	xrt::bo::flags flags = xrt::bo::flags::normal;
	external_memory_pointer = new xrt::bo(*device, 4096, flags, kernel->group_id(5));   // external memory used for a gpc external IO
	external_memory_map = external_memory_pointer->map<char*>();
}

// Constructor
gpc_io::~gpc_io() {
	free(mq_send_th);
	free(buf_write_th);
	free(mq_receive_th);
	free(buf_read_th);
	free(external_memory_pointer);
}
//Thread version

// Load RV kernel binary to the GPN

void gpc_io::load_sw_kernel(char *gpnbin,xrt::bo *global_memory_ptr,xrt::bo *ddr_memory_ptr){
	ddr_memory_pointer = ddr_memory_ptr;
	global_memory_pointer = global_memory_ptr;
	gpc::load_sw_kernel(gpnbin);
}

char* gpc_io::external_memory_create_buffer(unsigned int size){
	// Map IO user space buffers to external memory
	free(external_memory_pointer);
	xrt::bo::flags flags = xrt::bo::flags::normal;
	external_memory_pointer = new xrt::bo(*device, size, flags, kernel->group_id(5));   // external memory used for a gpc external IO
	external_memory_map = external_memory_pointer->map<char*>();
	run.set_arg(5,*external_memory_pointer);
	return external_memory_map;
}

// Send message to gpc
void gpc_io::mq_send_thread(unsigned int message)
{
	using namespace std::chrono_literals;
	if (++host2gpc_credit > MQ_CREDITS) {
		while ((kernel->read_register(MQ_SR) & HOST2GPC_MQ_AFULL) == HOST2GPC_MQ_AFULL) {
			std::this_thread::sleep_for(10us);
		}
		host2gpc_credit = 0;
	}
	kernel->write_register(HOST2GPC_MQ, message);
}


// Send multiple messages to gpc
void gpc_io::mq_send_buf_thread(unsigned int size, unsigned int *buffer)
{
	for (unsigned int i=0; i<size; i+=4) {
		mq_send_thread(*(unsigned int*)((char*)buffer + i));
	}
}

// Wait and read single message from gpc
unsigned int gpc_io::mq_receive_thread()
{
	using namespace std::chrono_literals;
	volatile unsigned int mq_st = kernel->read_register(MQ_SR);
	while ((mq_st & GPC2HOST_MQ_EMPTY) == GPC2HOST_MQ_EMPTY) {
			std::this_thread::sleep_for(10us);
			mq_st = kernel->read_register(MQ_SR);
	}
	return kernel->read_register(GPC2HOST_MQ);
}

// Wait and read multiple messages from gpc
void gpc_io::mq_receive_buf_thread(unsigned int size, unsigned int *buffer)
{
	for (unsigned int i=0; i<size; i+=4) {
		*(unsigned int*)((char*)buffer + i) = mq_receive_thread();
	}
}

// Write data to HOST2GPC buffer
void gpc_io::buf_write_thread(unsigned int size, char *buffer) {
	for (unsigned int i = 0; i < size; i+=4)
		*(volatile unsigned int*)(global_memory_map + host2gpc_buffer + i) = *(unsigned int*)(buffer + i);
	global_memory_pointer->sync(XCL_BO_SYNC_BO_TO_DEVICE, size, host2gpc_buffer);


}

// Read data from GPC2HOST buffer
void gpc_io::buf_read_thread(unsigned int size, char *buffer) {
	global_memory_pointer->sync(XCL_BO_SYNC_BO_FROM_DEVICE , size, gpc2host_buffer);
	for (unsigned int i = 0; i < size; i+=4)
		*(unsigned int*)(buffer + i) = *(volatile unsigned int*)(global_memory_map + gpc2host_buffer + i);
}

// Send single message to gpc
void gpc_io::mq_send(unsigned int message)
{
	mq_send_th_alive.lock();
	mq_send_thread(message);
	mq_send_th_alive.unlock();
}

// Send multiple messages to gpc asynchronously
void gpc_io::mq_send(unsigned int size, unsigned int *buffer)
{
	mq_send_th_alive.lock();
    mq_send_th = new std::thread(&gpc_io::mq_send_buf_thread, this,  size, buffer);
	mq_send_th_alive.unlock();
}

// Wait for the
void gpc_io::mq_send_join()
{
	mq_send_th->join();
	free(mq_send_th);

}

// Wait and read single message from gpc
unsigned int gpc_io::mq_receive()
{
	mq_receive_th_alive.lock();
	unsigned int message  = mq_receive_thread();
	mq_receive_th_alive.unlock();
	return message;
}

void gpc_io::mq_receive(unsigned int size, unsigned int *buffer)
{
	mq_receive_th_alive.lock();
    mq_receive_th = new std::thread(&gpc_io::mq_receive_buf_thread, this,  size, buffer);
	mq_receive_th_alive.unlock();
}

void gpc_io::mq_receive_join()
{
	mq_receive_th->join();
	free(mq_receive_th);
}

// Write data to HOST2GPC buffer
void gpc_io::buf_write(unsigned int size, char *buffer)
{
	buf_write_th_alive.lock();
    buf_write_th = new std::thread(&gpc_io::buf_write_thread, this,  size, buffer);
	buf_write_th_alive.unlock();
}

void gpc_io::buf_write_join()
{
	buf_write_th->join();
	free(buf_write_th);
}

// Read data from GPC2HOST buffer
void gpc_io::buf_read(unsigned int size, char *buffer)
{
	buf_read_th_alive.lock();
    buf_read_th = new std::thread(&gpc_io::buf_read_thread, this,  size, buffer);
	buf_read_th_alive.unlock();
}

void gpc_io::buf_read_join()
{
	buf_read_th->join();
	free(buf_read_th);

}

void gpc_io::external_memory_sync_to_device(unsigned int offset, unsigned int size)
{
	external_memory_pointer->sync(XCL_BO_SYNC_BO_TO_DEVICE, size, offset);
}

void gpc_io::external_memory_sync_from_device(unsigned int offset, unsigned int size)
{
	external_memory_pointer->sync(XCL_BO_SYNC_BO_FROM_DEVICE, size, offset);
}

long long gpc_io::external_memory_address()
{
	return external_memory_pointer->address();
}


// Syncronization point with gpc
void gpc_io::sync_with_gpc()
{
	mq_send(0);
	mq_receive();
}
