rm a.out
gcc -std=c99 birds.c -lpthread
./a.out $1 $2
