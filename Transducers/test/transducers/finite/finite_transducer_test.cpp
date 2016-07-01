#include <cppunit/extensions/HelperMacros.h>
#include "transducers/finite/finite_transducer.h"
#include "transducers/aggregation/symbol_buffer.h"
#include "transducers/compose.h"

class finite_transducer_test : public CppUnit::TestFixture {
public:
	CPPUNIT_TEST_SUITE(finite_transducer_test);
	CPPUNIT_TEST(simple_test);
	CPPUNIT_TEST(merge_test);
	CPPUNIT_TEST_SUITE_END();

public:
	void setUp() {}
	void tearDown() {}

	finite_transducer_test() {
		description.transitions.insert(std::make_pair(std::make_pair(1, 'a'), 2));
		description.transitions.insert(std::make_pair(std::make_pair(2, 'b'), 3));
		description.transitions.insert(std::make_pair(std::make_pair(3, 'c'), 1));
		description.output.insert(std::make_pair(std::make_pair(3, 'c'), 20));
		description.start_state = 1;
	}

	typedef transducers::aggregation::symbol_buffer<int> buffer;
	representation::dft_description description;
	void simple_test() {
		buffer b;
		auto trans = transducers::compose<transducers::finite::finite_transducer>(b, description);
		auto pr1 = trans.initial_result();
		trans.process_symbol(pr1, 'a', 0);
		trans.process_symbol(pr1, 'b', 1);
		trans.process_symbol(pr1, 'c', 2);

		const auto& result = trans.last_stage_result(pr1);
		CPPUNIT_ASSERT_EQUAL((size_t)1, result.size());
		CPPUNIT_ASSERT_EQUAL(20, result.at(0));
	}

	void merge_test() {
		buffer b;
		auto trans = transducers::compose<transducers::finite::finite_transducer>(b, description);
		auto pr1 = trans.initial_result();
		trans.process_symbol(pr1, 'a', 0);
		trans.process_symbol(pr1, 'b', 1);
		trans.process_symbol(pr1, 'c', 2);

		auto pr2 = trans.identity_result();
		trans.process_symbol(pr2, 'a', 3);
		trans.process_symbol(pr2, 'b', 4);
		trans.process_symbol(pr2, 'c', 5);

		trans.merge_results(pr1, pr2);

		const auto& result = trans.last_stage_result(pr1);
		CPPUNIT_ASSERT_EQUAL((size_t)2, result.size());
		CPPUNIT_ASSERT_EQUAL(20, result.at(0));
		CPPUNIT_ASSERT_EQUAL(20, result.at(1));
	}
};

CPPUNIT_TEST_SUITE_REGISTRATION(finite_transducer_test);
