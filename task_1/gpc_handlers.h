/*
 * gpc_handlers.h
 *
 * host and sw_kernel library
 *
 * Macro instantiation for handlers
 * !!! This file should be identical for sw_kernel and host libraries !!!
 *
 * Created on: April 23, 2021
 * Author: A.Popov
 */
#ifndef DEF_HANDLERS_H_
#define DEF_HANDLERS_H_
#define DECLARE_EVENT_HANDLER(handler) \
            const unsigned int event_ ## handler =__LINE__; \
            void handler ();
#define __event__(handler) event_ ## handler
//  Event handlers declarations by declaration line number!!! 
DECLARE_EVENT_HANDLER(insert_burst);
DECLARE_EVENT_HANDLER(search_burst);

#endif
