CC=gcc
CFLAGS=-Wall -Wpedantic -g
LDFLAGS=-lyaml -lm

PREFIX=/usr/local
BINDIR=$(PREFIX)/bin
SYSCONFDIR=/etc
UNITDIR=/usr/lib/systemd/system

main:
	$(CC) $(CFLAGS) hwmon.c fan.c sensor.c zone.c curve.c daemon.c config.c -o main $(LDFLAGS)

test:
	$(CC) $(CFLAGS) hwmon.c fan.c sensor.c zone.c curve.c config.c test.c -o test_runner $(LDFLAGS)
	./test_runner

install: main
	install -Dm755 main $(DESTDIR)$(BINDIR)/fand
	install -Dm644 fand.service $(DESTDIR)$(UNITDIR)/fand.service
	install -Dm644 -b config.yaml $(DESTDIR)$(SYSCONFDIR)/fand.conf

uninstall:
	rm -f $(DESTDIR)$(BINDIR)/fand
	rm -f $(DESTDIR)$(UNITDIR)/fand.service

clean:
	rm -f main test_runner

.PHONY: test install uninstall clean
