all:
	$(MAKE) -C src
	$(MAKE) -C gui

clean:
	/bin/rm gui/*.class src/*.o ./bin/*

run:
	./bin/dhtnode dht.mikkokivela.com 80 127.0.0.1 50000

run-gui:
	java -cp ./gui GUI
