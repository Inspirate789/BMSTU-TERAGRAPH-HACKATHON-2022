/*
 * lnh64.c
 *
 * sw_kernel library
 *
 *  Created on: April 23, 2021
 *      Author: A.Popov
 */

#include "lnh64.h"

using namespace std;

//====================================================
// Глобальная структура ядра Leonhard
//====================================================

lnh lnh_core;

//====================================================
// Аппаратный сброс процессора Leonhard
//====================================================

void lnh_hw_reset() {
    //Reset Leonhard and RISCV, clock pulse formed automatically
    lnh_wr_reg32_byval(LNH_CNTL_HIGH,(uint32_t)(RESET_GPN));
    //Set Leonhard control register
    lnh_wr_reg32_byval(LNH_CNTL,(uint32_t)0);
}

//====================================================
// Програмный сброс процессора Leonhard
//====================================================

void lnh_sw_reset() {
    //Delete all structures, that requires O(1) for each
    for (int i=1; i<lnh_s_count; i++) lnh_del_str_sync(i);
    //Reset TSC counter
    lnh_wr_reg32_byval(LNH_CNTL_HIGH,(uint32_t)(1<<(RESET_TSC-32)));
}

//====================================================
// Инициализация структуры
//====================================================

void lnh_init() {
    //Initialise Leonhard microprocessor descriptor
    //Set AXI4 buffers and pointers
    lnh_core.buf32_ptr = &(lnh_core.buf32);
    lnh_core.buf64_ptr = &(lnh_core.buf64);
    //now check memory controller BIST status
    lnh_rd_reg64_byref(LNH_STATE,&lnh_core.result.status);
    //Get partition and region
    lnh_core.partition.data_partition=(lnh_core.result.status>>LNH_DATA_PARTITION) & 0x7;
	lnh_core.partition.index_partition=(lnh_core.result.status>>LNH_INDEX_PARTITION) & 0x7;
	lnh_core.partition.index_region=(lnh_core.result.status>>LNH_INDEX_REGION) & 0x3;
    //Send Hardware Reset signal to Leonhard
    #ifdef LNH_HW_RESET_ENABLE
    lnh_hw_reset();
    #endif
    //Delete all structures
    lnh_sw_reset();
	//Initialise IO credits
    lnh_core.axi2lnh_queue_credits = AXI2LNH_QUEUE_CREDITS_MAX;
}


//====================================================
// Чтение регистра 64 бит (Адрес) => Данные по значению
//====================================================

uint64_t lnh_rd_reg64(int adr) {
    //READ FROM LEONHARD
    lnh_rd_reg64_byref(adr, &lnh_core.buf64);
    return lnh_core.buf64;    
}

//====================================================
// Чтение регистра 64 бит (Адрес) => Данные по значению
//====================================================

uint32_t lnh_rd_reg32(int adr) {
    //READ FROM LEONHARD
    lnh_rd_reg32_byref(adr, &lnh_core.buf32);
    return lnh_core.buf32;    
}


//=======================================================================
// Преобразовать float в uint для хранения в целочисленном порядке ключей
//=======================================================================

uint32_t float2uint(float value){

    return * (uint32_t *)&value + (1<<31);
}

//=======================================================================
// Преобразовать uint в float для хранения в целочисленном порядке ключей
//=======================================================================

float uint2float(uint32_t value){

    uint32_t tmp = value + (1<<31);
    return *(float *)&tmp;
}


//=======================================================================
// Преобразовать double в ull для хранения в целочисленном порядке ключей
//=======================================================================

uint64_t double2ull(double value){

    return * (uint64_t *)&value + (1ull<<63);
}

//=======================================================================
// Преобразовать ull в double для хранения в целочисленном порядке ключей
//=======================================================================

double ull2double(uint64_t value){

    uint64_t tmp = value + (1ull<<63);
    return *(double *)&tmp;
}






//====================================================
// Служебная команда ожидания готовности очереди
//====================================================

void lnh_sync() {
	// Очередь пуста и процеcсор готов к выполнению следующей команды (двойная проверка)
    do {lnh_rd_reg32_byval(LNH_STATE_LOW); wait;} while ((lnh_core.buf32 & ( 1<<SPU_ALL_DONE  )) == 0);
}

//=========================================================================================
// Служебная команда ожидания готовности очереди и записи рузультата в регистр mailbox
//=========================================================================================

void lnh_syncm(int mbr) {
	// Очередь пуста и процеcсор готов к выполнению следующей команды (двойная проверка)
    do {lnh_rd_reg32_byval(LNH_STATE_LOW); wait;} while ((lnh_core.buf32 & ( 1<<SPU_ALL_DONE  )) == 0);
    // Результат записан в регистр mailbox
    do {lnh_rd_reg32_byval(LNH_STATE_HIGH); wait;} while ((lnh_core.buf32 & ( 1<<MBOX_VFLAG[mbr] )) == 0);
}

//================================================================================
// Служебные функции повторения предыдущей команды с изменение ключа иили значения
//================================================================================


void lnh_fast_recall(uint32_t key) {
    lnh_axi2lnh_queue_credits_check; 
    lnh_wr_reg32_byref(KEY2LNH_LOW, &key); 
    lnh_wr_reg32_byval(CMD2LNH_HIGH,0);
}

void lnh_fast_recall(uint32_t key, uint32_t value) { 
    lnh_axi2lnh_queue_credits_check; 
    lnh_wr_reg32_byref(KEY2LNH_LOW, &key); 
    lnh_wr_reg32_byref(VAL2LNH_LOW, &value); 
    lnh_wr_reg32_byval(CMD2LNH_HIGH,0);
}

void lnh_fast_recall(uint64_t key) { 
    lnh_axi2lnh_queue_credits_check; 
    lnh_wr_reg64_byref(KEY2LNH, &key); 
    lnh_wr_reg32_byval(CMD2LNH_HIGH,0);
}

void lnh_fast_recall(uint64_t key, uint64_t value) { 
    lnh_axi2lnh_queue_credits_check; 
    lnh_wr_reg64_byref(KEY2LNH, &key); 
    lnh_wr_reg64_byref(VAL2LNH, &value); 
    lnh_wr_reg32_byval(CMD2LNH_HIGH,0);
}



//====================================================
// Команды с ожиданием результата без очереди
//====================================================

//====================================================
// Добавление (Структура, Ключ, Значение)
//====================================================

bool lnh_ins_sync(uint64_t str, uint64_t key, uint64_t value)
{
    //проверка готовности устройства
	    lnh_axi2lnh_queue_credits_check;

    //запись исходных данных
	    lnh_wr_reg64_byref(KEY2LNH, &key);
        lnh_wr_reg64_byref(VAL2LNH, &value);
    	lnh_wr_reg64_byval(CMD2LNH, (INS<<lnh_cmd)|str);

    //ожидание готовности очереди команд
		lnh_sync();

    //чтение результата
	    lnh_rd_reg64_byref(LNH_STATE,&lnh_core.result.status);

    //results
	    if ((lnh_core.result.status & (1<<SPU_ERROR_FLAG)) != 0) {
			return false;
		} else {
			return true;
		}
}

