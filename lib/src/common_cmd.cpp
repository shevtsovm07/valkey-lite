#include "common_cmd.hpp"
#include "database.hpp"
#include <algorithm>
#include <cctype>
#include <cstddef>
#include <regex>
#include <string>
#include <vector>

std::string GlobToRegex(const std::string& glob) {
  std::string res;
  res.reserve(glob.size() * 2);
  res += '^';
  for (char c : glob) {
    switch (c) {
      case '*': 
        res += ".*"; 
        break;
      case '?': 
        res += '.'; 
        break;
      case '.': case '(': case ')': case '+': case '[': case ']':
      case '{': case '}': case '^': case '$': case '|': case '\\':
        res += '\\';
        res += c;
        break;
      default:
        res += c;
        break;
    }
  }
  res += '$';
  return res;
}

size_t UnitsToBytes(const std::string& max_memory) {
  auto iterator = std::find_if(max_memory.begin(), max_memory.end(), [](unsigned char ch) {
    return std::isalpha(ch);
  });
  std::string string_value(max_memory.begin(), iterator);
  size_t value = std::stoull(string_value);
  std::string unit(iterator, max_memory.end());
  if (unit == "kb") return value * 1024;
  if (unit == "mb") return value * 1024 * 1024;
  if (unit == "gb") return value * 1024 * 1024 * 1024;
  return value;
}

std::string CommonDel(const std::vector<std::string>& keys, DataBase& data_base) {
  int deleted_count = 0;
  for (auto& key: keys) {
    if (data_base.Exists(key)) {
      data_base.Del(key);
      deleted_count++;
    }
  }
  return "(integer) " + std::to_string(deleted_count);
}

std::string CommonExists(const std::vector<std::string>& keys, DataBase& data_base) {
  int exists_count = 0;
  for (auto& key: keys) {
    if (data_base.Exists(key)) exists_count++;
  }
  return "(integer) " + std::to_string(exists_count);
}

std::string CommonKeys(const std::string& pattern, DataBase& data_base) {
  std::regex re(GlobToRegex(pattern));
  int counter = 0;
  std::string result;
  for (auto& key : data_base.GetAllKeys()) {
    if (std::regex_match(key, re) && data_base.Exists(key)) {
      result += ("(" + std::to_string(++counter) + ") " + "\"" + key + "\"" + "\n");
    }
  }
  return result;
}

std::string CommonFlushDb(DataBase& data_base) {
  std::vector<std::string> all_keys = data_base.GetAllKeys();
  for (auto& key : all_keys) {
    data_base.Del(key);
  }
  return "OK";
}

std::string CommonType(const std::string& key, DataBase& data_base) {
  return data_base.Type(key);
}

std::string CommonDbSize(DataBase& data_base) {
  std::vector<std::string> all_keys = data_base.GetAllKeys();
  int active = 0;
  for (auto& key : all_keys) {
    if (data_base.Exists(key)) active++;
  }
  return "(integer) " + std::to_string(active);
}

std::string CommonConfigSet(const std::string& max_memory, DataBase& data_base) {
  data_base.SetMaxMemory(static_cast<size_t>(UnitsToBytes(max_memory)));
  return "OK";
}

std::string CommonConfigGet(DataBase& data_base) {
  return "1) \"maxmemory\"\n2) \"" + std::to_string(data_base.MaxMemoryGetter()) + "\"";
}

std::string CommonMemoryUsage(const std::string& key, DataBase& data_base) {
  if (!data_base.Exists(key)) return "(nil)";
  return "(integer) " + std::to_string(data_base.EstimateSize(key));
}