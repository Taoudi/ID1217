g++-8 -fopenmp palindromes.c -o pal -O2
g++ performanceAnalyser.c -o pa
for((i=1;i<=10;i++))
do
	for((j=0;j<10;j++))
		do
			./pal $i $1  >> pal$i
		done
done
./pa
gnuplot performance.p
bash scriptrm.sh
