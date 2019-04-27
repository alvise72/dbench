
#include "utils.h"
#include <random>
#include <algorithm>
#include <cmath>
#include <chrono>
#include <ctime>
#include <ratio>
#include <sstream>
#include <iomanip>

std::string utils::prettyPrintSize(unsigned long long num) {
  std::stringstream ss;
  ss << std::fixed << std::setprecision(2);
  if(num>=1099511627776) {
    float res = (double)num / 1099511627776.0;
    ss << res << " TB";
    return ss.str();
  }
  if(num>=1073741824) {
    float res = (double)num / 1073741824.0;
    ss << res << " GB";
    return ss.str();
  }
  if(num>=1048576) {
    float res = (double)num / 1048576.0;
    ss << res << " MB";
    return ss.str();
  }
  if(num>=1024) {
    float res = (double)num / 1024.0;
    ss << res << " kB";
    return ss.str();
  }

  ss << num << " bytes";
  return ss.str();
}

unsigned long long utils::get_microseconds( ) {
  // the following code is less efficient than extracting the number from the structure filled by gettimeofday
  // but it is more portable.
  // This code is never called inside a long loop and doesn't count in final performance measurement
  auto now = std::chrono::high_resolution_clock::now();
  auto now_ms = std::chrono::time_point_cast<std::chrono::microseconds>(now);
  auto epoch = now_ms.time_since_epoch();
  auto value = std::chrono::duration_cast<std::chrono::microseconds>(epoch);
  return value.count();
}

bool utils::check_power_of_two( unsigned long long to_check ) {
  return (to_check>0 && ((to_check & (to_check-1)) == 0));
}

void utils::shuffle_vector( std::vector<off_t> &V ) {
  std::random_device r;
  auto engine = std::default_random_engine{ r() };
  std::shuffle(std::begin(V), std::end(V), engine);
}

bool utils::check_multiple_of_512( unsigned long long i ) {
  if(i<512) return false;
  if (i % 512 == 0) return true;
  return false;

}

bool utils::is_digits(const std::string &str)
{
    return std::all_of(str.begin(), str.end(), ::isdigit); // C++11
}
