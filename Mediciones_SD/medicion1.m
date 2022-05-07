offset_row = 1  % si es = 0, incluye encabezado
offset_col = 1  %  columna inicial
xmin=0
xmax=6000
x = dlmread('A5068595.CSV', ',', offset_row, offset_col);
plot(x, 'color', 'b')
xlim([xmin xmax])
xlabel('tiempo[mseg]')
ylabel('Valor ADC')
title('Gráfico de la señal en función del tiempo')

xbins=linspace(1712,2373,10)
histogram(x,xbins)