#ifndef DATA_STRUCTURES_TREE_STATE_MAP_H_
#define DATA_STRUCTURES_TREE_STATE_MAP_H_

#include <boost/intrusive/set.hpp>
#include <boost/intrusive/list.hpp>

#include <util/range.h>

namespace data_structures {

template <typename Value>
class tree_state_map {
public:
private:
	class finish_node;
	struct start_node : public boost::intrusive::set_base_hook<>,
		public boost::intrusive::list_base_hook<> {
		typedef boost::intrusive::set<start_node,
			boost::intrusive::base_hook<
				boost::intrusive::set_base_hook<>>> children_set;

		typedef typename children_set::const_iterator start_layer_iterator;
		start_layer_iterator children_begin() const { return m_children.begin(); }
		start_layer_iterator children_end() const { return m_children.end(); }
		start_layer_iterator find_child(int s) const { 
			return m_children.find(s, child_finder{});
		}

		int state() const { return m_state; }
		bool operator<(const start_node& other) const {
			return m_state < other.m_state;
		}
	private:
		struct child_finder {
			bool operator()(int s, const start_node& n) const {
				return s < n.state();
			}
			bool operator()(const start_node& n, int s) const {
				return n.state() < s;
			}
		};
		typename children_set::iterator find_child_mutable(int s) {
			return m_children.find(s, child_finder{});
		}
		int m_state = -1;
		children_set m_children;
		start_node* m_parent = nullptr;
		finish_node* m_finish_node = nullptr;
		Value m_value;
		friend class entry_iterator;
		friend class tree_state_map;
	};
	struct finish_node : public boost::intrusive::set_base_hook<> {
		typedef boost::intrusive::set<finish_node,
			boost::intrusive::base_hook<
				boost::intrusive::set_base_hook<>>> children_set;
		typedef boost::intrusive::list<start_node> start_node_list;
		typedef typename children_set::const_iterator layer_iterator;

		layer_iterator children_begin() const { return m_children.begin(); }
		layer_iterator children_end() const { return m_children.end(); }
		layer_iterator find_child(int s) const { 
			return m_children.find(s, child_finder{});
		}
		bool has_values() const { return !m_start_nodes.empty(); }
		const children_set& children() const { return m_children; }
		int state() const { return m_state; }
		bool operator<(const finish_node& other) const {
			return m_state < other.m_state;
		}

	private:
		int m_state = -1;
		children_set m_children;
		start_node_list m_start_nodes;
		finish_node* m_parent = nullptr;

		struct child_finder {
			bool operator()(int s, const finish_node& n) const {
				return s < n.state();
			}
			bool operator()(const finish_node& n, int s) const {
				return n.state() < s;
			}
		};
		typename children_set::iterator find_child_mutable(int s) {
			return m_children.find(s, child_finder{});
		}
		friend class entry_iterator;
		friend class tree_state_map;
	};
public:
	typedef typename finish_node::layer_iterator layer_iterator;
	typedef typename start_node::start_layer_iterator start_layer_iterator;

	tree_state_map():
		m_start_root(new start_node()),
		m_finish_root(new finish_node()),
		m_next_root(new finish_node()) {
	}
	tree_state_map(const tree_state_map& other):
		tree_state_map() {
		for (const auto& e: other.entries()) {
			add_entry(e.start_stack_begin(), e.start_stack_end(),
				e.finish_stack_begin(), e.finish_stack_end(), e.value());
		}
	}
	tree_state_map(tree_state_map&& other):
		tree_state_map() {
		swap(other);
	}
	tree_state_map& operator=(const tree_state_map& other) {
		tree_state_map new_map(other);
		this->swap(new_map);
		return *this;
	}
	tree_state_map& operator=(tree_state_map&& other) {
		this->swap(other);
		return *this;
	}

	~tree_state_map() {
		clear();
	}
	void swap(tree_state_map& other) {
		using std::swap;
		m_start_root.swap(other.m_start_root);
		m_finish_root.swap(other.m_finish_root);
		m_next_root.swap(other.m_next_root);
	}
	class entry_iterator;
	class entry {
	public:
		typedef std::vector<int>::const_iterator iterator;
		iterator start_stack_begin() const { return m_start_stack.begin(); }
		iterator start_stack_end() const { return m_start_stack.end(); }
		util::range<iterator> start_stack() const {
			return util::range<iterator>(start_stack_begin(), start_stack_end());
		}

