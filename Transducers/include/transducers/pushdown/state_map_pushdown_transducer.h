#ifndef TRANSDUCERS_STATE_MAP_PUSHDOWN_TRANSDUCER_H_
#define TRANSDUCERS_STATE_MAP_PUSHDOWN_TRANSDUCER_H_

#include <unordered_map>

#include <data_structures/pushdown_state_map.h>
#include <representation/transition_description.h>

namespace transducers {
namespace pushdown {

template <typename Next, template <typename> class MapType>
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
			int output = -1;
			auto lookup = dft.output.find(p.first);
			if (lookup != dft.output.end()) output = lookup->second;
			m_transitions[to_int(p.first.first, p.first.second)].pop.emplace_back(
					p.second.first, p.second.second, output);
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

	typedef MapType<typename Next::partial_result> map_type;
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
		return pr.m_map.entry_begin()->value();
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
				if (iter->has_values()) {
					for (const auto& p: details.pop) {
						if (p.output == -1) {
							pr.m_map.pop_unknown_state(iter, p.state, p.state);
						} else {
							pr.m_map.pop_unknown_state(iter, p.state, p.state, update_value(p.output, offset));
						}
					}
				}
				for (const auto& p: details.pop) {
					const auto& child_iter = iter->find_child(p.label);
					if (child_iter != iter->children_end()) {
						if (p.output == -1) {
							pr.m_map.pop_state(child_iter, p.state);
						} else {
							pr.m_map.pop_state(child_iter, p.state, update_value(p.output, offset));
						}
					}
				}
			}
			if (details.push != -1) {
				assert(details.next != -1);
				if (details.output == -1) {
					pr.m_map.push_state(iter, details.next, details.push);
				} else {
					pr.m_map.push_state(iter, details.next, details.push, update_value(details.output, offset));
				}
			} else if (details.next != -1) {
				if (details.output == -1) {
					pr.m_map.transition_state(iter, details.next);
				} else {
					pr.m_map.transition_state(iter, details.next, update_value(details.output, offset));
				}
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

	// This is a function primarily designed for testing purposes so that the internal state map
	// can be set prior to testing the operation of a transition
	partial_result map_to_result(map_type m) {
		partial_result ret;
		ret.m_map = std::move(m);
		return ret;
	}

	void merge_results(partial_result& lhs, const partial_result& rhs) {
		auto old = std::move(lhs.m_map);
		rhs.m_map.start_stack_finalise();
		lhs.m_map.clear();
		for (auto& e1: old.entries()) {
			const auto& found = rhs.m_map.matching_entries(e1.finish_stack());
			for(auto& e2: found) {
				unify_entries(e1, e2, lhs.m_map);
			}
		}
		lhs.m_map.finalise();
	}
private:
	struct update_value_function {
		void operator()(typename Next::partial_result& pr) const {
			next->process_symbol(pr, symbol, offset);
		}
		const Next* next;
		typename Next::input_symbol symbol;
		std::size_t offset;
		update_value_function(const Next* next, const typename Next::input_symbol& symbol, std::size_t offset):
			next(next), symbol(symbol), offset(offset) {}
	};

	update_value_function update_value(const typename Next::input_symbol& symbol, std::size_t offset) {
		return update_value_function(m_next, symbol, offset);
	}

	typedef typename map_type::entry map_entry;
	void unify_entries(map_entry lhs, const map_entry& rhs, map_type& the_map) {
		std::vector<int> new_start(lhs.start_stack_begin(), lhs.start_stack_end());
		std::vector<int> new_finish(rhs.finish_stack_begin(), rhs.finish_stack_end());

		auto lhs_finish_begin = lhs.finish_stack_begin();
		auto lhs_finish_end = lhs.finish_stack_end();
		auto rhs_start_begin = rhs.start_stack_begin();
		auto rhs_start_end = rhs.start_stack_end();

		auto lhs_finish_size = lhs_finish_end - lhs_finish_begin;
		auto rhs_start_size = rhs_start_end - rhs_start_begin;

		if (lhs_finish_size > rhs_start_size) {
			new_finish.insert(new_finish.end(), lhs_finish_begin + rhs_start_size, lhs_finish_end);
		} else {
			new_start.insert(new_start.end(), rhs_start_begin + lhs_finish_size, rhs_start_end);
		}
		auto r = lhs.value();
		m_next->merge_results(r, rhs.value());
		the_map.add_entry(new_start.begin(), new_start.end(), new_finish.begin(), new_finish.end(), std::move(r));
	}
	struct transition_details {
		int output = -1;
		int next = -1;
		int push = -1;
		// Pops are stored in a <stored state, new state> pairs
		struct pop_details {
			pop_details(int label, int state, int output):
				label(label), state(state), output(output) {}
			int label;
			int state;
			int output;
		};
		std::vector<pop_details> pop;
	};

	static int to_int(int c, int s) { return c | (s << 16); }
	std::unordered_map<int, transition_details> m_transitions;
	int m_start_state;
	std::set<int> m_states;
	const Next* m_next;
};


template <typename Next>
using explicit_state_map_pushdown_transducer = state_map_pushdown_transducer<Next, data_structures::pushdown_state_map>;

}
}


#endif
