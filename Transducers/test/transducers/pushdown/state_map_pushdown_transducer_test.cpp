#include <cppunit/extensions/HelperMacros.h>
#include "transducers/pushdown/state_map_pushdown_transducer.h"
#include "transducers/aggregation/symbol_buffer.h"
#include "transducers/compose.h"

namespace std {
	std::ostream& operator<<(std::ostream& s, const std::vector<uint32_t> & v) {
		for (auto i: v) {
			s << i << ", ";
		}
		return s;
	}
}

class state_map_pushdown_transducer_test : public CppUnit::TestFixture {
public:
	CPPUNIT_TEST_SUITE(state_map_pushdown_transducer_test);
	CPPUNIT_TEST(simple_test);
	CPPUNIT_TEST(construction_test);
	CPPUNIT_TEST(transition_test);
	CPPUNIT_TEST(discard_test);
	CPPUNIT_TEST(simple_pop_test);
	CPPUNIT_TEST(unknown_pop_test);
	CPPUNIT_TEST(pop_and_plain_test);
	CPPUNIT_TEST(simple_push_test);
	CPPUNIT_TEST(push_combine_test);
	CPPUNIT_TEST(simple_combine_test);
	CPPUNIT_TEST(pop_drop_test);
	CPPUNIT_TEST(double_pop_test);
	CPPUNIT_TEST(identity_merge_test);
	CPPUNIT_TEST(simple_merge_test);
	CPPUNIT_TEST(pop_merge_test);
	CPPUNIT_TEST(merge_all_3_test);
	CPPUNIT_TEST_SUITE_END();
public:
	representation::dft_description description;

	void setUp() {
		description = representation::dft_description{};
		description.pop.insert(std::make_pair(std::make_pair(4, 'f'), std::make_pair(1, 1)));
		description.pop.insert(std::make_pair(std::make_pair(4, 'f'), std::make_pair(2, 2)));
		description.push.insert(std::make_pair(std::make_pair(1, 'b'), 4));
		description.push.insert(std::make_pair(std::make_pair(2, 'b'), 2));
		description.transitions.insert(std::make_pair(std::make_pair(1, 'a'), 2));
		description.transitions.insert(std::make_pair(std::make_pair(1, 'b'), 3));
		description.transitions.insert(std::make_pair(std::make_pair(2, 'b'), 3));
		description.transitions.insert(std::make_pair(std::make_pair(3, 'c'), 4));
		description.transitions.insert(std::make_pair(std::make_pair(2, 'e'), 1));
		description.transitions.insert(std::make_pair(std::make_pair(3, 'e'), 1));
		description.transitions.insert(std::make_pair(std::make_pair(1, 'f'), 2));
		description.transitions.insert(std::make_pair(std::make_pair(2, 'g'), 4));
		description.start_state = 1;
		// Transitions leading to state 3
		description.output.insert(std::make_pair(std::make_pair(1,'b'), 1));
		description.output.insert(std::make_pair(std::make_pair(2,'b'), 1));
		// Transitions leading to state 4
		description.output.insert(std::make_pair(std::make_pair(4,'f'), 2));

	}
	void tearDown() {}

	typedef transducers::pushdown::state_map_pushdown_transducer<transducers::aggregation::symbol_buffer<uint32_t>> transducer_type;
	typedef transducers::aggregation::symbol_buffer<uint32_t> buffer;
	typedef data_structures::pushdown_state_map<buffer::partial_result> map_type;

	void simple_test() {
		buffer b;
		auto trans = transducers::compose<transducers::pushdown::state_map_pushdown_transducer>(b, description);
		auto pr1 = trans.initial_result();
		CPPUNIT_ASSERT_EQUAL(pr1, pr1);
		auto pr2 = pr1;
		CPPUNIT_ASSERT_EQUAL(pr1, pr2);
	}

	void construction_test() {
		buffer b;
		auto trans = transducers::compose<transducers::pushdown::state_map_pushdown_transducer>(b, description);
		auto pr1 = trans.initial_result();
		const auto& pr1_map = pr1.map();
		CPPUNIT_ASSERT_EQUAL(ssize_t(1), std::distance(pr1_map.entries_begin(), pr1_map.entries_end()));
		auto e = pr1_map.entries_begin();
		CPPUNIT_ASSERT_EQUAL(ssize_t(1), std::distance(e->start_stack_begin(), e->start_stack_end()));
		CPPUNIT_ASSERT_EQUAL(ssize_t(1), std::distance(e->finish_stack_begin(), e->finish_stack_end()));

		auto pr2 = trans.identity_result();
		const auto& pr2_map = pr2.map();
		CPPUNIT_ASSERT_EQUAL(ssize_t(4), std::distance(pr2_map.entries_begin(), pr2_map.entries_end()));
		std::set<int> all_states = {1,2,3,4};
		for (const auto& en: pr2_map.entries()) {
			CPPUNIT_ASSERT_EQUAL(ssize_t(1), std::distance(en.start_stack_begin(), en.start_stack_end()));
			CPPUNIT_ASSERT_EQUAL(ssize_t(1), std::distance(en.finish_stack_begin(), en.finish_stack_end()));
			CPPUNIT_ASSERT_EQUAL(*en.start_stack_begin(), *en.finish_stack_begin());
			auto find_iter = all_states.find(*en.start_stack_begin());
			CPPUNIT_ASSERT(find_iter != all_states.end());
			all_states.erase(find_iter);
		}
		CPPUNIT_ASSERT(all_states.empty());
	}