		iterator finish_stack_begin() const { return m_finish_stack.begin(); }
		iterator finish_stack_end() const { return m_finish_stack.end(); }
		util::range<iterator> finish_stack() const {
			return util::range<iterator>(finish_stack_begin(), finish_stack_end());
		}
		const Value& value() const { return *m_value; }
		friend bool operator==(const entry& lhs, const entry& rhs) {
			return std::equal(lhs.start_stack_begin(), lhs.start_stack_end(),
					rhs.start_stack_begin(), rhs.start_stack_end()) &&
				std::equal(lhs.finish_stack_begin(), lhs.finish_stack_end(),
					rhs.finish_stack_begin(), rhs.finish_stack_end()) &&
				lhs.value() == rhs.value();
		}
	private:
		const Value* m_value;
		std::vector<int> m_start_stack;
		std::vector<int> m_finish_stack;
		friend class entry_iterator;
	};

	class entry_iterator : public boost::iterator_facade<entry_iterator, const entry, boost::forward_traversal_tag> {
	public:
		entry_iterator(start_layer_iterator start, start_layer_iterator end) {
			m_base_iter = start;
			m_end = end;
			m_invalidated = true;
			while (m_base_iter != m_end && m_base_iter->m_finish_node == nullptr) {
				next_iterator();
			}
		}
	private:
		void next_iterator() {
			m_invalidated = true;
			if (!m_base_iter->m_children.empty()) {
				m_base_iter = m_base_iter->m_children.begin();
			} else {
				auto parent = m_base_iter->m_parent;
				++m_base_iter;
				while (m_base_iter != m_end && m_base_iter == parent->children_end()) {
					if (parent->m_parent == nullptr) {
						m_base_iter = parent->children_end();
						break;
					}
					m_base_iter = parent->m_parent->find_child(parent->m_state);
					parent = parent->m_parent;
					++m_base_iter;
				}
			}
		}
		void increment() {
			do {
				next_iterator();
			} while (m_end != m_base_iter && !m_base_iter->m_finish_node);
		}
		const entry& dereference() const { 
			if (m_invalidated) {
				assert(m_base_iter != m_end);
				m_invalidated = false;
				m_entry.m_start_stack.clear();
				m_entry.m_finish_stack.clear();
				m_entry.m_value = &m_base_iter->m_value;
				auto sn = &*m_base_iter;
				while (sn != nullptr) {
					m_entry.m_start_stack.push_back(sn->m_state);
					sn = sn->m_parent;
				}
				m_entry.m_start_stack.pop_back();
				std::reverse(m_entry.m_start_stack.begin(), m_entry.m_start_stack.end());
				auto fn = m_base_iter->m_finish_node;
				while (fn != nullptr) {
					m_entry.m_finish_stack.push_back(fn->m_state);
					fn = fn->m_parent;
					
				}
				m_entry.m_finish_stack.pop_back();
				std::reverse(m_entry.m_finish_stack.begin(), m_entry.m_finish_stack.end());
			}
			return m_entry;
		}
		bool equal(const entry_iterator& other) const { 
			return m_base_iter == other.m_base_iter;
		}
	
		mutable bool m_invalidated;	
		mutable entry m_entry;
		start_layer_iterator m_end;
		start_layer_iterator m_base_iter;
		friend class boost::iterator_core_access;
	};

	entry_iterator entries_begin() const { 
		return entry_iterator{m_start_root->children_begin(), m_start_root->children_end()}; 
	}
	entry_iterator entries_end() const { 
		return entry_iterator{m_start_root->children_end(), m_start_root->children_end()}; }
	
	util::range<entry_iterator> entries() const {
		return util::range<entry_iterator>(entries_begin(), entries_end());
	}

	layer_iterator layer_begin() { return m_finish_root->children_begin(); }
	layer_iterator layer_end() { return m_finish_root->children_end(); }
	util::range<layer_iterator> layers() { return util::range<layer_iterator>(layer_begin(), layer_end()); }

