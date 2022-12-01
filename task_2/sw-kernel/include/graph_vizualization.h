#ifndef GVIZ_H_
#define GVIZ_H_

#include "graph_iterators.hxx"
#include <cmath>

#define X_MAX           (1<<13)                                 //размер поля визуализации по X
#define Y_MAX           (1<<12)                               //размер поля визуализации по Y
#define S_MAX           (1<<7)                                  //максимальный размер вершины
#define ALPHA_DEFAULT   0xF0                                    //Альфа канал по умолчанию

//Given a temperature (in Kelvin), estimate an RGB equivalent
const unsigned int kelvin_to_rgb[] = {     0xFF3800, 0xFF4700, 0xFF5300, 0xFF5D00, 0xFF6500,
	                                   0xFF6D00, 0xFF7300, 0xFF7900, 0xFF7E00, 0xFF8300,
	                                   0xFF8A12, 0xFF8E21, 0xFF932C, 0xFF9836, 0xFF9D3F,
	                                   0xFFA148, 0xFFA54F, 0xFFA957, 0xFFAD5E, 0xFFB165,
	                                   0xFFB46B, 0xFFB872, 0xFFBB78, 0xFFBE7E, 0xFFC184,
	                                   0xFFC489, 0xFFC78F, 0xFFC994, 0xFFCC99, 0xFFCE9F,
	                                   0xFFD1A3, 0xFFD3A8, 0xFFD5AD, 0xFFD7B1, 0xFFD9B6,
	                                   0xFFDBBA, 0xFFDDBE, 0xFFDFC2, 0xFFE1C6, 0xFFE3CA,
	                                   0xFFE4CE, 0xFFE6D2, 0xFFE8D5, 0xFFE9D9, 0xFFEBDC,
	                                   0xFFECE0, 0xFFEEE3, 0xFFEFE6, 0xFFF0E9, 0xFFF2EC,
	                                   0xFFF3EF, 0xFFF4F2, 0xFFF5F5, 0xFFF6F7, 0xFFF8FB,
	                                   0xFFF9FD, 0xFEF9FF, 0xFCF7FF, 0xF9F6FF, 0xF7F5FF,
	                                   0xF5F3FF, 0xF3F2FF, 0xF0F1FF, 0xEFF0FF, 0xEDEFFF,
	                                   0xEBEEFF, 0xE9EDFF, 0xE7ECFF, 0xE6EBFF, 0xE4EAFF,
	                                   0xE3E9FF, 0xE1E8FF, 0xE0E7FF, 0xDEE6FF, 0xDDE6FF,
	                                   0xDCE5FF, 0xDAE5FF, 0xD9E3FF, 0xD8E3FF, 0xD7E2FF,
	                                   0xD6E1FF, 0xD4E1FF, 0xD3E0FF, 0xD2DFFF, 0xD1DFFF,
	                                   0xD0DEFF, 0xCFDDFF, 0xCFDDFF, 0xCEDCFF, 0xCDDCFF,
	                                   0xCFDAFF, 0xCFDAFF, 0xCED9FF, 0xCDD9FF, 0xCCD8FF,
	                                   0xCCD8FF, 0xCBD7FF, 0xCAD7FF, 0xCAD6FF, 0xC9D6FF,
	                                   0xC8D5FF, 0xC8D5FF, 0xC7D4FF, 0xC6D4FF, 0xC6D4FF,
	                                   0xC5D3FF, 0xC5D3FF, 0xC5D2FF, 0xC4D2FF, 0xC3D2FF,
	                                   0xC3D1FF};


//-----------------------------------------------------------------------
// Определение цвета в формате RGB (24 бит) по температуре (1000..12000K)
//-----------------------------------------------------------------------

unsigned int kelvin2rgb(unsigned int temperature);

//-----------------------------------------------------------------------
// Определение цвета в формате RGB (24 бит) по интенсивности (0..255)
//-----------------------------------------------------------------------

unsigned int violet2rgb(unsigned int uv);

//------------------------------------------------------------------------------
// Рекурсивная функция для определения границ визуализации сообществ
//------------------------------------------------------------------------------

void set_xy(cTree cT1, unsigned int comunity, unsigned int x0, unsigned int y0, unsigned int x1, unsigned int y1, bool horisontal);

//------------------------------------------------------------------------------
// Рекурсивный обход дерева сообществ и определение границ прямоугольных областей
//------------------------------------------------------------------------------

void set_box_bounds(cTree cT1);

//------------------------------------------------------------------------------
// Обход вершин сообществ и формирование графа визуализации
//------------------------------------------------------------------------------

void generate_boxes_graph(Graph G1, Queue Q1, Community cG1, Graph vG1, cTree cT1);

//------------------------------------------------------------------------------
// Определение размеров областей и масштабирование в выделенную область (w,h)
//------------------------------------------------------------------------------

void set_community_boxes(Community cG1, int w, int h);


//------------------------------------------------------------------------------
// Определение размеров областей и масштабирование в выделенную область (w,h)
//------------------------------------------------------------------------------

void set_community_boxes(Community cG1, int w, int h);

#endif

