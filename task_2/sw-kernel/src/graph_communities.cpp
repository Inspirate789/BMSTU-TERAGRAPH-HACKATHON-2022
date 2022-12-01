#include "graph_communities.h"

//-----------------------------------------------------------------------
// Инициализация структуры сообществ и очередей модулярности по графу
//-----------------------------------------------------------------------
void init_communities(Graph G, Community cG, mQueue mQ, iQueue iQ) {

	short int adj_c, adj_ic;
	short int wu;
	unsigned int w, w_sum = 0;
	unsigned int m = 0;
	unsigned int n = 0;
	int delta_mod = 0;
	int modularity = 0;
	unsigned int btwc, btwc_max = 0;
	mqueue_range mqr{mQ, iQ};


	//Удаление структуры сообществ
	cG.del_str_async();
	//удаление очереди модулярности
	mQ.del_str_async();
	//удаление структуры индексов
	iQ.del_str_async();
	//Обход всех вершин графа для определения количества полуребер (2*m) и количества вершин (n)
	for (auto com_u : vertex_range{G}) {
		//Инициировать цепь записей о вершинах в сообществе в поле pu.
		auto adj_c = G.search(Graph::Base_key{.u = com_u}).value().adj_c;
		//Для каждого ребра создать запись в очередь модулярности и в структуру индексов
		for (auto e : edge_range(G, com_u)) {
			w_sum += e.w;
		}
		//Добавить кратность вершины к m
		m += adj_c;
		n++;
	}
	//Средний вес ребра = сумма весов всех полуребер / количество полуребер
	w = w_sum / m;
	//Обход всех вершин графа, создание очереди модулярности, структуры индексов и структуры сообществ, вычисление среднего веса ребра
	for (auto com_u : vertex_range{G}) {
		adj_c = G.search(Graph::Base_key{.u = com_u}).value().adj_c;
		//В начале алгоритма номер вершины совпадает с начальным сообществом. Записываем в поле pu для создания цепочки вершин.
		G.ins_async(Graph::Base_key{.u = com_u}, Graph::Attributes{.pu = com_u, .eQ = false, .non = 0, .adj_c = adj_c});
		//Создать запись о сообщесте, начальная и конечные вершины указывают на вершину с аналогичным номером
		cG.ins_async(Community::Key{.adj = (unsigned int)adj_c, .id = com_u}, Community::Value{.first_vertex = com_u, .last_vertex = com_u});
		//Создать запись о количестве вершин в сообществе
		cG.ins_async(Community::Space_key{.id = com_u}, Community::Space{.d = 0, .vertex_count = 1});
		//Для каждого ребра создать запись в очередь модулярности и в структуру индексов
		for (auto [com_v, wu, attr] : edge_range(G, com_u)) {
			//Определить степень инцидентной вершины
			adj_ic = G.search(Graph::Base_key{.u = com_v}).value().adj_c;
			//Вычислить целочисленную модулярность вершин при объединении
			delta_mod = m * wu - w * adj_c * adj_ic;
			//Добавить записи об атрибутах связи сообществ v->u в очередь и в индекс
			mqr.push_back(mQueue_record{.com_u = com_v, .com_v = com_u,
			                            .id = 0, .delta_mod = delta_mod, .w_u_v = (unsigned int)wu});
		}
	}
	//Отладочное сообщение
#ifdef DEBUG
	lnh_sync();
	mq_send(-1);
	unsigned int mq_ = mQ.get_num();
	unsigned int cg_ = cG.get_num();
	mq_send(mq_);
	mq_send(cg_);
	mq_send(n);
#endif
	//Отладочное сообщение
#ifdef DEBUG
	mq_send(0);
#endif


}


