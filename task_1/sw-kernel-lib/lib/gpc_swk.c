/*
 * gpc_swk.h
 *
 * sw_kernel library
 *
 *  Created on: April 23, 2021
 *      Author: A.Popov
 */

#include "gpc_swk.h"

using namespace std;

//====================================================
// Установка флага состояния ядра 
//====================================================

void set_gpc_state(unsigned int val) {
    
   *((volatile unsigned int *)(AXI4GPIO_STATE_BASE)) = val;
}

//====================================================
// Чтение конфигурации ядра 
//====================================================

unsigned int gpc_config() {
    
   return *((volatile unsigned int *)(AXI4GPIO_CONFIG_BASE)); 
}

//====================================================
// Чтение бита запуска ядра 
//====================================================

bool gpc_start() {
    
   return *((volatile unsigned int *)(AXI4GPIO_START_BASE)); 
}

//====================================================
// Функция записи 32 бит по адресу  
//====================================================

void axi_wr_int32(unsigned int offs, unsigned int val) {
    
   *((volatile unsigned int *)(offs)) = val;
}

//====================================================
// Функция чтения 32 бит по адресу  
//====================================================

unsigned int axi_rd_int32(unsigned int offs) {
    
   return *((volatile unsigned int *)(offs)); 
}

//====================================================
// Функция записи 64 бит по адресу  
//====================================================

void axi_rd_int64(unsigned int offs, void *buf) {

    *((unsigned int*)buf  ) = *((volatile unsigned int *)(offs    ));
    *((unsigned int*)buf+1) = *((volatile unsigned int *)(offs + 4));
}

//====================================================
// Функция записи 64 бит по адресу и по ссылке 
//====================================================

void axi_wr_int64(unsigned int offs, void *buf) {
    
   *((volatile unsigned int *)(offs    )) = *((unsigned int*)buf);
   *((volatile unsigned int *)(offs + 4)) = *((unsigned int*)buf+1);
}

//====================================================
// Функция записи 64 бит по адресу и по значению 
//====================================================

void axi_wr_int64(unsigned int offs, unsigned long long buf) {
    
   *((volatile unsigned int *)(offs    )) = *((unsigned int*)&buf);
   *((volatile unsigned int *)(offs + 4)) = *((unsigned int*)&buf+1);
}
