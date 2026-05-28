#include "gtest/gtest.h"
#include "string_cmd.hpp"
#include "list_cmd.hpp"
#include "executor.hpp"
#include "database.hpp"
#include <string>
#include <stdexcept>
#include <thread>
#include <chrono>

class StringCmdTest : public ::testing::Test {
public:
  DataBase db_;
  Executor ex_;
};

TEST_F(StringCmdTest, string_set_returns_ok_get_value) {
  EXPECT_EQ(StringSet("key", "hello", db_), "OK");
  EXPECT_EQ(StringGet("key", db_), "hello");
}

TEST_F(StringCmdTest, string_set_overwrites_existing_value) {
  StringSet("key", "first", db_);
  StringSet("key", "second", db_);
  EXPECT_EQ(StringGet("key", db_), "second");
}

TEST_F(StringCmdTest, returns_nil_if_doesnt_exist) {
  EXPECT_EQ(StringGet("missing", db_), "(nil)");
}

TEST_F(StringCmdTest, get_wrong_type_key_returns_nil) {
  ListPush(true, "listkey", {"x"}, db_);
  EXPECT_EQ(StringGet("listkey", db_),
  "(error) WRONGTYPE operation against a key holding the wrong kind of value");
}

TEST_F(StringCmdTest, stlren_correct_length) {
  StringSet("key", "hello", db_);
  EXPECT_EQ(StringLen("key", db_), "(integer) 5");
}

TEST_F(StringCmdTest, stlren_non_existing_key_length_equals_zero) {
  EXPECT_EQ(StringLen("missing", db_), "(integer) 0");
}

TEST_F(StringCmdTest, concatenates_correctly) {
  StringSet("k", "Hello", db_);
  EXPECT_EQ(StringAppend("k", " World", db_), "(integer) 11");
  EXPECT_EQ(StringGet("k", db_), "Hello World");
}

TEST_F(StringCmdTest, append_not_existing_key_creates_new) {
  EXPECT_EQ(StringAppend("k", "val", db_), "(integer) 3");
  EXPECT_EQ(StringGet("k", db_), "val");
}

TEST_F(StringCmdTest, append_wrong_type_error) {
  ListPush(true, "listkey", {"x"}, db_);
  EXPECT_EQ(StringAppend("listkey", "suffix", db_),
  "(error) WRONGTYPE operation against a key holding the wrong kind of value");
}

TEST_F(StringCmdTest, expire_existing_key_returns_one_and_ttl_is_positive) {
  StringSet("k", "v", db_);
  EXPECT_EQ(StringExpire("k", 60, db_), "(integer) 1");
  std::string ttl = StringTtl("k", db_);
  int ttl_val = std::stoi(ttl.substr(std::string("(integer) ").size()));
  EXPECT_GT(ttl_val, 0);
  EXPECT_LE(ttl_val, 60);
}

TEST_F(StringCmdTest, expire_of_non_existing_key_returns_zero) {
  EXPECT_EQ(StringExpire("missing", 10, db_), "(integer) 0");
}

TEST_F(StringCmdTest, key_without_ttl_returns_minus_one) {
  StringSet("k", "v", db_);
  EXPECT_EQ(StringTtl("k", db_), "(integer) -1");
}

TEST_F(StringCmdTest, non_existing_key_ttl_returns_minus_two) {
  EXPECT_EQ(StringTtl("missing", db_), "(integer) -2");
}

TEST_F(StringCmdTest, key_dissapears_after_ttl_expired) {
  StringSet("k", "v", db_);
  StringExpire("k", 1, db_);
  std::this_thread::sleep_for(std::chrono::seconds(2));
  EXPECT_EQ(StringGet("k", db_), "(nil)");
}

TEST_F(StringCmdTest, set_resers_ttl_when_key_overwritten) {
  StringSet("k", "v", db_);
  StringExpire("k", 100, db_);
  StringSet("k", "new", db_);
  EXPECT_EQ(StringTtl("k", db_), "(integer) -1");
}

TEST_F(StringCmdTest, max_mem_limit_exceeded) {
  db_.SetMaxMemory(1);
  EXPECT_THROW(StringSet("key", "value", db_), std::runtime_error);
}

class StringExecutorTest : public ::testing::Test {
protected:
  Executor ex_;
};

TEST_F(StringExecutorTest, executor_set_and_get) {
  ex_.Execute({"SET", "key", "value"});
  EXPECT_EQ(ex_.Execute({"GET", "key"}), "value");
}

TEST_F(StringExecutorTest, executor_set_and_get_lower_case) {
  ex_.Execute({"set", "key", "value"});
  EXPECT_EQ(ex_.Execute({"get", "key"}), "value");
}

TEST_F(StringExecutorTest, executor_set_and_get_different_case_keys) {
  ex_.Execute({"set", "Key", "value1"});
  ex_.Execute({"set", "key", "value2"});
  EXPECT_EQ(ex_.Execute({"get", "Key"}), "value1");
  EXPECT_EQ(ex_.Execute({"get", "key"}), "value2");
}

TEST_F(StringExecutorTest, executor_strlen) {
  ex_.Execute({"SET", "k", "hello"});
  EXPECT_EQ(ex_.Execute({"STRLEN", "k"}), "(integer) 5");
}

TEST_F(StringExecutorTest, executor_append) {
  ex_.Execute({"SET", "k", "foo"});
  EXPECT_EQ(ex_.Execute({"APPEND", "k", "bar"}), "(integer) 6");
  EXPECT_EQ(ex_.Execute({"GET", "k"}), "foobar");
}

TEST_F(StringExecutorTest, executor_expire_and_ttl) {
  ex_.Execute({"SET", "k", "v"});
  ex_.Execute({"EXPIRE", "k", "100"});
  std::string ttl = ex_.Execute({"TTL", "k"});
  int ttl_val = std::stoi(ttl.substr(std::string("(integer) ").size()));
  EXPECT_GT(ttl_val, 0);
}

TEST_F(StringExecutorTest, executer_wrong_amount_of_arguments) {
  EXPECT_EQ(ex_.Execute({"SET"}), "");
}
