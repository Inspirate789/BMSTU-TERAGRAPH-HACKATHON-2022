/*
 * gpc_io_host.h
 *
 * host library (XRT runtime version)
 *
 *  Created on: April 23, 2021
 *      Author: A.Popov
 */

#ifndef MQ_H_
#define MQ_H_

#include <chrono>
#include <thread>
#include <mutex>
#include <unistd.h>
#include "gpc_xrt_host.h"


//====================================
// Адресация ресурсов в Global Memory
//====================================
#define MSG_BLOCK_START 0x00010000                  			// area for IO

// Check parameters to avoid global memory conflict
// MSG_BLOCK_SIZE * (GPC cores in GROUP, 6 is default) < 64K
#define MSG_BLOCK_SIZE  0x2800                                  // 10K Block size for every GPC
#define MSG_BUFFER_SIZE 0x1000                                  // 4096 bytes buffers for GPC IO

// Automatically calculated parameters
#define HOST2GPC_BUFFER 0                     			// Offset for Host2GPC queue
#define GPC2HOST_BUFFER (MSG_BUFFER_SIZE)     			// Offset for GPC2Host queue


//=============================================
// Класс для взаимодействия с GPC через очереди
//=============================================

class gpc_io : public gpc
{
private:
  // host2gpn queue credits
  unsigned int host2gpc_credit;
  // Buffers offsets
  unsigned int gpc2host_buffer;
  unsigned int host2gpc_buffer;
  // Threads and alive flags
  std::thread *mq_send_th;
  std::mutex mq_send_th_alive;
  std::thread *mq_receive_th;
  std::mutex mq_receive_th_alive;
  std::thread *buf_write_th;
  std::mutex buf_write_th_alive;
  std::thread *buf_read_th;
  std::mutex buf_read_th_alive;
  // Send message to GPC queues
  void mq_send_thread(unsigned int message);
  void mq_send_buf_thread(unsigned int size, unsigned int *buffer);
  // Wait and read single message from GPC queue
  unsigned int mq_receive_thread();
  void mq_receive_buf_thread(unsigned int size, unsigned int *buffer);
  // Write data to HOST2GPC buffer
  void buf_write_thread(unsigned int size, char *buffer);
  // Read data from GPC2HOST buffer
  void buf_read_thread(unsigned int size, char *buffer);
public:
  // Constructor
  gpc_io(xrt::device *xrt_device,xrt::uuid *xrt_uuid,unsigned int group_number,unsigned int core_number);
  // Destructor
  ~gpc_io();
  void load_sw_kernel(char *gpnbin,xrt::bo *global_memory_ptr, xrt::bo *ddr_memory_ptr);
  char* external_memory_create_buffer(unsigned int size);
  void external_memory_sync_to_device(unsigned int offset, unsigned int size);
  void external_memory_sync_from_device(unsigned int offset, unsigned int size);
  long long external_memory_address();
  // Send message to GPC queues
  void mq_send(unsigned int message);
  void mq_send(unsigned int size, unsigned int *buffer);
  void mq_send_join();
  // Wait and read single message from GPC queue
  unsigned int mq_receive();
  void mq_receive(unsigned int size, unsigned int *buffer);
  void mq_receive_join();
  // Write data to HOST2GPC buffer
  void buf_write(unsigned int size, char *buffer);
  void buf_write_join();
  // Read data from GPC2HOST buffer
  void buf_read(unsigned int size, char *buffer);
  void buf_read_join();
    // Sync point with GPC
  void sync_with_gpc();

protected:

};

#endif
