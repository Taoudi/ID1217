set terminal png
set output "performance.png"

set title "Performance for Matrix"

set key right center

set xlabel "Amount of Threads"
set ylabel "Speedup Time in seconds"


plot "performance.dat" u 1:2 w linespoints title "Speedup time per amount of workers"
