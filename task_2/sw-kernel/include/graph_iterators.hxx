#ifndef GITERS_H_
#define GITERS_H_


#include <stdlib.h>
#include <stdint.h>
#include <cmath>
#include <cassert>
#include "lnh64.hxx"
#include "map.h"
#include "gpc_io_swk.h"
#include "compose_keys.hxx"
#include "vector.hxx"


extern lnh lnh_core;
extern global_memory_io gmio;

template<typename Range>
struct reverse {
        Range r;
        [[gnu::always_inline]] reverse(Range r) : r(r) {}
        [[gnu::always_inline]] auto begin() {return r.rbegin();}
        [[gnu::always_inline]] auto end() {return r.rend();}
};

template<typename K, typename V>
struct Handle {
        bool ret_val;
        K k{get_result_key<K>()};
        V v{get_result_value<V>()};
        [[gnu::always_inline]] Handle(bool ret_val) : ret_val(ret_val) {
        }

        [[gnu::always_inline]] operator bool() const {
                return ret_val;
        }

        [[gnu::always_inline]] K key() const {
                return k;
        }

        [[gnu::always_inline]] V value() const {
                return v;
        }
};

///////////////////////////////////
// Граф  и граф визуализации
///////////////////////////////////


struct Graph {
        using vertex_t = uint32_t;
        int struct_number;
        constexpr Graph(int struct_number) : struct_number(struct_number) {}
        static const uint32_t adj_c_bits = 32;
        static const uint32_t idx_max = (1ull << adj_c_bits) - 1;
        static const uint32_t pth_idx = idx_max;
        static const uint32_t base_idx = idx_max - 1;
        static const uint32_t viz_idx = idx_max - 2;
	static const uint32_t inf     = UINT32_MAX; //значение бесконечности для задания неинициализированного значения пути
        static const uint32_t placement_idx = idx_max - 3;
        static const uint32_t displacement_idx = idx_max - 4;
        static const uint32_t idx_min = idx_max - 4;

        //регистр ключа для вершины:
        /* STRUCT(u_key) - G - описание графа
         * key[63..32] - номер вершины
         * key[31..0] -  индекс записи о вершине (0,1..adj_u)
         */
        STRUCT(Key) {
                unsigned int index: adj_c_bits;
                vertex_t     u: 32;
        };
        STRUCT(Path_key) {
                unsigned int index: adj_c_bits = pth_idx;
                vertex_t     u: 32;
        };
        STRUCT(Base_key) {
                unsigned int index: adj_c_bits = base_idx;
                vertex_t     u: 32;
        };
        //граф визуализации
        STRUCT(Viz_key) {
                unsigned int index: adj_c_bits = viz_idx;
                vertex_t     u: 32;
        };
        STRUCT(Plc_key) { //Data structure for graph operations
                unsigned int index: 32 = placement_idx;       
                unsigned int u : 32;   
        };
        STRUCT(Displc_key) { //Data structure for graph operations
                unsigned int index: 32 = displacement_idx;       
                unsigned int u : 32;  
        };

        //регистр значения для записей о смежных вершинах:
        /*
         * key[INDEX] = 0..IDX_MAX-3
         * data[15..0] - w[u,v] вес ребра
         * data[47..16] - Adj[u]
         * atr[63..48] - vertex atributes
         */
        STRUCT(Edge) {
                vertex_t     v: 32;
                short int    w: 16;
                short int    attr: 16;
        };

        //регистр значения индексной записи для вершины (с индексом PTH_IDX):
        /* key[16..0] = PTH_IDX
        * data[31..0] - d[u] - кратчайший путь
        */
        STRUCT(Shortest_path) {
                vertex_t     du: 32;
                unsigned int btwc: 32;
        };

        //регистр значения атрибутов для вершины с индексом BASE_IDX: STRUCT(u_attributes)
        /* для поля key[31..0] =  BASE_IDX
        * data[31..0] - p[u] - pred nomer vershini v kratchajshem puti
        * data[40..32] - 1 - u is in Q; 0 is not in Q
        * data[63..48] - |Adj[u]| - kol-vo svjazej s vershinoj u
        */
        STRUCT(Attributes) {
                unsigned int pu: 32;
                bool         eQ: 8;
                int          non: 8 = 0;
                short int    adj_c: 16;
        };

