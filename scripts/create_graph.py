import geopandas
import matplotlib.pyplot as plt
import pandas as pd
import numpy as np
from shapely.geometry import Point
import math
import json
from collections import defaultdict

excel_reader = pd.ExcelFile(r'../dataset/ГрафДанные.xlsx', engine='openpyxl')
df_ports = excel_reader.parse('points')
df_ports.drop(df_ports.columns[[4,5,6]], axis=1, inplace=True)
df_edges = excel_reader.parse('edges')
df_edges.drop(df_edges.columns[[4,5,6,7]], axis=1, inplace=True)

excel_reader = pd.ExcelFile(r'../dataset/IntegrVelocity.xlsx', engine='openpyxl')

df_lon = excel_reader.parse('lon')
df_lat = excel_reader.parse('lat')

df_iceweeks = []

for week in excel_reader.sheet_names[2:]:
    df_iceweeks.append({"ice" : excel_reader.parse(week), "week" : week })

lons, lats = [], []

iceweeks = [{"ice": [], "week" : df_iceweeks[i]["week"]} for i in range(len(df_iceweeks))]
    
# from pandas to arrays
# also for cut not needed latitudes (only lats in [55,85] are valid for our problem)
for i in range(df_lon.shape[0]):
    for j in range(df_lon.shape[1]):
        if df_lat.iloc[i,j] < 55 or df_lat.iloc[i,j] > 85:
            continue
        lons.append(df_lon.iloc[i,j])
        lats.append(df_lat.iloc[i,j])
        for k in range(len(df_iceweeks)):
            iceweeks[k]["ice"].append(df_iceweeks[k]["ice"].iloc[i,j])

# distance in meters between origin [lon, lat] and dest. points
def distance(origin, destination):
    lon1, lat1 = origin
    lon2, lat2 = destination
    radius = 6378137 # m

    dlat = math.radians(lat2-lat1)
    dlon = math.radians(lon2-lon1)
    a = math.sin(dlat/2) * math.sin(dlat/2) + math.cos(math.radians(lat1)) \
        * math.cos(math.radians(lat2)) * math.sin(dlon/2) * math.sin(dlon/2)
    c = 2 * math.atan2(math.sqrt(a), math.sqrt(1-a))
    d = radius * c

    return d

def find_closest_idx(lon_arr, lat_arr, lon_point, lat_point):
    best_idx = -1
    bestdist = 100000000
    for i in range(len(lon_arr)):
        if lon_point >= lon_arr[i] and lat_point >= lat_arr[i]:
            d = distance([lon_point, lat_point], [lon_arr[i], lat_arr[i]])
            if bestdist > d:
                best_idx = i
                bestdist = d
    return best_idx

def line_func(x, x1, y1, x2, y2):
    m = (y1 - y2) / (x1 - x2)
    b = (x1 * y2 - x2 * y1) / (x1 - x2)
    return m * x + b

all_vertices = []

for index, vertex in df_ports.iterrows():
    all_vertices.append({"id" : vertex["point_id"], "lon" : vertex["longitude"] if vertex["longitude"] >= 0 else 360 + vertex["longitude"], "lat" : vertex["latitude"], "name" : vertex["point_name"]})

ii = 1
new_edges = []

for index, edge in df_edges.iterrows():
    x1, y1 = all_vertices[int(edge["start_point_id"])]["lon"], all_vertices[int(edge["start_point_id"])]["lat"]
    x2, y2 = all_vertices[int(edge["end_point_id"])]["lon"], all_vertices[int(edge["end_point_id"])]["lat"]
    x = np.linspace(x1, x2, num=100)
    y = []
    if x1 == x2:
        y = np.linspace(y1, y2, num=100)
    else:
        y = line_func(x, x1, y1, x2, y2)

    # generate vertices between ports
    n_dots = 3
    xxi = []
    for i in range(n_dots):
        xxi.append(int(len(x)/(n_dots + 1) * (i+1) - 1))
    xxi.append(len(x) - 1)
    
    next_k = 0
    edges_cap = []
    
    #cap = [{"ice_freq" : [0,0,0,0], "week" : ""}]
    cap = {}
    for name in excel_reader.sheet_names[2:]:
        cap[name] = [0,0,0,0]
    
    # 0 if ice >= 20
    # 1 if ice \in [15, 20)
    # 2 if ice \in [10, 15)
    # 3 if ice < 10 
    for i in range(len(x)):
        closest_idx = find_closest_idx(lons, lats, x[i], y[i])
        if i > xxi[next_k]:
            to_append = {}
            for key in cap:
                to_append[key] = int(np.argmax(cap[key]))
            edges_cap.append(to_append)
            next_k += 1
            cap = {}
            for name in excel_reader.sheet_names[2:]:
                cap[name] = [0,0,0,0]
        for k in range(len(excel_reader.sheet_names[2:])):
            val = iceweeks[k]["ice"][closest_idx]
            if val >= 20:
                cap[iceweeks[k]["week"]][0] += 1
            elif val >= 15:
                cap[iceweeks[k]["week"]][1] += 1
            elif val >= 10:
                cap[iceweeks[k]["week"]][2] += 1
            else:
                cap[iceweeks[k]["week"]][3] += 1
        if i == len(x) - 1:
            to_append = {}
            for key in cap:
                to_append[key] = int(np.argmax(cap[key]))
            edges_cap.append(to_append)
    
    for week in excel_reader.sheet_names[2:]: 
        # 3 3 3 2 3 0 3 3 3 3 3
        i = 0
        while i < len(edges_cap) and edges_cap[i][week] == 3:
            i+=1
        if i >= len(edges_cap):
            continue
        c = edges_cap[i][week]
        i-=1
        while i >= 0:
            edges_cap[i][week] = c
            i-=1
            
        # 0 0 0 2 3 0 3 3 3 3 3
        
        i = len(edges_cap) - 1
        while edges_cap[i][week] == 3:
            i-=1
        c = edges_cap[i][week]
        i+=1
        while i < len(edges_cap):
            edges_cap[i][week] = c
            i+=1
        
        # 0 0 0 2 3 0 0 0 0 0 0
        
        i = 0
        
        while i < len(edges_cap)-1:
            if edges_cap[i+1][week] == 3:
                edges_cap[i+1][week] = edges_cap[i][week]
            i+=1

    saved_idxs = []
    for vertex_i in xxi[:-1]:
        saved_idxs.append(all_vertices[-1]["id"]+1)
        all_vertices.append({"id" : saved_idxs[-1], "lon" : x[vertex_i], "lat" : y[vertex_i], "name" : ""})        
        
    all_idxs = [int(edge["start_point_id"])] + saved_idxs + [int(edge["end_point_id"])]

    for i in range(len(all_idxs) - 1):
        d = distance([all_vertices[all_idxs[i]]["lon"], all_vertices[all_idxs[i]]["lat"]], [all_vertices[all_idxs[i+1]]["lon"], all_vertices[all_idxs[i+1]]["lat"]])
        new_edges.append({"start": int(all_idxs[i]), "end" : int(all_idxs[i+1]), "len" : d, "type" : edges_cap[i]})        

    print("done: ", ii, " / ", len(df_edges))
    ii+=1
    
