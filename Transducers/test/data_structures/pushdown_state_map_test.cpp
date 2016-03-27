#include "data_structures/pushdown_state_map.h"

#include <cppunit/extensions/HelperMacros.h>

using namespace data_structures;

class pushdown_state_map_test: public CppUnit::TestFixture {
public:
	CPPUNIT_TEST_SUITE(pushdown_state_map_test);
	CPPUNIT_TEST(simple_test);
	CPPUNIT_TEST(converge_test);
	CPPUNIT_TEST(push_test);
	CPPUNIT_TEST(pop_test);
	CPPUNIT_TEST(unknown_pop_test);
	CPPUNIT_TEST_SUITE_END();
public:
	typedef data_structures::pushdown_state_map<int> state_map;
	void setUp() {}
	void tearDown() {}

	void simple_test() {
		state_map m1;
		for (int i = 0; i < 5; ++i) {
			m1.add_entry(&i, &i + 1, &i, &i + 1, i);
		}
		state_map m2 = m1;
		CPPUNIT_ASSERT_EQUAL(m1, m2);
		CPPUNIT_ASSERT_EQUAL(ssize_t(5), std::distance(m1.layer_begin(), m1.layer_end()));
		auto layer = m1.layer_begin();
		for (int i = 0; i < 5; ++i) {
			CPPUNIT_ASSERT_EQUAL(ssize_t(0), std::distance(layer->children_begin(), layer->children_end()));
			CPPUNIT_ASSERT_EQUAL(ssize_t(1), std::distance(layer->values_begin(), layer->values_end()));
			CPPUNIT_ASSERT_EQUAL(i, *layer->values_begin());
			CPPUNIT_ASSERT_EQUAL(i, layer->state());
			layer++;
		}
	}

	void converge_test() {
		state_map m1;
		state_map exp;
		int constant = 1;
		for (int i = 0; i < 5; ++i) {
			m1.add_entry(&i, &i + 1, &i, &i + 1, i);
			exp.add_entry(&i, &i + 1, &constant, &constant + 1, i);
		}
		for (auto iter = m1.layer_begin(); iter != m1.layer_end(); ++iter) {
			m1.transition_state(iter, 1);
		}
		m1.finalise();

		CPPUNIT_ASSERT_EQUAL(exp, m1);
		CPPUNIT_ASSERT_EQUAL(ssize_t(1), std::distance(m1.layer_begin(), m1.layer_end()));
		auto layer = m1.layer_begin();
		CPPUNIT_ASSERT_EQUAL(ssize_t(5), std::distance(layer->values_begin(), layer->values_end()));
		CPPUNIT_ASSERT_EQUAL(ssize_t(0), std::distance(layer->children_begin(), layer->children_end()));
		auto value_iter = layer->values_begin();
		for (int i = 0; i < 5; ++i) {
			CPPUNIT_ASSERT_EQUAL(i, *value_iter);
			value_iter++;
		}
	}

	void push_test() {
		state_map m1;
		state_map exp;

		int stack[2];
		for (int i = 0; i < 5; ++i) {
			stack[0] = 0;
			stack[1] = i;
			m1.add_entry(&i, &i + 1, &i, &i + 1, i);
			exp.add_entry(&i, &i + 1, stack + (i<3?0:1), stack + 2, i);
		};
		auto layer_iter = m1.layer_begin();
		for (int i = 0; i < 3; ++i) {
			m1.push_state(layer_iter++, 0, i);
		}
		m1.finalise();
		exp.finalise();

		CPPUNIT_ASSERT_EQUAL(exp, m1);
		CPPUNIT_ASSERT_EQUAL(ssize_t(3), std::distance(m1.layer_begin(), m1.layer_end()));
		layer_iter = m1.layer_begin();
		CPPUNIT_ASSERT_EQUAL(ssize_t(3), std::distance(layer_iter->children_begin(), layer_iter->children_end()));
		
	}

	void pop_test() {
		state_map m1;
		state_map exp;
		int stack[2];
		for (int i = 0; i < 5; ++i) {
			stack[0] = 0;
			stack[1] = i;
			m1.add_entry(&i, &i + 1, stack, stack + 2, i);
			exp.add_entry(&i, &i + 1, stack + (i<3?1:0), stack + 2, i);
		}
		exp.finalise();
		m1.finalise();

		auto layer = m1.layer_begin();
		CPPUNIT_ASSERT_EQUAL(ssize_t(5), std::distance(layer->children_begin(), layer->children_end()));
		auto sub_layer = layer->children_begin();
		for (int i = 0; i < 3; ++i) {
			m1.pop_state(sub_layer++, sub_layer->state());
		}
		m1.finalise();
		CPPUNIT_ASSERT_EQUAL(exp, m1);
	}

	void unknown_pop_test() {
		state_map m1;
		state_map exp;
		int stack[2];
		for (int i = 0; i < 4; ++i) {
			stack[0] = i / 2;
			stack[1] = i;
			exp.add_entry(stack, stack + 2, &i, &i + 1, stack[0]);
		}
		stack[0] = 2;
		exp.add_entry(stack, stack + 1, stack, stack + 1, *stack);
		for (int i = 0; i < 3; ++i) {
			m1.add_entry(&i, &i + 1, &i, &i + 1, i);
		}
		m1.finalise();
		exp.finalise();

		auto iter = m1.layer_begin();
		m1.pop_unknown_state(iter, 0, 0);
		m1.pop_unknown_state(iter, 1, 1);
		m1.clear_values(iter);
		++iter;
		m1.pop_unknown_state(iter, 2, 2);
		m1.pop_unknown_state(iter, 3, 3);
		m1.clear_values(iter);

		m1.finalise();
		CPPUNIT_ASSERT_EQUAL(exp, m1);
		exp.start_stack_finalise();
		CPPUNIT_ASSERT_EQUAL(ssize_t(3), std::distance(exp.start_layer_begin(), exp.start_layer_end()));
	}
};

CPPUNIT_TEST_SUITE_REGISTRATION(pushdown_state_map_test);