        //регистр значения для записи атрибутов визуализации вершинах
        /*
         * key[INDEX] = IDX_MAX-2
         * data[15..0]  - координата x визуализации вершины
         * data[31..16] - координата y визуализации вершины
         * data[23..24] - Adj[u]
         * atr[63..48] - vertex atributes
         */
        STRUCT(vAttributes) { //Data structure for graph operations
                unsigned short int                              x: 16;          //Поле 1: координата x [0..64K]
                unsigned short int                              y: 16;          //Поле 2: координата y [0..64K]
                unsigned short int                              size: 8;        //Поле 3: размер [0..255]
                unsigned int                                    color: 24;      //Поле 4: цвет [0x00000000..0xFFFFFFFF]
        };
        //регистр значения индексной записи для вершины (с индексом placement_idx)
        /*
        * data[31..0] - координата x
        * data[63..32] - координата y
        */
        struct Plc_val:iVector{int x:32; int y:32;};

        //регистр значения индексной записи для вершины (с индексом displacement_idx)
        /*
        * data[31..0] - изменение координаты x
        * data[63..32] - изменение координаты y
        */
        struct Displc_val:iVector{int x:32; int y:32;};
        //Обязательная типизация
        DEFINE_DEFAULT_KEYVAL(Key, Edge)
        //Дополнительная типизация
        DEFINE_KEYVAL(Base_key, Attributes)
        DEFINE_KEYVAL(Path_key, Shortest_path)
        DEFINE_KEYVAL(Viz_key, vAttributes)
        DEFINE_KEYVAL(Plc_key, iVector)
        DEFINE_KEYVAL(Displc_key, iVector)
};

struct vertex_sentinel {};
struct vertex_iterator {
        Graph G1;
        Handle<Graph::Key, Graph::Edge> uh; //Текущая вершина
        [[gnu::always_inline]] vertex_iterator(Graph G1, Handle<Graph::Key, Graph::Edge> uh) : G1(G1), uh(uh) {}

        [[gnu::always_inline]] Graph::vertex_t operator*() const {
                return uh.key().u;
        }
        [[gnu::always_inline]] vertex_iterator& operator++() { //Переход к следующей вершине
                uh = G1.ngr(Graph::Key{.index = Graph::idx_max, .u = uh.key().u});
                return *this;
        }

        [[gnu::always_inline]] bool operator==(const vertex_iterator rhs) {
                assert(G1.struct_number == rhs.G1.struct_number);
                return (((!uh) && (!rhs.uh)) || (**this == *rhs));
        }

        //Достигнут ли конец
        [[gnu::always_inline]] bool operator==(const vertex_sentinel rhs) {
                return (!uh);
        }
};


struct vertex_range {
        Graph G1;
        [[gnu::always_inline]] vertex_range(Graph G1) : G1(G1) {}
        [[gnu::always_inline]] auto begin() {return vertex_iterator(G1, G1.get_first());}
        [[gnu::always_inline]] auto end() {return vertex_sentinel{};}
};



struct edge_sentinel {};
struct edge_iterator {
        Graph G1;
        Graph::vertex_t u; //Исходная вершина
        Handle<Graph::Key, Graph::Edge> vh; //Текущее ребро
        [[gnu::always_inline]] edge_iterator(Graph G1, Graph::vertex_t u, Handle<Graph::Key, Graph::Edge> vh) : G1(G1), u(u), vh(vh) {}


        [[gnu::always_inline]] Graph::Edge operator*() const {
                return vh.value();
        }


        [[gnu::always_inline]] edge_iterator& operator++() { //Обход всегда в обратном порядке
                vh = G1.nsm(vh.key());
                return *this;
        }


        [[gnu::always_inline]] bool operator==(const edge_iterator rhs) {
                assert(u == rhs.vh.key().u); //Сравнение итераторов ребер разных вершин не имеет смысла
                return ((!vh) || (u != vh.key().u));
        }

        //Достигнут ли конец
        [[gnu::always_inline]] bool operator==(const edge_sentinel rhs) {
                return ((!vh) || (u != vh.key().u));
        }

};
struct edge_range {
        Graph G;
        Graph::vertex_t u;
        [[gnu::always_inline]] edge_range(Graph G, Graph::vertex_t u) : G(G), u(u) {}
        [[gnu::always_inline]] auto begin() {return edge_iterator(G, u, G.nsm(Graph::Key{.index = Graph::idx_min, .u = u}));}
        [[gnu::always_inline]] auto end() {return edge_sentinel{};}
};


