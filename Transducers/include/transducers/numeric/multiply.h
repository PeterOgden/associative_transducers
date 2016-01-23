#ifndef TRANSDUCERS_NUMERIC_MULTIPLY_H_
#define TRANSDUCERS_NUMERIC_MULTIPLY_H_

#include <transducers/base/transducer.h>

namespace transducers {
namespace numeric {

/** Simple transducer to test the composition machinery */
template <typename NumType, typename Next>
class multiply : 
	public base::transducer<Next, NumType>
{
public:
	using base_transducer = base::transducer<Next, NumType>;
	using partial_result = typename base_transducer::partial_result;
	using input_symbol = typename base_transducer::input_symbol;
	using output_symbol = typename base_transducer::output_symbol;

	void process_symbol(partial_result& p, const input_symbol& s, std::size_t offset) const {
		this->output(p, s * m_constant, offset);
	}

	multiply(const Next& n, const NumType& constant):
		base_transducer(n),
		m_constant(constant) {}	

private:
	NumType m_constant;
};

template <typename Next>
using multiply_int = multiply<int, Next>;

}
}

#endif
