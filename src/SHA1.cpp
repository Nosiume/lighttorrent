#include "SHA1.h"
#include <algorithm>
#include <netinet/in.h>
#include <sstream>
#include <stdlib.h>

using namespace sha1;

uint32_t rotl(uint32_t value, int shift) {
	return (value << shift) | (value >> (32 - shift));
}

std::string sha1::pad_message(const std::string& message) {
	//Calculate pad
	size_t l = message.length() * 8;	
	int pad = (56 - (l / 8 + 1)) % 64;
	if(pad < 0) {
		pad += 64;
	}

	//Apply pad
	std::stringstream ss;
	ss << message << '\x80';
	for(int i = 0 ; i < pad ; i++) {
		ss << '\0';
	}

	// big endian size from l's memory
	for (int i = 7; i >= 0; --i) {
		ss << (char)((l >> (i * 8)) & 0xFF);
	}
	return ss.str();
}

std::vector<std::string> sha1::get_blocks(const std::string &padded) {
	if(padded.length() % 64 != 0)
		throw std::runtime_error("trying to get blocks from non 512 bits multiple");

	std::vector<std::string> blocks;
	for(int i = 0 ; i < padded.length(); i+= 64) {
		blocks.push_back(padded.substr(i, 64));
	}
	return blocks;	
}

std::array<uint32_t, 16> sha1::divide_block(const std::string &block) {
	std::array<uint32_t, 16> res;
	const uint8_t* ptr = reinterpret_cast<const uint8_t*>(block.c_str());

	for (int i = 0; i < 16; i++) {
		res[i] = (ptr[0] << 24) | (ptr[1] << 16) | (ptr[2] << 8) | ptr[3];
		ptr += 4;
	}
	return res;
}

std::array<uint32_t, 80> sha1::expand_words(const std::array<uint32_t, 16> &words) {
	std::array<uint32_t, 80> res;
	std::copy(words.begin(), words.end(), res.begin());
	for(int i = 16 ; i < 80 ; i++) {
		res[i] = rotl(res[i-3] ^ res[i-8] ^ res[i-14] ^ res[i-16], 1);
	}
	return res;
}

std::string sha1::hash(const std::string &message) {
	std::string padded = pad_message(message);
	std::vector<std::string> blocks = get_blocks(padded);

	uint32_t hash[5] = {
		IVs[0],
		IVs[1],
		IVs[2],
		IVs[3],
		IVs[4]
	};

	for(std::string block : blocks) {
		std::array<uint32_t, 16> parts = sha1::divide_block(block);
		std::array<uint32_t, 80> expanded = sha1::expand_words(parts);

		uint32_t cur[5] = {
			hash[0],
			hash[1],
			hash[2],
			hash[3],
			hash[4]
		};

		for(int i = 0 ; i < 80 ; i++) {
			uint32_t f, k;
			if(i >= 0 && i <= 19) {
				f = (cur[1] & cur[2]) | (~cur[1] & cur[3]);
				k = K[0];
			} else if (i >= 20 && i <= 39) {
				f = cur[1] ^ cur[2] ^ cur[3];
				k = K[1];
			} else if (i >= 40 && i <= 59) {
				f = (cur[1] & cur[2]) | (cur[1] & cur[3]) | (cur[2] & cur[3]);
				k = K[2];
			} else {
				f = cur[1] ^ cur[2] ^ cur[3];
				k = K[3];
			}

			uint32_t temp = rotl(cur[0], 5) + f + cur[4] + k + expanded[i];
			cur[4] = cur[3];
			cur[3] = cur[2];
			cur[2] = rotl(cur[1], 30);
			cur[1] = cur[0];
			cur[0] = temp;
		}

		for(int i = 0 ; i < 5 ; i++)
			hash[i] += cur[i];
	}

	char outHash[20];
	for(int i = 0 ; i < 5 ; i++) {
		outHash[i*4] = (hash[i] >> 24) & 0xff;
		outHash[i*4 + 1] = (hash[i] >> 16) & 0xff;
		outHash[i*4 + 2] = (hash[i] >> 8) & 0xff;
		outHash[i*4 + 3] = hash[i] & 0xff;
	}
	return std::string(outHash, 20);
}

