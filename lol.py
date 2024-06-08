import geopandas
import matplotlib.pyplot as plt
import pandas as pd
from mpl_toolkits.basemap import Basemap
import numpy as np


sheet_names = ["03-Mar-2020", "10-Mar-2020", "17-Mar-2020", "24-Mar-2020", 
"31-Mar-2020", "02-Apr-2020", "07-Apr-2020", "14-Apr-2020", "21-Apr-2020", "28-Apr-2020", "05-May-2020", "12-May-2020",
"19-May-2020", "26-May-2020"]

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
            if lat.iloc[i,j] < 65:
                continue
            lons.append(lon.iloc[i, j])
            lats.append(lat.iloc[i, j])
            ices.append(ice.iloc[i, j])
    
    # x, y = m(lons, lats)
    return lons, lats, ices

shp = geopandas.read_file("goas_v01.shp")

# xmin, ymin, xmax, ymax
shp_right = shp.clip_by_rect(20, 65, 180, 90)
shp_left = shp.clip_by_rect(-180, 65, -160, 90)
shp_left_trans = shp_left.translate(360, 0, 0)
joined = pd.concat([shp_right, shp_left_trans])

idx = 0
for name in sheet_names:
    x, y, ices = form_matrix(name)
    fig, ax = plt.subplots(figsize=(20, 10))
    joined.plot(ax=ax)
    plt.scatter(x, y, color=['green' if ice >= 20 else ('blue' if ice >= 15 else ("yellow" if ice >= 10 else "red")) for ice in ices], marker='.',s=3)
    plt.savefig('dataset/' + str(idx) + "_" + name + '.png', bbox_inches='tight', dpi=300)
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