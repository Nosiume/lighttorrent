#include "Tracker.h"
#include "Bencoding.h"
#include "HTTP.h"
#include <string>

Tracker::Tracker(const std::string& url) {
	if(!http::session(url, &m_session)) {
		throw std::runtime_error("failed to connect to tracker.");
	}
	//TODO: initialize peerid
}

Tracker::~Tracker() {
	http::close_session(m_session);
}

// Util

TrackerResponse Tracker::query(const TrackerRequest& req) {
	std::map<std::string, std::string> params = {
		{"info_hash", req.info_hash},
		{"peer_id", m_peerId},
		{"port", std::to_string(req.port)},
		{"uploaded", std::to_string(req.uploaded)},
		{"downloaded", std::to_string(req.downloaded)},
		{"left", std::to_string(req.left)},
		{"compact", std::to_string(req.compact)},
		{"no_peer_id", std::to_string(req.no_peer_id)},
	};

	//Add event
	switch(req.event) {
		case STARTED:
			params.emplace("event", "started");
			break;
		case STOPPED:
			params.emplace("event", "stopped");
			break;
		case COMPLETED:
			params.emplace("event", "completed");
			break;
		default:
			break;
	}
	
	std::string reqUrl = http::make_get_url(params, "/announce");
	http::HTTPHeaders headers = {
		{"User-Agent", "lighttorrent"},
		{"Accept", "*/*"}
	};

	http::HTTPRequest httpRequest("GET", headers, reqUrl);
	http::HTTPResponse res = httpRequest.send_request(m_session);

	// Here we need to do bparser on the response data 
}
