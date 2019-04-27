#ifndef __BUTILS_H_
#define __BUTILS_H_

#include <string>
#include <vector>
#include <string>
#include <numeric>

namespace utils {
  std::string prettyPrintSize(unsigned long long num);
  unsigned long long get_microseconds( /*struct timeval&*/ );
  bool check_power_of_two( unsigned long long );
  bool check_multiple_of_512( unsigned long long );
  void shuffle_vector( std::vector<off_t>& );
  bool is_digits(const std::string&);
}
#endif
