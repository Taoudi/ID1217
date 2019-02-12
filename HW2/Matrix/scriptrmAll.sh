	for((j=1;j<11;j++))
		do
			rm mat$j
		done
		for((j=10;j<12000;j=j+2500))
			do
				rm performance$j.dat
			done
rm performance.dat
rm mat
rm pa
