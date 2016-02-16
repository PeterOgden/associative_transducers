#ifndef DATA_STRUCTURES_PUSHDOWN_STATE_MAP_H_
#define DATA_STRUCTURES_PUSHDOWN_STATE_MAP_H_

#include <algorithm>
#include <cassert>
#include <vector>

#include <boost/iterator/iterator_adaptor.hpp>
#include <boost/iterator/iterator_facade.hpp>

namespace data_structures {



/** class pushdown_state_map
 *
 * This class should never be used in a real environment. It exists
 * to serve as a functional description and reference model for the
 * double ended tree version
 */

template <typename Value>
class pushdown_state_map {

public:
	class entry {
	public:
		entry(std::vector<int> start_stack, std::vector<int> finish_stack, Value value):
			m_start_stack(std::move(start_stack)),
			m_finish_stack(std::move(finish_stack)),
			m_value(std::move(value)) {}
		typedef typename std::vector<int>::const_iterator stack_iterator;
		stack_iterator start_stack_begin() const { return m_start_stack.begin(); }
		stack_iterator start_stack_end() const  { return m_start_stack.end(); }
		stack_iterator finish_stack_begin() const { return m_finish_stack.begin(); }
		stack_iterator finish_stack_end() const  { return m_finish_stack.end(); }

		const Value& value() const { return m_value; }
		Value& value() { return m_value; }

		bool operator<(const entry& other) {
			if (std::lexicographical_compare(m_finish_stack.begin(), m_finish_stack.end(),
				other.m_finish_stack.begin(), other.m_finish_stack.end())) return true;
			if (std::lexicographical_compare(other.m_finish_stack.begin(), other.m_finish_stack.end(),
				m_finish_stack.begin(), m_finish_stack.end())) return false;
			return std::lexicographical_compare(m_start_stack.begin(), m_start_stack.end(),
				other.m_start_stack.begin(), other.m_start_stack.end());
		}
	public:	// Should  be private but access issues inside algoritms
		std::vector<int> m_start_stack;
		std::vector<int> m_finish_stack;
		Value m_value;
		bool m_erase = false;
		friend class pushdown_state_map;
	};

	typedef typename std::vector<entry>::iterator entry_iterator;
	typedef typename std::vector<entry>::const_iterator const_entry_iterator;

	entry_iterator entries_begin() { return m_entries.begin(); }
	entry_iterator entries_end() { return m_entries.end(); }
	const_entry_iterator entries_begin() const { return m_entries.begin(); }
	const_entry_iterator entries_end() const { return m_entries.end(); }

	class value_iterator : public boost::iterator_adaptor<value_iterator,
		entry_iterator, Value> {
	public:
		typedef boost::iterator_adaptor<value_iterator, entry_iterator, Value> base_iterator;
		value_iterator(entry_iterator it): base_iterator(it) {}
		value_iterator(): base_iterator() {}

	private:
		friend class boost::iterator_core_access;
		Value& dereference() const { return base_iterator::base_reference()->value(); }
	};

	class layer_iterator;

	class layer_info {
	public:
		int state() {
			assert (m_begin != m_end);
			assert (m_begin->m_finish_stack.size() > m_layer);
			return m_begin->m_finish_stack[m_layer];
		};
		void state(int s) {
			std::for_each(m_begin, m_end,
				[=](entry& e) {
					assert(e.m_finish_stack.size() > m_layer);
					e.m_finish_stack[m_layer] = s;
			});
		}

		value_iterator values_begin() { return value_iterator(m_begin); }
		value_iterator values_end() { return value_iterator(m_children_begin); }

		layer_iterator children_begin() { return layer_iterator(m_layer + 1, m_children_begin, m_end); }
		layer_iterator children_end() { return layer_iterator(m_layer + 1, m_end, m_end); }
	private:
		unsigned int m_layer;
		entry_iterator m_begin;
		entry_iterator m_children_begin;
		entry_iterator m_end;
		friend class layer_iterator;
		friend class pushdown_state_map;
		
	};

	class layer_iterator: public boost::iterator_facade<layer_iterator, layer_info, boost::forward_traversal_tag> {
	public:
		layer_iterator(unsigned int layer, entry_iterator begin, entry_iterator end) {
			m_end = end;
			m_info.m_layer = layer;
			update_info(begin);
		}
	private:
		mutable layer_info m_info;
		entry_iterator m_end;
			
		void update_info(entry_iterator begin) {
			m_info.m_begin = begin;
			if (begin == m_end) return;
			assert(begin->m_finish_stack.size() > m_info.m_layer);
			unsigned int layer = m_info.m_layer;
			int s = begin->m_finish_stack[layer];
			m_info.m_end = std::find_if(begin, m_end, [=](const entry& e) {
				return (e.m_finish_stack.size() <= layer || e.m_finish_stack[layer] != s);
			});
			m_info.m_children_begin = std::find_if(begin, m_info.m_end, [=](const entry& e) {
				return e.m_finish_stack.size() != layer + 1;
			});
		}

