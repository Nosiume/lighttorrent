#include "Tracker.h"
#include "Bencoding.h"
#include "HTTP.h"
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unistd.h>

using namespace bparser;

Tracker::Tracker(const std::string& url) {
	if(!http::session(url, &m_session)) {
		throw std::runtime_error("failed to connect to tracker.");
	}

	std::stringstream sstream;
	sstream << "-LT0001-";
	srand(time(0));
	for(int i = 0 ; i < 6 ; i++) {
		unsigned char byte = rand() % 256;
		sstream << std::setw(2) << std::setfill('0') << std::hex << (int)byte;
	}
	m_peerId = sstream.str();
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
		{"Accept", "*/*"},
		{"Host", m_session.host}
	};

	http::HTTPRequest httpRequest("GET", headers, reqUrl);
	http::HTTPResponse res = httpRequest.send_request(m_session);

	// Here we need to do bparser on the response data 
	std::string data = res.data();
	BDict root = parse(data).asDict();

	TrackerResponse trackRes;
	if(root.contains("failure reason")) {
		trackRes.failure_reason = root.at("failure reason").asString();
		return trackRes;
	}

	if(root.contains("tracker id"))
		trackRes.tracker_id = root.at("tracker id").asString();
	trackRes.interval = root.at("interval").asInteger();
	trackRes.min_interval = root.at("min interval").asInteger();
	trackRes.seeders = root.at("complete").asInteger();
	trackRes.leechers = root.at("incomplete").asInteger();
	
	BObject peers = root.at("peers");
	if(peers.isList()) {
		for(const BObject& peer : peers.asList()) {
			BDict peerInfo = peer.asDict();
			Peer p {
				.id = peerInfo.at("peer id").asString(),
				.ip = peerInfo.at("ip").asString(),
				.port = (uint16_t) peerInfo.at("port").asInteger()
			};
			trackRes.peers.push_back(p);
		}
	} else {
		//TODO: implement binary wise peer parsing
	}

	return trackRes;
}
