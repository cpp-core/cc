// Copyright 2018, 2019, 2022 by Mark Melton
//

#include <gtest/gtest.h>
#include "core/cc/queue/buffered_spsc.h"
#include "ranges/views.h"
#include "ranges/to.h"

using ::testing::StaticAssertTypeEq;

void consumer(auto& queue, const ints& data)
{
    int i = 0, n;
    while (queue.active())
    {
	while (queue.pop(n))
	{
	    EXPECT_EQ(n, data[i]);
	    ++i;
	}
    }
}

TEST(BufferedQueueSpSc, SingleThread)
{
    core::cc::queue::BufferedSpSc<int> queue;
    for (auto i = 0; i < 32; ++i)
	queue.push(i);
    queue.push_sentinel();
    
    int count{0}, data{0};
    while (queue.pop(data))
    {
	EXPECT_LT(count, 32);
	EXPECT_EQ(data, count);
	++count;
    }
    EXPECT_EQ(count, 32);
}

TEST(BufferedQueueSpSc, MultiThreaded)
{
    const auto NumberExamples = 32;
    auto uniform_size = v::uniform(0u, 2048u)
	| v::take(NumberExamples);
    
    for (auto size : uniform_size)
    {
	auto data = v::uniform<int>()
	    | v::take(size)
	    | r::to<vector>;

	core::cc::queue::BufferedSpSc<int> queue;
	std::thread sink([&]() { consumer(queue, data); });
    	for (size_t i = 0; i < data.size(); ++i)
    	    queue.push(data[i]);
    	queue.push_sentinel();
	sink.join();
    }
}

int main(int argc, char *argv[])
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}



