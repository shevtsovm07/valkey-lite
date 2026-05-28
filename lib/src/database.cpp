#include "database.hpp"
#include <unordered_map>
#include <string>
#include <chrono>
#include <utility>
#include <stdexcept>
#include <variant>
#include <vector>
#include <cstddef>

Value& DataBase::Get(const std::string& key) {
  if (data_.contains(key) && IsExpired(key)) {
    Del(key);
  }

  auto iterator = data_.find(key);
  
  if (iterator == data_.end()) {
    throw std::runtime_error("key not found");
  }

  return iterator->second;
}


void DataBase::Del(const std::string& key) {
  data_.erase(key);
  ttl_.erase(key);
}

bool DataBase::Exists(const std::string& key) {
  if (data_.contains(key) && IsExpired(key)) {
    Del(key);
  }
  return data_.contains(key);
}

std::string DataBase::Type(const std::string& key) const {
  auto it = data_.find(key);

  if (it == data_.end()) return "none";

  return std::visit(Overload{
    [](const StringType&) {return std::string("string"); },
    [](const ListType&) {return std::string("list"); },
    [](const SetType&) {return std::string("set"); },
    [](const GeoType&) {return std::string("zset"); },
  }, it->second);
}

void DataBase::SetTtl(const std::string& key, int seconds) {
  auto now = std::chrono::steady_clock::now();
  auto expires_at = now + std::chrono::seconds(seconds);
  ttl_[key] = expires_at;
}

bool DataBase::IsExpired(const std::string& key) const {
  if (!ttl_.contains(key)) return false;
  return (std::chrono::steady_clock::now() > ttl_.at(key));
}

bool DataBase::HasTtl(const std::string& key) const {
  return ttl_.contains(key);
}

int DataBase::GetRemainingTtl(const std::string& key) const {
  if (!HasTtl(key)) {
    return 0;
  }
  auto remaining = ttl_.at(key) - std::chrono::steady_clock::now();
  return std::chrono::duration_cast<std::chrono::seconds>(remaining).count();
}

std::vector<std::string> DataBase::GetAllKeys() const {
  std::vector<std::string> all_keys;
  for (const auto& pair : data_) {
    all_keys.push_back(pair.first);
  }
  return all_keys;
}

size_t DataBase::MaxMemoryGetter() const {
  return maxmemory_;
}

void DataBase::SetMaxMemory(size_t bytes) {
  maxmemory_ = bytes;
}

size_t DataBase::EstimateSize(const std::string& key) const {
  auto it = data_.find(key);
  if (it == data_.end()) return 0;
  size_t size = key.size() + kKeyOverhead;
  std::visit(Overload{
    [&](const StringType& string) {size += string.size();},
    [&](const ListType& list) {for (auto& el : list) size += el.size() + kNodeOverhead; },
    [&](const SetType& set) {for (auto& el : set) size += el.size() + kNodeOverhead; },
    [&](const GeoType& geo) {size += geo.size() * kCoordinatesOverhead;},
  }, it->second);
  return size;
}

void DataBase::CheckMemory(size_t additional) const {
  if (maxmemory_ > 0 && CurrentMemory() + additional > maxmemory_) {
    throw std::runtime_error("OOM command not allowed when used memory > 'maxmemory'");
  }
}

size_t DataBase::CurrentMemory() const {
  size_t total = 0;
  for (auto& [key, val] : data_) {
    total += EstimateSize(key);
  }
  return total;
}