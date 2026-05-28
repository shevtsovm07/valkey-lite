#include "gtest/gtest.h"
#include "list_cmd.hpp"
#include "string_cmd.hpp"
#include "executor.hpp"
#include "database.hpp"
#include <vector>
#include <string>
#include <cstddef>

std::vector<std::string> ParseListOutput(const std::string& output) {
  std::vector<std::string> result;
  size_t pos = 0;
  while (pos < output.size()) {
    size_t q1 = output.find('"', pos);
    if (q1 == std::string::npos) break;
    size_t q2 = output.find('"', q1+1);
    if (q2 == std::string::npos) break;
    result.push_back(output.substr(q1+1, q2 - q1 - 1));
    pos = q2 + 1;
  }
  return result;
}

class ListCmdTest : public ::testing::Test {
public:
  DataBase db_;
  Executor ex_;
};

TEST_F(ListCmdTest, list_push_single_el_return_one) {
  EXPECT_EQ(ListPush(true, "l", {"a"}, db_), "(integer) 1");
}

TEST_F(ListCmdTest, lpush_in_correct_order_output) {
  ListPush(true, "l", {"a", "b", "c"}, db_);
  auto elems = ParseListOutput(ListRange("l", 0, -1, db_));
  EXPECT_EQ(elems, (std::vector<std::string>{"c", "b", "a"}));
}

TEST_F(ListCmdTest, rpush_in_correct_order_output) {
  ListPush(false, "l", {"a", "b", "c"}, db_);
  auto elems = ParseListOutput(ListRange("l", 0, -1, db_));
  EXPECT_EQ(elems, (std::vector<std::string>{"a", "b", "c"}));
}

TEST_F(ListCmdTest, lpush_wrong_type_error) {
  StringSet("k", "val", db_);
  EXPECT_EQ(ListPush(true, "k", {"x"}, db_),
      "(error) WRONGTYPE operation against a key holding the wrong kind of value");
}

TEST_F(ListCmdTest, lpop_single_element_implicit_count_returns_value) {
  ListPush(false, "l", {"a", "b", "c"}, db_);
  EXPECT_EQ(ListPop(true, "l", 1, false, db_), "a");
}

TEST_F(ListCmdTest, lpop_single_element_explicit_count_returns_value) {
  ListPush(false, "l", {"a", "b", "c"}, db_);
  EXPECT_EQ(ListPop(true, "l", 1, true, db_), "1) \"a\"\n");
}

TEST_F(ListCmdTest, rpop_single_element_returns_value) {
  ListPush(false, "l", {"a", "b", "c"}, db_);
  EXPECT_EQ(ListPop(false, "l", 1, false, db_), "c");
}

TEST_F(ListCmdTest, lpop_with_count_return_multiple) {
  ListPush(false, "l", {"a", "b", "c"}, db_);
  auto elems = ParseListOutput(ListPop(true, "l", 2, false, db_));
  EXPECT_EQ(elems, (std::vector<std::string>{"a", "b"}));
}

TEST_F(ListCmdTest, lpop_with_count_more_than_size_of_set_return_multiple) {
  ListPush(false, "l", {"a", "b"}, db_);
  auto elems = ParseListOutput(ListPop(true, "l", 10, false, db_));
  EXPECT_EQ(elems, (std::vector<std::string>{"a", "b"}));
}

TEST_F(ListCmdTest, lpop_non_existing_key_returns_nil) {
  EXPECT_EQ(ListPop(true, "missing", 1, false, db_), "(nil)");
}

TEST_F(ListCmdTest, lpop_last_element_pop_deletes_list) {
  ListPush(false, "l", {"only"}, db_);
  ListPop(true, "l", 1, false, db_);
  EXPECT_EQ(ListLen("l", db_), "(integer) 0");
  EXPECT_FALSE(db_.Exists("l"));
}

TEST_F(ListCmdTest, list_len_returns_correct_count) {
  ListPush(false, "l", {"a", "b", "c"}, db_);
  EXPECT_EQ(ListLen("l", db_), "(integer) 3");
}

TEST_F(ListCmdTest, list_len_non_existent_key_returns_zero) {
  EXPECT_EQ(ListLen("missing", db_), "(integer) 0");
}

TEST_F(ListCmdTest, list_len_decreases_after_pop) {
  ListPush(false, "l", {"a", "b", "c"}, db_);
  ListPop(true, "l", 1, false, db_);
  EXPECT_EQ(ListLen("l", db_), "(integer) 2");
}

TEST_F(ListCmdTest, list_range_full_list) {
  ListPush(false, "l", {"a", "b", "c"}, db_);
  auto elems = ParseListOutput(ListRange("l", 0, -1, db_));
  EXPECT_EQ(elems, (std::vector<std::string>{"a", "b", "c"}));
}

TEST_F(ListCmdTest, list_range_sub_range) {
  ListPush(false, "l", {"a", "b", "c", "d"}, db_);
  auto elems = ParseListOutput(ListRange("l", 1, 2, db_));
  EXPECT_EQ(elems, (std::vector<std::string>{"b", "c"}));
}

TEST_F(ListCmdTest, list_range_negative_indexes) {
  ListPush(false, "l", {"a", "b", "c"}, db_);
  auto elems = ParseListOutput(ListRange("l", -2, -1, db_));
  EXPECT_EQ(elems, (std::vector<std::string>{"b", "c"}));
}

