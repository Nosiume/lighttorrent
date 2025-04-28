#pragma once

#include "HTTP.h"
#include <optional>
#include <string>
#include <vector>


enum TrackerEvent {
	STARTED,
	STOPPED,
	COMPLETED,
	NONE // Instead of using optional
};

struct TrackerRequest {
	std::string info_hash;
	short port;
	size_t uploaded;
	size_t downloaded;
	size_t left;
	bool compact = false; // Not supported yet
	bool no_peer_id = false; // Not supported yet
	TrackerEvent event = NONE;
	// Note: there are a bunch of other optional parameters but they add 
	// more complexity and aren't needed for the base functionnality so
	// we'll see if I implement them later.
};

struct Peer {
	std::string id;
	std::string ip;
	uint16_t port;
};

struct TrackerResponse {
	std::optional<std::string> failure_reason;
	std::optional<std::string> warning_message;
	size_t interval;
	size_t min_interval;
	std::string tracker_id;
	size_t seeders;
	size_t leechers;
	std::vector<Peer> peers;
};

class Tracker {
	private:
		http::HTTPSession m_session;
		std::string m_peerId;

	public:
		Tracker(const std::string& url);
		~Tracker();

		TrackerResponse query(const TrackerRequest& req);

};
