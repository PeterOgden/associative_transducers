#include <cppunit/extensions/HelperMacros.h>
#include <transducers/util/match_adapter.h>

class match_adapter_test {
public:
	CPPUNIT_TEST_SUITE(match_adapter_test);
	CPPUNIT_TEST(simple_test);
	CPPUNIT_TEST_SUITE_END();

public:
	void setUp() {}
	void tearDown() {}

	void simple_test() {

	}
};

CPPUNIT_TEST_SUITE_REGISTRATION(match_adapter_test);
