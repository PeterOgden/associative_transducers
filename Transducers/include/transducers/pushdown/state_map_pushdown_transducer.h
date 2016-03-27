#ifndef TRANSDUCERS_STATE_MAP_PUSHDOWN_TRANSDUCER_H_
#define TRANSDUCERS_STATE_MAP_PUSHDOWN_TRANSDUCER_H_

#include <unordered_map>

#include <data_structures/pushdown_state_map.h>
#include <representation/transition_description.h>

namespace transducers {
namespace pushdown {

template <typename Next>
class state_map_pushdown_transducer {
public:
	explicit state_map_pushdown_transducer(const Next& next, const representation::dft_description& dft) {
		for (const auto& p: dft.transitions) {
			m_transitions[to_int(p.first.first, p.first.second)].next = p.second;
			m_states.insert(p.first.first);
			m_states.insert(p.second);
		}
		for (const auto& p: dft.push) {
			m_transitions[to_int(p.first.first, p.first.second)].push = p.second;
		}
		for (const auto& p: dft.pop) {
			m_transitions[to_int(p.first.first, p.first.second)].pop.push_back(p.second);
		}
		for (const auto& p: dft.output) {
			m_transitions[to_int(p.first.first, p.first.second)].output = p.second;
		}
		m_start_state = dft.start_state;
		m_next = &next;
	}

	typedef unsigned int input_symbol;
	typedef unsigned int output_symbol;
	typedef typename Next::terminal_result terminal_result;

	typedef data_structures::pushdown_state_map<typename Next::partial_result> map_type;
	class partial_result {
	public:
		const map_type& map() { return m_map; }
	private:
		map_type m_map;
		friend class state_map_pushdown_transducer;
		friend bool operator==(const partial_result& lhs, const partial_result& rhs) {
			return lhs.m_map == rhs.m_map;
		}
		friend std::ostream& operator<<(std::ostream& s, const partial_result& pr) {
			s << pr.m_map;
			return s;
		};
	};

	const terminal_result& last_stage_result(const partial_result& pr) const {
		assert (pr.m_map.size() == 1);
		return pr.begin_entry()->value();
	};

	void process_symbol(partial_result& pr, const input_symbol& s, std::size_t offset) {
		auto layer_begin = pr.m_map.layer_begin();
		auto layer_end = pr.m_map.layer_end();

		auto next_iter = layer_begin;
		for (auto iter = layer_begin; iter != layer_end; iter = next_iter) {
			++next_iter;

			const auto& find_iter = m_transitions.find(to_int(iter->state(), s));
			if (find_iter == m_transitions.end()) {
				continue;
			}
			const auto& details = find_iter->second;
			if (!details.pop.empty()) {
				if (iter->values_begin() != iter->values_end()) {
					for (const auto& p: details.pop) {
						pr.m_map.pop_unknown_state(iter, p.second, p.second);
					}
				}
				for (const auto& p: details.pop) {
					const auto& child_iter = iter->find_child(p.first);
					if (child_iter != iter->children_end()) {
						pr.m_map.pop_state(child_iter, p.second);
					}
				}
			}
			if (details.push != -1) {
				assert(details.next != -1);
				pr.m_map.push_state(iter, details.next, details.push);
			} else if (details.next != -1) {
				pr.m_map.transition_state(iter, details.next);

			}
		}
		pr.m_map.finalise(false);
	}

	partial_result initial_result() {
		partial_result ret;
		std::vector<int> stack(1, m_start_state);
		ret.m_map.add_entry(stack.begin(), stack.end(), stack.begin(), stack.end(), m_next->initial_result());
		return ret;
	}

	partial_result identity_result() {
		partial_result ret;
		std::vector<int> stack(1);
		for (int s: m_states) {
			stack[0] = s;
			ret.m_map.add_entry(stack.begin(), stack.end(), stack.begin(), stack.end(), m_next->identity_result());
		}
		return ret;
	}

	partial_result map_to_result(const map_type& m) {
		partial_result ret;
		ret.m_map = m;
		return ret;
	}
private:
	struct transition_details {
		int output = -1;
		int next = -1;
		int push = -1;
		// Pops are stored in a <stored state, new state> pairs
		std::vector<std::pair<int, int>> pop;
	};
	static int to_int(int c, int s) { return c | (s << 16); }
	std::unordered_map<int, transition_details> m_transitions;
	int m_start_state;
	std::set<int> m_states;
	const Next* m_next;
};

}
}

#endif
