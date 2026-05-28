#pragma once
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <list>
#include <chrono>
#include <utility>
#include <stdexcept>
#include <variant>
#include <vector>
#include <algorithm>

inline constexpr size_t kKeyOverhead  = 64;
inline constexpr size_t kNodeOverhead = 8;
inline constexpr size_t kCoordinatesOverhead = 32;
inline const std::string kWrongTypeErr = "(error) WRONGTYPE operation against a key holding the wrong kind of value";
inline const std::string kWrongKeyErr ="(error) no such key";
inline const std::string kEmptyResult ="(nil)";
inline const std::string kInvalidCoordinates = "(error) ERR invalid longitude,latitude pair";

struct GeoPoint {
  double longitude_;
  double latitude_;

  explicit GeoPoint(double longitude, double latitude) : longitude_(longitude), latitude_(latitude) {};
  GeoPoint() = default;
  
  bool operator==(const GeoPoint& other) {
    return (longitude_ == other.longitude_ && latitude_ == other.latitude_);
  }
};

template<typename... Ts>
struct Overload : Ts... {
  using Ts::operator()...;
};


using StringType = std::string;
using ListType = std::list<std::string>;
using SetType = std::unordered_set<std::string>;
using GeoType = std::unordered_map<std::string, GeoPoint>;
using Value = std::variant<StringType, ListType, SetType, GeoType>;
using TimePoint = std::chrono::steady_clock::time_point;

template <typename Type>
Type* GetAs(Value& val) {
  return std::holds_alternative<Type>(val) ? &std::get<Type>(val) : nullptr;
}

class DataBase {
private:
  std::unordered_map<std::string, Value> data_;
  std::unordered_map<std::string, TimePoint> ttl_;

  size_t maxmemory_ = 0;

public:
  Value& Get(const std::string& key);

  template <typename Type>
  void Set(const std::string& key, Type&& value) {
    using CleanType = std::decay_t<Type>;

    if constexpr (
      std::is_same_v<CleanType, StringType> ||
      std::is_same_v<CleanType, ListType> ||
      std::is_same_v<CleanType, SetType> ||
      std::is_same_v<CleanType, GeoType>
    ) {
      data_[key] = std::forward<Type>(value);
      ttl_.erase(key);
    } else {
      throw std::runtime_error("incorrect type");
    }
  }


  void Del(const std::string& key);

  bool Exists(const std::string& key);

  std::string Type(const std::string& key) const;

  void SetTtl(const std::string& key, int seconds);

  bool IsExpired(const std::string& key) const;

  bool HasTtl(const std::string& key) const;

  int GetRemainingTtl(const std::string& key) const;

  std::vector<std::string> GetAllKeys() const;

  size_t MaxMemoryGetter() const;

  void SetMaxMemory(size_t bytes);

  size_t EstimateSize(const std::string& key) const;

  void CheckMemory(size_t additional) const;

  size_t CurrentMemory() const;
};