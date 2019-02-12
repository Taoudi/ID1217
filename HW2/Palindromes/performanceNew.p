set terminal png
set output "performance.png"

set title "Performance for Palindromes size 21453"

set key right bottom

set xlabel "Amount of Threads"
set ylabel "Time in seconds"


plot "performance100.dat" u 1:2 w linespoints title " dictionary size 100" ,\
 "performance5100.dat" u 1:2 w linespoints title "dictionary size 5000" ,\
 "performance10100.dat" u 1:2 w linespoints title " dictionary size 10000" ,\
 "performance15100.dat" u 1:2 w linespoints title " dictionary size 15000" ,\
 "performance20100.dat" u 1:2 w linespoints title " dictionary size 20000"
