#include "HTTP.h"
#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <sstream>
#include <sys/socket.h>
#include <netdb.h>

namespace http {
	
	HTTPBase::HTTPBase(const HTTPHeaders& headers, const std::string& data, 
			const std::string& document) 
		: m_data(data), m_document(document)
	{
		m_headers = std::move(headers);	
	}

	std::string HTTPBase::data() const {
		return m_data;
	}

	std::string HTTPBase::header(const std::string& name) const {
		try {
			return m_headers.at(name);
		} catch (const std::out_of_range& e) {
			return ""; // Return nothing if the header doesn't exist
		}
	}

	HTTPHeaders& HTTPBase::headers() {
		return m_headers;
	}

	// HTTPRequest
	
	HTTPRequest::HTTPRequest(const HTTPMethod& method, 
			const HTTPHeaders& headers, const std::string& data,
			const std::string& document) 
		:m_method(method), HTTPBase(headers, data, document) 
	{}

	HTTPResponse HTTPRequest::send_request(int sessionId) {
		std::string request = toString();
		send(sessionId, request.c_str(), request.length(), 0);		

		std::stringstream sstream;
		char buf[512];
		size_t len;
		while((len = recv(sessionId, buf, 512, 0)) > 0) {
			sstream << buf;
		}
		std::string response = sstream.str();
		std::cout << response << std::endl;

		return HTTPResponse(200, {}, "", "");
	}

	std::string HTTPRequest::toString() {
		std::stringstream sstream;
		sstream << m_method << " " << m_document << " HTTP/1.1" << "\r\n";
		
		for(const auto& pair : m_headers) {
			sstream << pair.first << ": " << pair.second << "\r\n";
		}

		sstream << "\r\n";
		sstream << m_data;
		return sstream.str();
	}

	HTTPMethod HTTPRequest::method() const {
		return m_method;
	}

	// HTTPResponse

	HTTPResponse::HTTPResponse(int status, const HTTPHeaders& headers, 
			const std::string& data, const std::string& document)
		:m_status(status), HTTPBase(headers, data, document)
	{}

	std::string HTTPResponse::toString() {
		return "";
	}

	int HTTPResponse::status() const {
		return m_status;
	}

	// Open connection to a http server
	
	bool session(const std::string &server, int* sessionId) {
		int socketId = socket(AF_INET, SOCK_STREAM, 0);
		if(socketId < 0) { return false; }

		struct addrinfo hints, *res;
		memset(&hints, 0, sizeof(hints));
		hints.ai_family = AF_INET;
		hints.ai_socktype = SOCK_STREAM;

		if(getaddrinfo(server.c_str(), NULL, &hints, &res) != 0) {
			perror("failed to resolve host name");
			close(socketId);
			return false;
		}

		struct sockaddr_in* server_addr = (struct sockaddr_in*) res->ai_addr;
		server_addr->sin_port = htons(80);

		if(connect(socketId, 
					(struct sockaddr*)server_addr,
					res->ai_addrlen) != 0) {
			perror("failed to connect");
			close(socketId);
			freeaddrinfo(res);
			return false;
		}


		*sessionId = socketId;	
		freeaddrinfo(res);
		return true;
	}
}
