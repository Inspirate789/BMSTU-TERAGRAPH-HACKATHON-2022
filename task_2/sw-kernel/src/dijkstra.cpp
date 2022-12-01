#include "dijkstra.h"


//-------------------------------------------------------------
// Удаление графа
//-------------------------------------------------------------

void delete_graph() {
    G.del_str_async();
}

//-------------------------------------------------------------
// Удаление визуализации графа
//-------------------------------------------------------------

void delete_visualization() {
    vG.del_str_async();
}


//-------------------------------------------------------------
// Создание визуализации графа в виде решетки 20x20
//-------------------------------------------------------------

void create_visualization() {

    matrix_layout(G,vG,X_MAX/100,Y_MAX/100);

}


//------------------------------------------------------------------------------
// Создание визуализации графа на основе обхода матрицы
//------------------------------------------------------------------------------

void create_centrality_visualization() {

    matrix_spiral_layout(G, vG, Q);


}

//------------------------------------------------------------------------------
// Создание визуализации графа на основе обхода по спирали центральности
//------------------------------------------------------------------------------

void create_centrality_spiral_visualization() {

    spiral_layout(G, vG, Q);

}

//------------------------------------------------------------------------------
// Выделение сильно связанных сообществ на основе модулярности
//------------------------------------------------------------------------------

void create_communities_forced_vizualization() {


    /************************************************************************************
     *
        I - Инициализация структур
     *
    *************************************************************************************/

    init_communities(G, cG, mQ, iQ);


    /************************************************************************************
     *
        II - Выделение сообществ до максимального значения модулярности
     *
    *************************************************************************************/


    merge_communities(G, cG, mQ, iQ);


    /************************************************************************************
     *

        III - Создание графа из сообществ и распределение сообществ силовым алгоритмом
     *
    *************************************************************************************/

    force_directed_layout_communities(cG, mQ, iQ, X_MAX, Y_MAX);




    /**************************************************************************************
     *

        IV - Определить координаты для вершин сообществ
     *
    ***************************************************************************************/


    set_community_boxes(cG, X_MAX, Y_MAX);


    /**************************************************************************************
     *
        V - Обход вершин сообществ и формирование графа визуализации
     *
    ***************************************************************************************/

    //Удаление структуры визуализации
    vG.del_str_async();
    //Копирование графа G -> vG
    G.greq_sync(Graph::Key{.index = 0, .u = 0}, vG.struct_number);


    force_directed_layout_vertices(cG, vG, Q);


    //удаление структуры сообществ
    cT.del_str_async();
    //удаление очереди модулярности
    mQ.del_str_async();
    //удаление структуры индексов
    iQ.del_str_async();
    //удаление очереди
    Q.del_str_async();
    //удаление очереди
    cG.del_str_async();

}


//------------------------------------------------------------------------------
// Выделение сильно связанных сообществ на основе модулярности
//------------------------------------------------------------------------------

void create_communities_forest_vizualization() {


    /************************************************************************************
     *
        I - Инициализация структур
     *
    *************************************************************************************/

    init_communities(G, cG, mQ, iQ);


    /************************************************************************************
     *
        II - Выделение сообществ до максимального значения модулярности
     *
    *************************************************************************************/


    merge_communities(G, cG, mQ, iQ);


    /************************************************************************************
     *
        III - Создание дерева сообществ для формирования блочной раскладки визуализации
     *
    *************************************************************************************/


    make_communities_tree(G, cG, mQ, iQ, cT, Q);


    /**************************************************************************************
     *
        IV - Рекурсивный обход дерева сообществ и определение границ прямоугольных областей
     *
    ***************************************************************************************/

    set_box_bounds(cT);


    /**************************************************************************************
     *
        V - Обход вершин сообществ и формирование графа визуализации
     *
    ***************************************************************************************/


    generate_boxes_graph(G, Q, cG, vG, cT);


    //удаление очереди модулярности
    mQ.del_str_async();
    //удаление структуры индексов
    iQ.del_str_async();

}




//-------------------------------------------------------------
// Получение информации о вершине
//-------------------------------------------------------------
/*
    Функция получает номер вершины и читает информацию из lnh64.
    Информация возвращается через очередь сообщений:
    @1 - Adj_c - количество ребер / 0 - если вершина не найдена
    @2 - Предыдущая вершина в кратчайшем пути
    @3 - Кратчайший путь
    @4 - Центральность
    @5 - Координата x
    @6 - Координата y
    @7 - Размер вершины size
    @8 - Цвет вершины color
    @9 - Для каждого ребра:
     @9.1 - Индекс вершины v
     @9.2 - Вес ребра w
*/
void get_vertex_data () {

    //Переменные
    unsigned int du, pu, btwc;
    short int adj_c, x, y;
    unsigned char size;
    unsigned int color;
    unsigned int u = mq_receive();
    //Чтение u из lnh64
    auto vertex = vG.search(Graph::Base_key{.u = u});
    if (vertex) {
        mq_send(vertex.value().adj_c);
        //pu
        mq_send(vertex.value().pu);
        auto vertex_atr = vG.search(Graph::Path_key{.u = u});
        //du
        mq_send(vertex_atr.value().du);
        //btwc
        mq_send(vertex_atr.value().btwc);
        auto vertex_vatr = vG.search(Graph::Viz_key{.u = u});
        mq_send(vertex_vatr.value().x);
        mq_send(vertex_vatr.value().y);
        mq_send(vertex_vatr.value().size);
        mq_send(((vertex_vatr.value().color) << 8) | ALPHA_DEFAULT);
        for (auto e:edge_range{vG,u}){
            mq_send(e.v);
            mq_send(e.w);
        }
    } else
        mq_send(false);
}


