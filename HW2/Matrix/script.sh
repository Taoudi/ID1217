gcc-8 -fopenmp lockMatrix.c -o mat
g++ performanceAnalyser.c -o pa
for((i=1;i<=8;i++))
do
	for((j=0;j<3;j++))
		do
			./mat $1 $i >> mat$i
		done
done
./pa
gnuplot performance.p
bash scriptrm.sh
