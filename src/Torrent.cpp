#include "Torrent.h"
#include "Bencoding.h"
#include <fstream>
#include <sstream>
#include <stdexcept>

using namespace bparser;

Torrent::Torrent(const std::string& path) {
	std::ifstream in(path);
	if(!in.good()) throw std::invalid_argument("Couldn't open torrent file.");
	
	std::stringstream ss;
	ss << in.rdbuf();
	std::string contents = ss.str();

	// Will throw error if file is badly structured anyways
	BDict root = parse(contents).asDict();
	if(root.contains("comment")) m_comment = root.at("comment").asString();
	if(root.contains("created by")) m_createdBy = root.at("created by").asString();
	if(root.contains("creation date")) m_creationDate = root.at("creation date").asInteger();

	// Read the tracker(s)
	m_tracker = root.at("announce").asString();
	if(root.contains("announce-list")) {
		m_trackerList = std::make_optional(std::vector<std::string>{});
		for(const BObject& obj : root.at("announce-list").asList()) {
			m_trackerList->push_back(obj.asList()[0].asString());
		}
	}

	//TODO: read into file objects	
	
	BDict info = root.at("info").asDict();
	m_pieceLength = info.at("piece length").asInteger();
	m_pieces = info.at("pieces").asString();
}

std::string Torrent::toString() const {
	std::stringstream sstream;
	if(m_comment.has_value()) sstream << "comment: " << m_comment.value() << '\n';
	if(m_createdBy.has_value()) sstream << "created by: " << m_createdBy.value() << '\n';
	if(m_creationDate.has_value())sstream << "creation date: " << m_creationDate.value() << '\n';

	if(m_trackerList.has_value()) {
		sstream << "trackers: \n";
		for(const std::string& tracker : m_trackerList.value()) {
			sstream << " - " << tracker << '\n';
		}
	} else {
		sstream << "tracker: " << m_tracker << '\n';
	}

	sstream << "piece length: " << m_pieceLength << '\n';
	sstream << "pieces: " << m_pieces << '\n';
	return sstream.str();
}