class Graph:
    def __init__(self):
        self.graph = defaultdict(list)
        self.path = []
        self.type = -1
        self.end = False
    def addEdge(self, u, v, w):
        self.graph[u].append([v,w])
        self.graph[v].append([u,w])
    def DFSUtil(self, v, visited):
        visited.add(v)
        self.path.append(v)
        for neighbour in self.graph[v]:
            if v == 22 and neighbour[1] != 3:
                continue
            if v == 20 and neighbour[1] != 3:
                continue
            if self.end == False:
                if neighbour[0] not in visited:
                    if neighbour[1] != 3:
                        self.path.append(neighbour[0])
                        self.type = neighbour[1]
                        self.end = True
                        return
                    self.DFSUtil(neighbour[0], visited)
    def DFS(self, v):
        visited = set()
        self.DFSUtil(v, visited)

for week in excel_reader.sheet_names[2:]:
    g = Graph()
    
    for edge in new_edges:
        g.addEdge(edge["start"], edge["end"], edge["type"][week])

    for vertex in all_vertices:
        g.path = []
        g.type = -1
        g.end = False
        if vertex["name"] == "":
            continue
        check = False
        for neighbour in g.graph[vertex["id"]]:
            if neighbour[1] == 3:
                check = True
        if check:
            g.DFS(vertex["id"])
            if g.type != -1:
                for i in range(len(g.path) - 1):
                    for edge in new_edges:
                        if edge["start"] == g.path[i] and edge["end"] == g.path[i+1] or edge["start"] == g.path[i+1] and edge["end"] == g.path[i]:
                            edge["type"][week] = g.type

with open("edges.json", "w") as f:
    json.dump(new_edges, f, ensure_ascii=False, indent=2)
    
with open("vertices.json", "w") as f:
    json.dump(all_vertices, f, ensure_ascii=False, indent=2)

shp = geopandas.read_file("../dataset/goas/goas_v01.shp").set_crs("EPSG:4326")
shp_right = shp.clip_by_rect(20, 55, 180, 85)
shp_left = shp.clip_by_rect(-180, 55, -160, 85)
shp_left_trans = shp_left.translate(360, 0, 0)
joined = pd.concat([shp_right, shp_left_trans])

idx = 0
for week in excel_reader.sheet_names[2:]:
    fig = plt.figure()
    ax = fig.add_subplot(111)
    t = joined.plot(ax=ax)
    t.set_axis_off()
    plt.tight_layout()
    plt.margins(x=0,y=0)

    edge_id = 0
    for edge in new_edges:
        color= 'green' if edge["type"][week] == 0 else ('blue' if edge["type"][week] == 1 else ("yellow" if edge["type"][week] == 2 else "red"))
        plt.plot([all_vertices[edge["start"]]["lon"], all_vertices[edge["end"]]["lon"]], [all_vertices[edge["start"]]["lat"], all_vertices[edge["end"]]["lat"]], color=color, linewidth=0.3)
        plt.annotate(edge_id, ((all_vertices[edge["start"]]["lon"] + all_vertices[edge["end"]]["lon"]) / 2, (all_vertices[edge["start"]]["lat"] + all_vertices[edge["end"]]["lat"]) / 2), fontsize=1)
        edge_id+=1
    

    for port in all_vertices:
        plt.scatter(port["lon"], port["lat"], s=1, marker='.', color='orange' if port["name"] == "" else 'purple', zorder=100)
        plt.annotate("[" + str(port["id"]) + "]" +port["name"], (port["lon"], port["lat"]), fontsize=1, zorder=101)
    
    

    plt.savefig('../dataset/main_' + str(idx) + '.png', bbox_inches='tight', dpi=1500, pad_inches=0.0)
    idx+=1