//-----------------------------------------------------------------------
// Объединение сообществ по свойству модулярности
//-----------------------------------------------------------------------
void merge_communities(Graph G, Community cG, mQueue mQ, iQueue iQ) {

	int mod = 0;
	mqueue_range mqr{mQ, iQ};

	//Основной цикл
	while (auto mq_it = mqr.rbegin())  {
		//u,v - номера объединяемых сообществ
		//w_u_v - атрибут связности u и v
		auto [com_v, com_u, com_u_index_val, com_u_delta_mod, w_u_v] = *mq_it;
		if (com_u_delta_mod < 0)
			break;
		//Удалить запись о модулярности связи сообществ u<->v
		mqr.erase(mq_it);
		//Обновить модуляность
		mod += com_u_delta_mod;
		//Отладочное сообщение
#ifdef DEBUG
		mq_send(-1);
		mq_send(com_u);
		mq_send(com_v);
		mq_send(com_u_delta_mod);
		mq_send(mod);
#endif
		//Получить информацию о вершинах сообщества v
		auto cmty_v = cG.nsm(Community::Key{.adj = Community::idx_min, .id = com_v});
		auto adj_v = cmty_v.key().adj;
		auto [first_vertex_v, last_vertex_v] = cmty_v.value();
		auto vertex_count_u = cG.search(Community::Space_key{.id = com_u}).value().vertex_count;
		auto vertex_count_v = cG.search(Community::Space_key{.id = com_v}).value().vertex_count;
		//Удалить сообщество v и структуры сообществ
		cG.del_async(Community::Key{.adj = adj_v, .id = com_v});
		cG.del_async(Community::Space_key{.id = com_v});
		//Получить информацию о вершинах сообщества u
		auto cmty_u = cG.nsm(Community::Key{.adj = Community::idx_min, .id = com_u});
		auto adj_u = cmty_u.key().adj;
		auto [first_vertex_u, last_vertex_u] = cmty_u.value();
		//Удалить сообщество u из структуры сообществ / Запись Space_key удалять не обязательно
		cG.del_async(Community::Key{.adj = adj_u, .id = com_u});
		//Объединить цепочки записей сообществ в графе
		auto adj_cc = G.search(Graph::Base_key{.u = last_vertex_u}).value().adj_c;
		G.ins_async(Graph::Base_key{.u = last_vertex_u}, Graph::Attributes{.pu = first_vertex_v, .eQ = false, .non = 0, .adj_c = adj_cc});
		//Обновить информацию о конечной вершине цепочки для сообщества u
		cG.ins_async(Community::Key{.adj = adj_u + adj_v - 2, .id = com_u}, Community::Value{.first_vertex = first_vertex_u, .last_vertex = last_vertex_v});
		//Создать запись о количестве вершин в сообществе
		cG.ins_async(Community::Space_key{.id = com_u}, Community::Space{.d = 0, .vertex_count = vertex_count_u + vertex_count_v});
		//Обойти все записи о сообществе u в очереди модулярности
		for (auto iQ_record : iqueue_range(iQ, com_u)) {
			auto com_k = iQ_record.key().com_u;
			auto [not_used, com_k_index_val, com_k_delta_mod] = iQ_record.value();
			//Отладочное сообщение
#ifdef DEBUG
			mq_send(-2);
			mq_send(com_k);
			mq_send(com_u);
			mq_send(com_k_delta_mod);
#endif
			//Для всех кандидатов, кроме com_u
			if (com_k != com_v)  {
				//Получить атрибуты сообщества из очереди модулярности
				auto w_k_v = mQ.search(modularity::Modularity_ext{.id = com_k_index_val, .delta_mod = com_k_delta_mod}).value().w_u_v;
				//Если сообщество com_k связано и с com_v тоже, то обновление уже произведено
				if (iQ.search(modularity::Communities{.com_u = com_v, .com_v = com_k})) {
					//Обновление будет произведено для v
					//Do nothing
				} else {
					//Если сообщество com_k не связано с com_v
					//Сложить изменения модуляности (Aaron Clauset, M.E.J. Newman and Cristopher Moore. Finding community structure in very large networks, 10.c)
					mqr.update_modularity(iQ_record,
					                      com_k_delta_mod - 2 * adj_u * adj_v * w_u_v,
					                      w_k_v);
				}
			}
		}
		//Обойти все записи о сообществе u в очереди модулярности
		for (auto iQ_record : iqueue_range(iQ, com_v)) {
			auto com_k = iQ_record.key().com_u;
			auto [not_used, com_k_index_val, com_k_delta_mod] = iQ_record.value();
			//Отладочное сообщение
#ifdef DEBUG
			mq_send(-2);
			mq_send(com_k);
			mq_send(com_v);
			mq_send(com_k_delta_mod);
#endif
			//Для всех кандидатов, кроме com_u
			if (com_k != com_u)  {
				//Получить атрибуты сообщества из очереди модулярности
				auto w_k_v = mQ.search(modularity::Modularity_ext{.id = com_k_index_val, .delta_mod = com_k_delta_mod}).value().w_u_v;

				//Удалить запись о модулярности связи сообществ v <-> k
				mqr.delete_modularity(iQ_record);

				//Если сообщество com_k связано и с com_u тоже,
				auto iQ_record1 = iQ.search(modularity::Communities{.com_u = com_u, .com_v = com_k});
				if (iQ_record1) {
					//Получить атрибуты связности сообществ k и u
					auto [not_used, mq_index, delta_mod] = iQ_record1.value();
					auto w_k_u = mQ.search(modularity::Modularity_ext{.id = mq_index, .delta_mod = delta_mod}).value().w_u_v;

					//Сложить изменения модуляности (Aaron Clauset, M.E.J. Newman and Cristopher Moore. Finding community structure in very large networks, 10.a)
					mqr.update_modularity(iQ_record1,
					                      com_k_delta_mod + delta_mod,
					                      w_k_u + w_k_v);
				} else {
					//Если сообщество com_k не связано с com_u

					//Сложить изменения модуляности (Aaron Clauset, M.E.J. Newman and Cristopher Moore. Finding community structure in very large networks, 10.b)
					com_k_delta_mod = com_k_delta_mod - 2 * adj_u * adj_v * w_u_v;

					//Добавить запись об атрибутах связи сообществ u->k и k->u в очередь и в индекс
					mqr.push_back(  mQueue_record{.com_u = com_u, .com_v = com_k,
					                              .delta_mod = com_k_delta_mod, .w_u_v = w_k_v},
					                mQueue_record{.com_u = com_k, .com_v = com_u,
					                              .delta_mod = com_k_delta_mod, .w_u_v = w_k_v}
					             );
				}
			}
		}
	}
	//Отладочное сообщение
#ifdef DEBUG
	mq_send(0);
#endif

	//Отладочное сообщение
#ifdef DEBUG
	////////////////////////////////
	for (auto c : community_range(cG)) {
		mq_send(-1);
		mq_send(c.key().id);
		mq_send(c.value().first_vertex);
		mq_send(c.value().last_vertex);
		for (auto v : community_member_range(cG, c.key().id)) {
			mq_send(-1);
			mq_send(v);
		}
		mq_send(0);
	}
	mq_send(0);
	////////////////////////////////
#endif


}