	template <typename Range>
	util::range<entry_iterator> matching_entries(const Range& r) const {
		start_node* sn = &*m_start_root;
		start_layer_iterator find_iter;
		for (auto iter = r.begin(); iter != r.end() && sn->m_finish_node == nullptr; ++iter) {
			find_iter = sn->find_child(*iter);
			if (find_iter == sn->m_children.end()) {
				return util::range<entry_iterator>(entries_end(), entries_end());
			}
			sn = const_cast<start_node*>(&*find_iter);
		}
		auto first = find_iter;
		auto last = ++find_iter;
		return util::range<entry_iterator>(entry_iterator(first, last), entry_iterator(last,last));
	}

	template <typename It1, typename It2>
	void add_entry(It1 start_begin, It1 start_end,
		It2 finish_begin, It2 finish_end, Value value) {
		start_node* sn = m_start_root.get();
		finish_node* fn = m_finish_root.get();

		while (start_begin != start_end) {
			sn = find_or_add_child(*sn, *start_begin++);
		}
		while (finish_begin != finish_end) {
			fn = find_or_add_child(*fn, *finish_begin++);
		}
		sn->m_value = std::move(value);
		connect_leaves(*sn, *fn);
	}

	void clear() {
		for (auto iter = m_finish_root->m_children.begin(); iter != m_finish_root->m_children.end();) {
			erase_node(*iter++);
		}
		for (auto iter = m_next_root->m_children.begin(); iter != m_next_root->m_children.end();) {
			erase_node(*iter++);
		}
	}
	void finalise(bool keep_unmodified = true) {
		using std::swap;
		for (auto iter = m_finish_root->m_children.begin(); iter != m_finish_root->m_children.end(); ) {
			auto next = iter;
			++next;
			if (keep_unmodified) {
				auto& node = *iter;
				m_finish_root->m_children.erase(iter);
				merge_child(*m_next_root, node);
			} else {
				erase_node(*iter);
			}
			iter = next;
		}
		swap(m_finish_root, m_next_root);
		assert(m_next_root->m_children.empty());
	}
	void start_stack_finalise() const { }
	struct no_value_update {
		void operator() (const Value&) const {}
	};
	template <typename Fn>
	void update_values(finish_node& node, const Fn& fn) {
		for (auto& child: node.m_children) {
			update_values(child, fn);
		}
		for (auto& sn: node.m_start_nodes) {
			fn(sn.m_value);
		}
	}
	template <typename Fn = no_value_update>
	void transition_state(const layer_iterator& layer, int state, const Fn& fn = Fn{}) {
		finish_node& node = const_cast<finish_node&>(*layer);
		m_finish_root->m_children.erase(layer);
		node.m_state = state;
		update_values(node, fn);
		merge_child(*m_next_root, node);
	}

	template <typename Fn = no_value_update>
	void push_state(const layer_iterator& layer, int new_state, int push_state, const Fn& fn = Fn{}) {
		finish_node* new_node = find_or_add_child(*m_next_root, new_state);
		finish_node& old_node = const_cast<finish_node&>(*layer);
		m_finish_root->m_children.erase(layer);
		old_node.m_state = push_state;
		update_values(old_node, fn);
		merge_child(*new_node, old_node);
	}

	template <typename Fn = no_value_update>
	void pop_state(const layer_iterator& layer, int new_state, const Fn& fn = Fn{}) {
		finish_node& old_node = const_cast<finish_node&>(*layer);
		old_node.m_parent->m_children.erase(layer);
		old_node.m_state = new_state;
		update_values(old_node, fn);
		merge_child(*m_next_root, old_node);
	}

	template <typename Fn = no_value_update>
	void pop_unknown_state(const layer_iterator& layer, int start_state, int finish_state, const Fn& fn = Fn{}) {
		finish_node& old_node = const_cast<finish_node&>(*layer);
		finish_node* n = find_or_add_child(*m_next_root, finish_state);
		n->m_state = finish_state;
		add_child(*m_next_root, *n);
		for (auto& s: old_node.m_start_nodes) {
			start_node* sn = new_start_node();
			sn->m_state = start_state;
			sn->m_value = s.m_value;
			add_child(s, *sn);
			connect_leaves(*sn, *n);
			fn(sn->m_value);
		}
	}
private:
	std::unique_ptr<start_node> m_start_root;
	std::unique_ptr<finish_node> m_finish_root;
	std::unique_ptr<finish_node> m_next_root;
//	std::deque<start_node> m_start_nodes;
//	std::deque<finish_node> m_finish_nodes;
//	std::vector<start_node*> m_free_start_nodes;
//	std::vector<finish_node*> m_free_finish_nodes;

