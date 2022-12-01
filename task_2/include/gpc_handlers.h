/*
 * gpc_handlers.h
 *
 * host and sw_kernel library
 *
 * Macro instantiation for handlers
 * !!! This file should be identical for sw_kernel and host libraries !!!
 *
 */
#ifndef DEF_HANDLERS_H_
#define DEF_HANDLERS_H_
#define DECLARE_EVENT_HANDLER(handler) \
            const unsigned int event_ ## handler =__LINE__; \
            void handler ();
#define __event__(handler) event_ ## handler
//  Event handlers declarations by declaration line number 
DECLARE_EVENT_HANDLER(frequency_measurement);
DECLARE_EVENT_HANDLER(get_lnh_status_low);
DECLARE_EVENT_HANDLER(get_lnh_status_high);
DECLARE_EVENT_HANDLER(get_version);
DECLARE_EVENT_HANDLER(insert_edges);
DECLARE_EVENT_HANDLER(dijkstra);
DECLARE_EVENT_HANDLER(get_vertex_data);
DECLARE_EVENT_HANDLER(delete_graph);
DECLARE_EVENT_HANDLER(btwc);
DECLARE_EVENT_HANDLER(delete_visualization);
DECLARE_EVENT_HANDLER(create_visualization);
DECLARE_EVENT_HANDLER(get_first_vertex);
DECLARE_EVENT_HANDLER(get_next_vertex);
DECLARE_EVENT_HANDLER(set_visualization_attributes);
DECLARE_EVENT_HANDLER(create_centrality_visualization); 
DECLARE_EVENT_HANDLER(create_centrality_spiral_visualization); 
DECLARE_EVENT_HANDLER(create_communities_forest_vizualization);
DECLARE_EVENT_HANDLER(create_communities_forced_vizualization);
#endif
