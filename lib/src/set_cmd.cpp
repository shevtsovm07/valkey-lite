#include "set_cmd.hpp"
#include "database.hpp"
#include <vector>
#include <string>
#include <algorithm>
#include <variant>
#include <cstddef>

void SortSets(std::vector<SetType>& sets) {
  std::sort(sets.begin(), sets.end(),
    [](const SetType& a, const SetType& b) {
      return a.size() < b.size();
    });
}

std::string SetAdd(const std::string& key, const std::vector<std::string>& elements, DataBase& data_base) {
  size_t needed_mem = 0;
  if (!data_base.Exists(key)) {
    needed_mem += key.size() + kKeyOverhead;
  }
  for (auto& el : elements) {
    needed_mem += el.size() + kNodeOverhead;
  }
  data_base.CheckMemory(needed_mem);
  
  if (!data_base.Exists(key)) {
    data_base.Set(key, SetType{});
  }

  Value& val = data_base.Get(key);
  auto* set = GetAs<SetType>(val);
  if (!set) return kWrongTypeErr;

  int added = 0;
  for (auto& el : elements) {
    if(set->emplace(el).second) added++;
  }

  return "(integer) " + std::to_string(added);
}

std::string SetRemove(const std::string& key, const std::vector<std::string>& elements, DataBase& data_base) {
  if (!data_base.Exists(key)) {
    return "(integer) 0";
  }

  Value& val = data_base.Get(key);
  auto* set = GetAs<SetType>(val);
  if (!set) return kWrongTypeErr;

  int removed = 0;
  for (auto& el : elements) {
    removed += set->erase(el);
  }

  if (set->size() == 0) {
    data_base.Del(key);
  }

  return "(integer) " + std::to_string(removed);
}

std::string SetIsMember(const std::string& key, const std::string& element, DataBase& data_base) {
  if (!data_base.Exists(key)) {
    return "(integer) 0";
  }

  Value& val = data_base.Get(key);
  auto* set = GetAs<SetType>(val);
  if (!set) return kWrongTypeErr;

  if (set->contains(element)) {
    return "(integer) 1";
  } else {
    return "(integer) 0";
  }
}

std::string SetMembers(const std::string& key, DataBase& data_base) {
  if (!data_base.Exists(key)) {
    return "";
  }

  Value& val = data_base.Get(key);
  auto* set = GetAs<SetType>(val);
  if (!set) return kWrongTypeErr;

  auto iterator = set->begin();
  std::string result;
  int i = 0;
  while (iterator != set->end()) {
    result += std::to_string(++i) + ") \"" + *iterator + "\"\n";
    iterator++;
  }
  return result;
}

std::string SetCard(const std::string& key, DataBase& data_base) {
  if (!data_base.Exists(key)) {
    return "(integer) 0";
  }

  Value& val = data_base.Get(key);
  auto* set = GetAs<SetType>(val);
  if (!set) return kWrongTypeErr;

  return "(integer) " + std::to_string(set->size());
}

std::string SetUnion(const std::vector<std::string>& keys, DataBase& data_base) {
  SetType result;
  for (const auto& key : keys) {
    if (!data_base.Exists(key)) continue;

    Value& val = data_base.Get(key);
    auto* set = GetAs<SetType>(val);
    if (!set) return kWrongTypeErr;

    result.insert(set->begin(), set->end());
  }

  auto result_iterator = result.begin();
  std::string result_string;
  int i = 0;
  while (result_iterator != result.end()) {
    result_string += std::to_string(++i) + ") \"" + *result_iterator + "\"\n";
    result_iterator++;
  }

  return result_string;
}

std::string SetInter(const std::vector<std::string>& keys, DataBase& data_base) {
  std::vector<SetType> sets;
  for (auto key : keys) {
    if (!data_base.Exists(key)) {
      return "";
    }

    Value& val = data_base.Get(key);
    auto* set = GetAs<SetType>(val);
    if (!set) return kWrongTypeErr;

    sets.push_back(*set);
  }
  SortSets(sets);

  SetType& smallest_set = *(sets.begin());
  SetType result;
  for (auto& el : smallest_set) {
    bool belongs_to_every_set = true;
    for (auto& set : sets) {
      if (&set == &smallest_set) continue;
      if (!set.contains(el)) {
        belongs_to_every_set = false;
        break;
      }
    }
    if (belongs_to_every_set) {
      result.emplace(el);
    }
  }

  auto result_iterator = result.begin();
  std::string result_string;
  int i = 0;
  while (result_iterator != result.end()) {
    result_string += std::to_string(++i) + ") \"" + *result_iterator + "\"\n";
    result_iterator++;
  }

  return result_string;
}

std::string SetDifference(const std::vector<std::string>& keys, DataBase& data_base) {
  if (keys.empty() || !data_base.Exists(keys[0])) return "";

  Value& first_val = data_base.Get(keys[0]);
  auto* first_set = GetAs<SetType>(first_val);
  if (!first_set) return kWrongTypeErr;
  SetType result = *first_set;

  for (size_t index = 1; index < keys.size(); ++index) {
    if (!data_base.Exists(keys[index])) continue;

    Value& val = data_base.Get(keys[index]);
    auto* set = GetAs<SetType>(val);
    if (!set) return kWrongTypeErr;

    for (const auto& el : *set) {
      result.erase(el);
    }
    if (result.empty()) break;
  }

  auto result_iterator = result.begin();
  std::string result_string;
  int i = 0;
  while (result_iterator != result.end()) {
    result_string += std::to_string(++i) + ") \"" + *result_iterator + "\"\n";
    result_iterator++;
  }

  return result_string;
}

std::string SetMove(const std::string& key_source, const std::string& key_destination, const std::string& member, DataBase& data_base) {
  if (!data_base.Exists(key_source)) {
    return "(integer) 0";
  }

  if (!data_base.Exists(key_destination)) {
    data_base.Set(key_destination, SetType{});
  }

  Value& source = data_base.Get(key_source);
  Value& destination = data_base.Get(key_destination);

  if (!std::holds_alternative<SetType>(source) || !std::holds_alternative<SetType>(destination)) {
    return kWrongTypeErr;
  }

  SetType& source_set = std::get<SetType>(source);
  SetType& destination_set = std::get<SetType>(destination);

  if (key_source == key_destination) {
    return source_set.contains(member) ? "(integer) 1" : "(integer) 0";
  }

  if (source_set.erase(member)) {
    destination_set.insert(member);
    if (source_set.empty()) {
      data_base.Del(key_source);
    }
    return "(integer) 1";
  }
  return "(integer) 0";
}