	start_node* new_start_node() { return new start_node {}; }
	finish_node* new_finish_node() { return new finish_node {}; }
	start_node* new_node(start_node*) { return new_start_node(); }
	finish_node* new_node(finish_node*) { return new_finish_node(); }
	void free_start_node(const start_node* n) { delete n; }
	void free_finish_node(const finish_node* n) { delete n; }

	template <typename NodeType>
	void add_child(NodeType& parent, NodeType& child) {
		child.m_parent = &parent;
		parent.m_children.insert(child);
	}

	template <typename NodeType>
	NodeType* find_or_add_child(NodeType& n, int state) {
		auto find = n.find_child_mutable(state);
		if (find == n.children_end()) {
			auto ret = new_node(&n);
			ret->m_state = state;
			add_child(n, *ret);
			return ret;
		}
		return &*find;
	}

	template<typename NodeType>
	void merge_child(NodeType& n, NodeType& child) {
		auto find = n.find_child_mutable(child.state());
		if (find == n.children_end()) {
			add_child(n, child);
		} else {
			auto &old_n = *find;
			for (auto& gc: child.m_children) {
				merge_child(old_n, gc);
			}
			for (auto iter = child.m_start_nodes.begin(); iter != child.m_start_nodes.end();) {
				auto& sn = *iter;
				child.m_start_nodes.erase(iter++);
				connect_leaves(sn, old_n);
			}
			free_finish_node(&child);
		}
	}

	void connect_leaves(start_node& sn, finish_node& fn) {
		sn.m_finish_node = &fn;
		fn.m_start_nodes.push_back(sn);
	}

	void erase_node(finish_node& fn) {
		for (auto iter = fn.m_children.begin(); iter != fn.m_children.end();) {
			erase_node(*iter++);
		}
		for (auto iter = fn.m_start_nodes.begin(); iter != fn.m_start_nodes.end();) {
			auto& sn = *iter;
			fn.m_start_nodes.erase(iter++);
			sn.m_finish_node = nullptr;
			erase_node(sn);
		}
		if (fn.m_parent) {
			fn.m_parent->m_children.erase(fn);
		}
		free_finish_node(&fn);
	}
	void erase_node(start_node& sn) {
		if (!sn.m_parent) return;
		if (sn.m_finish_node) return;
		if (!sn.m_children.empty()) return;
		sn.m_parent->m_children.erase(sn);
		if (sn.m_parent->m_children.empty()) {
			erase_node(*sn.m_parent);
		}
		free_start_node(&sn);
	}
	void validate() {
		validate(*m_start_root);
	}
	void validate(start_node& sn) {
		if (sn.m_finish_node) {
			auto* fn = sn.m_finish_node;
			while (fn->m_parent) {
				fn = fn->m_parent;
			}
			assert(fn == &*m_finish_root);
		}
		for (auto& child: sn.m_children) {
			validate(child);
		}
	}
};
template <typename Value>
bool operator==(const tree_state_map<Value>& lhs, const tree_state_map<Value>& rhs) {
	return std::equal(lhs.entries_begin(), lhs.entries_end(), rhs.entries_begin(), rhs.entries_end());
}

template <typename Value>
void tree_state_map_entry_print(std::ostream& s, const typename tree_state_map<Value>::entry& e) {
	s << "Start: ";
	std::for_each(e.start_stack_begin(), e.start_stack_end(),
		[&](int i) { s << i << ", "; });
	s << "Finish: ";
	std::for_each(e.finish_stack_begin(), e.finish_stack_end(),
		[&](int i) { s << i << ", "; });
	s << "State: " << e.value() << "\n";
}

template <typename Value>
std::ostream& operator<<(std::ostream& s, const tree_state_map<Value>& m) {
	std::for_each(m.entries_begin(), m.entries_end(),
		[&](const typename tree_state_map<Value>::entry& e) {
		tree_state_map_entry_print<Value>(s,e);
	});
	return s;
}

template <typename Value>
void swap(tree_state_map<Value>& lhs, tree_state_map<Value>& rhs) {
	lhs.swap(rhs);
}

}

#endif