//====================================================
// Удаление (Структура, Ключ)
//====================================================

bool lnh_del_sync(uint64_t str, uint64_t key) {

    //проверка готовности устройства
	    lnh_axi2lnh_queue_credits_check;

    //запись исходных данных
	    lnh_wr_reg64_byref(KEY2LNH, &key);
        lnh_wr_reg64_byval(CMD2LNH,(DEL<<lnh_cmd)|str);

    //ожидание готовности очереди команд
		lnh_sync();

    //чтение результата
	    lnh_rd_reg64_byref(LNH2KEY,&lnh_core.result.key);
	    lnh_rd_reg64_byref(LNH2VAL,&lnh_core.result.value);
	    lnh_rd_reg64_byref(LNH_STATE,&lnh_core.result.status);

    //results
	    if ((lnh_core.result.status & (1<<SPU_ERROR_FLAG)) != 0) {
			return false;
		} else {
			return true;
		}
}

//====================================================
// Мощность (Структура)
//====================================================

uint32_t lnh_get_num(uint64_t str) {
    //проверка готовности устройства
	    lnh_axi2lnh_queue_credits_check;

	//запись исходных данных
		lnh_wr_reg64_byval(CMD2LNH, (NOP<<lnh_cmd)|str);

    //ожидание готовности очереди команд
		lnh_sync();
        
        lnh_rd_reg32_byval(CARDINALITY);

 	    return lnh_core.buf32;
}

//====================================================
// Удаление структуры (Структура)
//====================================================

bool lnh_del_str_sync(uint64_t str){
    //проверка готовности устройства
	    lnh_axi2lnh_queue_credits_check;

    //запись исходных данных
	    lnh_wr_reg64_byval(CMD2LNH,(DELSTR<<lnh_cmd)|str);

    //ожидание готовности очереди команд
		lnh_sync();

    //чтение результата
	    lnh_rd_reg64_byref(LNH_STATE,&lnh_core.result.status);

    //results
	    if ((lnh_core.result.status & (1<<SPU_ERROR_FLAG)) != 0) {
			return false;
		} else {
			return true;
		}
}


//====================================================
// Сжатие (Структура)
//====================================================

bool lnh_sq_sync(uint64_t str){
    //проверка готовности устройства
	    lnh_axi2lnh_queue_credits_check;

    //запись исходных данных
	    lnh_wr_reg64_byval(CMD2LNH,(SQUIZ<<lnh_cmd)|str);

    //ожидание готовности очереди команд
		lnh_sync();

    //чтение результата
	    lnh_rd_reg64_byref(LNH_STATE,&lnh_core.result.status);

    //results
	    if ((lnh_core.result.status & (1<<SPU_ERROR_FLAG)) != 0) {
			return false;
		} else {
			return true;
		}
}

//====================================================
// ИЛИ (Структура1, Структура2, Структура результата)
//====================================================

bool lnh_or_sync(uint64_t A, uint64_t B, uint64_t R){
    uint64_t str = (B<<2*lnh_s)|(A<<lnh_s)|(R<<0);
    //проверка готовности устройства
	    lnh_axi2lnh_queue_credits_check;

    //запись исходных данных
	    lnh_wr_reg64_byval(CMD2LNH,(POR<<lnh_cmd)|str);

    //ожидание готовности очереди команд
		lnh_sync();

    //чтение результата
	    lnh_rd_reg64_byref(LNH_STATE,&lnh_core.result.status);

    //results
	    if ((lnh_core.result.status & (1<<SPU_ERROR_FLAG)) != 0) {
			return false;
		} else {
			return true;
		}
}

//====================================================
// И (Структура1, Структура2, Структура результата)
//====================================================

bool lnh_and_sync(uint64_t A, uint64_t B, uint64_t R){
    uint64_t str = (B<<2*lnh_s)|(A<<lnh_s)|(R<<0);
    //проверка готовности устройства
	    lnh_axi2lnh_queue_credits_check;

    //запись исходных данных
	    lnh_wr_reg64_byval(CMD2LNH,(PAND<<lnh_cmd)|str);

    //ожидание готовности очереди команд
		lnh_sync();

    //чтение результата
	    lnh_rd_reg64_byref(LNH_STATE,&lnh_core.result.status);

    //results
	    if ((lnh_core.result.status & (1<<SPU_ERROR_FLAG)) != 0) {
			return false;
		} else {
			return true;
		}
}

//====================================================
// НЕ (Структура1, Структура2, Структура результата)
//====================================================

bool lnh_not_sync(uint64_t A, uint64_t B, uint64_t R){
    uint64_t str = (B<<2*lnh_s)|(A<<lnh_s)|(R<<0);
    //проверка готовности устройства
	    lnh_axi2lnh_queue_credits_check;

    //запись исходных данных
	    lnh_wr_reg64_byval(CMD2LNH,(PNOT<<lnh_cmd)|str);

    //ожидание готовности очереди команд
		lnh_sync();

    //чтение результата
	    lnh_rd_reg64_byref(LNH_STATE,&lnh_core.result.status);

    //results
	    if ((lnh_core.result.status & (1<<SPU_ERROR_FLAG)) != 0) {
			return false;
		} else {
			return true;
		}
}

//====================================================
// LSEQ (Структура1, Структура2, Структура результата)
//====================================================

bool lnh_lseq_sync(uint64_t key, uint64_t A, uint64_t R){
    uint64_t str = (A<<lnh_s)|(R<<0);
    //проверка готовности устройства
	    lnh_axi2lnh_queue_credits_check;

    //запись исходных данных
        lnh_wr_reg64_byref(KEY2LNH, &key);
	    lnh_wr_reg64_byval(CMD2LNH,(LSEQ<<lnh_cmd)|str);

    //ожидание готовности очереди команд
		lnh_sync();

    //чтение результата
	    lnh_rd_reg64_byref(LNH_STATE,&lnh_core.result.status);

    //results
	    if ((lnh_core.result.status & (1<<SPU_ERROR_FLAG)) != 0) {
			return false;
		} else {
			return true;
		}
}

//====================================================
// LS (Структура1, Структура2, Структура результата)
//====================================================

bool ls_sync(uint64_t key, uint64_t A, uint64_t R){
    uint64_t str = (A<<lnh_s)|(R<<0);
    //проверка готовности устройства
	    lnh_axi2lnh_queue_credits_check;

    //запись исходных данных
        lnh_wr_reg64_byref(KEY2LNH, &key);
        lnh_wr_reg64_byval(CMD2LNH,(LS<<lnh_cmd)|str);

    //ожидание готовности очереди команд
		lnh_sync();

    //чтение результата
	    lnh_rd_reg64_byref(LNH_STATE,&lnh_core.result.status);

    //results
	    if ((lnh_core.result.status & (1<<SPU_ERROR_FLAG)) != 0) {
			return false;
		} else {
			return true;
		}
}

