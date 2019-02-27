compile: rv64ins.c decodingRV32.c decodingRVC.c csr.c
	gcc -o rv64dis -std=c99 rv64ins.c decodingRV32.c decodingRVC.c csr.c
install: rv64dis
	install -m 0755 rv64dis /usr/local/bin
