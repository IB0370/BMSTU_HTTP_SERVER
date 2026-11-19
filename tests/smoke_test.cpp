#include <gtest/gtest.h>

#include <atomic>
#include <thread>
#include <vector>

#include "storage/storage.h"

// Инфраструктурный тест: фреймворк подключён, C++20 собирается,
// заголовки из src/ видны из тестов.
TEST(Infra, HeadersAndTypesAreVisible) {
    Item item{.id = 1, .name = "test", .value = 42, .version = 1};
    EXPECT_EQ(item.id, 1U);
    EXPECT_EQ(item.name, "test");
    EXPECT_EQ(item.value, 42);
    EXPECT_EQ(item.version, 1U);
    EXPECT_EQ(Status::Ok, Status::Ok);  // enum class доступен
}

TEST(Infra, ThreadsAreLinked) {
    std::atomic<int> counter{0};
    std::vector<std::thread> threads;
    for (int i = 0; i < 4; ++i) {
        threads.emplace_back([&counter] { counter.fetch_add(1); });
    }
    for (auto& t : threads) {
        t.join();
    }
    EXPECT_EQ(counter.load(), 4);
}
