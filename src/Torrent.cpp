#include "Torrent.h"
#include "Bencoding.h"
#include <fstream>
#include <iomanip>
#include <sstream>
#include <stdexcept>

using namespace bparser;

// Parsing of the torrent info into this class and TorrentOutFile struct
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

	BDict info = root.at("info").asDict();
	if(info.contains("files")) {
		// Multiple files mode
		for(const BObject& file : info.at("files").asList()) {
			BDict fileDict = file.asDict();

			TorrentOutFile f = {
				.checksum = fileDict.contains("md5sum") ? 
					std::make_optional(fileDict.at("md5sum").asString()) : std::nullopt,
				.path = fileDict.at("name").asString(),
				.length = fileDict.at("length").asInteger()
			};	
			m_files.push_back(f);
		}
	} else {
		// Single file mode
		TorrentOutFile file = {
			.checksum = info.contains("md5sum") ? 
				std::make_optional(info.at("md5sum").asString()) : std::nullopt,
			.path = info.at("name").asString(),
			.length = info.at("length").asInteger()
		};	
		m_files.push_back(file);
	}
	
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
	sstream << "pieces: \n";
	for(int i = 0 ; i < m_pieces.length() ; i+=20) {
		std::string piece = m_pieces.substr(i, 20);

		sstream << " - ";
		for(unsigned char c : piece) {
			sstream << std::setw(2) << std::setfill('0') << std::hex << (int) c;	
		}
		sstream << std::setw(0) << std::setfill(' ') << std::dec << "\n";
	}

	sstream << "files: \n";
	for(const TorrentOutFile& file : m_files) {
		sstream << " - " << file.path << " (" << file.length << " bytes)\n";	
		if(file.checksum.has_value()) 
			sstream << "\t" << file.checksum.value() << "\n";
	}
	return sstream.str();
}
