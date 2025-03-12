#pragma once

#include <string>
#include <map>
#include <stdexcept>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

namespace http {

	// Opens a file descriptor to a socket for our http connection
	bool session(const std::string& server, int* sessionId);
	void close_session(int sessionId);

	std::string make_get_url(const std::map<std::string, std::string>& params,
			const std::string& url);
	std::map<std::string, std::string> parse_params(const std::string& pString);

	std::string url_encode(const std::string& data);

	using HTTPHeaders = std::map<std::string, std::string>;
	using HTTPMethod = std::string;

	class HTTPBase {
		protected:
			HTTPHeaders m_headers;
			std::string m_data;
			std::string m_document;

		public:
			HTTPBase(const HTTPHeaders&, const std::string&, const std::string& document);

			std::string data() const;
			std::string header(const std::string&) const;
			HTTPHeaders& headers();

			virtual std::string toString() = 0;
	};

	class HTTPResponse : public HTTPBase {
		private:
			int m_status;

		public:
			HTTPResponse(int, const HTTPHeaders&, const std::string&);

			int status() const;
			std::string toString() override;

		static HTTPResponse fromRawResponse(std::string& res);
	};

	class HTTPRequest : public HTTPBase {
		private:
			HTTPMethod m_method;

		public:
			HTTPRequest(const HTTPMethod&, const HTTPHeaders&, 
					const std::string&); 

			HTTPResponse send_request(int sessionId);

			std::string toString() override;
			HTTPMethod method() const;
	};

}
