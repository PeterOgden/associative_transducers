#include <cppunit/extensions/HelperMacros.h>
#include <transducers/numeric/multiply.h>
#include <transducers/aggregation/symbol_buffer.h>
#include <transducers/compose.h>

class multiply_test : public CppUnit::TestFixture {
public:
	CPPUNIT_TEST_SUITE(multiply_test);
	CPPUNIT_TEST(simple_test);
	CPPUNIT_TEST(compose_test);
	CPPUNIT_TEST(merge_test);
	CPPUNIT_TEST_SUITE_END();
public:
	void setUp() {}
	void tearDown() {}

	void simple_test() {
		typedef transducers::aggregation::symbol_buffer<int> symbol_buffer_int;
		symbol_buffer_int buffer;
		transducers::numeric::multiply<int, symbol_buffer_int> mult(buffer, 2);
		auto f = mult.initial_result();
		mult.process_symbol(f, 3, 0);
		CPPUNIT_ASSERT_EQUAL(std::size_t(1), mult.last_stage_result(f).size());
		CPPUNIT_ASSERT_EQUAL(6, mult.last_stage_result(f).at(0));
	}

	void compose_test() {
		transducers::aggregation::symbol_buffer<int> buffer;
		auto mult = transducers::compose<transducers::numeric::multiply_int>(buffer, 2);
		auto f = mult.initial_result();
		mult.process_symbol(f, 3, 0);
		CPPUNIT_ASSERT_EQUAL(std::size_t(1), mult.last_stage_result(f).size());
		CPPUNIT_ASSERT_EQUAL(6, mult.last_stage_result(f).at(0));
	}

	void merge_test() {
		transducers::aggregation::symbol_buffer<int> buffer;
		auto mult = transducers::compose<transducers::numeric::multiply_int>(buffer, 2);
		auto f = mult.initial_result();
		mult.process_symbol(f, 3, 0);
		auto f2 = mult.identity_result();
		mult.process_symbol(f2, 4, 1);
		mult.merge_results(f, f2, 0);
		CPPUNIT_ASSERT_EQUAL(std::size_t(2), mult.last_stage_result(f).size());
		CPPUNIT_ASSERT_EQUAL(6, mult.last_stage_result(f).at(0));
		CPPUNIT_ASSERT_EQUAL(8, mult.last_stage_result(f).at(1));
	}
};

CPPUNIT_TEST_SUITE_REGISTRATION(multiply_test);
