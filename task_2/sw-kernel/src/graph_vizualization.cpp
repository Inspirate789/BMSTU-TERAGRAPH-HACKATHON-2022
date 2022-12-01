#include "graph_vizualization.h"

//-----------------------------------------------------------------------
// Определение цвета в формате RGB (24 бит) по температуре (1000..12000K)
//-----------------------------------------------------------------------

unsigned int kelvin2rgb(unsigned int temperature) {

	//Temperature must fall between 1000 and 40000 degrees
	if (temperature < 1000) return kelvin_to_rgb[0];
	if (temperature > 12000) return kelvin_to_rgb[110];
	return kelvin_to_rgb[(temperature - 1000) / 100];

}


//-----------------------------------------------------------------------
// Определение цвета в формате RGB (24 бит) по интенсивности (0..255)
//-----------------------------------------------------------------------

unsigned int violet2rgb(unsigned int uv) {

	if (uv > 255) uv = 255;
	return (uv << 16) | ((uv >> 2) << 8) | ((0xFF - uv) << 0);
}


//------------------------------------------------------------------------------
// Рекурсивная функция для определения границ визуализации сообществ
//------------------------------------------------------------------------------

void set_xy(cTree cT, unsigned int comunity, unsigned int x0, unsigned int y0, unsigned int x1, unsigned int y1, bool horisontal) {

	//unsigned int left_leaf, right_leaf;
	//unsigned int left_v_count, right_v_count;
	unsigned int left_x0, left_y0, left_x1, left_y1;
	unsigned int right_x0, right_y0, right_x1, right_y1;
	bool is_leaf;
	unsigned int left_factor;


	//Получить информацию о кратности сообщества
	auto viz = cT.search(cTree::Vcount_key{.com_id = comunity});
	is_leaf = viz.value().is_leaf;

	//Отладочное сообщение
#ifdef DEBUG
	mq_send(-5);
	mq_send(comunity);
	//Получить информацию о сообществе
	mq_send(viz.value().v_count);
	mq_send(x0);
	mq_send(y0);
	mq_send(x1);
	mq_send(y1);
	mq_send(is_leaf);
#endif

	if (is_leaf) {
		cT.ins_async(cTree::XY_key{.com_id = comunity}, cTree::XY_value{.x0 = (short unsigned int)x0, .y0 = (short unsigned int)y0, .x1 = (short unsigned int)x1, .y1 = (short unsigned int)y1});
	} else {
		//Получить информацию о левом и правом сообществах-потомках
		auto viz = cT.nsm(cTree::Key{.adj = cTree::cmty_xy_idx, .com_id = comunity});
		auto [left_leaf, right_leaf] = viz.value();
		//Запись о сообществе можно удалять
		cT.del_async(cTree::Key{viz.key()});
		cT.del_async(cTree::Vcount_key{.com_id = comunity});
		cT.del_async(cTree::XY_key{.com_id = comunity});
		//Получить информацию о левом сообществе
		auto left_v_count = cT.search(cTree::Vcount_key{.com_id = left_leaf}).value().v_count;
		//Получить информацию о правом сообществе
		auto right_v_count = cT.search(cTree::Vcount_key{.com_id = right_leaf}).value().v_count;
		//Соотношение площадей
		left_factor = (100 * left_v_count) / (left_v_count + right_v_count);
		if (horisontal) {
			//Left
			left_x0 = x0;
			left_y0 = y0;
			left_x1 = x0 + (left_factor * (x1 - x0)) / 100;
			left_y1 = y1;
			set_xy(cT, left_leaf, left_x0, left_y0, left_x1, left_y1, false);
			//Right
			right_x0 = x0 + (left_factor * (x1 - x0)) / 100;
			right_y0 = y0;
			right_x1 = x1;
			right_y1 = y1;
			set_xy(cT, right_leaf, right_x0, right_y0, right_x1, right_y1, false);
		} else {
			//Left
			left_x0 = x0;
			left_y0 = y0;
			left_x1 = x1;
			left_y1 = y0 + (left_factor * (y1 - y0)) / 100;
			set_xy(cT, left_leaf, left_x0, left_y0, left_x1, left_y1, true);
			//Right
			right_x0 = x0;
			right_y0 = y0 + (left_factor * (y1 - y0)) / 100;
			right_x1 = x1;
			right_y1 = y1;
			set_xy(cT, right_leaf, right_x0, right_y0, right_x1, right_y1, true);
		}
	}
}

