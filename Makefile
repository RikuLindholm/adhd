PORT ?= 50000
HOST ?= 127.0.0.1
UIPORT ?= 52000

all:
	mkdir -p bin
	$(MAKE) -C src
	$(MAKE) -C gui

clean:
	/bin/rm gui/*.class src/*.o ./bin/*

run:
	./bin/dhtnode 127.0.0.1 3200 $(HOST) $(PORT) $(UIPORT)

run-alt:
	./bin/dhtnode 127.0.0.1 3200 $(HOST) 50100 51000

run-prod:
	./bin/dhtnode dht.mikkokivela.com 80 $(HOST) $(PORT) $(UIPORT)

run-prod-alt:
	./bin/dhtnode dht.mikkokivela.com 80 $(HOST) 50100 51000

run-gui:
	java -cp ./gui GUI $(UIPORT)

run-gui-alt:
	java -cp ./gui GUI 51000
