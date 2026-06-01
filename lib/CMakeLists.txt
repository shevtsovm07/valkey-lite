add_library(valkey_lib
  src/database.cpp
  src/string_cmd.cpp
  src/list_cmd.cpp
  src/set_cmd.cpp
  src/geo_cmd.cpp
  src/common_cmd.cpp
  src/executor.cpp
  src/server.hpp
)

target_include_directories(valkey_lib PUBLIC src)