//------------------------------------------------------------------------------
// Рекурсивный обход дерева сообществ и определение границ прямоугольных областей
//------------------------------------------------------------------------------

void set_box_bounds(cTree cT) {

	//Получить номер сообщества вершины дерева
	auto com = cT.get_last().key().com_id;
	//Вызов рекурсивной функции для определения границ прямоугольных областей
	set_xy(cT, com, 0, 0, X_MAX, Y_MAX, true);
	//Отладочное сообщение
#ifdef DEBUG
	mq_send(0);
#endif

}


//------------------------------------------------------------------------------
// Обход вершин сообществ и формирование графа визуализации
//------------------------------------------------------------------------------

void generate_boxes_graph(Graph G, Queue Q, Community cG, Graph vG, cTree cT) {

	//Удаление структуры визуализации
	vG.del_str_async();

	//Копирование графа G -> vG
    G.greq_sync(Graph::Key{.index = 0, .u = 0}, vG.struct_number);

	unsigned int x0, y0, x1, y1;
	short int x, y;
	unsigned int color;
	unsigned int new_btwc, btwc_min, btwc_sum, btwc, btwc_count = 0;
	unsigned int btwc_max = 0;
	unsigned int vertex_count = 0;
	unsigned short int x_mid;
	unsigned short int y_mid;
	unsigned short int size;
	unsigned int adj_c_max, adj_c_sum;
	unsigned short int a, b;
	unsigned int u;


	//Обход всех вершин графа для определения максимальной центральности
	for (auto com_u : vertex_range{G}) {
		auto btwc = G.search(Graph::Path_key{.u = com_u}).value().btwc;
		if (btwc_max < btwc) btwc_max = btwc;
	}

	//Получить номер сообщества вершины дерева
	for (auto cT_record : ctree_range{cT}) {
		//Получить номер сообщества
		auto com_u = cT_record.key().com_id;
		//Получить информацию о вершинах сообщества u
		auto [first_vertex_u, last_vertex_u] = cT_record.value();
		auto v_count = cT.search(cTree::Vcount_key{.com_id = com_u}).value().v_count;
		//Отладочное сообщение
#ifdef DEBUG
		mq_send(-6);
		mq_send(com_u);
		mq_send(v_count);
		mq_send(first_vertex_u);
		mq_send(last_vertex_u);
#endif
		//Получить информацию о границах визуализации сообщества com_u
		auto [x0, y0, x1, y1] = cT.search(cTree::XY_key{.com_id = com_u}).value();
		x_mid = x0 + ((x1 - x0) >> 1);
		y_mid = y0 + ((y1 - y0) >> 1);
		if ((x1 - x0) > (y1 - y0)) {
			a = (x1 - x0) >> 1;
			b = (y1 - y0) >> 1;
		}
		else {
			a = (y1 - y0) >> 1;
			b = (x1 - x0) >> 1;
		}
		//Очистка очереди
		Q.del_str_async();
		//Обойти все вершины в цепочке и создать очередь центральности
		adj_c_max = 0;
		adj_c_sum = 0;
		for (auto v : community_member_range(cG, com_u)) {
			//Получить центральность
			auto btwc = G.search(Graph::Path_key{.u = v}).value().btwc;
			//Запись очереди вершин
			Q.ins_async(Queue::Record{.id = v, .du = btwc}, Queue::Attributes{});
			//Получить информацию о ребре Adj[i]
			adj_c_sum += G.search(Graph::Base_key{.u = v}).value().adj_c;
		};
		vertex_count = Q.get_num();
		//Начальный радиус вершины с максимальным btwc
		int radius = 0;
		size = 0;
		btwc = 0;
		new_btwc = 0;
		//Обойти все вершины в цепочке и получить значения x,y
		while(auto q_it=Q.rbegin()) {
			auto [u, new_btwc] = *q_it;
			Q.erase(q_it);
			//Если btwc отличается от предыдущего, изменить радиус
			if (new_btwc != btwc)
				radius = radius + size;
			btwc = new_btwc;
			//Размер вершины
            size = std::max((S_MAX * btwc) / btwc_max,(unsigned int)10); 	//Вариант с единой метрикой размера
			//Цвет вершины
			unsigned int intensity = (0xFF * btwc) / btwc_max;
			color = violet2rgb(intensity); //Light color
			//color=kelvin2rgb(intensity);
			//Угол положения на орбите
			float angle = float(rand_single() % 628) / 100.0;
			x = radius * cos(angle);
			y = radius * sin(angle);
			vG.ins_async(Graph::Viz_key{.u = u}, Graph::vAttributes{.x = x, .y = y, .size = size, .color = color});
		}
		//Масштабирование координат для размещения в заданной области
		float scale = b / float(radius + size);
		//Обойти все вершины в цепочке и получить значения x,y
		for (auto u : community_member_range(cG, com_u)) {
			auto viz = vG.search(Graph::Viz_key{.u = u}).value();
			x = x_mid + scale*(short int)viz.x;
			y = y_mid + scale*(short int)viz.y;
			vG.ins_async(Graph::Viz_key{.u = u}, Graph::vAttributes{.x = (uint16_t)x, .y = (uint16_t)y, .size = viz.size, .color = viz.color});

			//Отладочное сообщение
#ifdef DEBUG
			mq_send(-7);
			mq_send(com_u);
			mq_send(u);
			mq_send(x);
			mq_send(y);
			mq_send(color);
			mq_send(size);
			mq_send(btwc);
#endif
		}
	}

	//Отладочное сообщение
#ifdef DEBUG
	mq_send(0);
#endif
}