//====================================================
// GREQ (Структура1, Структура2, Структура результата)
//====================================================

bool lnh_greq_sync(uint64_t key, uint64_t A, uint64_t R){
    uint64_t str = (A<<lnh_s)|(R<<0);
    //проверка готовности устройства
	    lnh_axi2lnh_queue_credits_check;

    //запись исходных данных
        lnh_wr_reg64_byref(KEY2LNH, &key);
	    lnh_wr_reg64_byval(CMD2LNH,(GREQ<<lnh_cmd)|str);

    //ожидание готовности очереди команд
		lnh_sync();

    //чтение результата
	    lnh_rd_reg64_byref(LNH_STATE,&lnh_core.result.status);

    //results
	    if ((lnh_core.result.status & (1<<SPU_ERROR_FLAG)) != 0) {
			return false;
		} else {
			return true;
		}
}

//====================================================
// GR (Структура1, Структура2, Структура результата)
//====================================================

bool lnh_gr_sync(uint64_t key, uint64_t A, uint64_t R){
    uint64_t str = (A<<lnh_s)|(R<<0);
    //проверка готовности устройства
	    lnh_axi2lnh_queue_credits_check;

    //запись исходных данных
        lnh_wr_reg64_byref(KEY2LNH, &key);
	    lnh_wr_reg64_byval(CMD2LNH,(GR<<lnh_cmd)|str);

    //ожидание готовности очереди команд
		lnh_sync();

    //чтение результата
	    lnh_rd_reg64_byref(LNH_STATE,&lnh_core.result.status);

    //results
	    if ((lnh_core.result.status & (1<<SPU_ERROR_FLAG)) != 0) {
			return false;
		} else {
			return true;
		}
}

//=================================================================================
// GRLS (Ключ левой границы, Ключ правой границы, Структура1, Структура результата)
//=================================================================================

bool lnh_grls_sync(uint64_t key_ls, uint64_t key_gr, uint64_t A, uint64_t R) {

    uint64_t str = (A<<lnh_s)|(R<<0);
    //проверка готовности устройства
	    lnh_axi2lnh_queue_credits_check;

    //запись исходных данных
        lnh_wr_reg64_byval(KEY2LNH, key_ls);
        lnh_wr_reg64_byval(VAL2LNH, key_gr);
	    lnh_wr_reg64_byval(CMD2LNH,(GRLS<<lnh_cmd)|str);

    //ожидание готовности очереди команд
		lnh_sync();

    //чтение результата
	    lnh_rd_reg64_byref(LNH_STATE,&lnh_core.result.status);

    //results
	    if ((lnh_core.result.status & (1<<SPU_ERROR_FLAG)) != 0) {
			return false;
		} else {
			return true;
		}
}



//====================================================
// Поиск (Структура, Ключ, Результат)
//====================================================

bool lnh_search(uint64_t str, uint64_t key){

    //проверка готовности устройства
	    lnh_axi2lnh_queue_credits_check;

    //запись исходных данных
        lnh_wr_reg64_byref(KEY2LNH, &key);
        lnh_wr_reg64_byval(CMD2LNH,(SEARCH<<lnh_cmd)|str);

    //ожидание готовности очереди команд
		lnh_sync();

    //чтение результата
	    lnh_rd_reg64_byref(LNH2KEY,&lnh_core.result.key);
	    lnh_rd_reg64_byref(LNH2VAL,&lnh_core.result.value);
	    lnh_rd_reg64_byref(LNH_STATE,&lnh_core.result.status);

    //results
	    if ((lnh_core.result.status & (1<<SPU_ERROR_FLAG)) != 0) {
			return false;
		} else {
			return true;
		}
}

//====================================================
// Следующий (Структура, Ключ, Результат)
//====================================================

bool lnh_next(uint64_t str, uint64_t key){

    //проверка готовности устройства
	    lnh_axi2lnh_queue_credits_check;

    //запись исходных данных
        lnh_wr_reg64_byref(KEY2LNH, &key);
	    lnh_wr_reg64_byval(CMD2LNH,(NEXT<<lnh_cmd)|str);

    //ожидание готовности очереди команд
		lnh_sync();

    //чтение результата
	    lnh_rd_reg64_byref(LNH2KEY,&lnh_core.result.key);
	    lnh_rd_reg64_byref(LNH2VAL,&lnh_core.result.value);
	    lnh_rd_reg64_byref(LNH_STATE,&lnh_core.result.status);

    //results
	    if ((lnh_core.result.status & (1<<SPU_ERROR_FLAG)) != 0) {
			return false;
		} else {
			return true;
		}
}

//====================================================
// Предыдущий (Структура, Ключ, Результат)
//====================================================

bool lnh_prev(uint64_t str, uint64_t key){

    //проверка готовности устройства
	    lnh_axi2lnh_queue_credits_check;

    //запись исходных данных
        lnh_wr_reg64_byref(KEY2LNH, &key);
        lnh_wr_reg64_byval(CMD2LNH,(PREV<<lnh_cmd)|str);

    //ожидание готовности очереди команд
		lnh_sync();

    //чтение результата
	    lnh_rd_reg64_byref(LNH2KEY,&lnh_core.result.key);
	    lnh_rd_reg64_byref(LNH2VAL,&lnh_core.result.value);
	    lnh_rd_reg64_byref(LNH_STATE,&lnh_core.result.status);

    //results
	    if ((lnh_core.result.status & (1<<SPU_ERROR_FLAG)) != 0) {
			return false;
		} else {
			return true;
		}
}

//====================================================
// Меньший (Структура, Ключ, Результат)
//====================================================

bool lnh_nsm(uint64_t str, uint64_t key){

    //проверка готовности устройства
	    lnh_axi2lnh_queue_credits_check;

    //запись исходных данных
        lnh_wr_reg64_byref(KEY2LNH, &key);
        lnh_wr_reg64_byval(CMD2LNH,(NSM<<lnh_cmd)|str);

    //ожидание готовности очереди команд
		lnh_sync();

    //чтение результата
	    lnh_rd_reg64_byref(LNH2KEY,&lnh_core.result.key);
	    lnh_rd_reg64_byref(LNH2VAL,&lnh_core.result.value);
	    lnh_rd_reg64_byref(LNH_STATE,&lnh_core.result.status);

    //results
	    if ((lnh_core.result.status & (1<<SPU_ERROR_FLAG)) != 0) {
			return false;
		} else {
			return true;
		}
}

//====================================================
// Больший (Структура, Ключ, Результат)
//====================================================

