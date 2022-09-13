// Copyright 2018, 2019, 2022 by Mark Melton
//

#include <gtest/gtest.h>
#include "core/cc/queue/spsc.h"
#include "core/cc/scoped_task.h"
#include "ranges/views.h"
#include "ranges/to.h"

using ::testing::StaticAssertTypeEq;

void consumer(auto& queue, const ints& data)
{
    int i = 0, n;
    while (queue.pop(n))
    {
	EXPECT_EQ(n, data[i]);
	++i;
    }
}

TEST(QueueSpSc, SingleThread)
{
    core::cc::queue::SpSc<int> queue{8, 8};
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

TEST(QueueSpSc, MultiThreaded)
{
    const auto NumberExamples = 32;
    auto uniform_size = v::uniform(0u, 2048u)
	| v::take(NumberExamples);
    
    for (auto size : uniform_size)
    {
	auto data = v::uniform<int>()
	    | v::take(size)
	    | r::to<vector>;

	core::cc::queue::SpSc<int> queue{8, 8};
	std::thread sink([&]() { consumer(queue, data); });
    	for (size_t i = 0; i < data.size(); ++i)
    	    queue.push(data[i]);
    	queue.push_sentinel();
	sink.join();
    }
}

TEST(QueueSpSc, Throughput)
{
    for (auto i = 0; i < 1; ++i)
    {
	string input(1'000, ' ');
	string output(1'000, '.');
	core::cc::queue::SpSc<char> queue{4,4};
	core::cc::scoped_task producer([&]() { queue.push(input); queue.push_sentinel(); });
	auto ptr = output.data();
	auto end = ptr + output.size();
	size_t count{0};
	while (ptr < end and queue.pop(ptr, end, count))
	    ptr += count;
	producer.wait();
	EXPECT_EQ(input, output);
    }
}

int main(int argc, char *argv[])
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}



