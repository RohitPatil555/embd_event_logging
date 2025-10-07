#include <gtest/gtest.h>

#include <eventCollector.hpp>

TEST( BasicTest, testSingletonEventLog ) {
	eventCollector *ptr	 = nullptr;
	eventCollector *ptr1 = nullptr;

	ptr = eventCollector::getInstance();
	EXPECT_NE( ptr, nullptr );

	ptr1 = eventCollector::getInstance();
	EXPECT_EQ( ptr, ptr1 );
}