bool lnh_ngr(uint64_t str, uint64_t key){

    //проверка готовности устройства
	    lnh_axi2lnh_queue_credits_check;

    //запись исходных данных
        lnh_wr_reg64_byref(KEY2LNH, &key);
	    lnh_wr_reg64_byval(CMD2LNH,(NGR<<lnh_cmd)|str);

    //ожидание готовности очереди команд
		lnh_sync();

    //чтение результата
	    lnh_rd_reg64_byref(LNH2KEY,&lnh_core.result.key);
	    lnh_rd_reg64_byref(LNH2VAL,&lnh_core.result.value);
	    lnh_rd_reg64_byref(LNH_STATE,&lnh_core.result.status);

    //results
	    if ((lnh_core.result.status & (1<<SPU_ERROR_FLAG)) != 0) {
			return false;
		} else {
			return true;
		}
}


//====================================================
// Меньший (Структура, Ключ, Результат)
//====================================================

bool lnh_nsm_signed(uint64_t str, long long int key){

	bool nsm_exists = lnh_nsm(str,(uint64_t)key);

	if (key<0 && nsm_exists && (lnh_core.result.key<(1ull<<63)))  
		return false;
	if ((key>=0) && !nsm_exists && (lnh_search(str,-1ull)))
		return true; 
	if ((key>=0) && !nsm_exists && (lnh_nsm(str,-1ull)) && (lnh_core.result.key<(1ull<<63)))
		return false;
	return nsm_exists;

}

//====================================================
// Больший (Структура, Ключ, Результат)
//====================================================

bool lnh_ngr_signed(uint64_t str, long long int key){

	bool ngr_exists = lnh_ngr(str,(uint64_t)key);

	if (key>=0 && ngr_exists && (lnh_core.result.key>=(1ull<<63)))  
		return false;
	if ((key<0) && !ngr_exists && (lnh_search(str,0ull)))
		return true; 
	if ((key<0) && !ngr_exists && (lnh_ngr(str,0ull)) && (lnh_core.result.key>=(1ull<<63)))
		return false;
	return ngr_exists;

}


//====================================================
// MIN (Структура, Результат)
//====================================================

bool lnh_get_first(uint64_t str){
    //проверка готовности устройства
	    lnh_axi2lnh_queue_credits_check;

    //запись исходных данных
	    lnh_wr_reg64_byval(CMD2LNH,(FST<<lnh_cmd)|str);

    //ожидание готовности очереди команд
		lnh_sync();

    //чтение результата
	    lnh_rd_reg64_byref(LNH2KEY,&lnh_core.result.key);
	    lnh_rd_reg64_byref(LNH2VAL,&lnh_core.result.value);
	    lnh_rd_reg64_byref(LNH_STATE,&lnh_core.result.status);

    //results
	    if ((lnh_core.result.status & (1<<SPU_ERROR_FLAG)) != 0) {
			return false;
		} else {
			return true;
		}
}

//====================================================
// MAX (Структура, Результат)
//====================================================

bool lnh_get_last(uint64_t str){

    //проверка готовности устройства
	    lnh_axi2lnh_queue_credits_check;

    //запись исходных данных
    	lnh_wr_reg64_byval(CMD2LNH,(LST<<lnh_cmd)|str);

    //ожидание готовности очереди команд
		lnh_sync();

    //чтение результата
	    lnh_rd_reg64_byref(LNH2KEY,&lnh_core.result.key);
	    lnh_rd_reg64_byref(LNH2VAL,&lnh_core.result.value);
	    lnh_rd_reg64_byref(LNH_STATE,&lnh_core.result.status);

    //results
	    if ((lnh_core.result.status & (1<<SPU_ERROR_FLAG)) != 0) {
			return false;
		} else {
			return true;
		}
}

//====================================================
// MIN (Структура, Результат) знаковый
//====================================================

bool lnh_get_first_signed(uint64_t str){

	//Если есть числа из отрицателього диапазона
	if (lnh_ngr(str,(1ull<<63)-1)) return true;
	//Иначе выбрать из положительного диапазона
	else return lnh_get_first(str);

}

//====================================================
// MAX (Структура, Результат) знаковый
//====================================================

bool lnh_get_last_signed(uint64_t str){

	//Если есть числа из роложительного диапазона
	if (lnh_nsm(str,(1ull<<63))) return true;
	//Иначе выбрать из положительного диапазона
	else return lnh_get_last(str);

}


//====================================================
// Команды без ожидания результата и запиью в очередь
//====================================================

//====================================================
// Добавление (Структура, Ключ, Значение)
//====================================================

bool lnh_ins_syncq(uint64_t str, uint64_t key, uint64_t value)
{
    //проверка готовности устройства
	    lnh_axi2lnh_queue_credits_check;

    //запись исходных данных
	    lnh_wr_reg64_byref(KEY2LNH, &key);
        lnh_wr_reg64_byref(VAL2LNH, &value);
    	lnh_wr_reg64_byval(CMD2LNH, (INSQ<<lnh_cmd)|str);

    //results
		return true;
}

//====================================================
// Удаление (Структура, Ключ)
//====================================================

bool lnh_del_syncq(uint64_t str, uint64_t key) {

    //проверка готовности устройства
	    lnh_axi2lnh_queue_credits_check;

    //запись исходных данных
	    lnh_wr_reg64_byref(KEY2LNH, &key);
        lnh_wr_reg64_byval(CMD2LNH,(DELQ<<lnh_cmd)|str);

    //results
		return true;

}

//====================================================
// Мощность (Структура)
//====================================================

uint32_t lnh_get_numq(uint64_t str) {
    //проверка готовности устройства
	    lnh_axi2lnh_queue_credits_check;

        lnh_wr_reg64_byval(CMD2LNH,(NUMQ<<lnh_cmd)|str);

    //results
		return true;
}

//====================================================
// Удаление структуры (Структура)
//====================================================

bool lnh_del_str_syncq(uint64_t str){
    //проверка готовности устройства
	    lnh_axi2lnh_queue_credits_check;

    //запись исходных данных
	    lnh_wr_reg64_byval(CMD2LNH,(DELSTRQ<<lnh_cmd)|str);

    //results
		return true;
}


//====================================================
// Сжатие (Структура)
//====================================================

bool lnh_sq_syncq(uint64_t str){
    //проверка готовности устройства
	    lnh_axi2lnh_queue_credits_check;

    //запись исходных данных
	    lnh_wr_reg64_byval(CMD2LNH,(SQUIZQ<<lnh_cmd)|str);

    //results
		return true;
}

//====================================================
// ИЛИ (Структура1, Структура2, Структура результата)
//====================================================

bool lnh_or_syncq(uint64_t A, uint64_t B, uint64_t R){
    uint64_t str = (B<<2*lnh_s)|(A<<lnh_s)|(R<<0);
    //проверка готовности устройства
	    lnh_axi2lnh_queue_credits_check;

    //запись исходных данных
	    lnh_wr_reg64_byval(CMD2LNH,(PORQ<<lnh_cmd)|str);

    //results
		return true;
}