///////////////////////////////////
// Очередь для алгоритма Дейкстры
///////////////////////////////////
template <bool ascending_order> struct queue_iterator;
struct queue_sentinel;
struct Queue {
        using vertex_t = uint32_t;
        int struct_number;
        constexpr Queue(int struct_number) : struct_number(struct_number) {}

        //регистр ключа для записей очереди
        /*
         * Struktura 2 - Q - ochered'
         * key[31..0] - nomer vershini
         * key[63..32] - d[u] kratchajshij put'
        */
        STRUCT(Record) { //Data structure for queue operations
                vertex_t       id: 32;       //Поле 0: индекс вершины
                uint32_t       du: 32;      //Поле 1: кратчайший путь
        };

        STRUCT(Attributes) {
                uint64_t       attr: 64 = 0ull;
        };

        //Обязательная типизация
        DEFINE_DEFAULT_KEYVAL(Record, Attributes)
	[[gnu::always_inline]] inline queue_iterator<true> begin() const;
	[[gnu::always_inline]] inline queue_sentinel end() const;
	[[gnu::always_inline]] inline queue_iterator<false> rbegin() const;
	[[gnu::always_inline]] inline queue_sentinel rend() const;
	template <bool ascending_order>
	[[gnu::always_inline]] inline void erase(queue_iterator<ascending_order> it) const;
};

struct queue_sentinel {};
template <bool ascending_order>
struct queue_iterator {
        Queue Q;
        Handle<Queue::Record, Queue::Attributes> qh; //Текущая вершина
        [[gnu::always_inline]] queue_iterator(Queue Q, Handle<Queue::Record, Queue::Attributes> qh) : Q(Q), qh(qh) {}
        [[gnu::always_inline]] queue_iterator() {}

        [[gnu::always_inline]] Queue::Record operator*() const {
                return qh.key();
        }
        [[gnu::always_inline]] queue_iterator& operator++() {
                qh = ascending_order ? Q.ngr(qh.key()) : Q.nsm(qh.key());
                return *this;
        }

        [[gnu::always_inline]] bool operator==(const queue_iterator rhs) = delete; //TODO

        //Достигнут ли конец
        [[gnu::always_inline]] bool operator==(const queue_sentinel rhs) {
                return (!qh);
        }
	[[gnu::always_inline]] operator bool() const {return qh;}
};

[[gnu::always_inline]] inline queue_iterator<true> Queue::begin() const {return {*this, get_first()};}
[[gnu::always_inline]] inline queue_sentinel Queue::end() const{return {};}
[[gnu::always_inline]] inline queue_iterator<false> Queue::rbegin() const{return {*this, get_last()};}
[[gnu::always_inline]] inline queue_sentinel Queue::rend() const{return {};}
template <bool ascending_order>
[[gnu::always_inline]] inline void Queue::erase(queue_iterator<ascending_order> it) const {del_sync(*it);}

///////////////////////////////////
// Запись сообщества
///////////////////////////////////


struct alignas(uint64_t) Community {
        using vertex_t = uint32_t;
        const int struct_number;
        static const uint32_t adj_c_bits = 32;
        static const uint32_t idx_max = (1ull << adj_c_bits) - 1;
        static const uint32_t placement_idx = idx_max;
        static const uint32_t displacement_idx = idx_max-1;
        static const uint32_t space_idx = idx_max-2;
        static const uint32_t idx_min = idx_max-2;
        Graph gh;
        [[gnu::always_inline]] constexpr Community(int struct_number, Graph gh) 
		: struct_number(struct_number), gh(gh) {}
        //static const uint32_t com_idx = CMTY_IDX; //XXX: Нигде не используется???

        //регистр ключа для вершины в графе сообществ
        /*
         * key[63..32] - номер сообщества
         * key[31..0] -  количество ребер сообщества (adj)
         */
        STRUCT(Key) { //Data structure for graph operations
                unsigned int                                        adj: 32;        //Поле 0: кратность вершины сообщества
                unsigned int                                        id: 32;         //Поле 1: номер сообщества
        };
        //регистр ключа для вершины в графе сообществ
        /*
         * key[63..32] - номер сообщества
         * key[31..0] -  количество ребер сообщества (adj)
         */
        STRUCT(Plc_key) { //Data structure for graph operations
                unsigned int                                        adj: 32 = placement_idx;       
                unsigned int                                        id : 32;   
        };

