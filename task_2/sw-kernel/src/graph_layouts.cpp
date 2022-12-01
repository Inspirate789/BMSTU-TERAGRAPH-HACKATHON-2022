#include "graph_layouts.h"
#include "vector.hxx"

//-------------------------------------------------------------
// Создание визуализации графа в виде решетки
//-------------------------------------------------------------

void matrix_layout(Graph G, Graph vG, unsigned int x_grid, unsigned int y_grid) {

    //Удаление структуры визуализации
    vG.del_str_async();
    //Копирование графа G -> vG
    G.greq_sync(Graph::Key{.index = 0, .u = 0}, vG.struct_number);
    //Переменные
    unsigned short int x;
    unsigned short int y;
    unsigned short int size;
    unsigned int color;
    unsigned int btwc_max = 0;
    //Обход всех вершин графа и определение максимального значения btwc
    for (auto u : vertex_range(vG)) {
        auto btwc = vG.search(Graph::Path_key{.u = u}).value().btwc;
        if (btwc > btwc_max) btwc_max = btwc;
    }

    for (auto u : vertex_range(vG)) {
        x = X_MAX * (u % x_grid) / x_grid; //rand_single()%X_MAX;
        y = Y_MAX * (u / y_grid) / y_grid; //rand_single()%Y_MAX;
        auto btwc = vG.search(Graph::Path_key{.u = u}).value().btwc;
        size = (S_MAX * btwc) / btwc_max + 10;
        unsigned int intensity = (0xFF * btwc) / btwc_max;
        color = violet2rgb(intensity); //Light color
        //color=kelvin2rgb(intensity);
        vG.ins_async(Graph::Viz_key{.u = u}, Graph::vAttributes{.x = x, .y = y, .size = size, .color = color});
    }; //переход к следующей вершине
}


//------------------------------------------------------------------------------
// Создание визуализации графа на основе обхода матрицы
//------------------------------------------------------------------------------

void matrix_spiral_layout(Graph G, Graph vG, Queue Q) {

    //Удаление структуры визуализации
    vG.del_str_async();
    //Копирование графа G -> vG
    G.greq_sync(Graph::Key{.index = 0, .u = 0}, vG.struct_number);
    //Переменные
    unsigned int u;
    unsigned int color;
    unsigned int btwc, btwc_max = 0;
    unsigned int vertex_count = 0;
    enum { down, left, up, right } direction;
    short unsigned int max_size;
    short unsigned int x_mid = (X_MAX >> 1);
    short unsigned int y_mid = (Y_MAX >> 1);
    short unsigned int x;
    short unsigned int y;
    short unsigned int size;
    unsigned int state = down;//down
    const short int space = S_MAX / 2;


    //Очистка очереди
    Q.del_str_async();
    //Обход всех вершин графа и определение максимального значения btwc и количества вершин
    for (auto u : vertex_range(vG)) {
        btwc = vG.search(Graph::Path_key{.u = u}).value().btwc;
        //Запись очереди вершин
        Q.ins_async(Queue::Record{.id = u, .du = btwc}, Queue::Attributes{});
    };
    vertex_count = Q.get_num();
    btwc_max = Q.get_last().key().du;
    //set starting position
    x = x_mid + S_MAX + 10;
    y = y_mid;
    max_size = S_MAX + 10;

    short unsigned int left_up_x = x_mid - S_MAX - 10;
    short unsigned int left_up_y = y_mid - S_MAX - 10;
    short unsigned int right_down_x = x_mid + S_MAX + 10;
    short unsigned int right_down_y = y_mid + S_MAX + 10;

    while(auto q_it = Q.rbegin()) {
        Q.erase(q_it);
        auto [u, btwc] = *q_it;
        //Размер вершины
        size = (S_MAX * btwc) / btwc_max + 10;
        //Цвет вершины
        unsigned int intensity = (0xFF * btwc) / btwc_max;
        color = violet2rgb(intensity); //Light color
        //color=kelvin2rgb(intensity);
        vG.ins_async(Graph::Viz_key{.u = u}, Graph::vAttributes{.x = x, .y = y, .size = size, .color = color});
        //Определение положения вершины в пиксельной матрице
        switch (state) {
        case down:
            y += max_size;
            if (y >= right_down_y) {
                state = left;
                x = right_down_x - max_size;
                y = right_down_y;
            }
            break;
        case left:
            x -= max_size;
            if (x <= left_up_x) {
                state = up;
                x = left_up_x;
                y = right_down_y - max_size;
            }
            break;
        case up: //up

            y -= max_size;
            if (y <= left_up_y) {
                state = right;
                x = left_up_x + max_size;
                y = left_up_y;
            }
            break;
        case right: //right
            x += max_size;
            if (x >= right_down_x) {
                state = down;
                y = right_down_y - max_size;
                max_size = size + space;
                left_up_x = left_up_x - max_size;
                left_up_y = left_up_y - max_size;
                right_down_x = right_down_x + max_size;
                right_down_y = right_down_y + max_size;
                x = right_down_x;
                y = left_up_y + max_size;
            }
            break;
        }

    }
}

