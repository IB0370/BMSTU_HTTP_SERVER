#include <gtest/gtest.h>

#include "storage/storage.h"

// Заготовка тестов хранилища.
//
// Когда появится реализация IStorage (например InMemoryStorage):
//   1. добавьте её .cpp в SERVER_LIB_SOURCES в корневом CMakeLists.txt;
//   2. включите нужный заголовок здесь;
//   3. замените GTEST_SKIP() на проверки контракта.
//
// Контракт, обязательный для любой реализации (п. 3 задания):
//   * id монотонно растёт и уникален даже при параллельных create();
//   * version увеличивается при каждом изменении объекта;
//   * replace()/patch() с несовпадающим expected_version -> Status::VersionConflict;
//   * get() несуществующего id -> std::nullopt, remove() -> false;
//   * list(offset, limit) корректно обрезает выборку;
//   * increment() атомарен: N потоков * M инкрементов delta=1 дают ровно N*M.

TEST(Storage, ContractNotImplementedYet) {
    GTEST_SKIP() << "Подключите реализацию IStorage и замените тест проверками контракта";
}
