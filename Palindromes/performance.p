set terminal png
set output "performance.png"

set title "Performance for Palindromes"

set key right center

set xlabel "Amount of Threads"
set ylabel "Time in seconds"


plot "performance.dat" u 1:2 w linespoints title "time"