//------------------------------------------------------------------------------
// Создание визуализации графа на основе обхода по спирали центральности
//------------------------------------------------------------------------------

void spiral_layout(Graph G, Graph vG, Queue Q) {

    //Удаление структуры визуализации
    vG.del_str_async();
    //Копирование графа G -> vG
    G.greq_sync(Graph::Key{.index = 0, .u = 0}, vG.struct_number);
    //Переменные
    unsigned int u;
    unsigned int color;
    unsigned int btwc, btwc_max = 0;
    unsigned int vertex_count = 0;
    short unsigned int x_mid = (X_MAX >> 1);
    short unsigned int y_mid = (Y_MAX >> 1);
    short unsigned int x;
    short unsigned int y;
    short unsigned int size;
    short unsigned int max_size = S_MAX + 10;

    const short int space = S_MAX / 2;

    //Очистка очереди
    Q.del_str_async();
    //Обход всех вершин графа и определение максимального значения btwc и количества вершин
    for (auto u : vertex_range(vG)) {
        btwc = vG.search(Graph::Path_key{.u = u}).value().btwc;
        //Запись очереди вершин
        Q.ins_async(Queue::Record{.id = u, .du = btwc}, Queue::Attributes{});
    };

    vertex_count = Q.get_num();
    btwc_max = Q.get_last().key().du;
    //set starting position
    float angle = 2 * M_PI;
    int vertex_radius = S_MAX + 50;
    int radius;

    while(auto q_it = Q.rbegin()) {
        Q.erase(q_it);
        auto [u, btwc] = *q_it;
        //Размер вершины
        size = (S_MAX * btwc) / btwc_max + 10;
        //Цвет вершины
        unsigned int intensity = (0xFF * btwc) / btwc_max;
        color = violet2rgb(intensity); //Light color
        //color=kelvin2rgb(intensity);
        radius = float(vertex_radius) * angle / (2 * M_PI) ;
        x = x_mid + radius * cos(angle);
        y = y_mid + radius * sin(angle);
        vG.ins_async(Graph::Viz_key{.u = u}, Graph::vAttributes{.x = x, .y = y, .size = size, .color = color});
        //Определение положения вершины в пиксельной матрице
        angle += acos(1.0 - float(pow(S_MAX, 2)) / float(2 * pow(radius, 2)));
    }
}


//------------------------------------------------------------------------------
// Создание визуализации графа на основе алгоритма Фрюхтермана-Рейнгольда
//------------------------------------------------------------------------------

/* Сила притяжения */
[[gnu::always_inline]] inline int Fa(int distance, int k)
{
    return distance * distance / k;
}

/* Сила отталкивания */
[[gnu::always_inline]] inline int Fr(int distance, int k)
{
    return k * k / distance;
}

[[gnu::always_inline]] inline int cool(int t)
{
    int cold_t = 10;
    if (t > cold_t)
    {
        return t * 9 / 10;
    }
    else
    {
        return cold_t;
    }
}


//---------------------------------------------------------------------------------
// Создание визуализации сообществ графа на основе алгоритма Фрюхтермана-Рейнгольда
//---------------------------------------------------------------------------------

/* Параметры:
 * @1 vG    - граф визуализации с начальными позициями вершин (0,0)
 * @2 w     - ширина прямоугольника
 * @3 h     - высота прямоугольника
 * Результат работы:
 * Положения вершин в vG в атрибуте vAttributes.
 */

