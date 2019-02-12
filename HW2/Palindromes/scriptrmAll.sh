	for((j=1;j<11;j++))
		do
			rm pal$j
		done
		for((j=100;j<22000;j=j+5000))
			do
				rm performance$j.dat
			done
rm performance.dat
rm pal
rm pa
