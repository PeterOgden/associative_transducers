#include <cppunit/extensions/HelperMacros.h>

#include <transducers/util/match_adapter.h>
#include <transducers/aggregation/symbol_buffer.h>
#include <transducers/compose.h>

using namespace transducers;
using namespace transducers::util;
using namespace symbols;


namespace {
	std::vector<uint32_t> test_input = {
		0x80000001, // 0
		0x80000002, // 1
		0xC0000003, // 2
		0x40000002, // 3
		0xC0000004, // 4
		0x80000005, // 5
		0x40000005, // 6
		0x40000001, // 7
		0x80000006, // 8
		0x40000006  // 9
	};
	std::vector<symbols::match> test_output = {
		match{ 3, 2, 2},
		match{ 2, 1, 3},
		match{ 4, 4, 4},
		match{ 5, 5, 6},
		match{ 1, 0, 7},
		match{ 6, 8, 9}
	};

	void sort_matches(std::vector<symbols::match>& matches) {
		std::sort(matches.begin(), matches.end(),
			[](const symbols::match& lhs, const symbols::match& rhs) {
			return lhs.end_offset < rhs.end_offset;
		});
	}
};

class match_adapter_test : public CppUnit::TestFixture {
public:
	CPPUNIT_TEST_SUITE(match_adapter_test);
	CPPUNIT_TEST(simple_test);
	CPPUNIT_TEST(two_fragment_test);
	CPPUNIT_TEST_SUITE_END();
public:
	void setUp() {}
	void tearDown() {}
	typedef transducers::aggregation::symbol_buffer<symbols::match> match_buffer;

	void simple_test() {
		match_buffer matches;
		auto adapter = compose<match_adapter>(matches);
		auto f = adapter.initial_result();

		std::size_t offset = 0;
		for (const auto& i: test_input) {
			adapter.process_symbol(f, i, offset++);
		}

		CPPUNIT_ASSERT_EQUAL(test_output.size(), adapter.last_stage_result(f).size());
		for (std::size_t i = 0; i != test_output.size(); ++i) {
			CPPUNIT_ASSERT_EQUAL(test_output[i], adapter.last_stage_result(f)[i]);
		}
	}

	void two_fragment_test() {
		match_buffer matches;
		auto adapter = compose<match_adapter>(matches);

		for (std::size_t split_point = 0; split_point <= test_input.size(); ++split_point) {
			auto f = adapter.initial_result();
			auto f1 = adapter.identity_result();
			auto f2 = adapter.identity_result();

			std::size_t offset = 0;
			for (const auto& i: test_input) {
				adapter.process_symbol(offset < split_point? f1: f2, i, offset);
				++offset;
			}
			adapter.merge_results(f1, f2);
			adapter.merge_results(f, f1);
			auto matches = adapter.last_stage_result(f);
			sort_matches(matches);
			CPPUNIT_ASSERT_EQUAL(test_output.size(), matches.size());
			for (std::size_t i = 0; i != test_output.size(); ++i) {
				CPPUNIT_ASSERT_EQUAL(test_output[i], matches[i]);
			}
		}
	}
};

CPPUNIT_TEST_SUITE_REGISTRATION(match_adapter_test);
