#include <gtest/gtest.h>
#include <protocol_handler/http/1.0.h>

TEST(HTTP10, ParsesWholeRequestProperly)
{
	protocol::HTTP1 agent;
	auto parser = agent.get_praser();
}

int main(int argc, char** argv)
{
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
