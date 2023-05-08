/**
 * C++/STL includes
 */
//#include <cmath>
//#include <cstdio>
//#include <cstdlib>
//#include <cstring>
//#include <cerrno>
#include <string.h>
//#include <string>
//#include <vector>
#include <iostream>
//#include <list>
//#include <numeric>
//#include <thread>
#include <chrono>
//#include <regex>
//#include <sys/stat.h>

/**
 * System's includes
 */ 
#include <fcntl.h>
#include <unistd.h>
#include <ctype.h>
#include <signal.h>

/**
 * BOOST includes
 */
//#include <boost/program_options.hpp>

/**
 * Local includes
 */
//#include "writer_pool.h"
//#include "logger.h"
#include "utils.h"

//namespace po = boost::program_options;

#define ONEKILO 1024
#define ONEMEGA 1048576
#define ONEGIGA 1073741824
#define ONETERA 1099511627776

#define RANGEull(a, b) unsigned long long a=0; a<b; a++
#define RANGEll(a, b) long long a=0; a<b; a++
#define RANGEui(a, b) unsigned int a=0; a<b; a++
#define RANGEi(a, b) int a=0; a<b; a++

void manglefd( int, bool, bool );

unsigned long long bslen    = ONEMEGA;
unsigned long long segcount = ONEKILO;
unsigned long long filesize = ONEGIGA;

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

//------------------------------------------------------------------------------------------------------------------
int main( int argc, char** argv ) {
    
  std::string filename(argv[1]);

  unsigned long fsize = ONEGIGA * atoi(argv[2]);

  unsigned long bsize = ONEKILO * atoi(argv[3]);

  std::cout << "Unlinking pre-existing file " << argv[1] << std::endl;
  unlink(argv[1]);

  std::cout << "Creating file " << argv[1] << std::endl;
  int fd = open(argv[1], O_CREAT|O_WRONLY, S_IRUSR|S_IWUSR);
  if(fd<0) {

    std::cerr << strerror(errno) << std::endl;
    exit(1);

  }
  unsigned long long count = fsize / bsize;
  void *buf = (void*)malloc(bsize);
  std::cout << "Filling buf " << bsize << " bytes" << std::endl;
  std::cout << "Filesize will be " << fsize << " bytes" << std::endl;
  memset((void*)buf, ' ', bsize);
  std::cout << "Looping write call " << count << " times on FD " << fd << std::endl;
  unsigned long long before_micros = 0, after_micros = 0, write_delay = 0;
  for( unsigned long long j = 0; j< count; j++) {
    before_micros = utils::get_microseconds( );
    write(fd, buf, bsize);
    after_micros = utils::get_microseconds( );
    std::cout<< "Write " << bsize << " bytes in " << (after_micros-before_micros) << " microseconds" << std::endl;
    if(argv[4]!=0 && atoi(argv[4]) == 1) {
      before_micros = utils::get_microseconds( );
      fsync(fd);
      after_micros = utils::get_microseconds( );
      std::cout<< "fsync delay " << (after_micros-before_micros) << " microseconds" << std::endl;
    }
  }
  if(argv[4]!=0 && atoi(argv[4]) == 2) {
      before_micros = utils::get_microseconds( );
      fsync(fd);
      after_micros = utils::get_microseconds( );
      std::cout<< "Final fsync delay " << (after_micros-before_micros) << " microseconds" << std::endl;
  }
  close(fd);
}
