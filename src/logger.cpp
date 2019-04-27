#include "logger.h"
#include "utils.h"
#include <string>
#include <iostream>
#include <chrono>
#include <ctime>
#include <cstring>
#include <sys/time.h>

std::mutex logger::m;// = std::mutex();
std::mutex logger::l;// = std::mutex();
logger* logger::_instance = nullptr;

logger* logger::get()
{
  return _instance;
}

void logger::init( ) {
  std::lock_guard<std::mutex> guard( m );
  if(_instance == nullptr)
    _instance = new logger( );
}

void logger::log( const std::string& s ) {
  if(quiet) return;
  std::lock_guard<std::mutex> guard( l );

  struct timeval tval;
  gettimeofday(&tval, 0);
  time_t T = tval.tv_sec;
  char buffer[30];
  memset((void*)buffer, 0, 30);
  std::strftime(buffer,18,"%Y%m%d %H:%M:%S", std::localtime(&T));
  std::string timenow = std::string(buffer) + "." + std::to_string(tval.tv_usec);
  std::cout << timenow << " - " << s << std::endl;
}

void logger::err( const std::string& s ) {
  if(quiet) return;
  std::lock_guard<std::mutex> guard( l );
  
  struct timeval tval;
  gettimeofday(&tval, 0);
  time_t T = tval.tv_sec;
  char buffer[30];
  memset((void*)buffer, 0, 30);
  std::strftime(buffer,18,"%Y%m%d %H:%M:%S", std::localtime(&T));
  std::string timenow = std::string(buffer) + "." + std::to_string(tval.tv_usec);
  std::cerr << timenow << " - " << s << std::endl;
}

void logger::set_quiet( const bool q ) {
  quiet  = q;
}