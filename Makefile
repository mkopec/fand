CC=gcc
CFLAGS=-Wall -Wpedantic -lconfig -g

main:
	$(CC) $(CFLAGS) fan.c sensor.c zone.c curve.c daemon.c config.c -o main
