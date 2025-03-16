#include "HTTP.h"
#include <cctype>
#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <sstream>
#include <string>
#include <sys/socket.h>
#include <netdb.h>
#include <format>

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
			const HTTPHeaders& headers, const std::string& document) 
		:m_method(method), HTTPBase(headers, "", document) 
	{}

	HTTPResponse HTTPRequest::send_request(const HTTPSession& session) {
		std::string request = toString();

		if(session.prot == TCP) {
			send(session.fd, request.c_str(), request.length(), 0);		
		} else {
			sendto(session.fd, 
					request.c_str(), 
					request.length(), 0, 
					(struct sockaddr*)&session.sock, sizeof(struct sockaddr_in));
		}

		std::stringstream sstream;
		char buf[512];
		size_t len;

		if(session.prot == TCP) {
			while((len = recv(session.fd, buf, 512, 0)) > 0) {
				buf[len] = '\0';
				sstream << buf;
			}
		} else {
			socklen_t server_addr_len = sizeof(struct sockaddr_in);
			while((len = recvfrom(session.fd, buf, 512, 0,
							(struct sockaddr*)&session.sock, &server_addr_len))) {
				buf[len] = '\0';
				sstream << buf;
			}
		}

		// This holds the entire data
		std::string response = sstream.str();
		return HTTPResponse::fromRawResponse(response);
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
			const std::string& data)
		:m_status(status), HTTPBase(headers, data, "")
	{}

	std::string HTTPResponse::toString() {
		std::stringstream sstream;
		sstream << std::format("HTTP/1.1 {}\r\n", m_status);
		for(const auto& header: m_headers) {
			sstream << std::format("{}: {}\r\n", header.first, header.second);
		}
		sstream << "\r\n" << m_data << "\r\n\r\n";
		return sstream.str();
	}

	int HTTPResponse::status() const {
		return m_status;
	}

	// Open connection to a http server
	
	bool session(const std::string &server, HTTPSession* session) {
		Protocol prot = server.starts_with("udp") ? UDP : TCP;
		session->prot = prot;
		
		std::string ipport = server.substr(6);
		size_t sep = ipport.find(':');
		size_t port_end = ipport.find("/");

		std::string ip = ipport.substr(0, sep);
		session->host = ip;
		int port = std::stoi(ipport.substr(sep + 1, port_end - sep - 1));
		
		int socketId = socket(AF_INET, prot, 0);
		if(socketId < 0) { return false; }

		struct addrinfo hints, *res;
		memset(&hints, 0, sizeof(hints));
		hints.ai_family = AF_INET;
		hints.ai_socktype = prot;

		if(getaddrinfo(ip.c_str(), NULL, &hints, &res) != 0) {
			perror("failed to resolve host name");
			close(socketId);
			return false;
		}

		struct sockaddr_in* server_addr = (struct sockaddr_in*) res->ai_addr;
		server_addr->sin_port = htons(port);

		int code;
		if(prot == TCP) {
			code = connect(socketId,
					(struct sockaddr*)server_addr,
					res->ai_addrlen);
		}  else {
			struct sockaddr_in local;
			memset(&local, 0, sizeof(struct sockaddr_in));
			local.sin_port = htons(6881);
			local.sin_addr.s_addr = INADDR_ANY;
			local.sin_family = AF_INET;
			code = bind(socketId, (struct sockaddr*) &local, sizeof(struct sockaddr_in));
		}

		if(code < 0) {
			perror("failed to connect");
			close(socketId);
			freeaddrinfo(res);
			return false;
		}

		memcpy(server_addr, &session->sock, res->ai_addrlen);
		session->fd = socketId;	
		freeaddrinfo(res);
		return true;
	}

	void close_session(const HTTPSession& session) {
		close(session.fd);
	}

	std::string make_get_url(const std::map<std::string, std::string> &params, 
			const std::string &url) {
		std::stringstream ss;
		ss << url;
		ss << '?';

		bool hasPrev = false;
		for(const auto& entry : params) {
			if(hasPrev) ss << '&';
			ss << entry.first << '=' << url_encode(entry.second);
			if(!hasPrev) hasPrev = true;
		}
		return ss.str();
	}

	std::map<std::string, std::string> parse_params(const std::string& pString) {
		size_t start = pString.find('?');
		std::string params = pString.substr(start + 1);

		std::map<std::string, std::string> res;
		std::stringstream ss(params);
		std::string line;
		while(getline(ss, line, '&')) {
			size_t sep = line.find('=');
			std::string key = line.substr(0, sep);
			std::string value = line.substr(sep + 1);
			res.emplace(key, value);
		}
		return res;
	}

	std::string url_encode(const std::string& data) {
		std::stringstream ss;
		for(char c : data) {
			if(std::isalnum(c) || c == '_' || c == '-' || c == '.' | c == '~') ss << c;
			else ss << std::uppercase << std::format("%{:02x}", (unsigned char) c);
		}
		return ss.str();
	}

	// Util
	HTTPResponse HTTPResponse::fromRawResponse(std::string &res) {
		size_t start = res.find(' ') + 1;
		size_t end = res.find(' ', start);
		int response = std::stoi(res.substr(start, end - start));
		HTTPHeaders headers;

		std::stringstream sstream(res);
		std::string line;
		while(std::getline(sstream, line)) {
			if(line[0] == '\r') break;  // No need to check newline since it's deleted

			size_t delimPos = line.find(':');
			if(delimPos == std::string::npos) continue;

			std::string key = line.substr(0, delimPos);
			std::string value = line.substr(delimPos + 1);
			headers.emplace(key, value);
		}

		std::string body = sstream.str().substr(sstream.tellg());
		return HTTPResponse(response, headers, body);
	}
}
