#pragma once

#include <optional>
#include <string>
#include <vector>

struct TorrentOutFile {
	std::optional<std::string> checksum;
	std::string path;
	long long length;
};

class Torrent {
	public: // PUT THIS BACK TO PRIVATE TODO
		//Optional fields
		std::optional<std::string> m_createdBy;
		std::optional<long long> m_creationDate;
		std::optional<std::string> m_comment;
		std::optional<std::vector<std::string>> m_trackerList;

		std::string m_tracker;
		std::vector<TorrentOutFile> m_files;
		long long m_pieceLength;
		std::string m_pieces;
		std::string m_info_hash;


	public:
		Torrent(const std::string& path);
		~Torrent() {};

		bool download(const std::string& root, std::string* status);

		std::string toString() const;
};
