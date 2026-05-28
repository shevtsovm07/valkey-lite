#pragma once
#include "database.hpp"

std::string SetAdd(const std::string& key, const std::vector<std::string>& elements, DataBase& data_base);

std::string SetRemove(const std::string& key, const std::vector<std::string>& elements, DataBase& data_base);

std::string SetIsMember(const std::string& key, const std::string& element, DataBase& data_base);

std::string SetMembers(const std::string& key, DataBase& data_base);

std::string SetCard(const std::string& key, DataBase& data_base);

std::string SetUnion(const std::vector<std::string>& keys, DataBase& data_base);

std::string SetInter(const std::vector<std::string>& keys, DataBase& data_base);

std::string SetDifference(const std::vector<std::string>& keys, DataBase& data_base);

std::string SetMove(const std::string& key_source, const std::string& key_destination, const std::string& member, DataBase& data_base);