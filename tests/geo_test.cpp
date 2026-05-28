#include "gtest/gtest.h"
#include "geo_cmd.hpp"
#include "string_cmd.hpp"
#include "executor.hpp"
#include "database.hpp"
#include <string>
#include <vector>
#include <cstddef>

static int ParseInteger(const std::string& s) {
  return std::stoi(s.substr(std::string("(integer) ").size()));
}

static std::vector<std::string> ParseMembers(const std::string& s) {
  std::vector<std::string> result;
  size_t pos = 0;
  while (pos < s.size()) {
    size_t q1 = s.find('"', pos);
    if (q1 == std::string::npos) break;
    size_t q2 = s.find('"', q1 + 1);
    if (q2 == std::string::npos) break;
    result.push_back(s.substr(q1 + 1, q2 - q1 - 1));
    pos = q2 + 1;
  }
  return result;
}

static const double kMskLon = 37.6173;
static const double kMskLat = 55.7558;
static const double kSpbLon = 30.3141;
static const double kSpbLat = 59.9386;
static const double kMmkLon = 33.0833;
static const double kMmkLat = 68.9666;

class GeoCmdTest : public ::testing::Test {
public:
  DataBase db_;
  Executor ex_;
};

TEST_F(GeoCmdTest, geo_add_new_key_returns_count_added) {
  EXPECT_EQ(ParseInteger(GeoAdd("cities", {
    GeoEntry{kMskLon, kMskLat, "Moscow"},
    GeoEntry{kSpbLon, kSpbLat, "SPb"},
    GeoEntry{kMmkLon, kMmkLat, "Murmansk"}
  }, db_)), 3);
}

TEST_F(GeoCmdTest, geo_add_duplicate_member_returns_zero) {
  GeoAdd("cities", {GeoEntry{kMskLon, kMskLat, "Moscow"}}, db_);
  EXPECT_EQ(ParseInteger(GeoAdd("cities", {
    GeoEntry{kMskLon, kMskLat, "Moscow"}
  }, db_)), 0);
}

TEST_F(GeoCmdTest, geo_add_wrong_type_returns_error) {
  StringSet("k", "val", db_);
  EXPECT_EQ(GeoAdd("k", {GeoEntry{0.0, 0.0, "x"}}, db_),
    "(error) WRONGTYPE operation against a key holding the wrong kind of value");
}

TEST_F(GeoCmdTest, geo_pos_existing_member_contains_coordinates) {
  GeoAdd("cities", {GeoEntry{kMskLon, kMskLat, "Moscow"}}, db_);
  std::string out = GeoPos("cities", {"Moscow"}, db_);
  EXPECT_NE(out.find("37."), std::string::npos);
  EXPECT_NE(out.find("55."), std::string::npos);
}

TEST_F(GeoCmdTest, geo_pos_non_existent_member_returns_nil_entry) {
  GeoAdd("cities", {GeoEntry{kMskLon, kMskLat, "Moscow"}}, db_);
  std::string out = GeoPos("cities", {"Unknown"}, db_);
  EXPECT_NE(out.find("(nil)"), std::string::npos);
}

TEST_F(GeoCmdTest, geo_pos_non_existent_key_returns_nil) {
  EXPECT_EQ(GeoPos("missing", {"Moscow"}, db_), "(nil)");
}

TEST_F(GeoCmdTest, geo_dist_moscow_to_spb_in_km_approximate) {
  GeoAdd("cities", {
    GeoEntry{kMskLon, kMskLat, "Moscow"},
    GeoEntry{kSpbLon, kSpbLat, "SPb"}
  }, db_);
  double dist = std::stod(GeoDist("cities", "Moscow", "SPb", "km", db_));
  EXPECT_GT(dist, 600.0);
  EXPECT_LT(dist, 700.0);
}

TEST_F(GeoCmdTest, geo_dist_same_member_returns_zero) {
  GeoAdd("cities", {GeoEntry{kMskLon, kMskLat, "Moscow"}}, db_);
  double dist = std::stod(GeoDist("cities", "Moscow", "Moscow", "km", db_));
  EXPECT_NEAR(dist, 0.0, 0.001);
}

TEST_F(GeoCmdTest, geo_dist_meters_equals_km_times_thousand) {
  GeoAdd("cities", {
    GeoEntry{kMskLon, kMskLat, "Moscow"},
    GeoEntry{kSpbLon, kSpbLat, "SPb"}
  }, db_);
  double km = std::stod(GeoDist("cities", "Moscow", "SPb", "km", db_));
  double m  = std::stod(GeoDist("cities", "Moscow", "SPb", "m",  db_));
  EXPECT_NEAR(m, km * 1000.0, 1.0);
}

TEST_F(GeoCmdTest, geo_dist_non_existent_member_returns_nil) {
  GeoAdd("cities", {GeoEntry{kMskLon, kMskLat, "Moscow"}}, db_);
  EXPECT_EQ(GeoDist("cities", "Moscow", "Unknown", "km", db_), "(nil)");
}

TEST_F(GeoCmdTest, geo_dist_non_existent_key_returns_error) {
  EXPECT_EQ(GeoDist("missing", "A", "B", "km", db_), "(nil)");
}

TEST_F(GeoCmdTest, geo_dist_miles_and_feet_consistent) {
  GeoAdd("cities", {
    GeoEntry{kMskLon, kMskLat, "Moscow"},
    GeoEntry{kSpbLon, kSpbLat, "SPb"}
  }, db_);
  double mi = std::stod(GeoDist("cities", "Moscow", "SPb", "mi", db_));
  double ft = std::stod(GeoDist("cities", "Moscow", "SPb", "ft", db_));
  EXPECT_NEAR(mi * 5280.0, ft, ft * 0.01);
}

