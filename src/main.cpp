#include "HTTP.h"
#include <iostream>

int main() {
	std::map<std::string, std::string> headers = {
		{"User-Agent", "lighttorrent"},
		{"Host", "localhost"},
		{"Accept", "*/*"},
	};
	http::HTTPRequest req("GET", headers, "", "/");
	std::cout << req.toString() << std::endl;

	std::cout << "sending..." << std::endl;

	int id;
	if(!http::session("localhost", &id)) {
		std::cerr << "failed to open session" << std::endl;
		return -1;
	}
	req.send_request(id);

    return 0;
}
