#pragma once

#include <array>
#include <cstdint>
#include <string>
#include <vector>


namespace sha1 {
	const uint32_t IVs[5] = {
		0x67452301,
		0xEFCDAB89,
		0x98BADCFE,
		0x10325476,
		0xC3D2E1F0
	};

	const uint32_t K[4] = {
		0x5A827999,
		0x6ED9EBA1,
		0x8F1BBCDC,
		0xCA62C1D6
	};

	std::string pad_message(const std::string& message);
	std::vector<std::string> get_blocks(const std::string& padded);
	std::array<uint32_t, 16> divide_block(const std::string& block);
	std::array<uint32_t, 80> expand_words(const std::array<uint32_t, 16>& words);
	std::string hash(const std::string& message);
}