        //регистр ключа для вершины в графе сообществ
        /*
         * key[63..32] - номер сообщества
         * key[31..0] -  количество ребер сообщества (adj)
         */
        STRUCT(Displc_key) { //Data structure for graph operations
                unsigned int                                        adj: 32 = displacement_idx;       
                unsigned int                                        id : 32;   
        };

        //регистр ключа для вершины в графе сообществ
        /*
         * key[63..32] - номер сообщества
         * key[31..0] -  количество ребер сообщества (adj)
         */
        STRUCT(Space_key) { //Data structure for graph operations
                unsigned int                                        adj: 32 = space_idx;       
                unsigned int                                        id : 32;   
        };
        //регистр значения индексной записи для вершины (с индексом CMTY_IDX)
        /*
        * data[31..0] - первая верина в цепочке вершин графа G
        * data[63..32] - последняя верина в цепочке вершин графа G
        */
        STRUCT(Value) { //Data structure for graph operations
                vertex_t                                            first_vertex: 32;
                vertex_t                                            last_vertex: 32;
        };
        //регистр значения индексной записи для вершины (с индексом placement_idx)
        /*
        * data[31..0] - координата x
        * data[63..32] - координата y
        */
        struct Plc_val:iVector{int x:32; int y:32;};

        //регистр значения индексной записи для вершины (с индексом displacement_idx)
        /*
        * data[31..0] - изменение координаты x
        * data[63..32] - изменение координаты y
        */
        struct Displc_val:iVector{int x:32; int y:32;};
 
        //регистр значения индексной записи для вершины (с индексом space_idx)
        /*
        * data[31..0] - размер области сообщества (диаметр)
        * data[63..32] - количество вершин в сообществе
        */
        STRUCT(Space) { //Data structure for graph operations
                uint32_t                                            d: 32;
                uint32_t                                            vertex_count: 32;
        };

        //Обязательная типизация
        DEFINE_DEFAULT_KEYVAL(Key, Value)
        DEFINE_KEYVAL(Plc_key, iVector)
        DEFINE_KEYVAL(Displc_key, iVector)
        DEFINE_KEYVAL(Space_key, Space)
};

struct community_sentinel {};
struct community_iterator {
        Community C1;
        Handle<Community::Key, Community::Value> ch; //Текущее сообщество
        [[gnu::always_inline]] community_iterator(Community C1, Handle<Community::Key, Community::Value> ch) : C1(C1.struct_number, C1.gh), ch(ch) {}

        [[gnu::always_inline]] Handle<Community::Key, Community::Value> operator*() const {
                return ch;
        }

        [[gnu::always_inline]] community_iterator& operator++() {
                ch = C1.ngr(Community::Key{.adj=Community::idx_max,.id=ch.key().id});
                return *this;
        }

        [[gnu::always_inline]] bool operator==(const community_iterator rhs) {
                return (!ch);
        }

        //Достигнут ли конец
        [[gnu::always_inline]] bool operator==(const community_sentinel rhs) {
                return (!ch);
        }
};
struct community_range {
        Community C1;
        [[gnu::always_inline]] community_range(Community C1) : C1(C1) {}
        [[gnu::always_inline]] auto begin() {return community_iterator(C1, C1.get_first());}
        [[gnu::always_inline]] auto end() {return community_sentinel{};}
};


struct community_member_sentinel {};
struct community_member_iterator {
        Community C1;
        Graph::vertex_t uh; //Текущая вершина графа в сообществе
        bool finished;
        [[gnu::always_inline]] community_member_iterator(Community C1, Graph::vertex_t uh, bool f) : C1(C1.struct_number, C1.gh), uh(uh), finished(f) {}
        [[gnu::always_inline]] Graph::vertex_t operator*() const { return uh;}
        [[gnu::always_inline]] community_member_iterator& operator++() {
                Graph::vertex_t uhpu = C1.gh.search(Graph::Base_key{.u = uh}).value().pu;
                if (uh == uhpu) {finished = true;} else {uh = uhpu;}
                return *this;
        }
        [[gnu::always_inline]] bool operator==(const community_member_sentinel rhs) { return ( finished );}

};

