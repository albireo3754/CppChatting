#include "UnOrderedMapStudyTests.h"

using namespace std;

BOOST_AUTO_TEST_CASE(unorderd_map_subscript_increase_size)
{
	using namespace std;
	unordered_map<int, int> m{};

	m[3] = 4;
	m[3] = 5;
	auto k = m[1];
	BOOST_TEST(m[3] == 5);
	BOOST_TEST(m[2] == NULL);
	BOOST_TEST(m.size() == 3);
}

class Test {
public:
	Test() {

	}
};
BOOST_AUTO_TEST_CASE(unorderd_map_find_not_increase_size)
{
	using namespace std;
	unordered_map<int, Test*> m{};

	m.emplace(3, (new Test()));
	m.emplace(3, (new Test()));
	BOOST_TEST(m[2] == nullptr);
	if (m.find(1) == m.end()) {
		BOOST_TEST((m.find(1) == m.end()) == TRUE);
	}
	else {
		BOOST_TEST(false);
	}
	BOOST_TEST(m.size() == 2);
}

BOOST_AUTO_TEST_CASE(option_auto_condition_Check_Unwrap)
{
	using namespace std;
	optional<int> op1 = 3;
	optional<int> op2 = nullopt;

	if (auto op3 = op1) {
		BOOST_TEST(op1.value(), 3);
	}

	if (auto op3 = op2) {
		BOOST_TEST(false);
	}
	else {
		BOOST_TEST(true);
	}

	if (op2.has_value()) {
		BOOST_TEST(false);
	}
}
