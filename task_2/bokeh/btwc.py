from bokeh.plotting import figure, show, from_networkx
from bokeh.palettes import Spectral6
from bokeh.transform import linear_cmap
from bokeh.models import (ColumnDataSource, Slider, Ellipse, Circle, MultiLine, BoxSelectTool, 
                          EdgesAndLinkedNodes, HoverTool, NodesAndLinkedEdges, TapTool, PointDrawTool, StaticLayoutProvider, GraphRenderer)
#from bokeh.io import push_notebook, output_notebook
from bokeh.layouts import column, row
import networkx as nx
import numpy as np
#from time import monotonic_ns
import bokeh.settings
from bokeh.io import curdoc
import random
import time 
from threading import Thread
import scipy as sp
import scipy.spatial
import struct
import socket
import sys 

bokeh.settings.settings.minified.set_value(False)


def geometric_edges(nodes_pos, radius, p):
    coords = list(nodes_pos.values())
    kdtree = sp.spatial.cKDTree(coords)
    edge_indexes = kdtree.query_pairs(radius, p)
    return edge_indexes

def rescale_layout(pos, scale=[1,1]):
    # Find max length over all dimensions
    lim = 0  # max coordinate for all axes
    for i in range(pos.shape[1]):
        pos[:, i] -= pos[:, i].mean()
        lim = max(abs(pos[:, i]).max(), lim)
    # rescale to (-scale, scale) in all directions, preserves aspect
    if lim > 0:
        for i in range(pos.shape[1]):
            pos[:, i] *= scale[i] / lim
    return pos

def rescale_layout_dict(pos, scale=[1,1]):
    if not pos:  # empty_graph
        return {}
    pos_v = np.array(list(pos.values()))
    pos_v = rescale_layout(pos_v, scale=scale)
    return dict(zip(pos, pos_v))

def get_array(buf, dtype=np.int64) : 
    if(len(buf)==0) :
        raise
    arr = np.frombuffer(buf, dtype=dtype)
    return arr

def read_uint32(fd):
    buf = fd.recv(4)
    if(len(buf) < 4) :
        raise Exception('EOF')
    return int.from_bytes(buf, byteorder=sys.byteorder, signed=False)

host = '195.19.32.95'
port = 0x4747

doc = curdoc()
nodes_pos = {} #Узлы графа в формате {idx:[x,y]}, где idx - индекс вершины, x,y - координаты

graph = GraphRenderer()
graph.node_renderer.data_source.data = {'index': [], 'size':[], 'color':[], 'btwc':[], 'adj_c':[]}
graph.edge_renderer.data_source.data = {'start' : [], 'end':[], 'width':[]}
graph.layout_provider = StaticLayoutProvider(graph_layout=nodes_pos)     
index_arr=[]
sizes_arr=[]
color_arr=[]
edges_arr=[]
edges_start=[]
edges_stop=[]
edges_width=[]
btwc_arr=[]
adj_c_arr=[]

def init_graph():
    print('init_graph')
    print(edges_arr)     
    graph.node_renderer.data_source.stream({'index': index_arr, 'size':sizes_arr, 'color':color_arr, 'btwc':btwc_arr, 'adj_c':adj_c_arr})
    graph.edge_renderer.data_source.stream({'start' : edges_start, 'end':edges_stop, 'width':edges_width})
    graph.layout_provider.update(graph_layout=nodes_pos)
    #g_edges_arr = np.array([[u,v] for (u,v) in g_edges]).transpose()
    #graph.edge_renderer.data_source.stream({'start' : g_edges_arr[0], 'end':g_edges_arr[1]})

#def update(): 
#    global cur_centr
#    patch_size=[]     
#    if(cur_centr) :
#        patch_size = list(zip(cur_centr, [0.03]*len(cur_centr)))
#    cur_centr = random.choices(list(T.nodes), k=10) #TODO:  использовать index_arr, испраить ошибку bokeh с проверкой типа
#    patch_size += list(zip(cur_centr, [0.05]*len(cur_centr)))
#    graph.node_renderer.data_source.patch({'size': patch_size}) 
#    shortest_paths = np.ndarray((10, len(index_arr)), dtype=int)
#    for i in range(0, len(cur_centr)) :
#        sh = np.array(list(nx.single_target_shortest_path_length(T, cur_centr[i]))).transpose()
#        shortest_paths[i,sh[0,:]] = sh[1,:] 
#    shortest_paths = np.min(shortest_paths, 0)
#    graph.node_renderer.data_source.data['path_len'] = shortest_paths
     
def th1():
    global index_arr
    global sizes_arr
    global color_arr
    global edges_arr
    global adj_c_arr
    global nodes_pos
    srv_sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    srv_sock.connect((host, port))
    try:
        while True:
            u = read_uint32(srv_sock)
            btwc = read_uint32(srv_sock)
            adj_c = read_uint32(srv_sock)
            x = read_uint32(srv_sock)
            y = read_uint32(srv_sock)
            size = read_uint32(srv_sock)
            color = read_uint32(srv_sock)
            index_arr.append(u)
            sizes_arr.append(size)
            color_arr.append(color)
            btwc_arr.append(btwc)
            adj_c_arr.append(adj_c)
            nodes_pos[u]=[x,y]
            for i in range(0, adj_c):
                v = read_uint32(srv_sock)
                w = read_uint32(srv_sock)
                edges_start.append(u)
                edges_stop.append(v)
                edges_width.append(w)
    except Exception:
        pass
    finally:
        srv_sock.close()
    doc.add_next_tick_callback(init_graph)
 #   while True:
 #       time.sleep(2)
 #       doc.add_next_tick_callback(update)

TOOLTIPS_EDGE = [
    ("(start,end)", "($start, $end)"),
    ("width", "@width")
]

TOOLTIPS_VIRTEX = [
    ("Вершина", "@index"),
    ("Центральность", "@btwc"),
    ("Количество ребер", "@adj_c")
]

p = figure(
    title=None, 
    toolbar_location="right", #None,
    output_backend="webgl",
    sizing_mode="stretch_both",
    match_aspect = True,
)


p.add_tools(HoverTool(tooltips= TOOLTIPS_VIRTEX), TapTool())


p.background_fill_color = "black"
p.grid.grid_line_color = None
p.xaxis.visible = False 
p.yaxis.visible = False
p.output_backend = "svg"
graph.node_renderer.glyph = Ellipse(height="size", width="size", fill_color="color", fill_alpha=1)
graph.node_renderer.selection_glyph = Ellipse(height="size", width="size",  line_color="#FF0000")
graph.node_renderer.hover_glyph =  Ellipse(height="size", width="size", line_color="#00FF00")
graph.edge_renderer.glyph = MultiLine(line_color="#888888", line_alpha=0.2)
graph.edge_renderer.selection_glyph = MultiLine(line_color="#FF0000", line_alpha=1)
graph.edge_renderer.hover_glyph = MultiLine(line_color="#00FF00", line_alpha=1)
graph.inspection_policy = NodesAndLinkedEdges() 
graph.selection_policy = NodesAndLinkedEdges()  
#graph.inspection_policy = EdgesAndLinkedNodes()
p.renderers.append(graph)
doc.add_root(p)
thread = Thread(target=th1)
thread.start()