struct community_member_range {
        Community C1;
        unsigned int id;
        [[gnu::always_inline]] community_member_range(Community C1, unsigned int id) : C1(C1), id(id) {}
        [[gnu::always_inline]] auto begin() {
                auto tmp = C1.nsm(Community::Key{.adj = Community::idx_min, .id = id});
                if ((!tmp) || (tmp.key().id != id)) {return community_member_iterator(C1, tmp.value().first_vertex, true);} //Нет такого сообщества
                else {return community_member_iterator(C1, tmp.value().first_vertex, false);}
        }
        [[gnu::always_inline]] auto end() { return community_member_sentinel{}; }
};



/////////////////////////////////////////
// Двоичное дерево визуализации сообществ
/////////////////////////////////////////



struct cTree {
        int struct_number;
        constexpr cTree(int struct_number) : struct_number(struct_number) {}
        static const uint32_t adj_c_bits = 32;
        static const uint32_t idx_max = (1ull << adj_c_bits) - 1;
        static const uint32_t cmty_vcount_idx = idx_max; //номер индексной записи о количестве вершин сообщества
        static const uint32_t cmty_xy_idx = idx_max-1; //номер индексной записи о границах области визуализации сообщества


        //регистр ключа для вершины в графе сообществ
        /*
         * key[63..32] - номер сообщества
         * key[31..0] -  количество ребер сообщества (adj)
         */
        STRUCT(Key) { //Data structure for graph operations
                unsigned int                                            adj: adj_c_bits;
                unsigned int                                            com_id: 32;
        };
        STRUCT(Vcount_key) { //Data structure for graph operations
                unsigned int                                            index: adj_c_bits = cmty_vcount_idx;
                unsigned int                                            com_id: 32;
        };
        STRUCT(XY_key) { //Data structure for graph operations
                unsigned int                                            index: adj_c_bits = cmty_xy_idx;
                unsigned int                                            com_id: 32;
        };

        //регистр значения индексной записи для вершины (с индексом Adj)
        /*
        * data[31..0] - Номер левого сообщества в поддереве
        * data[63..32] - Номер правого сообщества в поддереве
        */
        //регистр значения индексной записи для вершины (с индексом Adj) для листа дерева (is_leaf=true)
        /*
        * data[31..0] - первая верина в цепочке вершин графа G
        * data[63..32] - последняя верина в цепочке вершин графа G
        */
        STRUCT(Value) { //Data structure for graph operations
                unsigned int                                            left_leaf: 32;
                unsigned int                                            right_leaf: 32;
        };

        //регистр значения индексной записи для вершины (с индексом CMTY_VCOUNT_IDX)
        /* com_viz_u_key[31..0] = CMTY_VCOUNT_IDX,
        * data[31..0] - количество вершин в сообществе
        * data[39..32] - флаг конечного листа двичного дерева сообществ
        * data[63..32] - не используется

        */
        STRUCT(Vcount_value) { //Data structure for graph operations
                unsigned int                                            v_count: 32;
                bool                                                    is_leaf: 8;
                unsigned int                                            non: 24;
        };

        //регистр значения индексной записи для вершины (с индексом CMTY_XY_IDX)
        /* com_viz_u_key[31..0] = CMTY_XY_IDX,
        * data[15..0] - левая верхняя координата x [0..4095]
        * data[63..32] - не используется

        */
        STRUCT(XY_value) { //Data structure for graph operations
                unsigned short int                                      x0: 16;
                unsigned short int                                      y0: 16;
                unsigned short int                                      x1: 16;
                unsigned short int                                      y1: 16;
        };

        //Обязательная типизация
        DEFINE_DEFAULT_KEYVAL(Key, Value)
        //Дополнительная типизация
        //DEFINE_KEYVAL(Key, Value_leaf)
        DEFINE_KEYVAL(Vcount_key, Vcount_value)
        DEFINE_KEYVAL(XY_key, XY_value)

};


struct ctree_sentinel {};
struct ctree_iterator {
        cTree cT;
        Handle<cTree::Key, cTree::Value> th; //Текущая вершина
        [[gnu::always_inline]] ctree_iterator(cTree cT, Handle<cTree::Key, cTree::Value> th) : cT(cT), th(th) {}

