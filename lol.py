import geopandas
import matplotlib.pyplot as plt
import pandas as pd
from mpl_toolkits.basemap import Basemap
import numpy as np
from scipy.interpolate import NearestNDInterpolator, RegularGridInterpolator, CloughTocher2DInterpolator


sheet_names = ["03-Mar-2020", "10-Mar-2020", "17-Mar-2020", "24-Mar-2020", 
"31-Mar-2020", "02-Apr-2020", "07-Apr-2020", "14-Apr-2020", "21-Apr-2020", "28-Apr-2020", "05-May-2020", "12-May-2020",
"19-May-2020", "26-May-2020"]

edges = [{"start" : 44, "end":	15, "len":	270.0166416},
{"start" : 10, "end":	11, "len":	277.1898363},
{"start" : 18, "end":	39, "len":	58.39035132},
{"start" : 13, "end":	16, "len":	238.8589885},
{"start" : 10, "end":	13, "len":	86.93004916},
{"start" : 21, "end":	5, "len":	655.3557374},
{"start" : 4, "end":	5, "len":	410.5701116},
{"start" : 45, "end":	37, "len":	119.5634492},
{"start" : 13, "end":	7, "len":	206.8821279},
{"start" : 9, "end":	24, "len":	191.1526107},
{"start" : 27, "end":	18, "len":	178.645412},
{"start" : 28, "end":	33, "len":	205.7748943},
{"start" : 40, "end":	22, "len":	134.3049847},
{"start" : 42, "end":	30, "len":	174.3916815},
{"start" : 31, "end":	3, "len":	81.8159608},
{"start" : 0, "end":	43, "len":	123.6831421},
{"start" : 12, "end":	24, "len":	246.2458649},
{"start" : 12, "end":	42, "len":	339.778226},
{"start" : 15, "end":	2, "len":	270.5921169},
{"start" : 10, "end":	35, "len":	127.6546236},
{"start" : 9, "end":	32, "len":	140.7360068},
{"start" : 2, "end":	3, "len":	278.1227556},
{"start" : 0, "end":	1, "len":	251.8626597},
{"start" : 33, "end":	18, "len":	225.0238977},
{"start" : 40, "end":	42, "len":	152.2019466},
{"start" : 4, "end":	41, "len":	288.1919175},
{"start" : 8, "end":	12, "len":	121.8205679},
{"start" : 2, "end":	29, "len":	315.3047628},
{"start" : 15, "end":	3, "len":	69.16861273},
{"start" : 12, "end":	40, "len":	234.8941148},
{"start" : 19, "end":	8, "len":	47.63018916},
{"start" : 28, "end":	30, "len":	308.4151996},
{"start" : 13, "end":	0, "len":	77.9639623},
{"start" : 21, "end":	2, "len":	445.9281382},
{"start" : 30, "end":	18, "len":	436.2716913},
{"start" : 40, "end":	9, "len":	229.4577372},
{"start" : 21, "end":	4, "len":	297.1508085},
{"start" : 16, "end":	34, "len":	296.3206745},
{"start" : 45, "end":	17, "len":	373.6308274},
{"start" : 17, "end":	18, "len":	281.0870454},
{"start" : 18, "end":	26, "len":	98.81552937},
{"start" : 38, "end":	46, "len":	460.2848327},
{"start" : 30, "end":	33, "len":	260.5701136},
{"start" : 24, "end":	22, "len":	179.7422785},
{"start" : 10, "end":	16, "len":	263.6393465},
{"start" : 27, "end":	45, "len":	355.6231614},
{"start" : 7, "end":	36, "len":	157.6035239},
{"start" : 7, "end":	20, "len":	192.4522229},
{"start" : 34, "end":	13, "len":	215.0532376},
{"start" : 43, "end":	36, "len":	223.5714878},
{"start" : 6, "end":	7, "len":	352.936821},
{"start" : 10, "end":	25, "len":	108.6630553},
{"start" : 37, "end":	38, "len":	414.6912251},
{"start" : 13, "end":	15, "len":	410.4549185},
{"start" : 7, "end":	16, "len":	260.1771091},
{"start" : 6, "end":	16, "len":	171.4172136},
{"start" : 36, "end":	19, "len":	116.7439955},
{"start" : 33, "end":	23, "len":	261.600589},
{"start" : 20, "end":	16, "len":	409.3111575},
{"start" : 10, "end":	15, "len":	326.4328417},
{"start" : 34, "end":	15, "len":	195.7328443},
{"start" : 44, "end":	2, "len":	156.0644819},
{"start" : 14, "end":	0, "len":	45.6421557},
{"start" : 8, "end":	9, "len":	206.5979139},
{"start" : 2, "end":	5, "len":	456.2676239},
{"start" : 16, "end":	21, "len":	145.1826364},
{"start" : 27, "end":	28, "len":	275.6604549},
{"start" : 13, "end":	43, "len":	130.3081116},
{"start" : 19, "end":	20, "len":	58.95750201},
{"start" : 6, "end":	15, "len":	280.7806599},
{"start" : 2, "end":	41, "len":	227.2269304},
{"start" : 22, "end":	23, "len":	124.9092209},
{"start" : 17, "end":	28, "len":	285.0795135}]

