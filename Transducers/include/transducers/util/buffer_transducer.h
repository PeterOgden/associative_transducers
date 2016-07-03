#ifndef TRANSDUCERS_UTIL_BUFFER_TRANSDUCER_H_
#define TRANSDUCERS_UTIL_BUFFER_TRANSDUCER_H_

#include <transducers/base/transducer.h>
#include <vector>

namespace transducers {
namespace util {

/** This transducer buffers all symbols until a particular symbol is seen
 *
 * The main use for this transducer is to wrap a non-associative transducer
 */
template <typename Next>
class buffer_transducer : 
	public base::transducer<Next,
		typename Next::input_symbol,
		typename Next::input_symbol,
		std::pair<bool, std::vector<std::pair<std::size_t, typename Next::input_symbol>>>>
{
public:
	typedef base::transducer<Next,
		typename Next::input_symbol,
		typename Next::input_symbol,
		std::pair<bool, std::vector<std::pair<std::size_t, typename Next::input_symbol>>>> base_transducer;

	typedef typename base_transducer::input_symbol input_symbol;
	typedef typename base_transducer::partial_result partial_result;

	buffer_transducer(const Next& next, const input_symbol& trigger_symbol):
		base_transducer(next),
		m_trigger_symbol(trigger_symbol) { }

	void process_symbol(partial_result& raw_pr, const input_symbol& s, std::size_t offset) {
		auto& pr = base_transducer::unwrap(raw_pr);
		if (pr.first) {
			base_transducer::output(raw_pr, s, offset);
		} else if (s == m_trigger_symbol) {
			pr.first = true;
			base_transducer::output(raw_pr, s, offset);
		} else {
			pr.second.emplace_back(s, offset);
		}
	}

	void merge_results(partial_result& r_lhs, const partial_result& r_rhs) {
		auto& lhs = base_transducer::unwrap(r_lhs);
		const auto& rhs = base_transducer::unwrap(r_rhs);
		if (rhs.first) {
			if (lhs.first) {
				for (const auto& p: rhs.second) {
					base_transducer::output(r_lhs, p.first, p.second);
				}
			} else {
				lhs.second.insert(lhs.second.end(), rhs.second.begin(), rhs.second.end());
				lhs.first = true;
			}
			base_transducer::merge_next(r_lhs, r_rhs);
		} else {
			if (lhs.first) {
				for (const auto& p: rhs.second) {
					base_transducer::output(r_lhs, p.first, p.second);
				}
			} else {
				lhs.second.insert(lhs.second.end(), rhs.second.begin(), rhs.second.end());
			}
		}
	}
private:
	input_symbol m_trigger_symbol;
};
}
}

#endif
