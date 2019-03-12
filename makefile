compile: src/rv64ins.c src/decodingRVG.c src/decodingRVC.c src/csr.c
	gcc -o rv64dis -std=c99 src/rv64ins.c src/decodingRVG.c src/decodingRVC.c src/csr.c
install: rv64dis
	install -m 0755 rv64dis /usr/local/bin
