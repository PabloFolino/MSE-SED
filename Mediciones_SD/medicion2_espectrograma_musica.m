offset_row = 1  % si es = 0, incluye encabezado
offset_col = 1  %  columna inicial
xmin=0
xmax=6000
x = dlmread('A5698295.CSV', ',', offset_row, offset_col);
%spectrogram(x,3,2.8,1024,100,'yaxis')
spectrogram(x , 3 , 2 , 1024 , 100, 'yaxis')
title('Espectograma músia del celular')
colormap('jet')
