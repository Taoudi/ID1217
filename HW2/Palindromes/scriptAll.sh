# To run benchmark test
g++-8 -fopenmp palindromes.c -o pal -O2
g++ performanceAnalyser.c -o pa

for((k=100;k<22000;k=k+5000))
do
	for((i=1;i<=10;i++))
		do
			for((j=0;j<5;j++))
				do
					./pal $i $k >> pal$i
				done
			done
			echo $k
	./pa $k
	for((j=1;j<11;j++))
		do
			rm pal$j
		done
done
gnuplot performanceNew.p
bash scriptrmAll.sh
