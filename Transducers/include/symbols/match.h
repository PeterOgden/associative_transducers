#ifndef SYMBOLS_MATCH_H_
#define SYMBOLS_MATCH_H_

#include <limits>
#include <cstddef>

namespace symbols {

struct match {
	static const uint32_t MATCH_FLAGS_MASK =  0xC0000000;
	static const uint32_t MATCH_FLAGS_START = 0x80000000;
	static const uint32_t MATCH_FLAGS_END =   0x40000000;

	std::size_t rule;
	std::size_t start_offset;
	std::size_t end_offset;
	static const std::size_t npos = std::numeric_limits<std::size_t>::max();

	explicit match(std::size_t rule = 0, std::size_t start_offset = npos, std::size_t end_offset = npos):
		rule(rule),
		start_offset(start_offset),
		end_offset(end_offset) {}

	bool operator==(const match& other) const {
		return rule == other.rule &&
		       start_offset == other.start_offset &&
		       end_offset == other.end_offset;
	}
	bool operator!=(const match& other) const {
		return !(*this == other);
	}
};


template <typename Stream>
Stream& operator<<(Stream& s, const match& m) {
	s << "match: " << m.rule << ", " << m.start_offset << ", " 
	  << m.end_offset << "\n";
}

}

#endif