//====================================================
// И (Структура1, Структура2, Структура результата)
//====================================================

bool lnh_and_syncq(uint64_t A, uint64_t B, uint64_t R){
    uint64_t str = (B<<2*lnh_s)|(A<<lnh_s)|(R<<0);
    //проверка готовности устройства
	    lnh_axi2lnh_queue_credits_check;

    //запись исходных данных
	    lnh_wr_reg64_byval(CMD2LNH,(PANDQ<<lnh_cmd)|str);

    //results
		return true;
}

//====================================================
// НЕ (Структура1, Структура2, Структура результата)
//====================================================

bool lnh_not_syncq(uint64_t A, uint64_t B, uint64_t R){
    uint64_t str = (B<<2*lnh_s)|(A<<lnh_s)|(R<<0);
    //проверка готовности устройства
	    lnh_axi2lnh_queue_credits_check;

    //запись исходных данных
	    lnh_wr_reg64_byval(CMD2LNH,(PNOTQ<<lnh_cmd)|str);

    //results
		return true;
}

//====================================================
// LSEQ (Структура1, Структура2, Структура результата)
//====================================================

bool lnh_lseq_syncq(uint64_t key, uint64_t A, uint64_t R){
    uint64_t str = (A<<lnh_s)|(R<<0);
    //проверка готовности устройства
	    lnh_axi2lnh_queue_credits_check;

    //запись исходных данных
        lnh_wr_reg64_byref(KEY2LNH, &key);
	    lnh_wr_reg64_byval(CMD2LNH,(LSEQQ<<lnh_cmd)|str);

    //results
		return true;
}

//====================================================
// LS (Структура1, Структура2, Структура результата)
//====================================================

bool lnh_ls_syncq(uint64_t key, uint64_t A, uint64_t R){
    uint64_t str = (A<<lnh_s)|(R<<0);
    //проверка готовности устройства
	    lnh_axi2lnh_queue_credits_check;

    //запись исходных данных
        lnh_wr_reg64_byref(KEY2LNH, &key);
        lnh_wr_reg64_byval(CMD2LNH,(LSQ<<lnh_cmd)|str);

    //results
		return true;
}

//====================================================
// GREQ (Структура1, Структура2, Структура результата)
//====================================================

bool lnh_greq_syncq(uint64_t key, uint64_t A, uint64_t R){
    uint64_t str = (A<<lnh_s)|(R<<0);
    //проверка готовности устройства
	    lnh_axi2lnh_queue_credits_check;

    //запись исходных данных
        lnh_wr_reg64_byref(KEY2LNH, &key);
	    lnh_wr_reg64_byval(CMD2LNH,(GREQQ<<lnh_cmd)|str);

    //results
		return true;
}

//====================================================
// GR (Структура1, Структура2, Структура результата)
//====================================================

bool lnh_gr_syncq(uint64_t key, uint64_t A, uint64_t R){
    uint64_t str = (A<<lnh_s)|(R<<0);
    //проверка готовности устройства
	    lnh_axi2lnh_queue_credits_check;

    //запись исходных данных
        lnh_wr_reg64_byref(KEY2LNH, &key);
	    lnh_wr_reg64_byval(CMD2LNH,(GRQ<<lnh_cmd)|str);

    //results
		return true;
}

//=================================================================================
// GRLS (Ключ левой границы, Ключ правой границы, Структура1, Структура результата)
//=================================================================================

bool lnh_grls_syncq(uint64_t key_ls, uint64_t key_gr, uint64_t A, uint64_t R) {

    uint64_t str = (A<<lnh_s)|(R<<0);
    //проверка готовности устройства
	    lnh_axi2lnh_queue_credits_check;

    //запись исходных данных
        lnh_wr_reg64_byval(KEY2LNH, key_ls);
        lnh_wr_reg64_byval(VAL2LNH, key_gr);
	    lnh_wr_reg64_byval(CMD2LNH,(GRLSQ<<lnh_cmd)|str);

    //results
		return true;
}



//====================================================
// Поиск (Структура, Ключ, Результат)
//====================================================

bool lnh_searchq(uint64_t str, uint64_t key){

    //проверка готовности устройства
	    lnh_axi2lnh_queue_credits_check;

    //запись исходных данных
        lnh_wr_reg64_byref(KEY2LNH, &key);
        lnh_wr_reg64_byval(CMD2LNH,(SEARCHQ<<lnh_cmd)|str);

    //results
		return true;
}

//====================================================
// Следующий (Структура, Ключ, Результат)
//====================================================

bool lnh_nextq(uint64_t str, uint64_t key){

    //проверка готовности устройства
	    lnh_axi2lnh_queue_credits_check;

    //запись исходных данных
        lnh_wr_reg64_byref(KEY2LNH, &key);
	    lnh_wr_reg64_byval(CMD2LNH,(NEXTQ<<lnh_cmd)|str);

    //results
		return true;
}

//====================================================
// Предыдущий (Структура, Ключ, Результат)
//====================================================

bool lnh_prevq(uint64_t str, uint64_t key){

    //проверка готовности устройства
	    lnh_axi2lnh_queue_credits_check;

    //запись исходных данных
        lnh_wr_reg64_byref(KEY2LNH, &key);
        lnh_wr_reg64_byval(CMD2LNH,(PREVQ<<lnh_cmd)|str);

    //results
		return true;

}

//====================================================
// Меньший (Структура, Ключ, Результат)
//====================================================

bool lnh_nsmq(uint64_t str, uint64_t key){

    //проверка готовности устройства
	    lnh_axi2lnh_queue_credits_check;

    //запись исходных данных
        lnh_wr_reg64_byref(KEY2LNH, &key);
        lnh_wr_reg64_byval(CMD2LNH,(NSMQ<<lnh_cmd)|str);

    //results
		return true;
}

//====================================================
// Больший (Структура, Ключ, Результат)
//====================================================

bool lnh_ngrq(uint64_t str, uint64_t key){

    //проверка готовности устройства
	    lnh_axi2lnh_queue_credits_check;

    //запись исходных данных
        lnh_wr_reg64_byref(KEY2LNH, &key);
	    lnh_wr_reg64_byval(CMD2LNH,(NGRQ<<lnh_cmd)|str);

    //results
		return true;
}

//====================================================
// MIN (Структура, Результат)
//====================================================

bool lnh_get_firstq(uint64_t str){
    //проверка готовности устройства
	    lnh_axi2lnh_queue_credits_check;

    //запись исходных данных
	    lnh_wr_reg64_byval(CMD2LNH,(FSTQ<<lnh_cmd)|str);

    //results
		return true;
}

//====================================================
// MAX (Структура, Результат)
//====================================================

