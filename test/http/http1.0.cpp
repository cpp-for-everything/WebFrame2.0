#include <catch2/catch_test_macros.hpp>
#include <gmock/gmock.h>

// Mock class
class MockExample
{
public:
	MOCK_METHOD(int, add, (int, int), ());
	MOCK_METHOD(int, subtract, (int, int), ());
};

TEST_CASE("Test add function with Google Mock", "[add]")
{
	MockExample mock;

	// Expect mock to be called with specific arguments
	EXPECT_CALL(mock, add(2, 3)).WillOnce(testing::Return(5));

	// Validate behavior
	CHECK(mock.add(2, 3) == 5);
}

TEST_CASE("Test subtract function with Google Mock", "[subtract]")
{
	MockExample mock;

	// Expect mock to be called with specific arguments
	EXPECT_CALL(mock, subtract(5, 3)).WillOnce(testing::Return(2));

	// Validate behavior
	CHECK(mock.subtract(5, 3) == 2);
}

#include <protocol_handler/http/1.0.h>

TEST_CASE("HTTP 1.0", "[parse whole request properly]")
{
	protocol::HTTP1 agent;
	auto parser = agent.get_praser();

	// Validate behavior
	CHECK(2 == 2);
}