        [[gnu::always_inline]] Handle<cTree::Key, cTree::Value> operator*() const {
                return th;
        }
        [[gnu::always_inline]] ctree_iterator& operator++() { //Переход к следующей вершине
                th = cT.ngr(cTree::Key{.adj = cTree::idx_max, .com_id = th.key().com_id});
                return *this;
        }

        [[gnu::always_inline]] bool operator==(const ctree_iterator rhs) {
                assert(cT.struct_number == rhs.cT.struct_number);
                return ((( !th) && (!rhs.th)) || (**this == *rhs));
        }

        //Достигнут ли конец
        [[gnu::always_inline]] bool operator==(const ctree_sentinel rhs) {
                return (!th);
        }
};


struct ctree_range {
        cTree cT;
        [[gnu::always_inline]] ctree_range(cTree cT) : cT(cT) {}
        [[gnu::always_inline]] auto begin() {return ctree_iterator(cT, cT.get_first());}
        [[gnu::always_inline]] auto end() {return ctree_sentinel{};}
};


// ////////////////////////////////////////////
// // Очередь модулярности и структура индексов
// ////////////////////////////////////////////


struct modularity {
        int struct_number;
        constexpr modularity(int struct_number) : struct_number(struct_number) {}


        //регистр ключа для очереди модулярности / регистр значения для структуры индексов
        /*
         * Struktura 2 - Q - ochered'
         * Key[0]     - номер записи о модулярности (0..1)
         * key[31..1] - индекс записи о модулярности
         * key[63..32] - изменение модулярности при объединении сообществ u и v
        */
        STRUCT(Modularity_ext) { //Data structure for queue operations
                unsigned char                                   index: 1 = 0;
                unsigned int                                    id: 31;
                int                                             delta_mod: 32;
        };
        STRUCT(Modularity) { //Data structure for queue operations
                unsigned char                                   index: 1 = 1;
                unsigned int                                    id: 31;
                int                                             delta_mod: 32;
        };

        //регистр атрибутов для очереди модулярности
        /*
         * value[31..0] - номер сообщества u
         * value[63..32] - номер сообщества v
        */
        STRUCT(Attributes) { //Data structure for queue operations
                unsigned int                                    w_u_v: 32;
                unsigned int                                    non: 32;
        };
        //регистр значения для очереди модулярности / регистр ключа для структуры индексов
        /*
         * value[31..0] - номер сообщества u
         * value[63..32] - номер сообщества v
        */
        STRUCT(Communities) { //Data structure for queue operations
                unsigned int                                    com_u: 32;
                unsigned int                                    com_v: 32;
        };

};

struct mQueue: modularity {

	//Метод для удаления обоих записей (с ключами Modularity и Modularity_ext
	[[gnu::always_inline]] void del_ext_async(unsigned int id, int delta_mod) const {
		del_async(Modularity{.id = id, .delta_mod = delta_mod});
		del_async(Modularity_ext{.id = id, .delta_mod = delta_mod});
	}	

	//Метод для вставки обоих записей (с ключами Modularity и Modularity_ext
	[[gnu::always_inline]] void ins_ext_async(unsigned int id, int delta_mod, unsigned int w_u_v, 
			unsigned int com_u, unsigned int com_v) const {
		ins_async(Modularity{.id = id, .delta_mod = delta_mod}, 
				Communities{.com_u = com_u, .com_v = com_v});
		ins_async(Modularity_ext{.id = id, .delta_mod = delta_mod}, 
				Attributes{.w_u_v = w_u_v, .non = 0});
	}
        //Дополнительная типизация
        DEFINE_DEFAULT_KEYVAL(Modularity, Communities)
        //Обязательная типизация
        DEFINE_KEYVAL(Modularity_ext, Attributes)

};

struct iQueue: modularity {
        //Обязательная типизация
        DEFINE_DEFAULT_KEYVAL(Communities, Modularity)
};

struct alignas(uint64_t) mQueue_record {
                unsigned int                                    com_u;
                unsigned int                                    com_v;
                unsigned int                                    id = 0; //31 бит
                int                                             delta_mod;
                unsigned int                                    w_u_v;
};

struct mqueue_sentinel {};
struct mqueue_iterator {
        mQueue mQ;
        Handle<modularity::Modularity, modularity::Communities> mh;             //Текущая запись

        [[gnu::always_inline]] mqueue_iterator(mQueue mQ, 
			Handle<modularity::Modularity, modularity::Communities> mh) 
		: mQ(mQ), mh(mh) {}