//-------------------------------------------------------------
// Получение информации о первой вершине
//-------------------------------------------------------------
/*
    Функция получает номер вершины и и ищет следующую в lnh64.
    Информация возвращается через очередь сообщений:
    @1 - true - вершина найдена / false - вершина не найдена
    @2 - Индекс вершины
    @3 - Adj_c - количество ребер / 0 - если вершина не найдена
    @4 - Предыдущая вершина в кратчайшем пути
    @5 - Кратчайший путь
    @6 - Центральность
    @7 - Для каждого ребра:
    @7.1 - Индекс вершины v
    @7.2 - Вес ребра w
*/
void get_first_vertex () {

    auto v = vG.get_first();
    if (v) {
        mq_send(true);
        mq_send(v.key().u);
    } else
        mq_send(false);

}


//-------------------------------------------------------------
// Получение информации о следующей вершине
//-------------------------------------------------------------
/*
    Функция получает номер вершины и и ищет следующую в lnh64.
    Информация возвращается через очередь сообщений:
    @1 - true - вершина найдена / false - вершина не найдена
    @2 - Индекс вершины
    @3 - Adj_c - количество ребер / 0 - если вершина не найдена
    @4 - Предыдущая вершина в кратчайшем пути
    @5 - Кратчайший путь
    @6 - Центральность
    @7 - Для каждого ребра:
    @7.1 - Индекс вершины v
    @7.2 - Вес ребра w
*/
void get_next_vertex () {

    //Переменные
    unsigned int u = mq_receive();
    //Поиск следующей вершины в  lnh64
    auto n = vG.ngr(Graph::Key{.index=Graph::idx_max,.u = u});
    if (n) {
        mq_send(true);
        mq_send(n.key().u);
    } else
        mq_send(false);

}


//-------------------------------------------------------------
// Изменение атрибутов визуализации графа
//-------------------------------------------------------------
/*
    Функция изменяет атрибуты для указанной вершины
    Функция получает через очередь сообщений:
    Информация возвращается через очередь сообщений:
    @1 - Индекс вершины
    @2 - координату x
    @3 - координату y
    @4 - размер вершины
    @5 - цвет вершины
*/

void set_visualization_attributes () {

    //Переменные
    unsigned int u = mq_receive();
    unsigned short int x = mq_receive();
    unsigned short int y = mq_receive();
    unsigned short int size = mq_receive();
    unsigned int color = mq_receive();
    vG.ins_async(Graph::Viz_key{.u=u},Graph::vAttributes{.x = x, .y = y, .size = size, .color = color});
}

//-------------------------------------------------------------
// Создание случайного графа
//-------------------------------------------------------------
/*
    Функция читает буфер buffer_pointer размером size байт
    Формат записи буфера: (u,v,w),(u,v,w),...,(u,v,w).
*/

void insert_edges () {

    //Переменные
    unsigned int u, v, du, pu, adj, dv, count = 0;
    short int wu, adj_c;
    bool eQ, eQc;
    unsigned int i;
    //get buffer pointer and size
    unsigned int buffer_pointer = mq_receive();
    unsigned int size = mq_receive();
    //read u and v, then insert (u,v) into the graph
    for (unsigned int i = buffer_pointer; i < buffer_pointer + size; i += 3 * sizeof(int)) {
        unsigned int    u = *((volatile unsigned int*)(AXI4EXTMEM_BASE + i));
        unsigned int    v = *((volatile unsigned int*)(AXI4EXTMEM_BASE + i + sizeof(int)));
        short int       w = *((volatile unsigned int*)(AXI4EXTMEM_BASE + i + 2 * sizeof(int)));
        if (v != u) {
            //u->v edge
            auto record = G.search(Graph::Base_key{.u = u});
            if (record) {
                bool flag = false;
                for (auto e : edge_range(G, u)) {
                    if (e.v == v) {
                        flag = true;
                        break;
                    }
                }
                if (!flag) {
                    G.ins_sync(Graph::Key{.index = (unsigned int)record.value().adj_c, .u = u}, Graph::Edge{.v = v, .w = w, .attr = 0});
                    G.ins_sync(Graph::Base_key{.u = u}, Graph::Attributes{.pu = u, .eQ = true, .non = 0, .adj_c = (short int)(record.value().adj_c + 1)});
                    G.ins_sync(Graph::Path_key{.u = u}, Graph::Shortest_path{.du = Graph::inf, .btwc = 0});
                }
            } else {
                G.ins_sync(Graph::Key{.index = 0, .u = u}, Graph::Edge{.v = v, .w = w, .attr = 0});
                G.ins_sync(Graph::Base_key{.u = u}, Graph::Attributes{.pu = u, .eQ = true, .non = 0, .adj_c = 1});
                G.ins_sync(Graph::Path_key{.u = u}, Graph::Shortest_path{.du = Graph::inf, .btwc = 0});
            }
        }
    }
    G.sq_sync();
}



