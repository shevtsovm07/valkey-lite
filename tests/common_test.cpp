#include "gtest/gtest.h"
#include "common_cmd.hpp"
#include "string_cmd.hpp"
#include "list_cmd.hpp"
#include "set_cmd.hpp"
#include "geo_cmd.hpp"
#include "executor.hpp"
#include <string>
#include "database.hpp"


static int ParseInteger(const std::string& s) {
  return std::stoi(s.substr(std::string("(integer) ").size()));
}

class CommonCmdTest : public ::testing::Test {
public:
  DataBase db;
  Executor ex;
};

TEST_F(CommonCmdTest, del_existing_key_returns_one) {
  StringSet("k", "v", db);
  EXPECT_EQ(ParseInteger(CommonDel({"k"}, db)), 1);
  EXPECT_FALSE(db.Exists("k"));
}

TEST_F(CommonCmdTest, del_multiple_keys_returns_count) {
  StringSet("a", "1", db);
  StringSet("b", "2", db);
  StringSet("c", "3", db);
  EXPECT_EQ(ParseInteger(CommonDel({"a", "b", "c"}, db)), 3);
}

TEST_F(CommonCmdTest, del_non_existent_key_returns_zero) {
  EXPECT_EQ(ParseInteger(CommonDel({"missing"}, db)), 0);
}

TEST_F(CommonCmdTest, del_mixed_existing_and_missing_returns_correct_count) {
  StringSet("a", "1", db);
  EXPECT_EQ(ParseInteger(CommonDel({"a", "missing"}, db)), 1);
}

TEST_F(CommonCmdTest, exists_existing_key_returns_one) {
  StringSet("k", "v", db);
  EXPECT_EQ(ParseInteger(CommonExists({"k"}, db)), 1);
}

TEST_F(CommonCmdTest, exists_non_existent_key_returns_zero) {
  EXPECT_EQ(ParseInteger(CommonExists({"missing"}, db)), 0);
}

TEST_F(CommonCmdTest, exists_after_del_returns_zero) {
  StringSet("k", "v", db);
  CommonDel({"k"}, db);
  EXPECT_EQ(ParseInteger(CommonExists({"k"}, db)), 0);
}

TEST_F(CommonCmdTest, type_string_returns_string) {
  StringSet("k", "v", db);
  EXPECT_EQ(CommonType("k", db), "string");
}

TEST_F(CommonCmdTest, type_list_returns_list) {
  ListPush(false, "k", {"a"}, db);
  EXPECT_EQ(CommonType("k", db), "list");
}

TEST_F(CommonCmdTest, type_set_returns_set) {
  SetAdd("k", {"a"}, db);
  EXPECT_EQ(CommonType("k", db), "set");
}

TEST_F(CommonCmdTest, type_geo_returns_zset) {
  GeoAdd("k", {GeoEntry{0.0, 0.0, "p"}}, db);
  EXPECT_EQ(CommonType("k", db), "zset");
}

TEST_F(CommonCmdTest, type_non_existent_key_returns_none) {
  EXPECT_EQ(CommonType("none", db), "none");
}

TEST_F(CommonCmdTest, dbsize_empty_db_returns_zero) {
  EXPECT_EQ(ParseInteger(CommonDbSize(db)), 0);
}

TEST_F(CommonCmdTest, dbsize_counts_all_key_types) {
  StringSet("s", "v", db);
  ListPush(false, "l", {"a"}, db);
  SetAdd("st", {"x"}, db);
  EXPECT_EQ(ParseInteger(CommonDbSize(db)), 3);
}

TEST_F(CommonCmdTest, keys_star_pattern_returns_all) {
  StringSet("foo", "1", db);
  StringSet("bar", "2", db);
  StringSet("baz", "3", db);
  std::string out = CommonKeys("*", db);
  EXPECT_NE(out.find("foo"), std::string::npos);
  EXPECT_NE(out.find("bar"), std::string::npos);
  EXPECT_NE(out.find("baz"), std::string::npos);
}

TEST_F(CommonCmdTest, keys_prefix_pattern_returns_matching) {
  StringSet("foo123", "1", db);
  StringSet("foo321", "2", db);
  StringSet("bar1", "3", db);
  std::string out = CommonKeys("foo*", db);
  EXPECT_NE(out.find("foo123"), std::string::npos);
  EXPECT_NE(out.find("foo321"), std::string::npos);
  EXPECT_EQ(out.find("bar1"), std::string::npos);
}

