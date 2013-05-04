PORT ?= 50000
HOST ?= 127.0.0.1

all:
	$(MAKE) -C src
	$(MAKE) -C gui

clean:
	/bin/rm gui/*.class src/*.o ./bin/*

run:
	./bin/dhtnode 127.0.0.1 3200 $(HOST) $(PORT)

run-prod:
	./bin/dhtnode dht.mikkokivela.com 80 $(HOST) $(PORT)

run-gui:
	java -cp ./gui GUI
