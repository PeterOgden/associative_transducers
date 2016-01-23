#include <cppunit/extensions/HelperMacros.h>
#include <transducers/aggregation/symbol_buffer.h>

using transducers::aggregation::symbol_buffer;

class symbol_buffer_test : public CppUnit::TestFixture {
public:
	CPPUNIT_TEST_SUITE(symbol_buffer_test);
	CPPUNIT_TEST(int_test);
	CPPUNIT_TEST_SUITE_END();

public:
	void setUp() {}
	void tearDown() {}

	void int_test() {
		symbol_buffer<int> buffer;
		auto f1 = buffer.initial_result();
		auto f2 = buffer.identity_result();
		buffer.process_symbol(f1, 1, 0);
		buffer.process_symbol(f2, 2, 1);
		buffer.merge_results(f1, f2, 0);
		CPPUNIT_ASSERT_EQUAL(std::size_t(2), f1.size());
		CPPUNIT_ASSERT_EQUAL(1, f1[0]);
		CPPUNIT_ASSERT_EQUAL(2, f1[1]);
	};
};

CPPUNIT_TEST_SUITE_REGISTRATION(symbol_buffer_test);
