#pragma once
#include "database.hpp"


std::string StringSet(const std::string& key, const std::string& value, DataBase& data_base);

std::string StringGet(const std::string& key, DataBase& data_base);

std::string StringLen(const std::string& key, DataBase& data_base);

std::string StringAppend(const std::string& key, const std::string& suffix, DataBase& data_base);

std::string StringExpire(const std::string& key, int seconds, DataBase& data_base);

std::string StringTtl(const std::string& key, DataBase& data_base);

