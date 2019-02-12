set terminal png
set output "performance.png"

set title "Performance for Palindromes size 5000"

set key right center

set xlabel "Amount of Threads"
set ylabel "Time in seconds"


plot "performance0.dat" u 1:2 w linespoints title "Speedup Time per amount of workers"