        [[gnu::always_inline]] mQueue_record operator*() const {
                auto w_u_v = mQ.search(modularity::Modularity_ext{.id=mh.key().id,.delta_mod=mh.key().delta_mod}).value().w_u_v;
                return {.com_u=mh.value().com_u,.com_v=mh.value().com_v,.id=mh.key().id,.delta_mod=mh.key().delta_mod,.w_u_v=w_u_v};
        }

	//На теккущий момент итератор используется для получения информации в 
	//виде mQueue_record - для симметрии с mqueue_range::push_back
	//TODO: реализовать перемещение по очереди 
        [[gnu::always_inline]] mqueue_iterator& operator++() = delete; //TODO
        [[gnu::always_inline]] bool operator==(const mqueue_iterator rhs) = delete; //TODO
        [[gnu::always_inline]] bool operator==(const mqueue_sentinel rhs) = delete; //TODO

	//Достигнут ли конец очереди?
	[[gnu::always_inline]] operator bool() const {return mh;}
};
struct mqueue_range {
        mQueue mQ;
        iQueue iQ;
        [[gnu::always_inline]] mqueue_range(mQueue mQ, iQueue iQ) : mQ(mQ), iQ(iQ) {}
        [[gnu::always_inline]] auto rbegin() {return mqueue_iterator(mQ, mQ.get_last_signed());}
        [[gnu::always_inline]] auto rend() {return mqueue_sentinel{};}
	[[gnu::always_inline]] auto begin() = delete; //TODO
	[[gnu::always_inline]] auto end() = delete; //TODO
	[[gnu::always_inline]] void push_back(mQueue_record val) const{
		//Определить номер в очереди за последним индексом для сегмента очереди delta_mod
		auto last_modularity = mQ.nsm(modularity::Modularity{.index = 0, .id = 0, 
				.delta_mod = val.delta_mod + 1});
		unsigned int mq_index;
		if (last_modularity && (last_modularity.key().delta_mod == val.delta_mod)) {
			mq_index = last_modularity.key().id + 1;
		} else {
			mq_index = 0;
		}
		//Добавить запись об атрибутах в очередь
		mQ.ins_ext_async(mq_index, val.delta_mod, val.w_u_v, val.com_u, val.com_v);

		//Добавить запись в структуру индексов
		iQ.ins_async(modularity::Communities{.com_u = val.com_u, .com_v = val.com_v}, 
				modularity::Modularity{.id = mq_index, .delta_mod = val.delta_mod});
	}

	[[gnu::always_inline]] void push_back(mQueue_record val1, mQueue_record val2) const{ //TODO: объединить в один 
		//Определить номер в очереди за последним индексом для сегмента очереди delta_mod
		auto last_modularity = mQ.nsm(modularity::Modularity{.index = 0, .id = 0, 
				.delta_mod = val1.delta_mod + 1});
		unsigned int mq_index;
		if (last_modularity && (last_modularity.key().delta_mod == val1.delta_mod)) {
			mq_index = last_modularity.key().id + 1;
		} else {
			mq_index = 0;
		}
		//Добавить запись об атрибутах в очередь
		mQ.ins_ext_async(mq_index, val1.delta_mod, val1.w_u_v, val1.com_u, val1.com_v);

		//Добавить запись в структуру индексов
		iQ.ins_async(modularity::Communities{.com_u = val1.com_u, .com_v = val1.com_v}, 
				modularity::Modularity{.id = mq_index, .delta_mod = val1.delta_mod});

		//Добавить запись об атрибутах в очередь
		mQ.ins_ext_async(mq_index + 1, val2.delta_mod, val2.w_u_v, val2.com_u, val2.com_v);

		//Добавить запись в структуру индексов
		iQ.ins_async(modularity::Communities{.com_u = val2.com_u, .com_v = val2.com_v}, 
				modularity::Modularity{.id = mq_index + 1, .delta_mod = val2.delta_mod});
	}
	//TODO: Принимать итератор
	[[gnu::always_inline]] void update_modularity(Handle<modularity::Communities,modularity::Modularity> r,
							int new_delta_mod, unsigned int new_w_u_v) const {
		auto [com_u,com_v] = r.key();
		auto [not_used, uv_index, uv_delta_mod] = r.value();

		//Удалить запись о модулярности связи сообществ u и v
		mQ.del_ext_async(uv_index, uv_delta_mod);

		//Получить атрибуты связности сообществ v и u
		auto [not_used_1, vu_index, vu_delta_mod] = iQ.search(modularity::Communities{.com_u = com_v, .com_v = com_u}).value();

		//Удалить запись о модулярности связи сообществ v и u
		mQ.del_ext_async(vu_index, vu_delta_mod);

		//Добавить записи об атрибутах связи сообществ u->v и v->u в очередь и в индекс
		push_back(  mQueue_record{.com_u = com_u, .com_v = com_v,
				.delta_mod = new_delta_mod, .w_u_v = new_w_u_v},
				mQueue_record{.com_u = com_v, .com_v = com_u,
				.delta_mod = new_delta_mod, .w_u_v = new_w_u_v}
			     );
	}
	//TODO: Принимать итератор
	[[gnu::always_inline]] void delete_modularity(Handle<modularity::Communities,modularity::Modularity> r) const {
		auto [com_u,com_v] = r.key();
		auto [not_used, uv_index, uv_delta_mod] = r.value();
		//Получить атрибуты связности сообществ v и u
		auto [not_used_1, vu_index, vu_delta_mod] = iQ.search(modularity::Communities{.com_u = com_v, .com_v = com_u}).value();
		//Удалить запись о модулярности связи сообществ u и v
		mQ.del_ext_async(uv_index, uv_delta_mod);
		iQ.del_async(modularity::Communities{.com_u = com_u, .com_v = com_v});
		//Удалить запись о модулярности связи сообществ v и u
		mQ.del_ext_async(vu_index, vu_delta_mod);
		iQ.del_async(modularity::Communities{.com_u = com_v, .com_v = com_u});
	}

