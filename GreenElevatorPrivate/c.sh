gcc -c hwAPI/hardwareAPI.c
gcc -o el elevator.c hwAPI/hardwareAPI.o -lpthread
./el $1 $2
