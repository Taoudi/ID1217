set terminal png
set output "performance.png"

set title "Performance for Matrix"

set key right bottom

set xlabel "Amount of Threads"
set ylabel "Time in seconds"


plot "performance10.dat" u 1:2 w linespoints title " matrix size 10" ,\
 "performance2510.dat" u 1:2 w linespoints title "matrix size 2500" ,\
 "performance5010.dat" u 1:2 w linespoints title " matrix size 5000" ,\
 "performance7510.dat" u 1:2 w linespoints title " matrix size 7500" ,\
 "performance10010.dat" u 1:2 w linespoints title " matrix size 10000"
