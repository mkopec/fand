CC=gcc
CFLAGS=-Wall -Wpedantic -lyaml -g

main:
	$(CC) $(CFLAGS) hwmon.c fan.c sensor.c zone.c curve.c daemon.c config.c -o main

test:
	$(CC) $(CFLAGS) hwmon.c fan.c sensor.c zone.c curve.c config.c test.c -o test_runner
	./test_runner

.PHONY: test
