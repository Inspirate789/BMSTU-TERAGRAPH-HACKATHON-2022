/*
 * gpc_test.c
 *
 * sw_kernel library
 *
 *  Created on: April 23, 2021
 *      Author: A.Popov
 */

#include <stdlib.h>
#include <unistd.h>
#include "lnh64.h"
#include "gpc_io_swk.h"
#include "gpc_handlers.h"

#define SW_KERNEL_VERSION 26
#define DEFINE_LNH_DRIVER
#define DEFINE_MQ_R2L
#define DEFINE_MQ_L2R
#define __fast_recall__

#define TEST_STRUCTURE 1

extern lnh lnh_core;
extern global_memory_io gmio;
volatile unsigned int event_source;

int main(void) {
    /////////////////////////////////////////////////////////
    //                  Main Event Loop
    /////////////////////////////////////////////////////////
    //Leonhard driver structure should be initialised
    lnh_init();
    //Initialise host2gpc and gpc2host queues
    gmio_init(lnh_core.partition.data_partition);
    for (;;) {
        //Wait for event
        while (!gpc_start());
        //Enable RW operations
        set_gpc_state(BUSY);
        //Wait for event
        event_source = gpc_config();
        switch(event_source) {
            /////////////////////////////////////////////
            //  Measure GPN operation frequency
            /////////////////////////////////////////////
            case __event__(insert_burst) : insert_burst(); break;
            case __event__(search_burst) : search_burst(); break;
        }
        //Disable RW operations
        set_gpc_state(IDLE);
        while (gpc_start());

    }
}

//-------------------------------------------------------------
//      Получить пакет из глобальной памяти и аписат в lnh64
//-------------------------------------------------------------
 
void insert_burst() {

    //Удаление данных из структур
    lnh_del_str_sync(TEST_STRUCTURE);
    //Объявление переменных
    unsigned int count = mq_receive();
    unsigned int size_in_bytes = 2*count*sizeof(uint64_t);
    //Создание буфера для приема пакета
    uint64_t *buffer = (uint64_t*)malloc(size_in_bytes);
    //Чтение пакета в RAM
    buf_read(size_in_bytes, (char*)buffer);
    //Обработка пакета - запись 
    for (int i=0; i<count; i++) {
        lnh_ins_sync(TEST_STRUCTURE,buffer[2*i],buffer[2*i+1]);
    }
    lnh_sync();
    free(buffer);
}


//-------------------------------------------------------------
//      Обход структуры lnh64 и запись в глобальную память 
//-------------------------------------------------------------
 
void search_burst() {

    //Ожидание завершения предыдущих команд
    lnh_sync(); 
    //Объявление переменных
    unsigned int count = lnh_get_num(TEST_STRUCTURE);
    unsigned int size_in_bytes = 2*count*sizeof(uint64_t);
    //Создание буфера для приема пакета
    uint64_t *buffer = (uint64_t*)malloc(size_in_bytes);
    //Выборка минимального ключа
    lnh_get_first(TEST_STRUCTURE);

    //Запись ключа и значения в буфер
    for (int i=0; i<count; i++) {
        buffer[2*i] = lnh_core.result.key;
        buffer[2*i+1] = lnh_core.result.value;
        lnh_next(TEST_STRUCTURE,lnh_core.result.key);
    }

    //Запись глобальной памяти из RAM
    buf_write(size_in_bytes, (char*)buffer);   

    //Передать количество key-value
    mq_send(count);
    //Получить ключ
    auto key = mq_receive();
    //Поиск по ключу
	lnh_search(1, key);
    //Отправка ответа
	mq_send(lnh_core.result.value);
    // mq_send(buffer[2*1+1]);
    free(buffer);
}
