
import numpy as np
import matplotlib.pyplot as plt
n=50

f = open ('A5068595.CSV','r')
#texto = f.read() # texto completo
texto = f.readlines(1) # leo el encabezado
linea = f.readlines(n) # línea n
f.close()

col = 1     # nro columna
rowinit = 1 # fila inicial
data = np.genfromtxt('lecturas.csv', delimiter = ',')
x = data[rowinit:,col] # array numpy


fig = plt.figure()
plt.title(texto)    
plt.plot(x, c='blue', label='señal')
plt.legend()
plt.xlabel('muestras')
plt.ylabel('RSSI[dBm]')
plt.show()
