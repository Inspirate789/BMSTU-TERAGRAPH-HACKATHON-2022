/*
 * gpc_io_swk.h
 *
 * sw_kernel library
 *
 *  Created on: April 23, 2021
 *      Author: A.Popov
 */

#ifndef GPC_IO_H_
#define GPC_IO_H_

#include "gpc_swk.h"

//====================================
// Адресация ресурсов на шине AXI
//====================================
#define MSG_BLOCK_START 0x00010000                              // area for IO

// Check parameters to avoid global memory conflict
// MSG_BLOCK_SIZE * (GPC cores in GROUP, 6 is default) < 64K
#define MSG_BLOCK_SIZE  0x2800                                  // 10K Block size for every GPC
#define MSG_BUFFER_SIZE 0x1000                                  // 4096 bytes buffers for GPC IO

// Automatically calculated parameters
#define HOST2GPC_BUFFER 0                     			// Offset for Host2GPC queue
#define GPC2HOST_BUFFER (MSG_BUFFER_SIZE)     			// Offset for GPC2Host queue
#define	MQ_CREDITS		255


//===================================
// Типа данных и структуры для работы
//===================================

// Define Leonhard x64 descriptor
typedef struct
    global_memory_io
{
        // Queue credit
        uint32_t gpc2host_credit;
        // Buffers offsets
        uint32_t gpc2host_buffer;
        uint32_t host2gpc_buffer;
        
} global_memory_io;

#ifdef DEFINE_GM_IO
#define EXTERN /* nothing */
#else
#define EXTERN extern
#endif /* DEFINE_GM_IO */
EXTERN global_memory_io gmio;

//========================================
// Функции для обмена данными с HOST
//========================================

void gmio_init(int core);
void mq_send(unsigned int message);
unsigned int mq_receive();
void buf_read(unsigned int size, char *local_buf);
void buf_write(unsigned int size, char *local_buf);
void ext_buf_read(unsigned int size, char *external_buf, char *local_buf);
void ext_buf_write(unsigned int size, char *external_buf, char *local_buf);

void sync_with_host();

#endif
