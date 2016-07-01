#ifndef TRANSDUCERS_FINITE_FINITE_TRANSDUCER_H_
#define TRANSDUCERS_FINITE_FINITE_TRANSDUCER_H_

#include <unordered_map>

#include <transducers/base/transducer.h>
#include <representation/transition_description.h>

namespace transducers {
namespace finite {

/** This class is a non associative finite transducer used generally used in combination
 * with a smart splitter or a buffered transducer to ensure that the lexer can only be in
 * the correct state when a new block of data is started
 */

template <typename Next>
class finite_transducer :
	public base::transducer<Next, unsigned short, int, int>
{
public:
	using base_transducer = base::transducer<Next, unsigned short, int, int>;
	using partial_result = typename base_transducer::partial_result;
	using input_symbol = typename base_transducer::input_symbol;
	using output_symbol = typename base_transducer::output_symbol;

	void process_symbol(partial_result& pr, const input_symbol& s, std::size_t offset) const {
		auto find_iter = m_transitions.find(to_int(base_transducer::unwrap(pr), s));
		assert(find_iter != m_transitions.end());
		const auto& details = find_iter->second;
		base_transducer::unwrap(pr) = details.state;
		if (details.output != -1) {
			base_transducer::output(pr, details.output, offset);
		}
	}

	finite_transducer(const Next& n, const representation::dft_description& dft):
		base_transducer(n)
	{
		for (const auto& p: dft.transitions) {
			m_transitions[to_int(p.first.first, p.first.second)].state = p.second;
		}
		for (const auto& p: dft.output) {
			m_transitions[to_int(p.first.first, p.first.second)].output = p.second;
		}
		m_start_state = dft.start_state;
	}

	partial_result initial_result() const {
		return base_transducer::initial_result(m_start_state);
	}

	partial_result identity_result() const {
		return base_transducer::identity_result(m_start_state);
	}
private:
	struct transition_details {
		int state;
		int output = -1;
	};
	static int to_int(int s, int c) { return c | (s << 16); }
	std::unordered_map<int, transition_details> m_transitions;
	int m_start_state;
};

}
}


#endif
