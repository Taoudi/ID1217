g++-8 -fopenmp palindromes.c -o pal -O2
g++ performanceAnalyser.c -o pa
for((i=1;i<=10;i++))
do 	
	for((j=0;j<100;j++))
		do
			./$1 $i  >> pal$i
		done
done
./pa 
gnuplot performance.p
bash scriptrm.sh
