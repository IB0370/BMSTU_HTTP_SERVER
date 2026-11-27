#include "config/config.h"

#include <gtest/gtest.h>

#include <string>
#include <vector>

namespace {

// Собирает argv из вектора строк так же, как это делает оболочка.
config::ParseResult parse_args(std::vector<std::string> args) {
    std::vector<char*> argv;
    argv.reserve(args.size());
    for (auto& a : args) {
        argv.push_back(a.data());
    }
    return config::parse(static_cast<int>(argv.size()), argv.data());
}

}  // namespace

TEST(Config, Defaults) {
    const auto r = parse_args({"server"});
    ASSERT_EQ(r.status, config::ParseStatus::Ok);
    EXPECT_EQ(r.config.port, 8080);
    EXPECT_EQ(r.config.threads, 4);
}

TEST(Config, ReadsBothFlags) {
    const auto r = parse_args({"server", "--port", "9090", "--threads", "8"});
    ASSERT_EQ(r.status, config::ParseStatus::Ok);
    EXPECT_EQ(r.config.port, 9090);
    EXPECT_EQ(r.config.threads, 8);
}

TEST(Config, OrderMattersNot) {
    const auto r = parse_args({"server", "--threads", "2", "--port", "1234"});
    ASSERT_EQ(r.status, config::ParseStatus::Ok);
    EXPECT_EQ(r.config.port, 1234);
    EXPECT_EQ(r.config.threads, 2);
}

TEST(Config, BadPort) {
    for (const char* bad : {"0", "65536", "-1", "abc"}) {
        const auto r = parse_args({"server", "--port", bad});
        EXPECT_EQ(r.status, config::ParseStatus::Error) << bad;
        EXPECT_FALSE(r.error.empty()) << bad;
    }
}

TEST(Config, BadThreads) {
    EXPECT_EQ(parse_args({"server", "--threads", "0"}).status, config::ParseStatus::Error);
}

TEST(Config, UnknownFlag) {
    EXPECT_EQ(parse_args({"server", "--verbose"}).status, config::ParseStatus::Error);
}

TEST(Config, MissingValue) {
    EXPECT_EQ(parse_args({"server", "--port"}).status, config::ParseStatus::Error);
}

TEST(Config, NotANumber) {
    EXPECT_EQ(parse_args({"server", "--port", "80a"}).status, config::ParseStatus::Error);
}

TEST(Config, Help) {
    EXPECT_EQ(parse_args({"server", "--help"}).status, config::ParseStatus::HelpRequested);
    EXPECT_FALSE(config::usage().empty());
}
