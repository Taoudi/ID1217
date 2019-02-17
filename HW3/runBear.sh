rm a.out
gcc -std=c99 bear.c -lpthread 
./a.out $1 $2