	void add_map_entry(map_type& m, std::initializer_list<int> start_list, std::initializer_list<int> finish_list) {
		m.add_entry(start_list.begin(), start_list.end(), finish_list.begin(), finish_list.end(), std::vector<uint32_t>{});
	}
	void add_map_entry(map_type& m, std::initializer_list<int> start_list,
			std::initializer_list<int> finish_list, std::initializer_list<uint32_t> symbols) {
			m.add_entry(start_list.begin(), start_list.end(), finish_list.begin(), finish_list.end(),
					std::vector<uint32_t>{symbols.begin(), symbols.end()});
	}
	void transition_test() {
		buffer b;
		auto trans = transducers::compose<transducers::pushdown::state_map_pushdown_transducer>(b, description);
		auto pr1 = trans.initial_result();
		trans.process_symbol(pr1, 'a', 0);
		map_type target;
		add_map_entry(target, {1}, {2});
		CPPUNIT_ASSERT_EQUAL(target, pr1.map());
	}

	void discard_test() {
		buffer b;
		auto trans = transducers::compose<transducers::pushdown::state_map_pushdown_transducer>(b, description);
		auto pr1 = trans.identity_result();
		trans.process_symbol(pr1, 'a', 0);
		map_type target;
		add_map_entry(target, {1}, {2});
		CPPUNIT_ASSERT_EQUAL(target, pr1.map());
	}

	void simple_pop_test() {
		map_type s,t;
		add_map_entry(s, {1}, {4,1});
		add_map_entry(t, {1}, {1}, {2});
		buffer b;
		auto trans = transducers::compose<transducers::pushdown::state_map_pushdown_transducer>(b, description);
		auto pr1 = trans.map_to_result(s);
		trans.process_symbol(pr1, 'f', 0);
		CPPUNIT_ASSERT_EQUAL(t, pr1.map());
	}

	void unknown_pop_test() {
		map_type s,t;
		add_map_entry(s, {4}, {4});
		add_map_entry(t, {4,1}, {1}, {2});
		add_map_entry(t, {4,2}, {2}, {2});

		buffer b;
		auto trans = transducers::compose<transducers::pushdown::state_map_pushdown_transducer>(b, description);
		auto pr1 = trans.map_to_result(s);
		trans.process_symbol(pr1, 'f', 0);
		CPPUNIT_ASSERT_EQUAL(t, pr1.map());
	}

	void pop_and_plain_test() {
		map_type s,t;
		add_map_entry(s,{1},{1});
		add_map_entry(s,{4},{4});
		add_map_entry(t,{4,1},{1}, {2});
		add_map_entry(t,{1},{2});
		add_map_entry(t,{4,2},{2}, {2});

		buffer b;
		auto trans = transducers::compose<transducers::pushdown::state_map_pushdown_transducer>(b, description);
		auto pr1 = trans.map_to_result(s);
		trans.process_symbol(pr1, 'f', 0);
		CPPUNIT_ASSERT_EQUAL(t, pr1.map());
	}

	void simple_push_test() {
		map_type s,t;
		add_map_entry(s,{1},{1});
		add_map_entry(t,{1},{3,4}, {1});

		buffer b;
		auto trans = transducers::compose<transducers::pushdown::state_map_pushdown_transducer>(b, description);
		auto pr1 = trans.map_to_result(s);
		trans.process_symbol(pr1, 'b', 0);
		CPPUNIT_ASSERT_EQUAL(t, pr1.map());
	}

	void push_combine_test() {
		map_type s,t;
		add_map_entry(s, {1},{1});
		add_map_entry(s, {2},{2});
		add_map_entry(t, {2},{3,2}, {1});
		add_map_entry(t, {1},{3,4}, {1});

		buffer b;
		auto trans = transducers::compose<transducers::pushdown::state_map_pushdown_transducer>(b, description);
		auto pr1 = trans.map_to_result(s);
		trans.process_symbol(pr1, 'b', 0);
		CPPUNIT_ASSERT_EQUAL(t, pr1.map());
	}