TEST_F(ListCmdTest, list_range_out_of_bonds_returns_empty) {
  ListPush(false, "l", {"a", "b"}, db_);
  EXPECT_EQ(ListRange("l", 10, 20, db_), "");
}

TEST_F(ListCmdTest, list_range_non_existing_key_returns_empty) {
  EXPECT_EQ(ListRange("missing", 0, -1, db_), "");
}

TEST_F(ListCmdTest, list_index_valid_index_returns_element) {
  ListPush(false, "l", {"a", "b", "c"}, db_);
  EXPECT_EQ(ListIndex("l", 1, db_), "b");
}

TEST_F(ListCmdTest, list_index_negative_index) {
  ListPush(false, "l", {"a", "b", "c"}, db_);
  EXPECT_EQ(ListIndex("l", -1, db_), "c");
}

TEST_F(ListCmdTest, list_index_out_of_bonds_returns_nil) {
  ListPush(false, "l", {"a"}, db_);
  EXPECT_EQ(ListIndex("l", 5, db_), "(nil)");
}

TEST_F(ListCmdTest, list_index_non_existent_key_returns_nil) {
  EXPECT_EQ(ListIndex("missing", 0, db_), "(nil)");
}

TEST_F(ListCmdTest, list_set_valid_index_returns_ok) {
  ListPush(false, "l", {"a", "b", "c"}, db_);
  EXPECT_EQ(ListSetIndex("l", 1, "X", db_), "OK");
  EXPECT_EQ(ListIndex("l", 1, db_), "X");
}

TEST_F(ListCmdTest, list_set_negative_index) {
  ListPush(false, "l", {"a", "b", "c"}, db_);
  ListSetIndex("l", -1, "Z", db_);
  EXPECT_EQ(ListIndex("l", 2, db_), "Z");
}

TEST_F(ListCmdTest, list_set_out_of_bonds_returns_error) {
  ListPush(false, "l", {"a"}, db_);
  EXPECT_EQ(ListSetIndex("l", 5, "X", db_), "(error) ERR index out of range");
}

TEST_F(ListCmdTest, list_set_non_existent_key_returns_error) {
  EXPECT_EQ(ListSetIndex("missing", 0, "X", db_), "(error) no such key");
}

TEST_F(ListCmdTest, list_insert_before_pivot) {
  ListPush(false, "l", {"a", "b", "c"}, db_);
  ListInsert(true, "l", "b", "X", db_);
  auto elems = ParseListOutput(ListRange("l", 0, -1, db_));
  EXPECT_EQ(elems, (std::vector<std::string>{"a", "X", "b", "c"}));
}

TEST_F(ListCmdTest, list_insert_after_pivot) {
  ListPush(false, "l", {"a", "b", "c"}, db_);
  ListInsert(false, "l", "b", "X", db_);
  auto elems = ParseListOutput(ListRange("l", 0, -1, db_));
  EXPECT_EQ(elems, (std::vector<std::string>{"a", "b", "X", "c"}));
}

TEST_F(ListCmdTest, list_insert_pivot_not_founf_returns_minus_one) {
  ListPush(false, "l", {"a", "b"}, db_);
  EXPECT_EQ(ListInsert(true, "l", "z", "X", db_), "(integer) -1");
}

TEST_F(ListCmdTest, list_insert_non_existent_key_returns_zero) {
  EXPECT_EQ(ListInsert(true, "missing", "b", "X", db_), "(integer) 0");
}

class ListExecutorTest : public ::testing::Test {
protected:
  Executor ex_;
};

TEST_F(ListExecutorTest, executor_rpush_and_len) {
  ex_.Execute({"RPUSH", "l", "a", "b", "c"});
  EXPECT_EQ(ex_.Execute({"LLEN", "l"}), "(integer) 3");
}

TEST_F(ListExecutorTest, executor_lpush_reverses_order) {
  ex_.Execute({"LPUSH", "l", "a", "b", "c"});
  auto elems = ParseListOutput(ex_.Execute({"LRANGE", "l", "0", "-1"}));
  EXPECT_EQ(elems, (std::vector<std::string>{"c", "b", "a"}));
}

TEST_F(ListExecutorTest, executor_lpop_and_rpop) {
  ex_.Execute({"RPUSH", "l", "a", "b", "c"});
  EXPECT_EQ(ex_.Execute({"LPOP", "l"}), "a");
  EXPECT_EQ(ex_.Execute({"RPOP", "l"}), "c");
  EXPECT_EQ(ex_.Execute({"LLEN", "l"}), "(integer) 1");
}

TEST_F(ListExecutorTest, executor_wrong_number_of_args_returns_empty) {
  testing::internal::CaptureStderr();
  EXPECT_EQ(ex_.Execute({"RPUSH"}), "");
  EXPECT_EQ(testing::internal::GetCapturedStderr(), "(error) wrong number of arguments for 'RPUSH'\n");

  testing::internal::CaptureStderr();
  EXPECT_EQ(ex_.Execute({"LRANGE", "l", "0"}), "");
  EXPECT_EQ(testing::internal::GetCapturedStderr(), "(error) wrong number of arguments for 'LRANGE'\n");
}