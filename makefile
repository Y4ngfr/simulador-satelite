run: 
	gcc ./cJSON/cJSON.c main.c -o ./out/main -lm

install: 
	git clone https://github.com/DaveGamble/cJSON.git
	cd cJSON
	mkdir build
	cd build
	cmake .. -DENABLE_CJSON_UTILS=On -DENABLE_CJSON_TEST=Off -DCMAKE_INSTALL_PREFIX=/usr
	make
	make DESTDIR=$pkgdir install