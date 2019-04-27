#ifndef _LOGGER_H_
#define _LOGGER_H_

#include <mutex>
#include <string>

class logger {
 private:
  static logger* _instance;
  logger() {}
  static std::mutex m, l;
  bool quiet;
  
 public:
  static logger* get( );
  void log( const std::string& );
  void err( const std::string& );
  static void init( );
  void set_quiet( const bool );
};

#endif
