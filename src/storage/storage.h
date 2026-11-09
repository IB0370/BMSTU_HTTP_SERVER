#pragma once
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

struct Item {
    std::uint64_t id;
    std::string name;
    std::int64_t value;
    std::uint64_t version;
};

enum class Status { Ok, NotFound, VersionConflict };

struct Result {
    Status status;
    Item item;  // читать только при Status::Ok!!!
                // в других случаях там мусор!!!
};

class IStorage {
   public:
    virtual ~IStorage() = default;
    virtual Item create(std::string name, std::int64_t value) = 0;
    virtual std::optional<Item> get(std::uint64_t id) const = 0;
    virtual std::vector<Item> list(std::size_t offset, std::size_t limit) const = 0;
    virtual Result replace(std::uint64_t id, std::string name, std::int64_t value,
                           std::optional<std::uint64_t> expected_version) = 0;
    virtual Result patch(std::uint64_t id, std::optional<std::string> name,
                         std::optional<std::int64_t> value,
                         std::optional<std::uint64_t> expected_version) = 0;
    virtual bool remove(std::uint64_t id) = 0;
    virtual Result increment(std::uint64_t id, std::int64_t delta) = 0;
};