	void simple_combine_test() {
		map_type s,t;
		add_map_entry(s, {2},{2});
		add_map_entry(s, {3},{3});
		add_map_entry(t, {2},{1});
		add_map_entry(t, {3},{1});

		buffer b;
		auto trans = transducers::compose<transducers::pushdown::state_map_pushdown_transducer>(b, description);
		auto pr1 = trans.map_to_result(s);
		trans.process_symbol(pr1, 'e', 0);
		CPPUNIT_ASSERT_EQUAL(t, pr1.map());
	}

	void pop_drop_test() {
		map_type s,t;
		add_map_entry(s,{3},{4});
		add_map_entry(s,{4},{4,2});
		add_map_entry(s,{1},{4,3});
		add_map_entry(t,{3,1},{1}, {2});
		add_map_entry(t,{3,2},{2}, {2});
		add_map_entry(t,{4},{2}, {2});

		buffer b;
		auto trans = transducers::compose<transducers::pushdown::state_map_pushdown_transducer>(b, description);
		auto pr1 = trans.map_to_result(s);
		trans.process_symbol(pr1, 'f', 0);
		CPPUNIT_ASSERT_EQUAL(t, pr1.map());
	}

	void double_pop_test() {
		map_type s,t,u,v;
		add_map_entry(s,{3},{4});
		add_map_entry(t,{3,1},{1}, {2});
		add_map_entry(t,{3,2},{2}, {2});
		add_map_entry(u,{3,2},{4}, {2});
		add_map_entry(v,{3,2,1},{1}, {2,2});
		add_map_entry(v,{3,2,2},{2}, {2,2});

		buffer b;
		auto trans = transducers::compose<transducers::pushdown::state_map_pushdown_transducer>(b, description);
		auto pr1 = trans.map_to_result(s);
		trans.process_symbol(pr1, 'f', 0);
		CPPUNIT_ASSERT_EQUAL(t, pr1.map());
		trans.process_symbol(pr1, 'g', 1);
		CPPUNIT_ASSERT_EQUAL(u, pr1.map());
		trans.process_symbol(pr1, 'f', 2);
		CPPUNIT_ASSERT_EQUAL(v, pr1.map());
	}

	void identity_merge_test() {
		buffer b;
		auto trans = transducers::compose<transducers::pushdown::state_map_pushdown_transducer>(b, description);
		auto pr1 = trans.identity_result();
		auto pr2 = trans.identity_result();
		auto pr3 = trans.identity_result();

		trans.merge_results(pr2, pr3);
		CPPUNIT_ASSERT_EQUAL(pr1.map(), pr2.map());
	}

	void simple_merge_test() {
		buffer b;
		auto trans = transducers::compose<transducers::pushdown::state_map_pushdown_transducer>(b, description);
		auto pr1 = trans.identity_result();
		auto pr2 = trans.identity_result();
		auto pr3 = trans.identity_result();

		trans.process_symbol(pr1, 'a', 0);
		trans.process_symbol(pr1, 'b', 1);
		trans.process_symbol(pr2, 'a', 0);
		trans.process_symbol(pr3, 'b', 1);

		trans.merge_results(pr2, pr3);
		CPPUNIT_ASSERT_EQUAL(pr1.map(), pr2.map());
	}

	void pop_merge_test() {
		buffer b;
		auto trans = transducers::compose<transducers::pushdown::state_map_pushdown_transducer>(b, description);
		auto pr1 = trans.identity_result();
		auto pr2 = trans.identity_result();
		auto pr3 = trans.identity_result();

		trans.process_symbol(pr1, 'f', 0);
		trans.process_symbol(pr1, 'f', 1);
		trans.process_symbol(pr2, 'f', 0);
		trans.process_symbol(pr3, 'f', 1);

		trans.merge_results(pr2, pr3);
		CPPUNIT_ASSERT_EQUAL(pr1.map(), pr2.map());
	}

	void merge_all_3_test() {
		buffer b;
		auto trans = transducers::compose<transducers::pushdown::state_map_pushdown_transducer>(b, description);

		for (char a = 'a'; a <= 'g'; ++a) {
			for (char b = 'a'; b <= 'g'; ++b) {
				for (char c = 'a'; c <= 'g'; ++c) {
					auto pr1 = trans.identity_result();
					auto pr2 = trans.identity_result();
					auto pr3 = trans.identity_result();
					auto pr4 = trans.identity_result();

					trans.process_symbol(pr1, a, 0);
					trans.process_symbol(pr1, b, 1);
					trans.process_symbol(pr1, c, 2);
					trans.process_symbol(pr2, a, 0);
					trans.process_symbol(pr3, b, 1);
					trans.process_symbol(pr4, c, 2);

					trans.merge_results(pr3, pr4);
					trans.merge_results(pr2, pr3);

					CPPUNIT_ASSERT_EQUAL(pr1.map(), pr2.map());
				}
			}
		}
	}
};

CPPUNIT_TEST_SUITE_REGISTRATION(state_map_pushdown_transducer_test);