//------------------------------------------------------------------------------
// Определение размеров областей и масштабирование в выделенную область (w,h)
//------------------------------------------------------------------------------


void set_community_boxes(Community cG1, int w, int h) {
	using namespace std;
	//Обойти все вершины в сообществе и определить их координаты
	int x_space = 0;
	int y_space = 0;
	const int sparse_factor = 2; //увеличение размера области
	const int condese_factor = 5; //Уменьшение размера области

	for (auto cmty_u : community_range{cG1}) {
		auto u = cmty_u.key().id;
		//Текущее положение вершины u
		iVector pos_u = cG1.search(Community::Plc_key{.id = u}).value();
		//Количество вершин в сообществе
		auto vertex_count_u = cG1.search(Community::Space_key{.id=u}).value().vertex_count;
		//Расстояние до ближайшего сообщества
		int distance = w*h;
		//Обойти все сообщества и определить ближайшее
		for (auto cmty_v : community_range{cG1})
		{
			auto v = cmty_v.key().id;
			if ( u != v) {
				//Количество вершин в сообществе
				auto vertex_count_v = cG1.search(Community::Space_key{.id=v}).value().vertex_count;
				//Текущее положение вершины v
				iVector pos_v = cG1.search(Community::Plc_key{.id = v}).value();
				iVector diff = pos_u - pos_v;
				int distanse_proportional = diff.abs() * vertex_count_u / vertex_count_v;
				distance = min(distance, distanse_proportional);
			}
		}
		int distance_sparced = sparse_factor * distance / condese_factor;
		//Определение размера пространства, выходящего за границы области (w,h)
		x_space 	= 	max(x_space,	abs(pos_u.x) + distance_sparced);
		y_space 	= 	max(y_space,	abs(pos_u.y) + distance_sparced);
		//Суммарная сила отталкивания, действующая на u
		cG1.ins_async(Community::Space_key{.id = u}, Community::Space{.d = distance_sparced, .vertex_count = vertex_count_u}); 
	}

	//Если сообщество выходит за границы области (space>0), то установить масштаб
	int scale_factor =  max(1000 * x_space / (w/2), 1000 * y_space / (h/2));
#ifdef DEBUG
			mq_send(-4);
			mq_send(scale_factor);
#endif
	if (scale_factor > 0) {
		for (auto cmty_u : community_range{cG1}) {
			auto u = cmty_u.key().id;
			//Количество вершин в сообществе
			auto vertex_count_u = cG1.search(Community::Space_key{.id=u}).value().vertex_count;
			//Текущее положение вершины u
			iVector pos_u = cG1.search(Community::Plc_key{.id = u}).value();
			pos_u.x = pos_u.x * 1000 / scale_factor;
			pos_u.y = pos_u.y * 1000 / scale_factor;
			uint32_t distance = cG1.search(Community::Space_key{.id = u}).value().d;
			distance = 10 + distance * 1000 / scale_factor;
			//Масштабируются координаты центра сообщества
			cG1.ins_async(Community::Plc_key{.id = u}, pos_u);
			//Масштабируется размер области
			cG1.ins_async(Community::Space_key{.id = u}, Community::Space{.d = distance, .vertex_count = vertex_count_u});
#ifdef DEBUG
			mq_send(-5);
			mq_send(u);
			mq_send(pos_u.x);
			mq_send(pos_u.y);
			mq_send(distance);
#endif
		}
	}
	//Отладочное сообщение
#ifdef DEBUG
	mq_send(0);
#endif

}
