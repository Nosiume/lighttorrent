#pragma once

#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>
#include <map>
#include <variant>
#include <iterator>

namespace bparser {
	
	class ParserException : public std::runtime_error {
		public:
			ParserException() : 
				std::runtime_error("Failed to parse bencoded data. Is the data valid ?") {}
	};

	enum BType {
		Integer,
		String,
		List,
		Dict,
		Unknown
	};

	class BObject;

	using BInteger = long long;
	using BString = std::string;
	using BList = std::vector<BObject>;
	using BDict = std::map<BString, BObject>;

	using BObjectVariant = std::variant<BInteger, BString, BList, BDict>;

	class BObject {
		private:
			BObjectVariant value;

		public:
			BObject(BInteger i): value(i) {}
			BObject(BString s): value(s) {}
			BObject(BList l): value(l) {}
			BObject(BDict d): value(d) {}

			bool isInteger() const { return std::holds_alternative<BInteger>(value); }
			bool isString() const { return std::holds_alternative<BString>(value); }
			bool isList() const { return std::holds_alternative<BList>(value); }
			bool isDict() const { return std::holds_alternative<BDict>(value); }
			
			BInteger asInteger() const {
				if (isInteger()) return std::get<BInteger>(value);
				throw new std::bad_variant_access();
			}

			BString asString() const {
				if (isString()) return std::get<BString>(value);
				throw new std::bad_variant_access();
			}

			BList asList() const {
				if (isList()) return std::get<BList>(value);
				throw new std::bad_variant_access();
			}

			BDict asDict() const {
				if (isDict()) return std::get<BDict>(value);
				throw new std::bad_variant_access();
			}

	};

	BObject parse(std::string& data);

	BType next_type(std::string::iterator* curr);
	BString read_bstring(std::string::iterator* curr);
	BInteger read_bint(std::string::iterator* curr); 
	BList read_blist(std::string::iterator* curr);
	BDict read_bdict(std::string::iterator* curr);
	BObject read_bobject(std::string::iterator* curr);

	std::string to_string(const BObject& obj);
}


