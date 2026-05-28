#pragma once
#include "database.hpp"
#include <regex>

std::string GlobToRegex(const std::string& glob);

size_t UnitsToBytes(const std::string& max_memory);

std::string CommonDel(const std::vector<std::string>& keys, DataBase& data_base);

std::string CommonExists(const std::vector<std::string>& keys, DataBase& data_base);

std::string CommonKeys(const std::string& pattern, DataBase& data_base);

std::string CommonFlushDb(DataBase& data_base);

std::string CommonType(const std::string& key, DataBase& data_base);

std::string CommonDbSize(DataBase& data_base);

std::string CommonConfigSet(const std::string& unit, DataBase& data_base);

std::string CommonConfigGet(DataBase& data_base);

std::string CommonMemoryUsage(const std::string& key, DataBase& data_base);