TEST_F(CommonCmdTest, keys_question_mark_matches_single_char) {
  StringSet("key1", "1", db);
  StringSet("key2", "2", db);
  StringSet("key12", "3", db);
  std::string out = CommonKeys("key?", db);
  EXPECT_NE(out.find("key1"), std::string::npos);
  EXPECT_NE(out.find("key2"), std::string::npos);
  EXPECT_EQ(out.find("key12"), std::string::npos);
}

TEST_F(CommonCmdTest, keys_no_match_returns_empty) {
  StringSet("foo", "1", db);
  EXPECT_EQ(CommonKeys("xyz*", db), "");
}

TEST_F(CommonCmdTest, keys_empty_db_returns_empty) {
  EXPECT_EQ(CommonKeys("*", db), "");
}

TEST_F(CommonCmdTest, flushdb_returns_ok) {
  StringSet("k", "v", db);
  EXPECT_EQ(CommonFlushDb(db), "OK");
}

TEST_F(CommonCmdTest, flushdb_removes_all_keys) {
  StringSet("a", "1", db);
  StringSet("b", "2", db);
  ListPush(false, "l", {"x"}, db);
  CommonFlushDb(db);
  EXPECT_EQ(ParseInteger(CommonDbSize(db)), 0);
}

TEST_F(CommonCmdTest, config_set_returns_ok) {
  EXPECT_EQ(CommonConfigSet("64mb", db), "OK");
}

TEST_F(CommonCmdTest, config_set_mb_sets_correct_bytes) {
  CommonConfigSet("64mb", db);
  EXPECT_EQ(db.MaxMemoryGetter(), 64 * 1024 * 1024);
}

TEST_F(CommonCmdTest, config_set_kb_sets_correct_bytes) {
  CommonConfigSet("512kb", db);
  EXPECT_EQ(db.MaxMemoryGetter(), 512 * 1024);
}

TEST_F(CommonCmdTest, config_set_gb_sets_correct_bytes) {
  CommonConfigSet("1gb", db);
  EXPECT_EQ(db.MaxMemoryGetter(), 1 * 1024 * 1024 * 1024);
}

TEST_F(CommonCmdTest, config_get_returns_current_maxmemory) {
  CommonConfigSet("32mb", db);
  std::string out = CommonConfigGet(db);
  EXPECT_NE(out.find("maxmemory"), std::string::npos);
  EXPECT_NE(out.find(std::to_string(32 * 1024 * 1024)), std::string::npos);
}

TEST_F(CommonCmdTest, memory_usage_existing_key_returns_positive) {
  StringSet("k", "hello", db);
  EXPECT_GT(ParseInteger(CommonMemoryUsage("k", db)), 0);
}

TEST_F(CommonCmdTest, memory_usage_non_existent_key_returns_nil) {
  EXPECT_EQ(CommonMemoryUsage("missing", db), "(nil)");
}

TEST_F(CommonCmdTest, executor_del_and_exists) {
  ex.Execute({"SET", "k", "v"});
  ex.Execute({"DEL", "k"});
  EXPECT_EQ(ex.Execute({"EXISTS", "k"}), "(integer) 0");
}

TEST_F(CommonCmdTest, executor_type_and_dbsize) {
  ex.Execute({"SET", "s", "v"});
  ex.Execute({"LPUSH", "l", "a"});
  EXPECT_EQ(ex.Execute({"TYPE", "s"}), "string");
  EXPECT_EQ(ex.Execute({"TYPE", "l"}), "list");
  EXPECT_EQ(ex.Execute({"DBSIZE"}), "(integer) 2");
}

TEST_F(CommonCmdTest, executor_flushdb_clears_everything) {
  ex.Execute({"SET", "a", "1"});
  ex.Execute({"SET", "b", "2"});
  ex.Execute({"FLUSHDB"});
  EXPECT_EQ(ex.Execute({"DBSIZE"}), "(integer) 0");
}

TEST_F(CommonCmdTest, executor_config_set_and_get) {
  ex.Execute({"CONFIG", "SET", "maxmemory", "64mb"});
  std::string out = ex.Execute({"CONFIG", "GET", "maxmemory"});
  EXPECT_NE(out.find("maxmemory"), std::string::npos);
  EXPECT_NE(out.find(std::to_string(64 * 1024 * 1024)), std::string::npos);
}

TEST_F(CommonCmdTest, executor_keys_pattern) {
  ex.Execute({"SET", "foo1", "1"});
  ex.Execute({"SET", "foo2", "2"});
  ex.Execute({"SET", "bar",  "3"});
  std::string out = ex.Execute({"KEYS", "foo*"});
  EXPECT_NE(out.find("foo1"), std::string::npos);
  EXPECT_NE(out.find("foo2"), std::string::npos);
  EXPECT_EQ(out.find("bar"),  std::string::npos);
}