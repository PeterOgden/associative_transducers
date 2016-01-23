#ifndef TRANSDUCER_BASE_TRANSDUCER_H_
#define TRANSDUCER_BASE_TRANSDUCER_H_

#include <transducers/base/empty_state.h>

namespace transducers {
namespace base {

template <typename Next, typename InputType,
	typename OutputType=InputType,
	typename PartialType = empty_state>
class transducer {
public:
	using input_symbol = InputType;
	using output_symbol = OutputType;
	using partial_result = std::pair<PartialType, typename Next::partial_result>;
	using terminal_result = typename Next::terminal_result;

	void output(partial_result& p, const output_symbol& s, std::size_t offset) const {
		m_next->process_symbol(p.second, s, offset);
	}
	PartialType& unwrap(partial_result& p) const { return p.first; }

	void process_symbol(partial_result& p, const input_symbol& s, std::size_t offset) const {
		this->output(p, s, offset);
	}
	void merge_results(partial_result& lhs, const partial_result& rhs, std::size_t offset) const {
		m_next->merge_results(lhs.second, rhs.second, offset);
	}

	partial_result initial_result() const {
		return std::make_pair(PartialType(), m_next->initial_result());
	}

	partial_result identity_result() const {
		return std::make_pair(PartialType(), m_next->identity_result());
	}

	const terminal_result& last_stage_result(const partial_result& p) const {
		return m_next->last_stage_result(p.second);
	}
	transducer(const Next& n):
		m_next(&n) {}

private:
	const Next* m_next;
};

}
}

#endif
