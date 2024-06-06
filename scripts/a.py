from mpl_toolkits.basemap import Basemap
import numpy as np
import matplotlib.pyplot as plt

import pandas as pd
lon = pd.read_excel("dataset/IntegrVelocity.xlsx", sheet_name="lon")
lat = pd.read_excel("dataset/IntegrVelocity.xlsx", sheet_name="lat")
ice = pd.read_excel("dataset/IntegrVelocity.xlsx", sheet_name="03-Mar-2020")

print(lon.shape)
print(lat.shape)
print(ice.shape)
# .iloc[row, col]

lons, lats, ices = [], [], []
for i in range(lon.shape[0]):
    for j in range(lon.shape[1]):
        # if lon.iloc[i, j] > 180:
        #     continue
        lons.append(lon.iloc[i, j])
        lats.append(lat.iloc[i, j])
        ices.append(ice.iloc[i, j])

def map_colors(x, _1, _2):
    print(x, type(x))
    pass

m = Basemap(boundinglat=65, lon_0=90, resolution='i', round = False, llcrnrlon = 20, urcrnrlon = 180, llcrnrlat = 60, urcrnrlat = 85)   

plt.figure(figsize=(12,8))
    
m.drawcoastlines(linewidth=0.2)
m.fillcontinents(color='coral',lake_color='aqua')
# draw parallels and meridians.
m.drawparallels(np.arange(-80.,81.,20.),labels=[1,0,0,0])
m.drawmeridians(np.arange(-180.,181.,20.),labels=[1,0,0,1])
m.drawmapboundary(fill_color='grey')
x,y=m(lons,lats)
# USE C parameter FOR COLORS FOR COLORIZE VALUES
m.scatter(x,y, color=['green' if ice >= 20 else ('blue' if ice >= 15 else ("yellow" if ice >= 10 else "red")) for ice in ices], marker='.',s=3, latlon=True)
m.colorbar(label='Brightness Temperature (K)')
plt.rcParams["figure.dpi"] = 300
plt.title ("Говна")
plt.show()