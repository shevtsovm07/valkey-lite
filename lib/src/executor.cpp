#include "executor.hpp"
#include "database.hpp"
#include "common_cmd.hpp"
#include "string_cmd.hpp"
#include "list_cmd.hpp"
#include "set_cmd.hpp"
#include "geo_cmd.hpp"
#include <iostream>
#include <vector>
#include <string>
#include <stdexcept>
#include <cstddef>

Executor::Executor() {
  RegisterCommands();
}

Executor::Executor(const std::string& maxmem) {
  RegisterCommands();
  data_base_.SetMaxMemory(static_cast<size_t>(UnitsToBytes(maxmem)));
}

void Executor::RegisterCommands() {
  using namespace commands;

  commands_[kSet] = {2, 2, [this](const auto& args) {
    return StringSet(args[1], args[2], data_base_); }};
  commands_[kGet] = {1, 1, [this](const auto& args) {
    return StringGet(args[1], data_base_); }};
  commands_[kStrlen] = {1, 1, [this](const auto& args) {
    return StringLen(args[1], data_base_); }};
  commands_[kAppend] = {2, 2, [this](const auto& args) {
    return StringAppend(args[1], args[2], data_base_); }};
  commands_[kExpire] = {2, 2, [this](const auto& args) {
    return StringExpire(args[1], std::stoi(args[2]), data_base_); }};
  commands_[kTtl] = {1, 1, [this](const auto& args) {
    return StringTtl(args[1], data_base_); }};

  commands_[kLpush] = {2, SIZE_MAX, [this](const auto& args) {
    return ListPush(true, args[1], std::vector<std::string>(args.begin()+2, args.end()), data_base_); }};
  commands_[kRpush] = {2, SIZE_MAX, [this](const auto& args) {
    return ListPush(false, args[1], std::vector<std::string>(args.begin()+2, args.end()), data_base_); }};
  commands_[kLpop] = {1, 2, [this](const auto& args) {
    bool explicit_count = args.size() > 2;
    size_t count = explicit_count ? std::stoul(args[2]) : 1;
    return ListPop(true, args[1], count, explicit_count, data_base_); }};
  commands_[kRpop] = {1, 2, [this](const auto& args) {
    bool explicit_count = args.size() > 2;
    size_t count = explicit_count ? std::stoul(args[2]) : 1;
    return ListPop(false, args[1], count, explicit_count, data_base_); }};
  commands_[kLlen] = {1, 1, [this](const auto& args) {
    return ListLen(args[1], data_base_); }};
  commands_[kLrange] = {3, 3, [this](const auto& args) {
    return ListRange(args[1], std::stoi(args[2]), std::stoi(args[3]), data_base_); }};
  commands_[kLindex] = {2, 2, [this](const auto& args) {
    return ListIndex(args[1], std::stoi(args[2]), data_base_); }};
  commands_[kLset] = {3, 3, [this](const auto& args) {
    return ListSetIndex(args[1], std::stoi(args[2]), args[3], data_base_); }};
  commands_[kLinsert] = {4, 4, [this](const auto& args) {
    bool before = args[2] == "BEFORE";
    return ListInsert(before, args[1], args[3], args[4], data_base_); }};

  commands_[kSadd] = {2, SIZE_MAX, [this](const auto& args) {
    return SetAdd(args[1], std::vector<std::string>(args.begin() + 2, args.end()), data_base_); }};
  commands_[kSrem] = {2, SIZE_MAX, [this](const auto& args) {
    return SetRemove(args[1], std::vector<std::string>(args.begin() + 2, args.end()), data_base_); }};
  commands_[kSismember] = {2, 2, [this](const auto& args) {
    return SetIsMember(args[1], args[2], data_base_); }};
  commands_[kSmembers] = {1, 1, [this](const auto& args) {
    return SetMembers(args[1], data_base_); }};
  commands_[kScard] = {1, 1, [this](const auto& args) {
    return SetCard(args[1], data_base_); }};
  commands_[kSunion] = {1, SIZE_MAX, [this](const auto& args) {
    return SetUnion(std::vector<std::string>(args.begin() + 1, args.end()), data_base_); }};
  commands_[kSinter] = {1, SIZE_MAX, [this](const auto& args) {
    return SetInter(std::vector<std::string>(args.begin() + 1, args.end()), data_base_); }};
  commands_[kSdiff] = {1, SIZE_MAX, [this](const auto& args) {
    return SetDifference(std::vector<std::string>(args.begin() + 1, args.end()), data_base_); }};
  commands_[kSmove] = {3, 3, [this](const auto& args) {
    return SetMove(args[1], args[2], args[3], data_base_); }};

  commands_[kGeoadd] = {4, SIZE_MAX, [this](const auto& args) {
    std::vector<GeoEntry> entries;
    for (size_t i = 2; i + 2 < args.size(); i += 3) {
      entries.push_back(GeoEntry{std::stod(args[i]), std::stod(args[i+1]), args[i+2]});
    }
    return GeoAdd(args[1], entries, data_base_); }};
  commands_[kGeopos] = {2, SIZE_MAX, [this](const auto& args) {
    return GeoPos(args[1], std::vector<std::string>(args.begin() + 2, args.end()), data_base_); }};
  commands_[kGeodist] = {4, 5, [this](const auto& args) {
    std::string unit = args.size() > 4 ? args[4] : "m";
    return GeoDist(args[1], args[2], args[3], unit, data_base_); }};
  commands_[kGeosearch] = {7, 10, [this](const auto& args) {
    bool asc = true;
    int args_count = 0;
    for (size_t index = 8; index < args.size(); index++) {
      if (args[index] == "DESC") asc = false;
      if (args[index] == "COUNT" && index + 1 < args.size()) args_count = std::stoi(args[index + 1]);
    }
    return GeoSearch(args[1], std::stod(args[3]), std::stod(args[4]), std::stod(args[6]), args[7], asc, args_count, data_base_); }};
  commands_[kGeosearchstore] = {8, 11, [this](const auto& args) {
    bool asc = true;
    int args_count = 0;
    for (size_t index = 9; index < args.size(); index++) {
      if (args[index] == "DESC") asc = false;
      if (args[index] == "COUNT" && index + 1 < args.size()) args_count = std::stoi(args[index + 1]);
    }
    return GeoSearchStore(args[1], args[2], std::stod(args[4]), std::stod(args[5]), std::stod(args[7]), args[8], asc, args_count, data_base_); }};

  commands_[kType] = {1, 1, [this](const auto& args) {
    return CommonType(args[1], data_base_); }};
  commands_[kDel] = {1, SIZE_MAX, [this](const auto& args) {
    return CommonDel(std::vector<std::string>(args.begin() + 1, args.end()), data_base_); }};
  commands_[kExists] = {1, SIZE_MAX, [this](const auto& args) {
    return CommonExists(std::vector<std::string>(args.begin() + 1, args.end()), data_base_); }};
  commands_[kKeys] = {1, 1, [this](const auto& args) {
    return CommonKeys(args[1], data_base_); }};
  commands_[kFlushdb] = {0, 0, [this](const auto& args) {
    return CommonFlushDb(data_base_); }};
  commands_[kDbsize] = {0, 0, [this](const auto& args) {
    return CommonDbSize(data_base_); }};
  commands_[kConfigSet] = {3, 3, [this](const auto& args) {
    return CommonConfigSet(args[3], data_base_); }};
  commands_[kConfigGet] = {2, 2, [this](const auto& args) {
    return CommonConfigGet(data_base_); }};
  commands_[kMemoryUsage] = {1, 1, [this](const auto& args) {
    return CommonMemoryUsage(args[2], data_base_); }};
}

