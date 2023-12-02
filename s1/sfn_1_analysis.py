import pandas as pd
import numpy as np
import matplotlib.pyplot as plt

df = pd.read_csv("kDistribution_data.csv", names = ['K', ' P(k)'])
df = df[df[' P(k)'] != 0]
df = df[df['K'] != 0]
df = np.log(df)

plt.scatter(df['K'], df[' P(k)'])
plt.xlabel('Degree (K)')
plt.ylabel('Number of Nodes')
plt.show()

df2 = pd.read_csv("locations.csv", header = None ,names = ['x1', 'y1', 'x2', 'y2', 'x3', 'y3', 'x4', 'y4', 'i'])
df2 = df2.drop(columns=['i'])
df2.bfill(inplace=True)


x1 = df2['x1']
y1 = df2['y1']

x2 = df2['x2']
y2 = df2['y2']

x3 = df2['x3']
y3 = df2['y3']

x4 = df2['x4']
y4 = df2['y4']

plt.figure(figsize=(100, 100), dpi = 70)  
plt.plot([x1, x2], [y1, y2], 'b-', alpha = 0.7)
plt.plot([x1, x3], [y1, y3], 'g-', alpha = 0.7)
plt.plot([x1, x4], [y1, y4], 'r-', alpha = 0.7)

plt.title('Connections between Points')
plt.xlabel('X')
plt.ylabel('Y')

plt.savefig('plot.png', dpi=300)  

plt.show()