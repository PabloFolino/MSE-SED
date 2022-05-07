offset_row = 1  % si es = 0, incluye encabezado
offset_col = 1  %  columna inicial
xmin=0
xmax=6000
x = dlmread('A5068595.CSV', ',', offset_row, offset_col);
xbins=linspace(1712,2373,50)
histogram(x,xbins)
xlabel('Valor ADC')
ylabel('N')
title('Histograma 50bins')
Media=mean(x)
Varianza=var(x)