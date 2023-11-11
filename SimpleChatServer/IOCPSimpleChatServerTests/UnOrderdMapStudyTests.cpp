#define BOOST_TEST_MODULE mytests
#include <boost/test/included/unit_test.hpp>
#include <unordered_map>
#include <iostream>
#include <algorithm>

using namespace std;

unordered_map<int, int> m{};
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