		void increment() { update_info(m_info.m_end); }
		layer_info& dereference() const { return m_info; }
		bool equal(const layer_iterator& other) const { return m_info.m_begin == other.m_info.m_begin; }
		friend class boost::iterator_core_access;
	};

	class start_layer_iterator: public boost::iterator_facade<start_layer_iterator, layer_info, boost::forward_traversal_tag> {
	public:
		start_layer_iterator(unsigned int layer, entry_iterator begin, entry_iterator end) {
			m_end = end;
			m_info.m_layer = layer;
			update_info(begin);
		}
	private:
		mutable layer_info m_info;
		entry_iterator m_end;
		void update_info(entry_iterator begin) {
			m_info.m_begin = begin;
			if (begin == m_end) return;
			assert(begin->m_start_stack.size() > m_info.m_layer);
			unsigned int layer = m_info.m_layer;
			int s = begin->m_start_stack[layer];
			m_info.m_end = std::find_if(begin, m_end, [=](const entry& e) {
				return (e.m_start_stack.size() <= layer || e.m_start_stack[layer] != s);
			});
			m_info.m_children_begin = std::find_if(begin, m_info.m_end, [=](const entry& e) {
				return e.m_start_stack.size() != layer + 1;
			});
		}

		void increment() { update_info(m_info.m_end); }
		layer_info& dereference() const { return m_info; }
		bool equal(const start_layer_iterator& other) const { return m_info.m_begin == other.m_info.m_begin; }
		friend class boost::iterator_core_access;
	};

	layer_iterator layer_begin() { return layer_iterator(0, m_entries.begin(), m_entries.end()); }
	layer_iterator layer_end() { return layer_iterator(0, m_entries.end(), m_entries.end()); }

	start_layer_iterator start_layer_begin() { return start_layer_iterator(0, m_entries.begin(), m_entries.end()); }
	start_layer_iterator start_layer_end() { return start_layer_iterator(0, m_entries.end(), m_entries.end()) ;}

	template<typename It1, typename It2>
	void add_entry(It1 start_begin, It1 start_end,
		It2 finish_begin, It2 finish_end, Value value) {
		m_entries.emplace_back(std::vector<int>(start_begin, start_end),
			std::vector<int>(finish_begin, finish_end),
			std::move(value));
	}

	void finalise() {
		auto erase_iter = std::remove_if(m_entries.begin(), m_entries.end(),
			[](const entry& e) { return e.m_erase; });
		m_entries.erase(erase_iter, m_entries.end());
		m_entries.reserve(m_entries.size() * 3);
		std::sort(m_entries.begin(), m_entries.end());
	}

	// Inserts another state for all elements between this element
	// and its parent
	void push_state(const layer_iterator& layer, int state) {
		std::for_each(layer->m_begin, layer->m_end,
			[&](entry& e) {
			e.m_finish_stack.insert(e.m_finish_stack.begin() + layer->m_layer, state);
		});
	}

	// Remove parent parent layer and attach to grandparent, cannot be
	// called on a child of the root
	void pop_state(const layer_iterator& layer) {
		assert(layer->m_layer > 0);
		std::for_each(layer->m_begin, layer->m_end,
			[&](entry& e) {
			e.m_finish_stack.erase(e.m_finish_stack.begin() + layer->m_layer - 1,
				e.m_finish_stack.begin() + layer->m_layer);
		});
	}

	void pop_unknown_state(const layer_iterator& layer, int start_state, int finish_state) {
		assert(m_entries.capacity() >= m_entries.size() + std::distance(layer->m_begin, layer->m_children_begin));
		std::for_each(layer->m_begin, layer->m_children_begin,
			[&](entry& e) {
			m_entries.push_back(e);
			auto& ne = m_entries.back();
			ne.m_finish_stack[layer->m_layer] = finish_state;
			ne.m_start_stack.push_back(start_state);
		});	
	}

	void clear_values(const layer_iterator& layer) {
		mark_for_erase(layer->m_begin, layer->m_children_begin);
	}

	void erase(const layer_iterator& layer) {
		mark_for_erase(layer->m_begin, layer->m_end);
	}

	void start_stack_finalise() {
		std::sort(m_entries.begin(), m_entries.end(),
			[](const entry& lhs, const entry& rhs) {
			return std::lexicographical_compare(lhs.m_start_stack.begin(), lhs.m_start_stack.end(),
				rhs.m_start_stack.begin(), rhs.m_start_stack.end());
		});
	}

private:
	void mark_for_erase(entry_iterator begin, entry_iterator end) {
		std::for_each(begin, end, [](entry& e) {
			e.m_erase = true;
		});
	}
	std::vector<entry> m_entries;
};

}

#endif
