#include "pch.h"
#include <boost/test/included/unit_test.hpp>

#define BOOST_TEST_MODULE My Test
BOOST_AUTO_TEST_CASE(myTestCase)
{
  BOOST_TEST(1 == 1);
  BOOST_TEST(true);
}