std::string Executor::Execute(const std::vector<std::string>& tokens) {
  if (tokens.empty()) return "";
  std::string cmd = tokens[0];
  for (auto& ch: cmd) ch = std::toupper(static_cast<unsigned char>(ch)); 

  if ((cmd == "CONFIG" || cmd == "MEMORY") && tokens.size() > 1) {
    std::string sub = tokens[1];
    for (auto& ch: sub) ch = std::toupper(static_cast<unsigned char>(ch));
    cmd = cmd + "_" + sub;
  }

  auto iterator = commands_.find(cmd);
  if (iterator == commands_.end()) {
    std::cerr << "(error) unknown command " << tokens[0] << "\n";
    return "";
  }

  size_t arguments_count = tokens.size() - 1;
  if (arguments_count < iterator->second.min_args || arguments_count > iterator->second.max_args) {
    std::cerr << "(error) wrong number of arguments for '" << tokens[0] << "'\n";
    return "";
  }

  try {
    return iterator->second.handler(tokens);
  } catch (const std::runtime_error& e) {
    return std::string("(error) ") + e.what();
  } catch (const std::invalid_argument&) {
    return "(error) ERR value is not an integer or out of range";
  } catch (const std::out_of_range&) {
    return "(error) ERR value is not an integer or out of range";
  }
}