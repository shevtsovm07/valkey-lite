#pragma once
#include "database.hpp"
#include <functional>

namespace commands {
  inline constexpr const char* kSet = "SET";
  inline constexpr const char* kGet = "GET";
  inline constexpr const char* kStrlen = "STRLEN";
  inline constexpr const char* kAppend = "APPEND";
  inline constexpr const char* kExpire = "EXPIRE";
  inline constexpr const char* kTtl = "TTL";

  inline constexpr const char* kLpush = "LPUSH";
  inline constexpr const char* kRpush = "RPUSH";
  inline constexpr const char* kLpop = "LPOP";
  inline constexpr const char* kRpop = "RPOP";
  inline constexpr const char* kLlen = "LLEN";
  inline constexpr const char* kLrange = "LRANGE";
  inline constexpr const char* kLindex = "LINDEX";
  inline constexpr const char* kLset = "LSET";
  inline constexpr const char* kLinsert = "LINSERT";

  inline constexpr const char* kSadd = "SADD";
  inline constexpr const char* kSrem = "SREM";
  inline constexpr const char* kSismember = "SISMEMBER";
  inline constexpr const char* kSmembers = "SMEMBERS";
  inline constexpr const char* kScard = "SCARD";
  inline constexpr const char* kSunion = "SUNION";
  inline constexpr const char* kSinter = "SINTER";
  inline constexpr const char* kSdiff = "SDIFF";
  inline constexpr const char* kSmove = "SMOVE";

  inline constexpr const char* kGeoadd = "GEOADD";
  inline constexpr const char* kGeopos = "GEOPOS";
  inline constexpr const char* kGeodist = "GEODIST";
  inline constexpr const char* kGeosearch = "GEOSEARCH";
  inline constexpr const char* kGeosearchstore = "GEOSEARCHSTORE";

  inline constexpr const char* kType = "TYPE";
  inline constexpr const char* kDel = "DEL";
  inline constexpr const char* kExists = "EXISTS";
  inline constexpr const char* kKeys = "KEYS";
  inline constexpr const char* kFlushdb = "FLUSHDB";
  inline constexpr const char* kDbsize = "DBSIZE";
  inline constexpr const char* kConfig = "CONFIG";
  inline constexpr const char* kMemory = "MEMORY";
  inline constexpr const char* kConfigGet = "CONFIG_GET";
  inline constexpr const char* kConfigSet = "CONFIG_SET";
  inline constexpr const char* kMemoryUsage = "MEMORY_USAGE";
} // namespace commands

struct Command {
  size_t min_args;
  size_t max_args;
  std::function<std::string(const std::vector<std::string>&)> handler;
};

class Executor {
  DataBase data_base_;
  std::unordered_map<std::string, Command> commands_;

  void RegisterCommands();

public:
  Executor();
  Executor(const std::string& maxmem);
  std::string Execute(const std::vector<std::string>& tokens);
};