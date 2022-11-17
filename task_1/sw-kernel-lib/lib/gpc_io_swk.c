/*
 * gpc_io_swk.c
 *
 * sw_kernel library
 *
 *  Created on: April 23, 2021
 *      Author: A.Popov
 */

#include "gpc_io_swk.h"

using namespace std;

//====================================================
// Глобальная структура очереди
//====================================================

global_memory_io gmio;

//====================================================
// Инициализация структуры
//====================================================

void gmio_init(int core)
{
    // Init queue and message offsets
    gmio.gpc2host_credit=0;
    // Init buffers offsets
    gmio.gpc2host_buffer = AXI4BRAM_BASE + MSG_BLOCK_START + core * MSG_BLOCK_SIZE + GPC2HOST_BUFFER;
    gmio.host2gpc_buffer = AXI4BRAM_BASE + MSG_BLOCK_START + core * MSG_BLOCK_SIZE + HOST2GPC_BUFFER;
}

//====================================================
// Передача сообщения в очередь
//====================================================

void mq_send(unsigned int message)
{
	if (++gmio.gpc2host_credit > MQ_CREDITS) {
		while (((*((volatile unsigned int *)(AXI4GPIO_MQ_ST))) & GPC2HOST_MQ_AFULL) == GPC2HOST_MQ_AFULL);
		gmio.gpc2host_credit = 0;
	}
	*(volatile unsigned int *)(AXI4GPIO_GPC2HOST_MQ) = message;    
	*(volatile unsigned int *)(AXI4GPIO_MQ_CTRL) = GPC2HOST_MQ_WRITE;    
	*(volatile unsigned int *)(AXI4GPIO_MQ_CTRL) = 0;    
}

void mq_send(unsigned int size, unsigned int *buffer)
{
    for (unsigned int i=0; i<size; i++) 
        mq_send(buffer[i]);
}


//====================================================
// Чтение сообщения из очереди
//====================================================

unsigned int mq_receive()
{
    while (((*((volatile unsigned int *)(AXI4GPIO_MQ_ST))) & HOST2GPC_MQ_EMPTY) == HOST2GPC_MQ_EMPTY) {
        wait;
    };    
    unsigned int message = *(volatile unsigned int *)(AXI4GPIO_HOST2GPC_MQ);
	*(volatile unsigned int *)(AXI4GPIO_MQ_CTRL) = HOST2GPC_MQ_NEXT;    
	*(volatile unsigned int *)(AXI4GPIO_MQ_CTRL) = 0;     
    return message;
}


//====================================================
// Чтение буфера из global memory в RAM
//====================================================

void buf_read(unsigned int size, char *local_buf)
{
    for (unsigned int i=0; i<size; i+=4) 
        *((volatile unsigned int *)(local_buf + i)) = *((volatile unsigned int*)(gmio.host2gpc_buffer + i));
}


//====================================================
// Чтение буфера из global memory в RAM
//====================================================

void buf_write(unsigned int size, char *local_buf)
{
    for (unsigned int i=0; i<size; i+=4) 
        *((volatile unsigned int*)(gmio.gpc2host_buffer + i)) = *((volatile unsigned int *)(local_buf + i));
}

//====================================================
// Чтение буфера из external memory в RAM
//====================================================

void ext_buf_read(unsigned int size, char *external_buf, char *local_buf)
{
    for (unsigned int i=0; i<size; i+=4) 
        *((unsigned int *)(local_buf + i)) = *((volatile unsigned int*)(AXI4EXTMEM_BASE + external_buf + i));
}


//====================================================
// Чтение буфера из external memory в RAM
//====================================================

void ext_buf_write(unsigned int size, char *external_buf, char *local_buf)
{
    for (unsigned int i=0; i<size; i+=4) 
        *((volatile unsigned int*)(AXI4EXTMEM_BASE + external_buf + i)) = *((unsigned int *)(local_buf + i));
}


//====================================================
// Синхронизация ядра и host через передачу сообщений
//====================================================

void sync_with_host()
{
    mq_send(mq_receive());
}
