CC=gcc
CFLAGS=-Wall -Wpedantic -lyaml -g

main:
	$(CC) $(CFLAGS) hwmon.c fan.c sensor.c zone.c curve.c daemon.c config.c -o main
