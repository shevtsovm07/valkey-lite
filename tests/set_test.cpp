#include "gtest/gtest.h"
#include "set_cmd.hpp"
#include "string_cmd.hpp"
#include "executor.hpp"
#include "database.hpp"
#include <string>
#include <vector>
#include <cstddef>

static bool ContainsMember(const std::string& output, const std::string& member) {
  return output.find("\"" + member + "\"") != std::string::npos;
}

static int ParseInteger(const std::string& s) {
  return std::stoi(s.substr(std::string("(integer) ").size()));
}

class SetCmdTest : public ::testing::Test {
public:
  DataBase db_;
  Executor ex_;
};

TEST_F(SetCmdTest, sadd_new_key_returns_size) {
  EXPECT_EQ(SetAdd("s", {"a", "b", "c"}, db_), "(integer) 3");
}

TEST_F(SetCmdTest, sadd_duplicates_not_added) {
  SetAdd("s", {"a", "b"}, db_);
  EXPECT_EQ(SetAdd("s", {"b", "c"}, db_), "(integer) 1");
}

TEST_F(SetCmdTest, sadd_all_duplicates_return_zero) {
  SetAdd("s", {"x"}, db_);
  EXPECT_EQ(SetAdd("s", {"x"}, db_), "(integer) 0");
}

TEST_F(SetCmdTest, sadd_wrong_type_returns_error) {
  StringSet("k", "val", db_);
  EXPECT_EQ(SetAdd("k", {"a"}, db_),
    "(error) WRONGTYPE operation against a key holding the wrong kind of value");
}

TEST_F(SetCmdTest, set_remove_existing_returns_count_removed) {
  SetAdd("s", {"a", "b", "c"}, db_);
  EXPECT_EQ(SetRemove("s", {"a", "b"}, db_), "(integer) 2");
}

TEST_F(SetCmdTest, set_remove_non_existing_returns_zero) {
  SetAdd("s", {"a"}, db_);
  EXPECT_EQ(SetRemove("s", {"z"}, db_), "(integer) 0");
}

TEST_F(SetCmdTest, Srem_NonExistentKey_ReturnsZero) {
    EXPECT_EQ(SetRemove("missing", {"a"}, db_), "(integer) 0");
}

TEST_F(SetCmdTest, set_remove_last_member_not_exists) {
  SetAdd("s", {"only"}, db_);
  SetRemove("s", {"only"}, db_);
  EXPECT_EQ(SetCard("s", db_), "(integer) 0");
  EXPECT_EQ(db_.Type("s"), "none");
}

TEST_F(SetCmdTest, set_is_member_existing_returns_one) {
  SetAdd("s", {"hello"}, db_);
  EXPECT_EQ(SetIsMember("s", "hello", db_), "(integer) 1");
}

TEST_F(SetCmdTest, set_is_member_missing_returns_zero) {
  SetAdd("s", {"hello"}, db_);
  EXPECT_EQ(SetIsMember("s", "world", db_), "(integer) 0");
}

TEST_F(SetCmdTest, set_is_member_non_existing_key_returns_one) {
  EXPECT_EQ(SetIsMember("missing", "x", db_), "(integer) 0");
}

TEST_F(SetCmdTest, set_members_returns_all) {
  SetAdd("s", {"a", "b", "c"}, db_);
  std::string out = SetMembers("s", db_);
  EXPECT_TRUE(ContainsMember(out, "a"));
  EXPECT_TRUE(ContainsMember(out, "b"));
  EXPECT_TRUE(ContainsMember(out, "c"));
}

TEST_F(SetCmdTest, set_members_non_existing_key_returns_nothing) {
  EXPECT_EQ(SetMembers("missing", db_), "");
}

TEST_F(SetCmdTest, set_card_returns_correct_amount) {
  SetAdd("s", {"a", "b", "c"}, db_);
  EXPECT_EQ(SetCard("s", db_), "(integer) 3");
}

TEST_F(SetCmdTest, set_card_non_existing_key_returns_zero) {
  EXPECT_EQ(SetCard("missing", db_), "(integer) 0");
}

TEST_F(SetCmdTest, set_card_decreass_count_after_remove) {
  SetAdd("s", {"a", "b", "c"}, db_);
  SetRemove("s", {"b"}, db_);
  EXPECT_EQ(SetCard("s", db_), "(integer) 2");
}

TEST_F(SetCmdTest, set_union_returns_all_unique_elements) {
  SetAdd("s1", {"a", "b"}, db_);
  SetAdd("s2", {"b", "c"}, db_);
  std::string out = SetUnion({"s1", "s2"}, db_);
  EXPECT_TRUE(ContainsMember(out, "a"));
  EXPECT_TRUE(ContainsMember(out, "b"));
  EXPECT_TRUE(ContainsMember(out, "c"));
  size_t first = out.find("\"b\"");
  size_t last  = out.rfind("\"b\"");
  EXPECT_EQ(first, last);
}

TEST_F(SetCmdTest, set_union_with_empty_key_returns_existing) {
  SetAdd("s1", {"a"}, db_);
  std::string out = SetUnion({"s1", "missing"}, db_);
  EXPECT_TRUE(ContainsMember(out, "a"));
  EXPECT_EQ(ParseInteger(SetCard("s1", db_)), 1);
}

TEST_F(SetCmdTest, set_inter_works_correct) {
  SetAdd("s1", {"a", "b", "c"}, db_);
  SetAdd("s2", {"b", "c", "d"}, db_);
  std::string out = SetInter({"s1", "s2"}, db_);
  EXPECT_FALSE(ContainsMember(out, "a"));
  EXPECT_TRUE(ContainsMember(out, "b"));
  EXPECT_TRUE(ContainsMember(out, "c"));
  EXPECT_FALSE(ContainsMember(out, "d"));
}

