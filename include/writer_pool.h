#ifndef __WRITER_POOL_H__
#define __WRITER_POOL_H__

#include <list>
#include <string>
#include <thread>

#include "writer.h"

struct perf {
  double  rate_min;      // over threads
  double  rate_max;      // over threads
  double  rate_avg;      // over threads
  double  rate_stddev;   // over threads
  unsigned long long delta_min;
  unsigned long long delta_max;
  double delta_avg;
};

class writer_pool {
  std::list<writer*> writers;
  int pool_size;

  bool flush;
  off_t buflen;
  void* buffer;
  perf p;
  std::string error_message;
  std::list<std::thread> m_threads;
 
 public:
  writer_pool( int, void*, off_t, bool );
  void join( void );
  void start( void );
  void collect_perf(  );
  void add_writer( int, const std::vector<off_t>&, const long long, const std::string& );
  bool get_status( );
  std::string get_error_message( void ) { return error_message; }
  void compute_perf( void );
  perf get_perf( void ) { return p; }
  
};

#endif
