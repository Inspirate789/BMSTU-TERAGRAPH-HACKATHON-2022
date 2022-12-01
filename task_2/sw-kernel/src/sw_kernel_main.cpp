/*
 * gpc_test.c
 *
 * sw_kernel library
 *
 *  Created on: April 23, 2021
 *      Author: A.Popov
 */

#include <stdlib.h>
#include "lnh64.h"
#include "gpc_io_swk.h"
#include "gpc_handlers.h"
#include "dijkstra.h"

#define VERSION 26
#define DEFINE_LNH_DRIVER
#define DEFINE_MQ_R2L
#define DEFINE_MQ_L2R
#define ROM_LOW_ADDR 0x00000000
#define ITERATIONS_COUNT    1
#define MEASURE_KEY_COUNT   1000000
#define __fast_recall__

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
            case __event__(frequency_measurement) : frequency_measurement(); break;
            case __event__(get_lnh_status_low) : get_lnh_status_low(); break;
            case __event__(get_lnh_status_high) : get_lnh_status_high(); break;
            case __event__(get_version): get_version(); break;
            case __event__(dijkstra): dijkstra(); break;
            case __event__(insert_edges): insert_edges(); break;
            case __event__(get_vertex_data): get_vertex_data(); break;
            case __event__(get_first_vertex): get_first_vertex(); break;
            case __event__(get_next_vertex): get_next_vertex(); break;
            case __event__(delete_graph): delete_graph(); break;
            case __event__(delete_visualization): delete_visualization(); break;
            case __event__(create_visualization): create_visualization(); break;
            case __event__(set_visualization_attributes): set_visualization_attributes(); break;
            case __event__(create_centrality_visualization): create_centrality_visualization(); break;
            case __event__(create_centrality_spiral_visualization): create_centrality_spiral_visualization(); break;
            case __event__(create_communities_forest_vizualization): create_communities_forest_vizualization(); break;
            case __event__(create_communities_forced_vizualization): create_communities_forced_vizualization(); break;
            case __event__(btwc): btwc(); break;

        }
        //Disable RW operations
        set_gpc_state(IDLE);
        while (gpc_start());

    }
}
    
//-------------------------------------------------------------
//      Глобальные переменные (для сокращения объема кода)
//-------------------------------------------------------------
    
        unsigned int LNH_key;
        unsigned int LNH_value;
        unsigned int LNH_status;
        uint64_t TSC_start;
        uint64_t TSC_stop;
        unsigned int interval;
        int i,j;
        unsigned int err=0;
     

//-------------------------------------------------------------
//      Измерение тактовой частоты GPN
//-------------------------------------------------------------
 
void frequency_measurement() {
    
        sync_with_host();
        lnh_sw_reset();
        lnh_rd_reg32_byref(TSC_LOW,&TSC_start);
        sync_with_host();
        lnh_rd_reg32_byref(TSC_LOW,&TSC_stop);
        interval = TSC_stop-TSC_start;
        mq_send(interval);

}


//-------------------------------------------------------------
//      Получить версию микрокода 
//-------------------------------------------------------------
 
void get_version() {
    
        mq_send(VERSION);

}
   

//-------------------------------------------------------------
//      Получить регистр статуса LOW Leonhard 
//-------------------------------------------------------------
 
void get_lnh_status_low() {
    
        lnh_rd_reg32_byref(LNH_STATE_LOW,&lnh_core.result.status);
        mq_send(lnh_core.result.status);

}

//-------------------------------------------------------------
//      Получить регистр статуса HIGH Leonhard 
//-------------------------------------------------------------
 
void get_lnh_status_high() {
    
        lnh_rd_reg32_byref(LNH_STATE_HIGH,&lnh_core.result.status);
        mq_send(lnh_core.result.status);

}

