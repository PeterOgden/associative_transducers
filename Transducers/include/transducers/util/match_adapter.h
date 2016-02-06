#ifndef TRANSDUCERS_UTIL_MATCH_ADAPTER_H_
#define TRANSDUCERS_UTIL_MATCH_ADAPTER_H_

#include <cassert>
#include <deque>
#include <symbols/match.h>
#include <transducers/base/transducer.h>

namespace transducers {
namespace util {

/* class match_adapter
 *
 * This is used with the pushdown transducer to convert
 * begin and end events into a series of match objects suitable
 * for further processing. Please note. At the moment this
 * does not constiture a true associative transducer, as some
 * reordering may occur when the data is processed in multiple 
 * blocks. This is not a problem at present as the only user of
 * this class is the tree_pushdown_transucer which internally
 * corrects the order.
 */

template <typename Next>
class match_adapter : 
	public base::transducer<Next, uint32_t, symbols::match,
		std::deque<symbols::match>>
{
public:
	using base_transducer = base::transducer<Next, uint32_t,
		symbols::match, std::deque<symbols::match>>;
	using partial_result = typename base_transducer::partial_result;
	using input_symbol = typename base_transducer::input_symbol;
	using output_symbol = typename base_transducer::output_symbol;

	using match = symbols::match;;

	match_adapter(const Next& next):
		base_transducer(next) {}

	void process_symbol(partial_result& r, const input_symbol& is, 
			std::size_t offset) {
		auto& partial = this->unwrap(r);
		int value = is & ~match::MATCH_FLAGS_MASK;
		if (is & match::MATCH_FLAGS_START) {
			partial.emplace_back(value, offset, static_cast<std::size_t>(match::npos));
		}
		if (is & match::MATCH_FLAGS_END) {
			if (!partial.empty() && partial.back().end_offset == match::npos) {
				partial.back().end_offset = offset;
				this->output(r, partial.back(), offset);
				partial.pop_back();
			} else {
				partial.emplace_back(value, static_cast<std::size_t>(match::npos), offset);
			}
		}
	}

	void merge_results(partial_result& lhs, const partial_result& rhs) {
		auto& lhs_partial = this->unwrap(lhs);
		const auto& rhs_partial = this->unwrap(rhs);
		auto front_iter = lhs_partial.rbegin();
		auto back_iter = rhs_partial.begin();
		while (front_iter != lhs_partial.rend() && back_iter != rhs_partial.end()) {
			if (front_iter->end_offset == match::npos && back_iter->start_offset == match::npos) {
				assert(front_iter->rule == back_iter->rule);
				this->output(lhs, match(front_iter->rule,
					front_iter->start_offset, back_iter->end_offset), back_iter->end_offset);
			}
			++front_iter;
			++back_iter;
		}
		lhs_partial.erase(front_iter.base(), lhs_partial.end());
		lhs_partial.insert(lhs_partial.end(), back_iter, rhs_partial.end());
		this->merge_next(lhs, rhs);
	}
};

}
}

#endif