//-----------------------------------------------------------------------
// Продолжить объединение сообществ для создания древовидной раскладки
//-----------------------------------------------------------------------
void make_communities_tree(Graph G, Community cG, mQueue mQ, iQueue iQ, cTree cT, Queue Q) {

	int mod = 0;
	mqueue_range mqr{mQ, iQ};
	uint32_t vertex_count;
	uint32_t com_r;

	//Отладочное сообщение
#ifdef DEBUG
	mq_send(-3);
	mq_send(mQ.get_num());
	mq_send(cG.get_num());
#endif

	//Удалить структуру двоичного дерева визуализации сообществ
	cT.del_str_async();
	//Создание начального значения для индексных записей
	for (auto cmty : community_range{cG}) {
		auto [adj_u, com_u] = cmty.key();
		auto [first_vertex_u, last_vertex_u] = cmty.value();
		//Добавить индексные записи / Для записей сообществ листов дерева принят аналогичный cG формат индекса (com_u_index), для остальных вершин дерева (com_viz_index)
		cT.ins_async(cTree::Key{.adj = adj_u, .com_id = com_u}, cTree::Value{.left_leaf = first_vertex_u, .right_leaf = last_vertex_u});

		//Создать запись о количестве вершин в сообществе
		vertex_count = cG.search(Community::Space_key{.id = com_u}).value().vertex_count;
		//Добавление запись в очередь
		Q.ins_async(Queue::Record{.id = com_u, .du = vertex_count}, Queue::Attributes{});
		//Добавить индексные записи
		cT.ins_async(cTree::Vcount_key{.com_id = com_u}, cTree::Vcount_value{.v_count = vertex_count, .is_leaf = true, .non = 0});
		cT.ins_async(cTree::XY_key{.com_id = com_u}, cTree::XY_value{.x0 = 0, .y0 = 0, .x1 = 0, .y1 = 0});
	}

	//Основной цикл для связанных сообществ
	while (auto mq_it = mqr.rbegin())  {
		//u,v - номера объединяемых сообществ
		//w_u_v - атрибут связности u и v
		auto [com_v, com_u, com_u_index_val, com_u_delta_mod, w_u_v] = *mq_it;
		//Удалить запись о модулярности связи сообществ u<->v
		mqr.erase(mq_it);

		//Обновить модуляность - не является обязательным
		mod += com_u_delta_mod;
		//Удалить информацию о сообществе com_v из очереди
		auto com_v_atributes = cT.search(cTree::Vcount_key{.com_id = com_v}).value();
		Q.del_async(Queue::Record{.id = com_v, .du = com_v_atributes.v_count});
		//Удалить информацию о сообществе com_u из очереди
		auto com_u_atributes = cT.search(cTree::Vcount_key{.com_id = com_u}).value();
		Q.del_async(Queue::Record{.id = com_u, .du = com_u_atributes.v_count});
		vertex_count = com_v_atributes.v_count + com_u_atributes.v_count;
		//Получить информацию о вершинах сообщества v
		auto adj_v = cT.nsm(cTree::XY_key{.com_id = com_v}).key().index;
		auto adj_u = cT.nsm(cTree::XY_key{.com_id = com_u}).key().index;
		//Определить номер нового сообщества
		com_r = cT.get_last().key().com_id + 1;
		cT.ins_async(cTree::Key{.adj = adj_u + adj_v - 2, .com_id = com_r}, cTree::Value{.left_leaf = com_u, .right_leaf = com_v});
		cT.ins_async(cTree::Vcount_key{.com_id = com_r}, cTree::Vcount_value{.v_count = vertex_count, .is_leaf = false, .non = 0});
		cT.ins_async(cTree::XY_key{.com_id = com_r}, cTree::XY_value{.x0 = 0, .y0 = 0, .x1 = 0, .y1 = 0});
		//Добавление запись в очередь
		Q.ins_async(Queue::Record{.id = com_r, .du = vertex_count}, Queue::Attributes{});
		//Отладочное сообщение
#ifdef DEBUG
		mq_send(-4);
		mq_send(com_u);
		mq_send(com_v);
		mq_send(com_u_delta_mod);
		mq_send(mod);
		mq_send(vertex_count);
		mq_send(com_r);
#endif

		for (auto iQ_record : iqueue_range(iQ, com_u)) {
			// //Обойти все записи о сообществе u в очереди модулярности
			auto com_k = iQ_record.key().com_u;
			auto [not_used, com_k_index_val, com_k_delta_mod] = iQ_record.value();
			//Для всех кандидатов, кроме com_u
			if (com_k != com_v) {
				//Получить атрибуты сообщества из очереди модулярности
				auto w_k_v = mQ.search(modularity::Modularity_ext{.id = com_k_index_val, .delta_mod = com_k_delta_mod});
				//Если сообщество com_k связано и с com_v тоже, то обновление уже произведено
				auto iQ_record1 = iQ.search(modularity::Communities{.com_u = com_v, .com_v = com_k});
				if (iQ_record1) {
					//Обновление уже произведено
					//Do nothing
				} else {
					//Если сообщество com_k не связано с com_v
					//Удалить запись о модулярности связи сообществ u<->k
					mqr.delete_modularity(iQ_record);
					//Сложить изменения модуляности (Aaron Clauset, M.E.J. Newman and Cristopher Moore. Finding community structure in very large networks, 10.c)
					com_k_delta_mod = com_k_delta_mod - 2 * adj_u * adj_v * w_u_v;

					//Добавить запись об атрибутах связи сообществ r<->k в очередь и в индекс
					mqr.push_back(  mQueue_record{.com_u = com_r, .com_v = com_k,
					                              .delta_mod = com_k_delta_mod, .w_u_v = w_k_v},
					                mQueue_record{.com_u = com_k, .com_v = com_r,
					                              .delta_mod = com_k_delta_mod, .w_u_v = w_k_v}
					             );
				}
			}
		}
		//Обойти все записи о сообществе v в очереди модулярности
		for (auto iQ_record : iqueue_range(iQ, com_v)) {
			auto com_k = iQ_record.key().com_u;
			auto [not_used, com_k_index_val, com_k_delta_mod] = iQ_record.value();
			//Для всех кандидатов, кроме com_u
			if (com_k != com_u)  {
				//Получить атрибуты сообщества из очереди модулярности
				auto w_k_v = mQ.search(modularity::Modularity_ext{.id = com_k_index_val, .delta_mod = com_k_delta_mod}).value().w_u_v;

				//Удалить запись о модулярности связи сообществ v<->k
				mqr.delete_modularity(iQ_record);

				//Если сообщество com_k связано и с com_u тоже
				auto iQ_record1 = iQ.search(modularity::Communities{.com_u = com_u, .com_v = com_k});
				if (iQ_record1) {
					auto [not_used, mq_index, delta_mod] = iQ_record1.value();
					//Получить атрибуты связности сообществ k и u
					auto w_k_u = mQ.search(modularity::Modularity_ext{.id = mq_index, .delta_mod = delta_mod}).value().w_u_v;

					//Сложить изменения модуляности (Aaron Clauset, M.E.J. Newman and Cristopher Moore. Finding community structure in very large networks, 10.a)
					com_k_delta_mod = com_k_delta_mod + delta_mod;
					//Удалить запись о модулярности связи сообществ k<->u
					mqr.delete_modularity(iQ_record1);
				} else {
					//Если сообщество com_k не связано с com_u
					//Сложить изменения модуляности (Aaron Clauset, M.E.J. Newman and Cristopher Moore. Finding community structure in very large networks, 10.b)
					com_k_delta_mod = com_k_delta_mod - 2 * adj_u * adj_v * w_u_v;

				}
				//Добавить запись об атрибутах связи сообществ r<->k в очередь и в индекс

				mqr.push_back(  mQueue_record{.com_u = com_r, .com_v = com_k,
				                              .delta_mod = com_k_delta_mod, .w_u_v = w_k_v},
				                mQueue_record{.com_u = com_k, .com_v = com_r,
				                              .delta_mod = com_k_delta_mod, .w_u_v = w_k_v}
				             );
			}
		}
	}

	// Если есть несвязанные сообщества
	if (Q.get_num() != 0)  {
		//Если дерево сообществ не пустое, то добавить вершину дерева сообществ в очередь 
		if (cT.get_num() != 0) Q.ins_async(Queue::Record{.id = com_r, .du = vertex_count}, Queue::Attributes{}); 
		//Связать сообщества в порядке увеличения количества вершин
		while (auto q_first = Q.begin()) {
			Q.erase(q_first);
			auto q_second = Q.begin();
			if (q_second) {
				Q.erase(q_second);
				//u,v - номера объединяемых сообществ
				auto [u, u_count] = *q_first;
				auto [v, v_count] = *q_second;

				//Определить номер нового сообщества
				auto r = cT.get_last().key().com_id + 1;
				cT.ins_async(cTree::Key{.adj = 0, .com_id = r}, cTree::Value{.left_leaf = u, .right_leaf = v});
				cT.ins_async(cTree::Vcount_key{.com_id = r}, cTree::Vcount_value{.v_count = u_count + v_count, .is_leaf = false, .non = 0});
				cT.ins_async(cTree::XY_key{.com_id = r}, cTree::XY_value{.x0 = 0, .y0 = 0, .x1 = 0, .y1 = 0});
				//Добавить сообщество в очередь
				Q.ins_async(Queue::Record{.id = r, .du = u_count + v_count}, Queue::Attributes{});
			}
		}
	}

	//Отладочное сообщение
#ifdef DEBUG
	mq_send(0);
#endif
}
