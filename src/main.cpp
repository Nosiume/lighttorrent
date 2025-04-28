#include <iostream>
#include "Torrent.h"
#include "Tracker.h"

int main() {
	Torrent torrent("../tests/journal.torrent");
	std::cout<< torrent.toString() << std::endl;

	Tracker tracker("tcp://p4p.arenabg.com:1337/announce");

	TrackerRequest req = {
		.info_hash = torrent.m_info_hash,
		.port = 6881,
		.uploaded = 0,
		.downloaded = 0,
		.left = 11736084,
		.compact = false,
		.no_peer_id = false,
		.event = STARTED,
	};
	TrackerResponse response = tracker.query(req);

	std::cout << "Tracker Id : " << response.tracker_id << std::endl;
	std::cout << "Interval : " << response.interval << std::endl;
	std::cout << "Min Interval : " << response.min_interval << std::endl;
	std::cout << "Seeders : " << response.seeders << std::endl;
	std::cout << "Leechers : " << response.leechers<< std::endl;
	for(const Peer& peer : response.peers) {
		std::cout << "Peer " << http::url_encode(peer.id) << std::endl;
		std::cout << "- ip : " << peer.ip << std::endl;
		std::cout << "- port : " << peer.port << std::endl;
	}

    return 0;
}
