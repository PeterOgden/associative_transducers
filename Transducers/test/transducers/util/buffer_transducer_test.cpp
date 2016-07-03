#include <cppunit/extensions/HelperMacros.h>
#include "transducers/util/buffer_transducer.h"
#include "transducers/finite/finite_transducer.h"
#include "transducers/aggregation/symbol_buffer.h"
#include "transducers/compose.h"

class buffer_transducer_test : public CppUnit::TestFixture {
public:
	CPPUNIT_TEST_SUITE(buffer_transducer_test);
	CPPUNIT_TEST(buffer_simple_test);
	CPPUNIT_TEST(transducer_test);
	CPPUNIT_TEST(double_transducer_test);
	CPPUNIT_TEST_SUITE_END();

public:
	void setUp() {}
	void tearDown() {}
	
	typedef transducers::aggregation::symbol_buffer<char> agg_buffer;
	representation::dft_description description;

	buffer_transducer_test() {
		description.transitions.insert(std::make_pair(std::make_pair(1, 'a'), 2));
		description.transitions.insert(std::make_pair(std::make_pair(2, 'b'), 3));
		description.transitions.insert(std::make_pair(std::make_pair(3, 'c'), 1));
		description.output.insert(std::make_pair(std::make_pair(3, 'c'), 20));
		description.start_state = 1;
	}

	void buffer_simple_test() {
		agg_buffer b;
		auto buffer = transducers::compose<transducers::util::buffer_transducer>(b, 'a');
		auto pr1 = buffer.initial_result();
		auto pr2 = buffer.identity_result();
		auto pr3 = buffer.identity_result();

		buffer.process_symbol(pr1, 'a', 0);
		buffer.process_symbol(pr2, 'b', 1);
		buffer.process_symbol(pr3, 'c', 2);
		
		buffer.merge_results(pr2, pr3);
		buffer.merge_results(pr1, pr2);

		const auto& terms = buffer.last_stage_result(pr1);
		CPPUNIT_ASSERT_EQUAL(std::size_t(3), terms.size());
		CPPUNIT_ASSERT_EQUAL('a', terms[0]);
		CPPUNIT_ASSERT_EQUAL('b', terms[1]);
		CPPUNIT_ASSERT_EQUAL('c', terms[2]);
	}

	void transducer_test() {
		transducers::aggregation::symbol_buffer<int> buf;
		auto ft = transducers::compose<transducers::finite::finite_transducer>(buf, description);
		auto bt = transducers::compose<transducers::util::buffer_transducer>(ft, 'a');
		auto pr1 = bt.initial_result();
		auto pr2 = bt.identity_result();
		auto pr3 = bt.identity_result();

		bt.process_symbol(pr1, 'a', 0);
		bt.process_symbol(pr2, 'b', 1);
		bt.process_symbol(pr3, 'c', 2);

		bt.merge_results(pr2, pr3);
		bt.merge_results(pr1, pr2);

		const auto& terms = bt.last_stage_result(pr1);
		CPPUNIT_ASSERT_EQUAL(std::size_t(1), terms.size());
		CPPUNIT_ASSERT_EQUAL(20, terms[0]);
	}

	void double_transducer_test() {
		transducers::aggregation::symbol_buffer<int> buf;
		auto ft = transducers::compose<transducers::finite::finite_transducer>(buf, description);
		auto bt = transducers::compose<transducers::util::buffer_transducer>(ft, 'a');
		auto pr1 = bt.initial_result();
		auto pr2 = bt.identity_result();
		auto pr3 = bt.identity_result();
		auto pr4 = bt.identity_result();
		auto pr5 = bt.identity_result();
		auto pr6 = bt.identity_result();

		bt.process_symbol(pr1, 'a', 0);
		bt.process_symbol(pr2, 'b', 1);
		bt.process_symbol(pr3, 'c', 2);
		bt.process_symbol(pr4, 'a', 3);
		bt.process_symbol(pr5, 'b', 4);
		bt.process_symbol(pr6, 'c', 5);

		bt.merge_results(pr5, pr6);
		bt.merge_results(pr4, pr5);
		bt.merge_results(pr3, pr4);
		bt.merge_results(pr2, pr3);
		bt.merge_results(pr1, pr2);

		const auto& terms = bt.last_stage_result(pr1);
		CPPUNIT_ASSERT_EQUAL(std::size_t(2), terms.size());
		CPPUNIT_ASSERT_EQUAL(20, terms[0]);
		CPPUNIT_ASSERT_EQUAL(20, terms[1]);
	}
};

CPPUNIT_TEST_SUITE_REGISTRATION(buffer_transducer_test);