void force_directed_layout_communities(Community cG1, mQueue mQ1, iQueue iQ1, int w, int h)
{
    const int max_iteration = 100;
    //Подсчет количества вершин
    uint32_t v_num = iQ1.get_num();
    int k = (w * h) / sqrt(v_num);
    int t = 20 ; //Текущая "температура" для симуляции отжига
    int stop_count = 0;
    //Начальное положение сообществ
    for (auto cmty_u : community_range{cG1}) {
        auto [adj_u, u] = cmty_u.key();
        iVector pos_u = iVector::random(w/2, h/2);
        cG1.ins_async(Community::Plc_key{.id = u}, pos_u);
    }
    // Stop when total movement falls under a certain range
    while (stop_count < max_iteration)
    {
        //Обойти все сообщества и рассчитать силы отталкивания
        for (auto cmty_u : community_range{cG1}) {
            auto [adj_u, u] = cmty_u.key();
            iVector displacement_u{.x = 0, .y = 0};
            //Текущее положение вершины u
            iVector pos_u = cG1.search(Community::Plc_key{.id = u}).value();
            for (auto cmty_v : community_range{cG1})
            {
                auto [adj_v, v] = cmty_v.key();
                if ( u != v) {
                    //Текущее положение вершины u
                    iVector pos_v = cG1.search(Community::Plc_key{.id = v}).value();
                    iVector diff = pos_u - pos_v;
                    // displacement = displacement + (diff / |diff|) * Fr
                    displacement_u = displacement_u + diff.norm() * Fr(diff.abs() , k) * adj_v * adj_u / 10000;
                }
            }
            //Суммарная сила отталкивания, действующая на u
            cG1.ins_async(Community::Displc_key{.id = u}, displacement_u);
        }
        //Обойти все вершины в сообществе и рассчитать силы притяжения
        for (auto cmty_u : community_range{cG1}) {
            auto [adj_u, u] = cmty_u.key();
            //Текущее положение вершины u
            iVector pos_u = cG1.search(Community::Plc_key{.id = u}).value();
            //Получить для сообщества значение displacement
            iVector displacement_u = cG1.search(Community::Displc_key{.id = u}).value();
            for (auto cmty_v : iqueue_range{iQ1, u}) {
                auto v = cmty_v.key().com_u;
                if ( u != v) {
                    //Связность сообществ u и v
                    auto w_u_v = mQ1.search(modularity::Modularity_ext{.id = cmty_v.value().id, .delta_mod = cmty_v.value().delta_mod}).value().w_u_v;
                    //Текущее положение вершины v
                    iVector pos_v = cG1.search(Community::Plc_key{.id = v}).value();
                    //Текущая равнодействующая сил, действующих на v
                    iVector displacement_v = cG1.search(Community::Displc_key{.id = v}).value();
                    iVector diff = pos_u - pos_v;
                    //delta = (diff / |diff|) * Fa
                    // displacement = displacement +- delta
                    iVector delta = diff.norm() * Fa(diff.abs(), k) * w_u_v / 10000;
                    //Учесть силу притяжения, действующую на u по ребру (u,v)
                    displacement_u = displacement_u - delta;
                    //Учесть силу притяжения, действующую на v по ребру (u,v)
                    displacement_v = displacement_v + delta;
                    //Обновить равнодействующую сил, действующих на v
                    cG1.ins_async(Community::Displc_key{.id = v}, displacement_v);
                }
            }
            //Суммарная сила отталкивания, действующая на u
            cG1.ins_async(Community::Displc_key{.id = u}, displacement_u);
        }

        int total_displacement = 0;
        //Обойти все вершины в сообществе и определить их координаты
        for (auto cmty_u : community_range{cG1}) {
            auto [adj_u, u] = cmty_u.key();
            //Текущее положение вершины u
            iVector pos_u = cG1.search(Community::Plc_key{.id = u}).value();
            //Получить для сообщества значение displacement
            iVector displacement_u = cG1.search(Community::Displc_key{.id = u}).value();
            //Учесть температуру отжига
            iVector lim_disp = displacement_u.norm() * std::min(displacement_u.abs() , t ) / 10000 ;
            //Изменить текущее положение u
            pos_u = pos_u + lim_disp;
            //Ограничить перемещение границами прямоугольника
            pos_u.x = std::min(w/2, std::max(-w/2, pos_u.x));
            pos_u.y = std::min(h/2, std::max(-h/2, pos_u.y));
            //Записать новое положение сообщества u
            cG1.ins_async(Community::Plc_key{.id = u}, pos_u);
            total_displacement += lim_disp.abs();
#ifdef DEBUG
            mq_send(-3);
            mq_send(u);
            mq_send(pos_u.x);
            mq_send(pos_u.y);
            mq_send(abs(displacement_u));
#endif
        }
        // Stop when total movement falls under a certain range
        if (total_displacement < v_num)
            stop_count++;
        t = cool(t);
    }
    //Отладочное сообщение
#ifdef DEBUG
    mq_send(0);
#endif
}