ports = [{"id":0 ,"lat": 73.1, "lon": 80,     "name": "Бухта Север и Диксон"},
         {"id":1 ,"lat": 69.4, "lon": 86.15	, "name": "Дудинка"},
         {"id":2 ,"lat": 69.9, "lon": 44.6	, "name": "кромка льда на Западе"},
         {"id":3 ,"lat": 69.15,"lon": 57.68	, "name": "Варандей-Приразломное"},
         {"id":4 ,"lat": 73,   "lon": 44	, "name": "Штокман"},
         {"id":5 ,"lat": 71.5, "lon": 22	, "name": "Окно в Европу"},
         {"id":6 ,"lat": 74.6, "lon": 63.9	, "name": "Победа месторождение"},
         {"id":7 ,"lat": 76.4, "lon": 86.4	, "name": "Карское - 3 (центр)"},
         {"id":8 ,"lat": 77.6, "lon": 107.7	, "name": "пролив Вилькицкого - 3"},
         {"id":9 ,"lat": 74.9, "lon": 116.7	, "name": "Лаптевых - 4 (юг)"},
         {"id":10 ,"lat": 73.1, "lon": 72.7	, "name": "Вход в Обскую губу"},
         {"id":11 ,"lat": 68.5, "lon": 73.7	, "name": "Новый порт"},
         {"id":12 ,"lat": 76.75,"lon": 116	, "name": "Лаптевых - 1 (центр)"},
         {"id":13 ,"lat": 74,   "lon": 76.7	, "name": "Карское - 1 (сбор каравана)"},
         {"id":14 ,"lat": 72.35,"lon": 79.6	, "name": "Лескинское м-е"},
         {"id":15 ,"lat": 70.3, "lon": 57.8	, "name": "Карские ворота"},
         {"id":16 ,"lat": 77.3, "lon": 67.7	, "name": "Мыс Желания"},
         {"id":17 ,"lat": 71.74,"lon": 184.7, "name": "остров Врангеля"},
         {"id":18 ,"lat": 70.7, "lon": 170.5, "name": "Восточно-Сибирское - 1 (восток)"},
         {"id":19 ,"lat": 77.8, "lon": 104.1, "name": "пролив Вилькицкого - восток"},
         {"id":20 ,"lat": 77.7, "lon": 99.5	, "name": "пролив Вилькицкого - запад"},
         {"id":21 ,"lat": 76.2, "lon": 58.3	, "name": "около Новой Земли"},
         {"id":22 ,"lat": 74.4, "lon": 139	, "name": "Пролив Санникова - 1"},
         {"id":23 ,"lat": 74.3, "lon": 146.7, "name": "Пролив Санникова - 2"},
         {"id":24 ,"lat": 74,   "lon": 128.1, "name": "устье Лены"},
         {"id":25 ,"lat": 71.3, "lon": 72.15, "name": "Сабетта"},
         {"id":26 ,"lat": 69.1, "lon": 169.4, "name": "мыс.Наглёйнын"},
         {"id":27 ,"lat": 69.9, "lon": 179	, "name": "пролив Лонга"},
         {"id":28 ,"lat": 73.5, "lon": 169.9, "name": "Восточно-Сибирское - 3 (север)"},
         {"id":29 ,"lat": 64.95,"lon": 40.05, "name": "Архангельск"},
         {"id":30 ,"lat": 75.9, "lon": 152.6, "name": "Лаптевых - 3 (восток)"},
         {"id":31 ,"lat": 68.37,"lon": 54.6	, "name": "МОТ Печора"},
         {"id":32 ,"lat": 73.7, "lon": 109.26, "name": "Хатангский залив"},
         {"id":33 ,"lat": 72,   "lon": 159.5, "name": "Восточно-Сибирское - 2 (запад)"},
         {"id":34 ,"lat": 72.4, "lon": 65.6	, "name": "Ленинградское-Русановское"},
         {"id":35 ,"lat": 71,   "lon": 73.73, "name": "терминал Утренний"},
         {"id":36 ,"lat": 76.5, "lon": 97.6	, "name": "Таймырский залив"},
         {"id":37 ,"lat": 64.2, "lon": 188.2, "name": "Берингово"},
         {"id":38 ,"lat": 60.7, "lon": 175.3, "name": "кромка льда на Востоке"},
         {"id":39 ,"lat": 69.75,"lon": 169.9, "name": "Рейд Певек"},
         {"id":40 ,"lat": 75.5, "lon": 131.5, "name": "Лаптевых - 2 (центр)"},
         {"id":41 ,"lat": 69.5, "lon": 33.75, "name": "Рейд Мурманска"},
         {"id":42 ,"lat": 76.7, "lon": 140.8, "name": "остров Котельный"},
         {"id":43 ,"lat": 74.8, "lon": 84.2	, "name": "Карское - 2 (прибрежный)"},
         {"id":44 ,"lat": 67.58,"lon": 47.82, "name": "Индига"},
         {"id":45 ,"lat": 65.9, "lon": 190.65, "name": "Берингов пролив"},
         {"id":46 ,"lat": 55.7, "lon": 164.25, "name": "Окно в Азию"}]





