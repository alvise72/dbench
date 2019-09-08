#ifndef __ABSWRITER_HH__
#define __ABSWRITER_HH__

#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <numeric>

class writer {
 protected:

  off_t               m_buffer_len;
  void*               m_buffer_to_write;
  bool                m_do_flush;
  double              m_rate;
  unsigned long long  m_microseconds;
  std::string         m_error_message;
  std::vector<off_t>  m_offsets;
  bool                m_status;
  long long           m_block_delay;
  int                 m_fd;
  std::string         m_filename;
  int                 m_forced_rate;

// static std::mutex   s_mutex;

  writer( ) {}
  writer( const writer& ) {}
  
 public:
  
  writer( int, void*, off_t, bool, const std::vector<off_t>&, const long long, const std::string& fname );
  
  virtual ~writer( );
  
  virtual void doit( void );
  
  std::thread spawn( ) { return std::thread( [this] { this->doit(); } ); }
  
  double get_rate( void ) const { return m_rate; }
  
  unsigned long long get_microseconds( void ) const { return m_microseconds; }

  std::string get_error_message( void ) const { return m_error_message; }
  
  bool get_status( void ) const { return m_status; }
  
};

#endif
