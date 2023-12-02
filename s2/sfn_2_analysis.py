import pandas as pd
import matplotlib.pyplot as plt
import numpy as np

df = pd.read_csv("output\degree_distribution.csv", header=None, names=['degree', 'num'])
df = df[df['num'] != 0]
df = np.log(df)
plt.scatter(x = df['degree'], y = df['num'], alpha = 0.7, color = "teal")
plt.grid()
plt.xlabel("Degree of Vertexes")
plt.ylabel("Number of Corresponding Vertex")
plt.show()