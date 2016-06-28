#ifndef DATA_STRUCTURES_PUSHDOWN_STATE_MAP_H_
#define DATA_STRUCTURES_PUSHDOWN_STATE_MAP_H_

#include <algorithm>
#include <cassert>
#include <vector>

#include <boost/iterator/iterator_adaptor.hpp>
#include <boost/iterator/iterator_facade.hpp>
#include <boost/iterator/filter_iterator.hpp>

#include <util/range.h>
#include <utility>

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

		util::range<stack_iterator> start_stack() const {
			return util::range<stack_iterator>(start_stack_begin(), start_stack_end());
		}
		util::range<stack_iterator> finish_stack() const {
			return util::range<stack_iterator>(finish_stack_begin(), finish_stack_end());
		}
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
		bool valid() const { return !updated && !m_erase; }
	public:	// Should  be private but access issues inside algorithms
		std::vector<int> m_start_stack;
		std::vector<int> m_finish_stack;
		Value m_value;
		bool m_erase = false;
		bool updated = false;
		friend class pushdown_state_map;
		friend bool operator==(const entry& lhs, const entry& rhs) {
			return std::equal(lhs.start_stack_begin(), lhs.start_stack_end(),
					rhs.start_stack_begin(), rhs.start_stack_end()) &&
				std::equal(lhs.finish_stack_begin(), lhs.finish_stack_end(),
					rhs.finish_stack_begin(), rhs.finish_stack_end()) &&
				lhs.value() == rhs.value();
		}
	};

	struct no_value_update {
		void operator()(Value&) const {}
	};

	struct valid_filter {
		bool operator()(const entry& e) { return e.valid(); }
	};

	typedef typename std::vector<entry>::iterator entry_iterator;
	typedef typename std::vector<entry>::const_iterator const_entry_iterator;
	typedef boost::filter_iterator<valid_filter, entry_iterator> valid_entry_iterator;
	typedef boost::filter_iterator<valid_filter, const_entry_iterator> const_valid_entry_iterator;

	entry_iterator entries_begin() { return m_entries.begin(); }
	entry_iterator entries_end() { return m_entries.end(); }

	const_entry_iterator entries_begin() const { return m_entries.begin(); }
	const_entry_iterator entries_end() const { return m_entries.end(); }

	valid_entry_iterator valid_entries_begin() { return valid_entry_iterator(entries_begin(), entries_end()); }
	valid_entry_iterator valid_entries_end() { return valid_entry_iterator(entries_end(), entries_end()); }

	util::range<entry_iterator> entries() { return util::range<entry_iterator>{ entries_begin(), entries_end() }; }
	util::range<const_entry_iterator> entries() const { return util::range<const_entry_iterator>{ entries_begin(), entries_end() }; }
	util::range<valid_entry_iterator> valid_entries() { return util::range<valid_entry_iterator>{ valid_entries_begin(), valid_entries_end() }; }

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
		int state() const {
			assert (m_begin != m_end);
			assert (m_begin->m_finish_stack.size() > m_layer);
			return m_begin->m_finish_stack[m_layer];
		};

		layer_iterator find_child(int state) {
			// This could be a binary search but I'm not sure it really matters
			// given that anyone using this implementation isn't going to care
			// about speed
			return std::find_if(children_begin(), children_end(),
				[&](const layer_info& l ) { return l.state() == state; });
		}

		value_iterator values_begin() { return value_iterator(m_begin); }
		value_iterator values_end() { return value_iterator(m_children_begin); }
		bool has_values() { return values_begin() != values_end(); }

		layer_iterator children_begin() { return layer_iterator(m_layer + 1, m_children_begin, m_end); }
		layer_iterator children_end() { return layer_iterator(m_layer + 1, m_end, m_end); }
		util::range<layer_iterator> children() {
			return util::range<layer_iterator>(children_begin(), children_end());
		}
	private:
		unsigned int m_layer;
		entry_iterator m_begin;
		entry_iterator m_children_begin;
		entry_iterator m_end;
		friend class layer_iterator;
		friend class pushdown_state_map;
		
		util::range<entry_iterator> entries() {
			return util::range<entry_iterator>(m_begin, m_end);
		}
		util::range<entry_iterator> value_entries() {
			return util::range<entry_iterator>(m_begin, m_children_begin);
		}
		util::range<valid_entry_iterator> valid_entries() {
			return util::range<valid_entry_iterator>(
					valid_entry_iterator(m_begin, m_end),
					valid_entry_iterator(m_end, m_end)
				);
		}

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
				return (e.updated == false && (e.m_finish_stack.size() <= layer || e.m_finish_stack[layer] != s));
			});
			m_info.m_children_begin = std::find_if(begin, m_info.m_end, [=](const entry& e) {
				return e.updated == false && e.m_finish_stack.size() != layer + 1;
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
	util::range<layer_iterator> layers() {
		return util::range<layer_iterator>(layer_begin(), layer_end());
	}
	start_layer_iterator start_layer_begin() { return start_layer_iterator(0, m_entries.begin(), m_entries.end()); }
	start_layer_iterator start_layer_end() { return start_layer_iterator(0, m_entries.end(), m_entries.end()) ;}

	template<typename It1, typename It2>
	void add_entry(It1 start_begin, It1 start_end,
		It2 finish_begin, It2 finish_end, Value value) {
		m_entries.emplace_back(std::vector<int>(start_begin, start_end),
			std::vector<int>(finish_begin, finish_end),
			std::move(value));
	}

	void insert(const entry& e) {
		m_new_entries.emplace_back(e);
	}
	void finalise(bool keep_unmodified = true) {
		if (keep_unmodified) {
			m_new_entries.insert(m_new_entries.end(),
					valid_entries_begin(), valid_entries_end());
		}
		m_entries = std::move(m_new_entries);
		m_new_entries.clear();

		std::sort(m_entries.begin(), m_entries.end());
	}

	template <typename Fn = no_value_update>
	void transition_state(const layer_iterator& layer, int state, const Fn& fn = Fn{}) {
		for (auto& e: layer->valid_entries()) {
			m_new_entries.push_back(e);
			m_new_entries.back().m_finish_stack[layer->m_layer] = state;
			fn(m_new_entries.back().value());
			e.updated = true;
		}
	}

	// Inserts another state for all elements between this element
	// and its parent
	template <typename Fn = no_value_update>
	void push_state(const layer_iterator& layer, int new_state, int push_state, const Fn& fn = Fn{}) {
		for (auto& e: layer->valid_entries()) {
			m_new_entries.push_back(e);
			auto& ne = m_new_entries.back();

			ne.m_finish_stack.insert(ne.m_finish_stack.begin() + layer->m_layer + 1, push_state);
			ne.m_finish_stack[layer->m_layer] = new_state;
			fn(ne.value());
			e.updated = true;
		};
	}

	// Remove parent parent layer and attach to grandparent, cannot be
	// called on a child of the root
	template <typename Fn = no_value_update>
	void pop_state(const layer_iterator& layer, int new_state, const Fn& fn = Fn{}) {
		assert(layer->m_layer > 0);
		for (auto& e: layer->valid_entries()) {
			m_new_entries.push_back(e);
			auto& ne = m_new_entries.back();
			ne.m_finish_stack.erase(ne.m_finish_stack.begin() + layer->m_layer - 1,
				ne.m_finish_stack.begin() + layer->m_layer);
			ne.m_finish_stack[layer->m_layer] = new_state;
			fn(ne.value());
			e.updated = true;
		};
	}

	template <typename Fn = no_value_update>
	void pop_unknown_state(const layer_iterator& layer, int start_state, int finish_state, const Fn& fn = Fn{}) {
//		assert(m_entries.capacity() >= m_entries.size() + std::distance(layer->m_begin, layer->m_children_begin));
		for (auto& e: layer->value_entries()) {
			m_new_entries.push_back(e);
			auto& ne = m_new_entries.back();
			ne.m_finish_stack[layer->m_layer] = finish_state;
			ne.m_start_stack.push_back(start_state);
			fn(ne.value());
		};
	}

	void clear_values(const layer_iterator& layer) {
		mark_for_erase(layer->m_begin, layer->m_children_begin);
	}

	void erase(const layer_iterator& layer) {
		mark_for_erase(layer->m_begin, layer->m_end);
	}

	// This is a workaround to allow this function to be const
	// This const is definitely not thread safe but given its use as a testing function
	// that shouldn't matter.
	void start_stack_finalise() const {
		const_cast<pushdown_state_map*>(this)->start_stack_finalise();
	}
	void start_stack_finalise() {
		std::sort(m_entries.begin(), m_entries.end(),
			[](const entry& lhs, const entry& rhs) {
			return std::lexicographical_compare(lhs.m_start_stack.begin(), lhs.m_start_stack.end(),
				rhs.m_start_stack.begin(), rhs.m_start_stack.end());
		});
	}

	struct matching_entries_cmp {
		template <typename Range>
		bool operator() (const entry& lhs, const Range& rhs) {
			auto compare_range = std::min(lhs.start_stack_end() - lhs.start_stack_begin(), rhs.end() - rhs.begin());
			return std::lexicographical_compare(lhs.start_stack_begin(),
					lhs.start_stack_begin() + compare_range, rhs.begin(), rhs.begin() + compare_range);
		}
		template <typename Range>
		bool operator() (const Range& lhs, const entry& rhs) {
			auto compare_range = std::min(lhs.end() - lhs.begin(), rhs.start_stack_end() - rhs.start_stack_begin());
			return std::lexicographical_compare(lhs.begin(), lhs.begin() + compare_range,
					rhs.start_stack_begin(), rhs.start_stack_begin() + compare_range);
		}
	};

	template <typename Range>
	util::range<const_entry_iterator> matching_entries(const Range& r) const {
		const auto& ret = std::equal_range(entries_begin(), entries_end(), r, matching_entries_cmp());
		return util::range<const_entry_iterator>(ret.first, ret.second);
	}
	void clear() {
		m_entries.clear();
		m_new_entries.clear();
	}
private:
	void mark_for_erase(entry_iterator begin, entry_iterator end) {
		std::for_each(begin, end, [](entry& e) {
			e.m_erase = true;
		});
	}
	std::vector<entry> m_entries;
	std::vector<entry> m_new_entries;
};

template <typename Value>
bool operator==(const pushdown_state_map<Value>& lhs, const pushdown_state_map<Value>& rhs) {
	return std::equal(lhs.entries_begin(), lhs.entries_end(), rhs.entries_begin(), rhs.entries_end());
}

template <typename Value>
void pushdown_state_map_entry_print(std::ostream& s, const typename pushdown_state_map<Value>::entry& e) {
	s << "Start: ";
	std::for_each(e.start_stack_begin(), e.start_stack_end(),
		[&](int i) { s << i << ", "; });
	s << "Finish: ";
	std::for_each(e.finish_stack_begin(), e.finish_stack_end(),
		[&](int i) { s << i << ", "; });
	s << "State: " << e.value() << "\n";
}

template <typename Value>
std::ostream& operator<<(std::ostream& s, const pushdown_state_map<Value>& m) {
	std::for_each(m.entries_begin(), m.entries_end(),
		[&](const typename pushdown_state_map<Value>::entry& e) {
		pushdown_state_map_entry_print<Value>(s,e);
	});
	return s;
}
}

#endif