def form_matrix(name):
    lon = pd.read_excel("dataset/IntegrVelocity.xlsx", sheet_name="lon")
    lat = pd.read_excel("dataset/IntegrVelocity.xlsx", sheet_name="lat")
    ice = pd.read_excel("dataset/IntegrVelocity.xlsx", sheet_name=name)

    print(lon.shape)
    print(lat.shape)
    print(ice.shape)

    lons, lats, ices = [], [], []
    for i in range(lon.shape[0]):
        for j in range(lon.shape[1]):
            if lat.iloc[i,j] < 55 or lat.iloc[i,j] > 85:
                continue
            lons.append(lon.iloc[i, j])
            lats.append(lat.iloc[i, j])
            ices.append(ice.iloc[i, j])
    
    # x, y = m(lons, lats)
    return lons, lats, ices

shp = geopandas.read_file("goas_v01.shp")

# xmin, ymin, xmax, ymax
shp_right = shp.clip_by_rect(20, 55, 180, 85)
shp_left = shp.clip_by_rect(-180, 55, -160, 85)
shp_left_trans = shp_left.translate(360, 0, 0)
joined = pd.concat([shp_right, shp_left_trans])


fig = plt.figure()
ax = fig.add_subplot(111)
t = joined.plot(ax=ax)
t.set_axis_off()
plt.tight_layout()

x, y, ices = form_matrix("03-Mar-2020")

grid_x = np.linspace(20, 200, 1000)
grid_y = np.linspace(55, 85, 1000)

X, Y = np.meshgrid(grid_x, grid_y) 
interp = NearestNDInterpolator(list(zip(x, y)), ices)
Z = interp(X, Y)
from matplotlib.colors import ListedColormap, BoundaryNorm
# define color map & norm it
colours = (["white", "magenta", "blue", "green"])
cmap = ListedColormap(colours)
bounds=[-10, 10, 15, 20, np.max(Z) + 1] # discrete values of Z
norm = BoundaryNorm(bounds, cmap.N)

plt.pcolormesh(X, Y, Z, cmap=cmap, norm=norm, alpha=0.7, shading="gouraud")

#plt.scatter(good_x, good_y, color=['green' if ice >= 20 else ('blue' if ice >= 15 else ("yellow" if ice >= 10 else "red")) for ice in good_val], marker='.',s=3, alpha=0.1)

for port in ports:
    plt.scatter(port["lon"], port["lat"], s=3, marker='.', color='orange')
    plt.annotate(port["name"], (port["lon"], port["lat"]), fontsize=2)

for edge in edges:
    plt.plot([ports[edge["start"]]["lon"], ports[edge["end"]]["lon"]], [ports[edge["start"]]["lat"], ports[edge["end"]]["lat"]], color='orange', linewidth=0.3)

# ax.imshow([x, y, ices], interpolation='sinc', cmap='viridis')
# plt.show()

plt.savefig('dataset/main.jpg', bbox_inches='tight', dpi=2000, pad_inches=0.0)
idx = 0
for name in sheet_names:
    x, y, ices = form_matrix(name)
    fig = plt.figure()
    ax = fig.add_subplot(111)
    t = joined.plot(ax=ax)
    t.set_axis_off()
    plt.scatter(x, y, color=['green' if ice >= 20 else ('blue' if ice >= 15 else ("yellow" if ice >= 10 else "red")) for ice in ices], marker='.',s=3)
    plt.tight_layout()
    plt.savefig('dataset/' + str(idx) + "_" + name + '.png', bbox_inches='tight', dpi=2000, pad_inches=0.0)
    idx += 1

# print(shp.__init__)
# fig, ax = plt.subplots()
# shp_new.plot(ax=ax)
#plt.show()
# round = True)   

# plt.figure(figsize=(12,8))
    
# m.drawcoastlines(linewidth=0.2)
# m.fillcontinents(color='coral',lake_color='aqua')
# # draw parallels and meridians.
# m.drawparallels(np.arange(-80.,81.,20.),labels=[1,0,0,0])
# m.drawmeridians(np.arange(-180.,181.,20.),labels=[1,0,0,1])
# m.drawmapboundary(fill_color='grey')
# x,y=m(lons,lats)
# # USE C parameter FOR COLORS FOR COLORIZE VALUES
# m.scatter(x,y, color=['green' if ice >= 20 else ('blue' if ice >= 15 else ("yellow" if ice >= 10 else "red")) for ice in ices], marker='.',s=3)
# m.colorbar(label='Brightness Temperature (K)')
# plt.rcParams["figure.dpi"] = 300
# plt.title ("Говна")
# plt.show()