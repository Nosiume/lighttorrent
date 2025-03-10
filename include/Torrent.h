#pragma once

#include <optional>
#include <string>
#include <vector>

struct TorrentOutFile {
	std::string checksum;
	std::string path;
	long long length;
};

class Torrent {
	private:
		//Optional fields
		std::optional<std::string> m_createdBy;
		std::optional<long long> m_creationDate;
		std::optional<std::string> m_comment;
		std::optional<std::vector<std::string>> m_trackerList;

		std::string m_tracker;
		std::vector<TorrentOutFile> m_files;
		std::string m_rootPath;
		long long m_pieceLength;
		std::string m_pieces;

	public:
		Torrent(const std::string& path);
		~Torrent();
};
