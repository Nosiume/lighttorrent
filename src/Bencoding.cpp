#include "Bencoding.h"
#include <sstream>

namespace bparser {
	BObject parse(std::string& data) {
		std::string::iterator cursor = data.begin();
		return read_bobject(&cursor);
	}

	BString read_bstring(std::string::iterator* curr) {
		int len = 0;
		while(**curr != ':') {
			len *= 10;
			len += **curr - '0';
			(*curr)++;
		}

		(*curr)++; // skip ':'
		BString res(*curr, *curr + len);
		(*curr) += len; // we need to keep cursor on next element
		return res;
	}

	BInteger read_bint(std::string::iterator* curr) {
		(*curr)++; // skip i

		bool negative = *(*curr) == '-';
		if(negative) (*curr)++;

		BInteger value = 0;
		while(**curr != 'e') {
			value *= 10;
			value += **curr - '0';
			(*curr)++;
		}
		(*curr)++; // skip e
		return (negative ? -1 : 1) * value;
	}

	BList read_blist(std::string::iterator* curr) {
		(*curr)++; // skip l
		BList list;
		while(**curr != 'e') {
			list.push_back(read_bobject(curr));
		}
		(*curr)++; // skip e
		return list;
	}

	BDict read_bdict(std::string::iterator* curr) {
		(*curr)++; // skip d
		BDict dict;
		while(**curr != 'e') {
			BString key = read_bstring(curr);	
			BObject value = read_bobject(curr);
			dict.emplace(key, value);
		}
		(*curr)++;
		return dict;
	}

	BObject read_bobject(std::string::iterator* curr) {
		switch(next_type(curr)) {
			case Integer: return BObject(read_bint(curr));
			case String: return BObject(read_bstring(curr));
			case List: return BObject(read_blist(curr));
			case Dict: return BObject(read_bdict(curr));
			case Unknown:
				throw bparser::ParserException();	
		}
	}

	BType next_type(std::string::iterator* curr) {
		switch(**curr) {
			case 'd': return Dict;
			case 'l': return List;
			case 'i': return Integer;
			case '1' ... '9': return String;
			default:
				return Unknown;
		}
	}

	std::string to_string(const BObject& obj) {
		std::stringstream out;
		if(obj.isString()) {
			BString s = obj.asString();
			out << s.length() << ':' << s;
		} else if (obj.isInteger()) {
			BInteger n = obj.asInteger();
			out << 'i' << n << 'e';
		} else if (obj.isList()) {
			BList l = obj.asList();
			out << 'l';
			for(const BObject& e : l) {
				out << to_string(e);
			}
			out << 'e';
		} else if (obj.isDict()) {
			BDict d = obj.asDict();
			out << 'd';
			for(const auto& entry : d) {
				out << to_string(entry.first);
				out << to_string(entry.second);
			}
			out << 'e';
		}
		return out.str();
	}
}
