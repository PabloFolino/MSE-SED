offset_row = 1  % si es = 0, incluye encabezado
offset_col = 1  %  columna inicial
xmin=0
xmax=6000
x = dlmread('A5847475.CSV', ',', offset_row, offset_col);
plot(x, 'color', 'b')
xlim([xmin xmax])
xlabel('tiempo[mseg]')
ylabel('Valor ADC')
title('Gráfico de la señal en función del tiempo movimiento a mano')
