#include <fstream>
#include <iostream>
#include "Torrent.h"


int main() {
	std::ifstream file("/dev/urandom");
	char buf[20];
	file >> buf;
	file.close();

	std::string peerId(buf, 20);
	Torrent torrent("../tests/journal.torrent");
	std::cout << torrent.toString() << std::endl;

    return 0;
}