//---------------------------------------------------------------------------------
// Создание визуализации сообщества на основе алгоритма Фрюхтермана-Рейнгольда
//---------------------------------------------------------------------------------

/* Параметры:
 * @1 vG    - граф визуализации с начальными позициями вершин и размером области
 * Результат работы:
 * Положения вершин в vG в атрибуте vAttributes.
 */

void force_directed_layout_vertices(Community cG1, Graph vG1, Queue vQ1)
{

    //Обход всех вершин графа для определения максимальной центральности
    uint32_t btwc_max = 0;
    for (auto com_u : vertex_range{vG1}) {
        auto btwc = vG1.search(Graph::Path_key{.u = com_u}).value().btwc;
        if (btwc_max < btwc) btwc_max = btwc;
    }

    //Обойти все сообщества и рассчитать силы отталкивания внутри них
    for (auto cmty : community_range{cG1})
    {
        auto community = cmty.key().id;
        iVector community_center = cG1.search(Community::Plc_key{.id = community}).value();
        auto [distance, vertex_count_u] = cG1.search(Community::Space_key{.id = community}).value();
        const int max_iteration = 100;
        //Подсчет количества вершин
        vQ1.del_str_async();
        for (auto vertex : community_member_range(cG1, community)) {
            //Установить начальное положение вершин сообщества
            iVector pos_v = iVector::random(distance, distance);
            vG1.ins_async(Graph::Plc_key{.u = vertex}, pos_v);
            //Запись о вершине сообщества в очереди для определения сил внетри сообщества
            vQ1.ins_async(Queue::Record{.id = vertex, .du = 0}, Queue::Attributes{});
        }
        int k =  4 * distance * distance / sqrt( vertex_count_u);
        int t = 20 ; //Текущая "температура" для симуляции отжига
        int stop_count = 0;
        // Stop when total movement falls under a certain range
        while (stop_count < max_iteration)
        {
            //Обойти все вершины сообщества и рассчитать силы отталкивания
            for (auto u : community_member_range(cG1, community))
            {
                //Текущее положение вершины u
                iVector pos_u = vG1.search(Graph::Plc_key{.u = u}).value();
                iVector displacement_u{.x = 0, .y = 0};
                //Количество ребер
                auto adj_u = vG1.search(Graph::Base_key{.u = u}).value().adj_c;
                for (auto v : community_member_range(cG1, u))
                {
                    if ( u != v) {
                        //Текущее положение вершины u
                        iVector pos_v = vG1.search(Graph::Plc_key{.u = v}).value();
                        iVector diff = pos_u - pos_v;
                        //Количество ребер
                        auto adj_v = vG1.search(Graph::Base_key{.u = v}).value().adj_c;
                        // displacement = displacement + (diff / |diff|) * Fr
                        displacement_u = displacement_u + diff.norm() * Fr(diff.abs() , k) * adj_u * adj_v / 10000;
                    }
                }
                //Суммарная сила отталкивания, действующая на u
                vG1.ins_async(Graph::Displc_key{.u = u}, displacement_u);
            }
            //Обойти все вершины в сообществе и рассчитать силы притяжения
            for (auto u : community_member_range(cG1, community))
            {
                //Количество ребер
                auto adj_u = vG1.search(Graph::Base_key{.u = u}).value().adj_c;
                //Текущее положение вершины u
                iVector pos_u = vG1.search(Graph::Plc_key{.u = u}).value();
                //Получить для сообщества значение displacement
                iVector displacement_u = vG1.search(Graph::Displc_key{.u = u}).value();
                for (auto [v, w, atr] : edge_range{vG1, u})
                {
                    //Если u и v различны, и v в том же сообществе
                    if ( u != v && (bool)vQ1.search(Queue::Record{.id = v, .du = 0}) == true)
                    {
                        //Количество ребер
                        auto adj_v = vG1.search(Graph::Base_key{.u = v}).value().adj_c;
                        //Связность сообществ u и v
                        iVector pos_v = vG1.search(Graph::Plc_key{.u = v}).value();
                        //Текущая равнодействующая сил, действующих на v
                        iVector displacement_v = vG1.search(Graph::Displc_key{.u = v}).value();
                        iVector diff = pos_u - pos_v;
                        //delta = (diff / |diff|) * Fa
                        // displacement = displacement +- delta
                        iVector delta = diff.norm() * Fa(diff.abs(), k) * w / 10000;
                        //Учесть силу притяжения, действующую на u по ребру (u,v)
                        displacement_u = displacement_u - delta;
                        //Учесть силу притяжения, действующую на v по ребру (u,v)
                        displacement_v = displacement_v + delta;
                        //Обновить равнодействующую сил, действующих на v
                        vG1.ins_async(Graph::Displc_key{.u = v}, displacement_v);
                    }
                }
                //Суммарная сила отталкивания, действующая на u
                vG1.ins_async(Graph::Displc_key{.u = u}, displacement_u);
            }

            int total_displacement = 0;
            //Обойти все вершины в сообществе и определить их координаты
            for (auto u : community_member_range(cG1, community)) {
                //Текущее положение вершины u
                iVector pos_u = vG1.search(Graph::Plc_key{.u = u}).value();
                //Центральность
                auto btwc_u = vG1.search(Graph::Path_key{.u = u}).value().btwc;
                //Получить для сообщества значение displacement
                iVector displacement_u = vG1.search(Graph::Displc_key{.u = u}).value();
                //Учесть температуру отжига
                iVector lim_disp = displacement_u.norm() * std::min(displacement_u.abs(), t ) / 10000;
                //Изменить текущее положение u
                pos_u = pos_u + lim_disp;
                //Учесть центральность, как силу притяжения к центру
                iVector c_disp = pos_u.norm() * Fa(pos_u.abs(), k) * btwc_u / 10000;
                //Изменить текущее положение u
                // pos_u = pos_u - c_disp;
                //Ограничить перемещение границами области
                int radius = pos_u.abs();
                if (radius > distance) {
                    pos_u.x = pos_u.x * distance / radius;
                    pos_u.y = pos_u.y * distance / radius;
                }
                //Записать новое положение сообщества u
                vG1.ins_async(Graph::Plc_key{.u = u}, pos_u);
                total_displacement += lim_disp.abs();
#ifdef DEBUG
                mq_send(-5);
                mq_send(community);
                mq_send(u);
                mq_send(pos_u.x);
                mq_send(pos_u.y);
#endif
            }
            // Stop when total movement falls under a certain range
            stop_count++;
            t = cool(t);
        }


        //Очистка очереди
        vQ1.del_str_async();
        iVector scene_center = {.x = X_MAX / 2, .y = Y_MAX / 2};
        scene_center = scene_center + community_center;
        for (auto u : community_member_range(cG1, community)) {
            //Текущее положение вершины u
            iVector pos_u = vG1.search(Graph::Plc_key{.u = u}).value();
            pos_u = pos_u + scene_center;
            //Центральность
            auto btwc = vG1.search(Graph::Path_key{.u = u}).value().btwc;
            //Размер вершины
            uint32_t  size = std::max((unsigned int)(S_MAX * btwc / btwc_max),(unsigned int)10);    //Вариант с единой метрикой размера
            //Цвет вершины
            unsigned int intensity = (0xFF * btwc) / btwc_max;
            uint32_t color = violet2rgb(intensity); //Light color
            //Записать положение вершины в граф
            vG1.ins_async(Graph::Viz_key{.u = u}, Graph::vAttributes{.x = (uint16_t)pos_u.x, .y = (uint16_t)pos_u.y, .size = (uint16_t)size, .color = (uint32_t)color});
#ifdef DEBUG
            mq_send(-5);
            mq_send(community);
            mq_send(u);
            mq_send(pos_u.x);
            mq_send(pos_u.y);
#endif
        }
    }
    //Отладочное сообщение
#ifdef DEBUG
    mq_send(0);
#endif

}
