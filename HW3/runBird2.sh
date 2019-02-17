rm a.out
gcc -std=c99 birds2.c -lpthread 
./a.out $1 $2
