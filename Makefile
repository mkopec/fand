CC=gcc
CFLAGS=-Wall -Wpedantic -g
LDFLAGS=-lyaml

main:
	$(CC) $(CFLAGS) hwmon.c fan.c sensor.c zone.c curve.c daemon.c config.c -o main $(LDFLAGS)

test:
	$(CC) $(CFLAGS) hwmon.c fan.c sensor.c zone.c curve.c config.c test.c -o test_runner $(LDFLAGS)
	./test_runner

.PHONY: test