TEST_F(GeoCmdTest, geo_search_small_radius_only_moscow) {
  GeoAdd("cities", {
    GeoEntry{kMskLon, kMskLat, "Moscow"},
    GeoEntry{kSpbLon, kSpbLat, "SPb"},
    GeoEntry{kMmkLon, kMmkLat, "Murmansk"}
  }, db_);
  auto members = ParseMembers(GeoSearch("cities", kMskLon, kMskLat, 100.0, "km", true, 0, db_));
  EXPECT_EQ(members.size(), 1);
  EXPECT_EQ(members[0], "Moscow");
}

TEST_F(GeoCmdTest, geo_search_large_radius_all_cities) {
  GeoAdd("cities", {
    GeoEntry{kMskLon, kMskLat, "Moscow"},
    GeoEntry{kSpbLon, kSpbLat, "SPb"},
    GeoEntry{kMmkLon, kMmkLat, "Murmansk"}
  }, db_);
  auto members = ParseMembers(GeoSearch("cities", kMskLon, kMskLat, 5000.0, "km", true, 0, db_));
  EXPECT_EQ(members.size(), 3);
}

TEST_F(GeoCmdTest, geo_search_asc_order_closest_first) {
  GeoAdd("cities", {
    GeoEntry{kMskLon, kMskLat, "Moscow"},
    GeoEntry{kSpbLon, kSpbLat, "SPb"},
    GeoEntry{kMmkLon, kMmkLat, "Murmansk"}
  }, db_);
  auto members = ParseMembers(GeoSearch("cities", kMskLon, kMskLat, 5000.0, "km", true, 0, db_));
  EXPECT_EQ(members[0], "Moscow");
}

TEST_F(GeoCmdTest, geo_search_desc_order_farthest_first) {
  GeoAdd("cities", {
    GeoEntry{kMskLon, kMskLat, "Moscow"},
    GeoEntry{kSpbLon, kSpbLat, "SPb"},
    GeoEntry{kMmkLon, kMmkLat, "Murmansk"}
  }, db_);
  auto members = ParseMembers(GeoSearch("cities", kMskLon, kMskLat, 5000.0, "km", false, 0, db_));
  EXPECT_EQ(members[0], "Murmansk");
  EXPECT_EQ(members[2], "Moscow");
}

TEST_F(GeoCmdTest, geo_search_count_limit_returns_only_n) {
  GeoAdd("cities", {
    GeoEntry{kMskLon, kMskLat, "Moscow"},
    GeoEntry{kSpbLon, kSpbLat, "SPb"},
    GeoEntry{kMmkLon, kMmkLat, "Murmansk"}
  }, db_);
  auto members = ParseMembers(GeoSearch("cities", kMskLon, kMskLat, 5000.0, "km", true, 2, db_));
  EXPECT_EQ(members.size(), 2);
}

TEST_F(GeoCmdTest, geo_search_non_existent_key_returns_empty) {
  EXPECT_EQ(GeoSearch("missing", kMskLon, kMskLat, 100.0, "km", true, 0, db_),
    "");
}

TEST_F(GeoCmdTest, geo_search_store_saves_nearby_and_returns_count) {
  GeoAdd("cities", {
    GeoEntry{kMskLon, kMskLat, "Moscow"},
    GeoEntry{kSpbLon, kSpbLat, "SPb"},
    GeoEntry{kMmkLon, kMmkLat, "Murmansk"}
  }, db_);
  EXPECT_EQ(ParseInteger(GeoSearchStore("nearby", "cities", kMskLon, kMskLat, 800.0, "km", true, 0, db_)), 2);
  EXPECT_TRUE(db_.Exists("nearby"));
}

TEST_F(GeoCmdTest, geo_search_store_non_existent_source_returns_zero) {
  EXPECT_EQ(GeoSearchStore("dst", "missing", kMskLon, kMskLat, 100.0, "km", true, 0, db_),
    "(integer) 0");
}

TEST_F(GeoCmdTest, executor_geoadd_and_geodist) {
  ex_.Execute({"GEOADD", "cities", "37.6173", "55.7558", "Moscow", "30.3141", "59.9386", "SPb"});
  double dist = std::stod(ex_.Execute({"GEODIST", "cities", "Moscow", "SPb", "km"}));
  EXPECT_GT(dist, 600.0);
  EXPECT_LT(dist, 700.0);
}

TEST_F(GeoCmdTest, executor_geosearch_asc) {
  ex_.Execute({"GEOADD", "cities",
    "37.6173", "55.7558", "Moscow",
    "30.3141", "59.9386", "SPb",
    "33.0833", "68.9666", "Murmansk"
  });
  auto members = ParseMembers(ex_.Execute({
    "GEOSEARCH", "cities",
    "FROMLONLAT", "37.6173", "55.7558",
    "BYRADIUS", "100", "km", "ASC"
  }));
  EXPECT_EQ(members.size(), 1);
  EXPECT_EQ(members[0], "Moscow");
}

TEST_F(GeoCmdTest, executor_wrong_args_returns_empty) {
  EXPECT_EQ(ex_.Execute({"GEOADD"}), "");
  EXPECT_EQ(ex_.Execute({"GEODIST", "cities", "Moscow"}), "");
}
