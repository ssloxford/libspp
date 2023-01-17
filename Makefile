DIRS=bin/

all: sppinfo spppack sppunpack sppfilter

sppinfo: src/sppinfo.cpp
	g++ -static --std=c++20 -o bin/sppinfo -Wl,-rpath=/usr/local/lib -I ./include/ -g src/sppinfo.cpp -lfec

spppack: src/spppack.cpp
	g++ -static --std=c++20 -o bin/spppack -Wl,-rpath=/usr/local/lib -I ./include/ -g src/spppack.cpp -lfec

sppunpack: src/sppunpack.cpp
	g++ -static --std=c++20 -o bin/sppunpack -Wl,-rpath=/usr/local/lib -I ./include/ -g src/sppunpack.cpp -lfec

sppfilter: src/sppfilter.cpp
	g++ -static --std=c++20 -o bin/sppfilter -Wl,-rpath=/usr/local/lib -I ./include/ -g src/sppfilter.cpp -lfec

.PHONY: install
install:
	install -Dm 755 -t /usr/local/include/libspp/ ./include/libspp/*
	install -D -m 755 bin/sppinfo /usr/local/bin/
	install -D -m 755 bin/spppack /usr/local/bin/
	install -D -m 755 bin/sppunpack /usr/local/bin/

$(shell mkdir -p $(DIRS))