bool lnh_get_lastq(uint64_t str){

    //проверка готовности устройства
	    lnh_axi2lnh_queue_credits_check;

    //запись исходных данных
    	lnh_wr_reg64_byval(CMD2LNH,(LSTQ<<lnh_cmd)|str);

    //results
		return true;
}

//====================================================
// Чтение результата из очереди
//====================================================

bool lnh_get_q() {

	//ожидание готовности очереди
   		//while ((lnh_rd_reg32_l2l_byval(LNH_STATE,lnh_core.buf32) & (1<<LNH2AXI_Q_EMP_FLAG)) != 0);
 	    do {lnh_rd_reg32_byval(LNH_STATE_LOW); wait;} while ((lnh_core.buf32 & (1<<SPU_ALL_DONE))==0);
        

    //чтение результата
	    lnh_rd_reg64_byref(LNH2KEY,&lnh_core.result.key);
        lnh_rd_reg64_byref(LNH2VAL, &lnh_core.result.value);
	    lnh_rd_reg64_byref(LNH_STATE,&lnh_core.result.status);

    //results
	    if ((lnh_core.result.status & (1<<SPU_ERROR_FLAG)) != 0) {
			return false;
		} else {
			return true;
		}
}




//====================================================
// Команды без ожидания результата с записью в mbox
//====================================================


//====================================================
// Добавление (Структура, Ключ, Значение)
//====================================================

bool lnh_ins_syncm(int st_mreg, uint64_t str, uint64_t key, uint64_t value)
{
    //проверка готовности устройства
	    lnh_axi2lnh_queue_credits_check;

    //запись исходных данных
	    lnh_wr_reg64_byref(KEY2LNH, &key);
        lnh_wr_reg64_byref(VAL2LNH, &value);
    	lnh_wr_reg64_byval(CMD2LNH, (st_mreg<<lnh_mbox_st)|(mrf<<lnh_mbox_val)|(mrf<<lnh_mbox_key)|(INS<<lnh_cmd)|str);

    //results
		return true;
}

//====================================================
// Удаление (Структура, Ключ)
//====================================================

bool lnh_del_syncm(int st_mreg, uint64_t str, uint64_t key) {

    //проверка готовности устройства
	    lnh_axi2lnh_queue_credits_check;

    //запись исходных данных
	    lnh_wr_reg64_byref(KEY2LNH, &key);
        lnh_wr_reg64_byval(CMD2LNH,(st_mreg<<lnh_mbox_st)|(mrf<<lnh_mbox_val)|(mrf<<lnh_mbox_key)|(DEL<<lnh_cmd)|str);

    //results
		return true;

}

//====================================================
// Мощность (Структура)
//====================================================

uint32_t lnh_get_numm(uint64_t str) {
    //проверка готовности устройства
	    lnh_axi2lnh_queue_credits_check;

        lnh_wr_reg64_byval(CMD2LNH,(mrf<<lnh_mbox_st)|(mrf<<lnh_mbox_val)|(mrf<<lnh_mbox_key)|(NOP<<lnh_cmd)|str);

    //ожидание готовности очереди команд
        lnh_syncm(mrf);

    //results
 	    uint64_t tmp = lnh_get_m(mrf);

	    return *((uint32_t*)&tmp);

}

//====================================================
// Удаление структуры (Структура)
//====================================================

bool lnh_del_str_syncm(int st_mreg, uint64_t str){
    //проверка готовности устройства
	    lnh_axi2lnh_queue_credits_check;

    //запись исходных данных
	    lnh_wr_reg64_byval(CMD2LNH,(st_mreg<<lnh_mbox_st)|(mrf<<lnh_mbox_val)|(mrf<<lnh_mbox_key)|(DELSTR<<lnh_cmd)|str);

    //results
		return true;
}


//====================================================
// Сжатие (Структура)
//====================================================

bool lnh_sq_syncm(int st_mreg, uint64_t str){
    //проверка готовности устройства
	    lnh_axi2lnh_queue_credits_check;

    //запись исходных данных
	    lnh_wr_reg64_byval(CMD2LNH,(st_mreg<<lnh_mbox_st)|(mrf<<lnh_mbox_val)|(mrf<<lnh_mbox_key)|(SQUIZ<<lnh_cmd)|str);

    //results
		return true;
}

//====================================================
// ИЛИ (Структура1, Структура2, Структура результата)
//====================================================

bool lnh_or_syncm(int st_mreg, uint64_t A, uint64_t B, uint64_t R){
    uint64_t str = (B<<2*lnh_s)|(A<<lnh_s)|(R<<0);
    //проверка готовности устройства
	    lnh_axi2lnh_queue_credits_check;

    //запись исходных данных
	    lnh_wr_reg64_byval(CMD2LNH,(st_mreg<<lnh_mbox_st)|(mrf<<lnh_mbox_val)|(mrf<<lnh_mbox_key)|(POR<<lnh_cmd)|str);

    //results
		return true;
}

//====================================================
// И (Структура1, Структура2, Структура результата)
//====================================================

bool lnh_and_syncm(int st_mreg, uint64_t A, uint64_t B, uint64_t R){
    uint64_t str = (B<<2*lnh_s)|(A<<lnh_s)|(R<<0);
    //проверка готовности устройства
	    lnh_axi2lnh_queue_credits_check;

    //запись исходных данных
	    lnh_wr_reg64_byval(CMD2LNH,(st_mreg<<lnh_mbox_st)|(mrf<<lnh_mbox_val)|(mrf<<lnh_mbox_key)|(PAND<<lnh_cmd)|str);

    //results
		return true;
}

//====================================================
// НЕ (Структура1, Структура2, Структура результата)
//====================================================

bool lnh_not_syncm(int st_mreg, uint64_t A, uint64_t B, uint64_t R){
    uint64_t str = (B<<2*lnh_s)|(A<<lnh_s)|(R<<0);
    //проверка готовности устройства
	    lnh_axi2lnh_queue_credits_check;

    //запись исходных данных
	    lnh_wr_reg64_byval(CMD2LNH,(st_mreg<<lnh_mbox_st)|(mrf<<lnh_mbox_val)|(mrf<<lnh_mbox_key)|(PNOT<<lnh_cmd)|str);

    //results
		return true;
}

//====================================================
// LSEQ (Структура1, Структура2, Структура результата)
//====================================================

bool lnh_lseq_syncm(int st_mreg, uint64_t key, uint64_t A, uint64_t R){
    uint64_t str = (A<<lnh_s)|(R<<0);
    //проверка готовности устройства
	    lnh_axi2lnh_queue_credits_check;

    //запись исходных данных
        lnh_wr_reg64_byref(KEY2LNH, &key);
	    lnh_wr_reg64_byval(CMD2LNH,(st_mreg<<lnh_mbox_st)|(mrf<<lnh_mbox_val)|(mrf<<lnh_mbox_key)|(LSEQ<<lnh_cmd)|str);

    //results
		return true;
}

//====================================================
// LS (Структура1, Структура2, Структура результата)
//====================================================

