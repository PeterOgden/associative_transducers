#ifndef TRANSDUCERS_BASE_SINK_TRANSDUCER_H_
#define TRANSDUCERS_BASE_SINK_TRANSDUCER_H_

#include <transducers/base/empty_state.h>

namespace transducers {
namespace base {

template <typename SymbolType, typename ResultType, typename TlsType = empty_state>
class sink_transducer {
public:
	typedef SymbolType input_symbol;
	typedef void output_symbol;
	typedef ResultType partial_result;
	typedef partial_result terminal_result;

	partial_result initial_result() const { return partial_result {}; }
	partial_result identity_result() const  { return partial_result {}; }
	const terminal_result& last_stage_result(const partial_result& p) const { return p; }
};

}
}

#endif