	//TODO: Принимать итератор
	[[gnu::always_inline]] void delete_modularity(Handle<modularity::Modularity, modularity::Communities> r) const {
		auto [com_u,com_v] = r.value();
		auto [not_used, uv_index, uv_delta_mod] = r.key();
		//Получить атрибуты связности сообществ v и u
		auto [not_used_1, vu_index, vu_delta_mod] = iQ.search(modularity::Communities{.com_u = com_v, .com_v = com_u}).value();
		//Удалить запись о модулярности связи сообществ u и v
		mQ.del_ext_async(uv_index, uv_delta_mod);
		iQ.del_async(modularity::Communities{.com_u = com_u, .com_v = com_v});
		//Удалить запись о модулярности связи сообществ v и u
		mQ.del_ext_async(vu_index, vu_delta_mod);
		iQ.del_async(modularity::Communities{.com_u = com_v, .com_v = com_u});
	}

	//TODO: выбрать один интерфейс
	[[gnu::always_inline]] void erase(mqueue_iterator it) {
		delete_modularity(it.mh);
	}


};

struct iqueue_sentinel {};
struct iqueue_iterator {
        iQueue iQ1;
        unsigned int c;                                                         //Текущее сообщество
        Handle<modularity::Communities,modularity::Modularity> ih;              //Текущая запись

        [[gnu::always_inline]] iqueue_iterator(iQueue iQ1, unsigned int c, Handle<modularity::Communities,modularity::Modularity> ih) : iQ1(iQ1), c(c), ih(ih) {
        }

        [[gnu::always_inline]] Handle<modularity::Communities,modularity::Modularity> operator*() const {
                return ih;
        }

        [[gnu::always_inline]] iqueue_iterator& operator++() {
                ih = iQ1.nsm(ih.key());
                return *this;
        }

        [[gnu::always_inline]] bool operator==(const iqueue_iterator rhs) {
                return ((!ih) || (ih.key().com_v!=c) );
        }

        //Достигнут ли конец
        [[gnu::always_inline]] bool operator==(const iqueue_sentinel rhs) {
                return ((!ih) || (ih.key().com_v!=c) );
        }
};
struct iqueue_range {
        iQueue iQ1;
        unsigned int c;
        [[gnu::always_inline]] iqueue_range(iQueue iQ1, unsigned int c) : iQ1(iQ1), c(c) {}
        [[gnu::always_inline]] auto begin() {return iqueue_iterator(iQ1, c, iQ1.nsm(modularity::Communities{.com_u=0,.com_v=c+1}));}
        [[gnu::always_inline]] auto end() {return iqueue_sentinel{};}
};

#endif
