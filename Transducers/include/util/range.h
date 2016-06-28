#ifndef UTIL_RANGE_H_
#define UTIL_RANGE_H_

namespace util {

template <typename It>
class range {
public:
	const It& begin() const { return m_begin; }
	const It& end() const { return m_end; }
	range(It b, It e):
		m_begin(std::move(b)),
		m_end(std::move(e)) {}
private:
	It m_begin;
	It m_end;
};

template <typename It>
range<It> make_range(It b, It e) {
	return range<It>(std::move(b), std::move(e));
}

}

#endif