TEST_F(SetCmdTest, set_inter_returns_empty) {
  SetAdd("s1", {"a"}, db_);
  SetAdd("s2", {"b"}, db_);
  EXPECT_EQ(SetInter({"s1", "s2"}, db_), "");
}

TEST_F(SetCmdTest, set_inter_with_no_existing_key_returns_empty) {
  SetAdd("s1", {"a", "b"}, db_);
  EXPECT_EQ(SetInter({"s1", "missing"}, db_), "");
}

TEST_F(SetCmdTest, set_diff_returns_elements_only_from_first_set) {
  SetAdd("s1", {"a", "b", "c"}, db_);
  SetAdd("s2", {"b", "c", "d"}, db_);
  std::string out = SetDifference({"s1", "s2"}, db_);
  EXPECT_TRUE(ContainsMember(out, "a"));
  EXPECT_FALSE(ContainsMember(out, "b"));
  EXPECT_FALSE(ContainsMember(out, "c"));
  EXPECT_FALSE(ContainsMember(out, "d"));
}

TEST_F(SetCmdTest, set_diff_no_inter_returns_first_set) {
  SetAdd("s1", {"a", "b"}, db_);
  SetAdd("s2", {"c", "d"}, db_);
  std::string out = SetDifference({"s1", "s2"}, db_);
  EXPECT_TRUE(ContainsMember(out, "a"));
  EXPECT_TRUE(ContainsMember(out, "b"));
}

TEST_F(SetCmdTest, set_diff_first_key_missing_returns_empty) {
  SetAdd("s2", {"a"}, db_);
  EXPECT_EQ(SetDifference({"missing", "s2"}, db_), "");
}

TEST_F(SetCmdTest, set_move_moves_and_returns_one) {
  SetAdd("src", {"a", "b"}, db_);
  SetAdd("dst", {"c"}, db_);
  EXPECT_EQ(SetMove("src", "dst", "a", db_), "(integer) 1");
  EXPECT_EQ(SetIsMember("src", "a", db_), "(integer) 0");
  EXPECT_EQ(SetIsMember("dst", "a", db_), "(integer) 1");
}

TEST_F(SetCmdTest, set_move_non_existing_member_returns_zero) {
  SetAdd("src", {"a"}, db_);
  SetAdd("dst", {}, db_);
  EXPECT_EQ(SetMove("src", "dst", "z", db_), "(integer) 0");
}

TEST_F(SetCmdTest, set_move_non_existing_source_returns_zero) {
  SetAdd("dst", {"a"}, db_);
  EXPECT_EQ(SetMove("missing", "dst", "a", db_), "(integer) 0");
}

TEST_F(SetCmdTest, set_move_source_and_destination_are_same_sets) {
  SetAdd("s", {"a", "b"}, db_);
  EXPECT_EQ(SetMove("s", "s", "a", db_), "(integer) 1");
  EXPECT_EQ(SetIsMember("s", "a", db_), "(integer) 1");
}

TEST_F(SetCmdTest, set_move_last_member_source_deleted) {
  SetAdd("src", {"only"}, db_);
  SetAdd("dst", {"x"}, db_);
  SetMove("src", "dst", "only", db_);
  EXPECT_EQ(SetCard("src", db_), "(integer) 0");
}

class SetExecutorTest : public ::testing::Test {
protected:
  Executor ex_;
};

TEST_F(SetExecutorTest, executor_set_add_and_set_card) {
  ex_.Execute({"SADD", "s", "a", "b", "c"});
  EXPECT_EQ(ex_.Execute({"SCARD", "s"}), "(integer) 3");
}

TEST_F(SetExecutorTest, executor_set_add_and_set_card_lower_case) {
  ex_.Execute({"sadd", "s", "a", "b", "c"});
  EXPECT_EQ(ex_.Execute({"SCARD", "s"}), "(integer) 3");
}

TEST_F(SetExecutorTest, executor_set_is_member) {
  ex_.Execute({"SADD", "s", "hello"});
  EXPECT_EQ(ex_.Execute({"SISMEMBER", "s", "hello"}), "(integer) 1");
  EXPECT_EQ(ex_.Execute({"SISMEMBER", "s", "world"}), "(integer) 0");
}

TEST_F(SetExecutorTest, executor_set_remove_then_set_card) {
  ex_.Execute({"SADD", "s", "a", "b", "c"});
  ex_.Execute({"SREM", "s", "a"});
  EXPECT_EQ(ex_.Execute({"SCARD", "s"}), "(integer) 2");
}

TEST_F(SetExecutorTest, executor_set_move) {
  ex_.Execute({"SADD", "src", "x"});
  ex_.Execute({"SADD", "dst", "y"});
  EXPECT_EQ(ex_.Execute({"SMOVE", "src", "dst", "x"}), "(integer) 1");
  EXPECT_EQ(ex_.Execute({"SISMEMBER", "src", "x"}), "(integer) 0");
  EXPECT_EQ(ex_.Execute({"SISMEMBER", "dst", "x"}), "(integer) 1");
}

TEST_F(SetExecutorTest, executor_wrong_number_of_args_returns_empty) {
  EXPECT_EQ(ex_.Execute({"SADD"}), "(error) wrong number of arguments for 'SADD'\n");
  EXPECT_EQ(ex_.Execute({"SISMEMBER", "s"}), "(error) wrong number of arguments for 'SISMEMBER'\n");
}