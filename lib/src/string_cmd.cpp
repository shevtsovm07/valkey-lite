#include "string_cmd.hpp"
#include "database.hpp"
#include <string>


std::string StringSet(const std::string& key, const std::string& value, DataBase& data_base) {
  size_t needed_mem = 0;
  if (!data_base.Exists(key)) {
    needed_mem += key.size() + kKeyOverhead;
  }
  needed_mem += value.size() + kNodeOverhead;
  data_base.CheckMemory(needed_mem);

  if (data_base.Exists(key) && data_base.Type(key) != "string") {
    return kWrongTypeErr;
  }
  data_base.Set(key, value);
  return "OK";
}

std::string StringGet(const std::string& key, DataBase& data_base) {
  if (!data_base.Exists(key)) return kEmptyResult;
  Value& val = data_base.Get(key);
  auto* string = GetAs<StringType>(val);
  if (!string) return kWrongTypeErr;
  return *string;
}

std::string StringLen(const std::string& key, DataBase& data_base) {
  if (!data_base.Exists(key)) {
    return "(integer) 0";
  }
  Value& val = data_base.Get(key);
  auto* string = GetAs<StringType>(val);
  if (!string) return kWrongTypeErr;
  return "(integer) " + std::to_string(string->size());
}

std::string StringAppend(const std::string& key, const std::string& suffix, DataBase& data_base) {
  size_t needed_mem = 0;
  if (!data_base.Exists(key)) {
    needed_mem += key.size() + kKeyOverhead;
  }
  needed_mem += suffix.size() + kNodeOverhead;
  data_base.CheckMemory(needed_mem);

  if (!data_base.Exists(key)) {
    data_base.Set(key, suffix);
    return "(integer) " + std::to_string(suffix.size());
  }
  Value& val = data_base.Get(key);
  auto* string = GetAs<StringType>(val);
  if (!string) return kWrongTypeErr;
  std::string& string_new_value = *string;
  string_new_value += suffix;
  return "(integer) " + std::to_string(string_new_value.size());
}

std::string StringExpire(const std::string& key, int seconds, DataBase& data_base) {
  if (!data_base.Exists(key)) {
    return "(integer) 0";
  }
  data_base.SetTtl(key, seconds);
  return "(integer) 1";
}

std::string StringTtl(const std::string& key, DataBase& data_base) {
  if (!data_base.Exists(key)) {
    return "(integer) -2";
  }

  if (!data_base.HasTtl(key)) {
    return "(integer) -1";
  }

  return "(integer) " + std::to_string(data_base.GetRemainingTtl(key));
}