bool lnh_ls_syncm(int st_mreg, uint64_t key, uint64_t A, uint64_t R){
    uint64_t str = (A<<lnh_s)|(R<<0);
    //проверка готовности устройства
	    lnh_axi2lnh_queue_credits_check;

    //запись исходных данных
        lnh_wr_reg64_byref(KEY2LNH, &key);
        lnh_wr_reg64_byval(CMD2LNH,(st_mreg<<lnh_mbox_st)|(mrf<<lnh_mbox_val)|(mrf<<lnh_mbox_key)|(LS<<lnh_cmd)|str);

    //results
		return true;
}

//====================================================
// GREQ (Структура1, Структура2, Структура результата)
//====================================================

bool lnh_greq_syncm(int st_mreg, uint64_t key, uint64_t A, uint64_t R){
    uint64_t str = (A<<lnh_s)|(R<<0);
    //проверка готовности устройства
	    lnh_axi2lnh_queue_credits_check;

    //запись исходных данных
        lnh_wr_reg64_byref(KEY2LNH, &key);
	    lnh_wr_reg64_byval(CMD2LNH,(st_mreg<<lnh_mbox_st)|(mrf<<lnh_mbox_val)|(mrf<<lnh_mbox_key)|(GREQ<<lnh_cmd)|str);

    //results
		return true;
}

//====================================================
// GR (Структура1, Структура2, Структура результата)
//====================================================

bool lnh_gr_syncm(int st_mreg, uint64_t key, uint64_t A, uint64_t R){
    uint64_t str = (A<<lnh_s)|(R<<0);
    //проверка готовности устройства
	    lnh_axi2lnh_queue_credits_check;

    //запись исходных данных
        lnh_wr_reg64_byref(KEY2LNH, &key);
	    lnh_wr_reg64_byval(CMD2LNH,(st_mreg<<lnh_mbox_st)|(mrf<<lnh_mbox_val)|(mrf<<lnh_mbox_key)|(GR<<lnh_cmd)|str);

    //results
		return true;
}

//=================================================================================
// GRLS (Ключ левой границы, Ключ правой границы, Структура1, Структура результата)
//=================================================================================

bool lnh_grls_syncm(int st_mreg, uint64_t key_ls, uint64_t key_gr, uint64_t A, uint64_t R) {

    uint64_t str = (A<<lnh_s)|(R<<0);
    //проверка готовности устройства
	    lnh_axi2lnh_queue_credits_check;

    //запись исходных данных
        lnh_wr_reg64_byval(KEY2LNH, key_ls);
        lnh_wr_reg64_byval(VAL2LNH, key_gr);
	    lnh_wr_reg64_byval(CMD2LNH,(st_mreg<<lnh_mbox_st)|(mrf<<lnh_mbox_val)|(mrf<<lnh_mbox_key)|(GRLS<<lnh_cmd)|str);

    //results
		return true;
}



//====================================================
// Поиск (Структура, Ключ, Результат)
//====================================================

bool lnh_searchm(int key_mreg, int val_mreg, int st_mreg, uint64_t str, uint64_t key){

    //проверка готовности устройства
	    lnh_axi2lnh_queue_credits_check;

    //запись исходных данных
        lnh_wr_reg64_byref(KEY2LNH, &key);
        lnh_wr_reg64_byval(CMD2LNH,(st_mreg<<lnh_mbox_st)|(val_mreg<<lnh_mbox_val)|(key_mreg<<lnh_mbox_key)|(SEARCH<<lnh_cmd)|str);

    //results
		return true;
}

//====================================================
// Следующий (Структура, Ключ, Результат)
//====================================================

bool lnh_nextm(int key_mreg, int val_mreg, int st_mreg, uint64_t str, uint64_t key){

    //проверка готовности устройства
	    lnh_axi2lnh_queue_credits_check;

    //запись исходных данных
        lnh_wr_reg64_byref(KEY2LNH, &key);
	    lnh_wr_reg64_byval(CMD2LNH,(st_mreg<<lnh_mbox_st)|(val_mreg<<lnh_mbox_val)|(key_mreg<<lnh_mbox_key)|(NEXT<<lnh_cmd)|str);

    //results
		return true;
}

//====================================================
// Предыдущий (Структура, Ключ, Результат)
//====================================================

bool lnh_prevm(int key_mreg, int val_mreg, int st_mreg, uint64_t str, uint64_t key){

    //проверка готовности устройства
	    lnh_axi2lnh_queue_credits_check;

    //запись исходных данных
        lnh_wr_reg64_byref(KEY2LNH, &key);
        lnh_wr_reg64_byval(CMD2LNH,(st_mreg<<lnh_mbox_st)|(val_mreg<<lnh_mbox_val)|(key_mreg<<lnh_mbox_key)|(PREV<<lnh_cmd)|str);

    //results
		return true;

}

//====================================================
// Меньший (Структура, Ключ, Результат)
//====================================================

bool lnh_nsmm(int key_mreg, int val_mreg, int st_mreg, uint64_t str, uint64_t key){

    //проверка готовности устройства
	    lnh_axi2lnh_queue_credits_check;

    //запись исходных данных
        lnh_wr_reg64_byref(KEY2LNH, &key);
        lnh_wr_reg64_byval(CMD2LNH,(st_mreg<<lnh_mbox_st)|(val_mreg<<lnh_mbox_val)|(key_mreg<<lnh_mbox_key)|(NSM<<lnh_cmd)|str);

    //results
		return true;
}

//====================================================
// Больший (Структура, Ключ, Результат)
//====================================================

bool lnh_ngrm(int key_mreg, int val_mreg, int st_mreg, uint64_t str, uint64_t key){

    //проверка готовности устройства
	    lnh_axi2lnh_queue_credits_check;

    //запись исходных данных
        lnh_wr_reg64_byref(KEY2LNH, &key);
	    lnh_wr_reg64_byval(CMD2LNH,(st_mreg<<lnh_mbox_st)|(val_mreg<<lnh_mbox_val)|(key_mreg<<lnh_mbox_key)|(NGR<<lnh_cmd)|str);

    //results
		return true;
}

//====================================================
// MIN (Структура, Результат)
//====================================================

bool lnh_get_firstm(int key_mreg, int val_mreg, int st_mreg, uint64_t str){
    //проверка готовности устройства
	    lnh_axi2lnh_queue_credits_check;

    //запись исходных данных
	    lnh_wr_reg64_byval(CMD2LNH,(st_mreg<<lnh_mbox_st)|(val_mreg<<lnh_mbox_val)|(key_mreg<<lnh_mbox_key)|(FST<<lnh_cmd)|str);

    //results
		return true;
}

//====================================================
// MAX (Структура, Результат)
//====================================================

