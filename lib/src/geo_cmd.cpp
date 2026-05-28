#include <cmath>
#include <string>
#include <vector>
#include "geo_cmd.hpp"
#include <algorithm>
#include "database.hpp"
#include <cstddef>

bool IsValidCoordinates(double longitude, double latitude) {
  using namespace haversine_constants;
  return longitude >= kMinLongitude && longitude <= kMaxLongitude &&
          latitude  >= kMinLatitude && latitude <= kMaxLatitude;
}

double Haversine(double lon1, double lat1, double lon2, double lat2) {
  using namespace haversine_constants;
  double diff_lat = (lat2 - lat1) * kPi / kPiToDegrees;
  double diff_lon = (lon2 - lon1) * kPi / kPiToDegrees;
  
  double chord_length_sq = std::sin(diff_lat/2) * std::sin(diff_lat/2) +
              std::cos(lat1 * kPi / kPiToDegrees) * std::cos(lat2 * kPi / kPiToDegrees) *
              std::sin(diff_lon/2) * std::sin(diff_lon/2);
  
  return kEarthRadius * 2 * std::asin(std::sqrt(chord_length_sq));
} //returns in kilometers

double ConvertKmToUnits(double dist_km, const std::string& unit) {
  using namespace distance_units;

  if (unit == kMeters) return dist_km * kKmToM;
  if (unit == kMiles) return dist_km * kKmToMiles;
  if (unit == kFeet) return dist_km * kKmToFeet;
  if (unit == kKilometers) return dist_km;

  return kBadUnit;
}

std::vector<std::pair<std::string, double>> GeoSearchInternal(
    GeoType& geo, double lon, double lat, 
    double radius, const std::string& unit, bool asc, int count) {

    std::vector<std::pair<std::string, double>> results;
    for (auto& [member, point] : geo) {
      double dist = Haversine(lon, lat, point.longitude_, point.latitude_);
      dist = ConvertKmToUnits(dist, unit);
      if (dist < 0) continue;
      if (dist <= radius) {
        results.push_back({member, dist});
      }
    }

    std::sort(results.begin(), results.end(),
    [asc](const auto& a, const auto& b) {
      return asc ? a.second < b.second : a.second > b.second;
    });
    if (count > 0 && results.size() > count) {
      results.resize(count);
    }

    return results;
}

std::string GeoAdd(const std::string& key, const std::vector<GeoEntry>& data, DataBase& data_base) {
  size_t needed_mem = 0;
  if (!data_base.Exists(key)) {
    needed_mem += key.size() + kKeyOverhead;
  }
  for (auto& el : data) {
    needed_mem += el.member_.size() + kCoordinatesOverhead;
  }
  data_base.CheckMemory(needed_mem);

  if (!data_base.Exists(key)) {
    data_base.Set(key, GeoType{});
  }

  Value& val = data_base.Get(key);
  auto* geo = GetAs<GeoType>(val);
  if (!geo) return kWrongTypeErr;

  int added = 0;
  for (auto& point : data) {
    if (!IsValidCoordinates(point.longitude_, point.latitude_)) {
      return kInvalidCoordinates;
    }
    auto [it, inserted] = geo->insert_or_assign(
      point.member_, GeoPoint{point.longitude_, point.latitude_}
    );
    if (inserted) added++;
  }

  return "(integer) " + std::to_string(added);
}

std::string GeoPos(const std::string& key, const std::vector<std::string>& members, DataBase& data_base) {
  if (!data_base.Exists(key)) {
    return "(nil)";
  }

  Value& val = data_base.Get(key);
  auto* geo = GetAs<GeoType>(val);
  if (!geo) return kWrongTypeErr;

  std::string result;
  int count = 0;
  for (auto& point_name : members) {
    auto it = geo->find(point_name);
    if (it != geo->end()) {
      result += std::to_string(++count) + ") 1) \"" + 
                std::to_string(it->second.longitude_) + "\"\n" +
                "   2) \"" + std::to_string(it->second.latitude_) + "\"\n";
    } else {
      result += std::to_string(++count) + ") (nil)\n";
    }
  }
  return result;

}

std::string GeoDist(const std::string& key, const std::string& member1, const std::string& member2, const std::string& unit, DataBase& data_base) {
  if (!data_base.Exists(key)) {
    return "(nil)";
  }

  Value& val = data_base.Get(key);
  auto* geo = GetAs<GeoType>(val);
  if (!geo) return kWrongTypeErr;

  auto member1_iterator = geo->find(member1);
  auto member2_iterator = geo->find(member2);
  if (member1_iterator != geo->end() && member2_iterator != geo->end()) {
    double longitude_source = member1_iterator->second.longitude_;
    double latitude_source = member1_iterator->second.latitude_;
    double longitude_destination = member2_iterator->second.longitude_;
    double latitude_destination = member2_iterator->second.latitude_;
    double distance = ConvertKmToUnits(Haversine(longitude_source, latitude_source, longitude_destination, latitude_destination), unit);
    if (distance < 0) return "(error) ERR incorrect unit";
    return std::to_string(distance);
  }
  return "(nil)";
}

std::string GeoSearch(const std::string& key, double lon, double lat, 
  double radius, const std::string& unit, 
  bool asc, int count, DataBase& data_base) {
    if (!data_base.Exists(key)) {
      return "";
    }

    Value& val = data_base.Get(key);
    auto* geo = GetAs<GeoType>(val);
    if (!geo) return kWrongTypeErr;

    std::vector<std::pair<std::string, double>> results;
    results = GeoSearchInternal(*geo, lon, lat, radius, unit, asc, count);
    std::string result;

    int out_counter = 0;
    for (auto& [member, dist] : results) {
      result += std::to_string(++out_counter) + ") \"" + member + "\"\n";
    }
    return result;
}

std::string GeoSearchStore(const std::string& dest_key, const std::string& source_key, 
  double lon, double lat, 
  double radius, const std::string& unit, 
  bool asc, int count, DataBase& data_base) {
    if (!data_base.Exists(source_key)) {
      return "(integer) 0";
    }

    Value& val = data_base.Get(source_key);
    auto* geo = GetAs<GeoType>(val);
    if (!geo) return kWrongTypeErr;

    GeoType new_geo;
    auto results = GeoSearchInternal(*geo, lon, lat, radius, unit, asc, count);
    for (auto& [member, dist] : results) {
      auto it = geo->find(member);
      new_geo[member] = it->second;
    }
    data_base.Set(dest_key, new_geo);
    return "(integer) " + std::to_string(results.size());
  }