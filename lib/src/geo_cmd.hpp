#pragma once
#include "database.hpp"

namespace haversine_constants {
  inline constexpr double kEarthRadius = 6372.8;
  inline constexpr double kPi = 3.14159265358979323846;
  inline constexpr double kPiToDegrees = 180.0;

  inline constexpr double kMinLongitude = -180.0;
  inline constexpr double kMaxLongitude = 180.0;
  inline constexpr double kMinLatitude = -90.0;
  inline constexpr double kMaxLatitude = 90.0;
} // namespace haversine_constants

namespace distance_units {
  inline constexpr const char* kKilometers = "km";
  inline constexpr const char* kMeters = "m";
  inline constexpr const char* kMiles = "mi";
  inline constexpr const char* kFeet = "ft";

  inline constexpr double kKmToM = 1000.0;
  inline constexpr double kKmToMiles = 0.621371;
  inline constexpr double kKmToFeet = 3280.84;

  inline constexpr double kBadUnit = -1.0;
} // namespace distance_units



struct GeoEntry {
  double longitude_;
  double latitude_;
  std::string member_;

  explicit GeoEntry(double longitude, double latitude, const std::string& member) :
    longitude_(longitude), latitude_(latitude), member_(member) {}

  bool operator==(const GeoEntry& other) {
    return (longitude_ == other.longitude_ && latitude_ == other.latitude_);
  }
};

bool IsValidCoordinates(double longitude, double latitude);

double Haversine(double lon1, double lat1, double lon2, double lat2);

std::string GeoAdd(const std::string& key, const std::vector<GeoEntry>& data, DataBase& data_base);

std::string GeoPos(const std::string& key, const std::vector<std::string>& members, DataBase& data_base);

std::string GeoDist(const std::string& key, const std::string& member1, const std::string& member2, const std::string& unit, DataBase& db);

std::string GeoSearch(const std::string& key, double lon, double lat, 
  double radius, const std::string& unit, 
  bool asc, int count, DataBase& db);

std::string GeoSearchStore(const std::string& dest, const std::string& source, 
  double lon, double lat, 
  double radius, const std::string& unit, 
  bool asc, int count, DataBase& db);

std::vector<std::pair<std::string, double>> GeoSearchInternal(
  GeoType& geo, double lon, double lat, 
  double radius, const std::string& unit, bool asc, int count);