bool lnh_get_lastm(int key_mreg, int val_mreg, int st_mreg, uint64_t str){

    //проверка готовности устройства
	    lnh_axi2lnh_queue_credits_check;

    //запись исходных данных
    	lnh_wr_reg64_byval(CMD2LNH,(st_mreg<<lnh_mbox_st)|(val_mreg<<lnh_mbox_val)|(key_mreg<<lnh_mbox_key)|(LST<<lnh_cmd)|str);

    //results
		return true;
}

//====================================================
// Чтение результата из очереди
//====================================================

uint64_t lnh_get_m(int mreg) {

	//ожидание готовности очереди
    do {lnh_rd_reg32_byval(LNH_STATE_HIGH);} while (~lnh_core.buf32 & (1<<MBOX_VFLAG[mreg]) != 0) wait;
    
    //чтение результата
    lnh_rd_reg64_byval(MBOX[mreg]);
    return lnh_core.buf64;

}



//====================================================
// Команды без ожидания результата
//====================================================

//====================================================
// Добавление (Структура, Ключ, Значение)
//====================================================

bool lnh_ins_async(uint64_t str, uint64_t key, uint64_t value)
{
    //проверка готовности устройства
	    lnh_axi2lnh_queue_credits_check;

        //запись исходных данных
	    lnh_wr_reg64_byref(KEY2LNH, &key);
	    lnh_wr_reg64_byref(VAL2LNH, &value);
		lnh_wr_reg64_byval(CMD2LNH,(INS<<lnh_cmd)|str);

		return true;
}

//====================================================
// Удаление (Структура, Ключ)
//====================================================

bool lnh_del_async(uint64_t str, uint64_t key) {
    //проверка готовности устройства
	    lnh_axi2lnh_queue_credits_check;

    //запись исходных данных
	    lnh_wr_reg64_byref(KEY2LNH, &key);
		lnh_wr_reg64_byval(CMD2LNH,(DEL<<lnh_cmd)|str);

		return true;
}

//====================================================
// Удаление структуры (Структура)
//====================================================

bool lnh_del_str_async(uint64_t str){
    //проверка готовности устройства
	    lnh_axi2lnh_queue_credits_check;

    //запись исходных данных
	    lnh_wr_reg64_byval(CMD2LNH,(DELSTR<<lnh_cmd)|str);

	    return true;
}


//====================================================
// Сжатие (Структура)
//====================================================

bool lnh_sq_async(uint64_t str){
    //проверка готовности устройства
	    lnh_axi2lnh_queue_credits_check;

    //запись исходных данных
		lnh_wr_reg64_byval(CMD2LNH,(SQUIZ<<lnh_cmd)|str);

		return true;
}

//====================================================
// ИЛИ (Структура1, Структура2, Структура результата)
//====================================================

bool lnh_or_async(uint64_t A, uint64_t B, uint64_t R){
	    
        uint64_t str = (B<<2*lnh_s)|(A<<lnh_s)|(R<<0);
        
    //проверка готовности устройства
	    lnh_axi2lnh_queue_credits_check;

    //запись исходных данных
		lnh_wr_reg64_byval(CMD2LNH,(POR<<lnh_cmd)|str);

		return true;
}

//====================================================
// И (Структура1, Структура2, Структура результата)
//====================================================

bool lnh_and_async(uint64_t A, uint64_t B, uint64_t R){
        
        uint64_t str = (B<<2*lnh_s)|(A<<lnh_s)|(R<<0);
    
    //проверка готовности устройства
        lnh_axi2lnh_queue_credits_check;

    //запись исходных данных
		lnh_wr_reg64_byval(CMD2LNH,(PAND<<lnh_cmd)|str);

		return true;

}

//====================================================
// НЕ (Структура1, Структура2, Структура результата)
//====================================================

bool lnh_not_async(uint64_t A, uint64_t B, uint64_t R){
        
        uint64_t str = (B<<2*lnh_s)|(A<<lnh_s)|(R<<0);
        
    //проверка готовности устройства
	    lnh_axi2lnh_queue_credits_check;

    //запись исходных данных
		lnh_wr_reg64_byval(CMD2LNH,(PNOT<<lnh_cmd)|str);

		return true;
}

//====================================================
// LSEQ (Структура1, Структура2, Структура результата)
//====================================================

bool lnh_lseq_async(uint64_t key, uint64_t A, uint64_t R){
        
        uint64_t str = (A<<lnh_s)|(R<<0);
    
    //проверка готовности устройства
	    lnh_axi2lnh_queue_credits_check;

    //запись исходных данных
	    lnh_wr_reg64_byref(KEY2LNH, &key);
		lnh_wr_reg64_byval(CMD2LNH,(LSEQ<<lnh_cmd)|str);

		return true;
}

//====================================================
// LS (Структура1, Структура2, Структура результата)
//====================================================

bool lnh_ls_async(uint64_t key, uint64_t A, uint64_t R){
        
        uint64_t str = (A<<lnh_s)|(R<<0);
        
    //проверка готовности устройства
	    lnh_axi2lnh_queue_credits_check;

    //запись исходных данных
		lnh_wr_reg64_byref(KEY2LNH, &key);
		lnh_wr_reg64_byval(CMD2LNH,(LS<<lnh_cmd)|str);

		return true;
}

//====================================================
// GREQ (Структура1, Структура2, Структура результата)
//====================================================

bool lnh_greq_async(uint64_t key, uint64_t A, uint64_t R){
        
        uint64_t str = (A<<lnh_s)|(R<<0);
    
    //проверка готовности устройства
	    lnh_axi2lnh_queue_credits_check;

    //запись исходных данных
		lnh_wr_reg64_byref(KEY2LNH, &key);
		lnh_wr_reg64_byval(CMD2LNH,(GREQ<<lnh_cmd)|str);

		return true;
}

//====================================================
// GR (Структура1, Структура2, Структура результата)
//====================================================

bool lnh_gr_async(uint64_t key, uint64_t A, uint64_t R){

        uint64_t str = (A<<lnh_s)|(R<<0);
        
    //проверка готовности устройства
	    lnh_axi2lnh_queue_credits_check;
        
    //запись исходных данных
		lnh_wr_reg64_byref(KEY2LNH, &key);
		lnh_wr_reg64_byval(CMD2LNH,(GR<<lnh_cmd)|str);

		return true;
}

//=================================================================================
// GRLS (Ключ левой границы, Ключ правой границы, Структура1, Структура результата)
//=================================================================================

bool lnh_grls_async(uint64_t key_ls, uint64_t key_gr, uint64_t A, uint64_t R) {

        uint64_t str = (A<<lnh_s)|(R<<0);
    
    //проверка готовности устройства
	    lnh_axi2lnh_queue_credits_check;

    //запись исходных данных
		lnh_wr_reg64_byval(KEY2LNH, key_ls);
		lnh_wr_reg64_byval(VAL2LNH, key_gr);
		lnh_wr_reg64_byval(CMD2LNH, (GRLS<<lnh_cmd)|str);

		return true;
}

