# To run benchmark test
gcc-8 -fopenmp lockMatrix.c -o mat
g++ performanceAnalyser.c -o pa

for((k=10;k<7700;k=k+2500))
do
	for((i=1;i<=8;i++))
		do
			for((j=0;j<3;j++))
				do
					./mat $k $i >> mat$i
				done
			done
			echo $k
	./pa $k
	for((j=1;j<9;j++))
		do
			rm mat$j
		done
done
gnuplot performanceNew.p
bash scriptrmAll.sh
