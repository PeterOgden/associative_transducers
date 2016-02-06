#ifndef TRANSDUCERS_AGGREGATION_SYMBOL_BUFFER_H_
#define TRANSDUCERS_AGGREGATION_SYMBOL_BUFFER_H_

#include <cstddef>
#include <vector>
#include <iterator>
#include <algorithm>

#include <transducers/base/sink_transducer.h>

namespace transducers {
namespace aggregation {

/** Stores all of the input symbols into an internal state */

template <typename SymbolType, typename StorageType = std::vector<SymbolType>>
class symbol_buffer : public base::sink_transducer<SymbolType, StorageType> {
public:
	using partial_result = typename base::sink_transducer<SymbolType, StorageType>::partial_result;
	using input_symbol = typename base::sink_transducer<SymbolType, StorageType>::input_symbol;

	void process_symbol(partial_result& pr, const input_symbol& s, std::size_t offset) const {
		pr.push_back(s);
	}

	void merge_results(partial_result& lhs, const partial_result& rhs) const {
		std::copy(rhs.begin(), rhs.end(), std::back_inserter(lhs));
	}
};

}
}
#endif
