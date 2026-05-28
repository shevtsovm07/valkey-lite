#pragma once
#include "database.hpp"

std::string ListPush(bool begin, const std::string& key, const std::vector<std::string>& elements, DataBase& data_base);

std::string ListPop(bool begin, const std::string& key, size_t count, bool explicit_count, DataBase& data_base);

std::string ListLen(const std::string& key, DataBase& data_base);

std::string ListRange(const std::string& key, int start, int end, DataBase& data_base);

std::string ListIndex(const std::string& key, int index, DataBase& data_base);

std::string ListSetIndex(const std::string& key, int index, const std::string& value, DataBase& data_base);

std::string ListInsert(bool before, const std::string& key, const std::string& pivot, const std::string& value, DataBase& data_base);

