/*
 * gpc_swk.h
 *
 * sw_kernel library
 *
 *  Created on: April 23, 2021
 *      Author: A.Popov
 */

#ifndef GPC_H_
#define GPC_H_

#include <stdint.h>


//====================================
// Адресация ресурсов на шине AXI
//====================================

#define AXI4BRAM_BASE           0xA0000000              //Global Memory Base Address
#define AXI4BRAM_SIZE           0x00020000              //Size: 128 KB
#define AXI4EXTMEM_BASE         0xA8000000              //External Memory Base Address
#define AXI4GPIO_START_BASE     0xA0030000              //GPIO port 0 IN  /START/
#define AXI4GPIO_CONFIG_BASE    0xA0030008              //GPIO port 1 IN  /CONFIG/
#define AXI4GPIO_STATE_BASE     0xA0020000              //GPIO port 2 OUT /STATE/
#define AXI4GPIO_GPC2HOST_MQ    0xA0040000              //GPIO port 3 OUT /GPC2HOST MQ/
#define AXI4GPIO_HOST2GPC_MQ    0xA0050000              //GPIO port 3 IN  /HOST2GPC MQ/
#define AXI4GPIO_MQ_ST          0xA0060000              //GPIO port 3 IN  /MQ_STATE/
#define AXI4GPIO_MQ_CTRL        0xA0060008              //GPIO port 3 IN  /MQ_CONTROL/


//====================================
// Адресация ресурсов на шине AXL
//====================================

#define AXL4LNH64_BASE          0x60000000              //lnh64 microprocessor Base Address

//====================================
// Биты конфигурационного регистра GPC
//====================================

#define 	START_REQUEST           16
#define 	CONTINUE_REQUEST        1

//====================================
// Биты статуса GPC
//====================================

#define 	STATUS_IDLE             0
#define 	STATUS_READY            1
#define 	IDLE                    0x1
#define 	BUSY                    0x0

//=====================================
// Биты статуса очередей регистра MQ_ST
//=====================================

#define 	HOST2GPC_MQ_EMPTY       (1<<0)
#define 	GPC2HOST_MQ_FULL        (1<<1)
#define 	GPC2HOST_MQ_AFULL       (1<<2)

//===========================================
// Биты управления очередями регистра MQ_CTRL
//===========================================

#define 	GPC2HOST_MQ_WRITE       (1<<0)
#define 	HOST2GPC_MQ_NEXT        (1<<1)


//========================================
// Служебные структуры
//========================================

union uint64 {
    unsigned long long 	u64;
    unsigned int 	u32[2];
    unsigned short int 	u16[4];   
    unsigned char 	u8[8];   
};

//========================================
// Генератор случайных чисел
//========================================


static int      seed {0xab1cd6e}; //	/* Seed, choose whatever you like! */
static unsigned int rand_single()
{
	seed = 8253729 * seed + 2396403;
	return seed;
}


//========================================
// Функции для обмена данными с HOST
//========================================

void                            axi_wr_int32(unsigned int offs, unsigned int val);
unsigned int                    axi_rd_int32(unsigned int offs);
void                            axi_rd_int64(unsigned int offs, void *buf);
void                            axi_wr_int64(unsigned int offs, void *buf);
void                            axi_wr_int64(unsigned int offs, unsigned long long buf);
void                            set_gpc_state(unsigned int val);
unsigned int                    gpc_config();
bool                            gpc_start();

#define wait 
/*
({ \
    for (int i;i<1000; i++); \
})
*/


#endif //GPC_H
