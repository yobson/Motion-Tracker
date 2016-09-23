set autoscale
set xlabel 'Time (s)'
set ylabel 'Pixles From Center'
plot "C://gnuplot/plot.dat" using 1:2 with lines
pause 1
reread