//-------------------------------------------------------------
// Алгоритм Дейкстры
//-------------------------------------------------------------

void dijkstra_core(unsigned int start_vertex) {
    //Очистка очереди
    Q.del_str_async();
    //добавление стартовой вершины с du=0
    Q.ins_async(Queue::Record{.id = start_vertex, .du = 0}, Queue::Attributes{});
    //Get btwc to store it again
    auto [du, btwc] = G.search(Graph::Path_key{.u = start_vertex}).value();
    //Save du for start vertex
    G.ins_async(Graph::Path_key{.u = start_vertex}, Graph::Shortest_path{.du = 0, .btwc = btwc});
    //обход всех вершины графа
    while(auto q_it = Q.rbegin()) {
        Q.erase(q_it);
	    auto [u, du] = *q_it;
        //Get pu, |Adj|, eQ
        auto result = G.search(Graph::Base_key{.u = u});
        auto [pu, eQ, non, adj_c] = result.value();
        // Clear eQ
        G.ins_async(result.key(), Graph::Attributes{.pu = pu, .eQ = false, .non = 0, .adj_c = adj_c});
        //For each Adj
        for (auto [adj, wu, attr] : edge_range(G, u)) {
            //Get information about Adj[i]
            auto [adj_pu, eQc, non, count] = G.search(Graph::Base_key{.u = adj}).value();
            auto [dv, btwc] = G.search(Graph::Path_key{.u = adj}).value();
            //Change distance
            if (dv > (du + wu)) {
                if (eQc) {
                    //if not loopback, push to Q
                    if (dv != Graph::inf) {
                        Q.del_async(Queue::Record{.id = adj, .du = dv});
                    }
                    Q.ins_async(Queue::Record{.id = adj, .du = du + wu}, Queue::Attributes{});
                }
                //change du
                G.ins_async(Graph::Path_key{.u = adj}, Graph::Shortest_path{.du = du + wu, .btwc = btwc});
                //change pu
                G.ins_async(Graph::Base_key{.u = adj}, Graph::Attributes{.pu = u, .eQ = eQc, .non = 0, .adj_c = count});
            }
        }
    }
}

void dijkstra () {
    //Создание стартовой вершины u=[0,0]
    unsigned int start_vertex = mq_receive(); //получаем номер старотовой вершины
    //Создание стоповой вершины v
    unsigned int stop_vertex = mq_receive(); //получаем номер стоповой вершины
    //Основной алгоритм
    dijkstra_core(start_vertex);
    //send shortest path
    mq_send(G.search(Graph::Path_key{.u = stop_vertex}).value().du);
}

//-------------------------------------------------------------
// Центральность
//-------------------------------------------------------------

void btwc () {
    //Iterate u
    for (Graph::vertex_t u : vertex_range{G}) {
        //Start Dijksra shortest path search
        dijkstra_core(u);
        //Iterate v
        for (Graph::vertex_t v : vertex_range{G}) {
            //For undirected graphs needs to route 1/2 shortest paths (u<v)
            if (u != v) {
                auto du = G.search(Graph::Path_key{.u = v}).value().du;
                //If there is a route to u
                if (du != Graph::inf) {
                    //Get pu
                    auto pu = G.search(Graph::Base_key{.u = v}).value().pu;
                    while (pu != u) {
                        //Get btwc
                        auto [du, btwc] = G.search(Graph::Path_key{.u = pu}).value();
                        //Write btwc, set du by the way
                        G.ins_async(Graph::Path_key{.u = pu}, Graph::Shortest_path{.du = du, .btwc = btwc + 1});
                        //Route shortest path
                        pu = G.search(Graph::Base_key{.u = pu}).value().pu;
                    }
                }
            }
        }
        //Init graph again
        for (Graph::vertex_t v : vertex_range{G}) {
            //Init graph again
            auto adj_c = G.search(Graph::Base_key{.u = v}).value().adj_c;
            G.ins_sync(Graph::Base_key{.u = v}, Graph::Attributes{.pu = v, .eQ = true, .non = 0, .adj_c = adj_c});
            auto btwc = G.search(Graph::Path_key{.u = v}).value().btwc;
            G.ins_sync(Graph::Path_key{.u = v}, Graph::Shortest_path{.du = Graph::inf, .btwc = btwc});
        }
    }
}
