#include "list_cmd.hpp"
#include <string>
#include <vector>
#include "database.hpp"
#include <variant>
#include <list>
#include <algorithm>
#include <cstddef>
#include <iterator>
#include <utility>

std::string ListPush(bool begin, const std::string& key, const std::vector<std::string>& elements, DataBase& data_base) {
  size_t needed_mem = 0;
  if (!data_base.Exists(key)) {
    needed_mem += key.size() + kKeyOverhead;
  }
  for (auto& el : elements) {
    needed_mem += el.size() + kNodeOverhead;
  }
  data_base.CheckMemory(needed_mem);

  if (!data_base.Exists(key)) {
    data_base.Set(key, ListType{});
  }

  Value& val = data_base.Get(key);
  auto* list = GetAs<ListType>(val);
  if (!list) return kWrongTypeErr;


  for (auto&& el : elements) {
    if (begin) {
      list->emplace_front(el);
    } else {
      list->emplace_back(el);
    }
  }
  
  return "(integer) " + std::to_string(list->size());
}

std::string ListPop(bool begin, const std::string& key, size_t count, bool explicit_count, DataBase& data_base) {
  if (!data_base.Exists(key)) {
    return "(nil)";
  }

  Value& val = data_base.Get(key);
  auto* list = GetAs<ListType>(val);
  if (!list) return kWrongTypeErr;

  std::vector<std::string> popped;
  for (size_t i = 0; i < count && !list->empty(); i++) {
    if (begin) {
      popped.push_back(list->front());
      list->pop_front();
    } else {
      popped.push_back(list->back());
      list->pop_back();
    }
  }

  if (list->empty()) {
    data_base.Del(key);
  }

  if (popped.size() == 1 && !explicit_count) {
    return popped[0];
  }

  std::string result;
  int i = 0;
  for (auto&& el : popped) {
    result += std::to_string(++i) + ") \"" + el + "\"\n";
  }

  return result;
}

std::string ListLen(const std::string& key, DataBase& data_base) {
  if (!data_base.Exists(key)) {
    return "(integer) 0";
  }

  Value& val = data_base.Get(key);
  auto* list = GetAs<ListType>(val);
  if (!list) return kWrongTypeErr;

  return "(integer) " + std::to_string(list->size());
}

std::string ListRange(const std::string& key, int start, int end, DataBase& data_base) {
  if (!data_base.Exists(key)) {
    return "";
  }

  Value& val = data_base.Get(key);
  auto* list = GetAs<ListType>(val);
  if (!list) return kWrongTypeErr;

  int size = list->size();
  int pure_index_start = start;
  if (start < 0) pure_index_start = size + start;
  if (pure_index_start < 0 || pure_index_start >= size) return "";

  int pure_index_end = end;
  if (end < 0) pure_index_end = size + end;
  if (pure_index_end < 0 || pure_index_end >= size) pure_index_end = size-1;

  if (pure_index_start > pure_index_end) return "";

  auto iterator = list->begin();
  std::advance(iterator, pure_index_start);
  auto iterator_end = list->begin();
  std::advance(iterator_end, pure_index_end);

  std::string result;
  int i = 0;
  while (iterator != std::next(iterator_end)) {
    result += std::to_string(++i) + ") \"" + *iterator + "\"\n";
    iterator++;
  }
  return result;
}

std::string ListIndex(const std::string& key, int index, DataBase& data_base) {
  if (!data_base.Exists(key)) {
    return "(nil)";
  }

  Value& val = data_base.Get(key);
  auto* list = GetAs<ListType>(val);
  if (!list) return kWrongTypeErr;

  auto iterator = list->begin();
  int size = list->size();
  int pure_index = index;
  if (index < 0) pure_index = size + index;
  if (pure_index < 0 || pure_index >= size) return "(nil)";
  for (int ind = 0; ind < pure_index && iterator != list->end(); ++ind) {
    ++iterator;
  }
  return *iterator;
}

std::string ListSetIndex(const std::string& key, int index, const std::string& value, DataBase& data_base) {
  if (!data_base.Exists(key)) {
    return "(error) no such key";
  }

  Value& val = data_base.Get(key);
  auto* list = GetAs<ListType>(val);
  if (!list) return kWrongTypeErr;

  int size = list->size();
  int pure_index = (index < 0) ? size + index : index;
  if (pure_index < 0 || pure_index >= size) {
    return "(error) ERR index out of range";
  }
  auto iterator = list->begin();
  std::advance(iterator, pure_index);
  *iterator = value;
  return "OK";
}

std::string ListInsert(bool before, const std::string& key, const std::string& pivot, const std::string& value, DataBase& data_base) {
  if (!data_base.Exists(key)) {
    return "(integer) 0";
  }

  Value& val = data_base.Get(key);
  auto* list = GetAs<ListType>(val);
  if (!list) return kWrongTypeErr;

  auto iterator = std::find(list->begin(), list->end(), pivot);
  if (iterator == list->end()) return "(integer) -1";

  if (before) {
    data_base.CheckMemory(value.size() + kNodeOverhead);
    list->emplace(iterator, value);
  } else {
    data_base.CheckMemory(value.size() + kNodeOverhead);
    list->emplace(std::next(iterator), value);
  }
  return "(integer) " + std::to_string(list->size());
}
