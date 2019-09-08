#include <sys/types.h>
#include <unistd.h>
#include <cstdio>
#include <string>
#include <cstring>
#include <cerrno>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include "writer.h"
#include "logger.h"
#include "utils.h"

#define PBSTR  "||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||"
#define PBSTR2 "||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||"
#define CLR    "                                                            "

#define PBWIDTH 60
#define PBWIDTH2 50

std::mutex writer::s_mutex; //(std::mutex());

/*
 *
 *
 * Write data to a file and get timing and avg write rate
 *
 */
void printProgress (double percentage)
{
    int val = (int) (percentage * 100);
    int lpad = (int) (percentage * PBWIDTH2);
    int rpad = PBWIDTH2 - lpad;
    printf ("\r%3d%% [%.*s%*s]", val, lpad, PBSTR2, rpad, "");
    fflush (stdout);
}

/*
 *
 *
 * Create a writer instance that is ready to write data to a file
 *
 */
writer::writer( int fd, void* data_buffer, off_t buflen, bool flush, const std::vector<off_t>& offsets, const long long block_delay, const std::string& fname )
  : m_buffer_len(buflen),
    m_do_flush( flush ),
    m_rate( 0 ),
    m_microseconds( 0 ),
    m_error_message( "" ),
    m_status(true),
    m_block_delay(block_delay),
    m_fd( fd ),
    m_filename( fname )
{
  m_buffer_to_write = malloc(buflen);
  memcpy( m_buffer_to_write, data_buffer, buflen );
  m_offsets = offsets;
}

/*
 *
 *
 *
 *
 */
writer::~writer( )
{
  if(m_buffer_to_write != nullptr) free(m_buffer_to_write);
  m_buffer_to_write = nullptr;
}


/*
 *
 *
 * Write data to a file and get timing and avg write rate
 *
 */
void writer::doit( void ) {
  std::ostringstream ss;
  
  ss << std::this_thread::get_id();

  logger::get()->log("writer::doit - thread [" + ss.str() + "] starts writing file [" + m_filename + "] descriptor " + std::to_string(m_fd) + " and timing...");

  unsigned long long k=0;

  off_t *offset_native_array = (off_t*)malloc(m_offsets.size()*sizeof(off_t));
  for( auto offset : m_offsets) {
    *(offset_native_array+k) = offset*m_buffer_len;
    k++;
  }
  
  
  for(unsigned j=0; j<m_offsets.size(); ++j) {
    unsigned long long before_micros = utils::get_microseconds( );
    s_mutex.lock( );
    if(lseek(m_fd, offset_native_array[j], SEEK_SET) < 0) {
      m_error_message = strerror(errno);
      m_status=false;
      return;
    }
    if(write(m_fd, m_buffer_to_write, m_buffer_len/*, offset_native_array[j]*/ ) < m_buffer_len) {      
      m_error_message = strerror(errno);
      m_status=false;
      return;
    }
    s_mutex.unlock( );
    unsigned long long after_micros = utils::get_microseconds( );
    if(m_block_delay) {
      usleep(m_block_delay);
      m_microseconds += m_block_delay;
    }  
    unsigned long long int iteration_delay = (after_micros - before_micros);
    m_microseconds += iteration_delay;
    printProgress( ((float)( ((float)j)/((float)m_offsets.size()) )));
    //printf("%.2f", 1000000.0*((double)(m_offsets.size()*m_buffer_len)/((double)m_microseconds)) );
    //std::cout <<" "<< utils::prettyPrintSize( 1000000 * m_buffer_len / iteration_delay )<< "/s"<<"         ";
  }

  if(m_do_flush) {
      unsigned long long before_micros = utils::get_microseconds( );
      fsync(m_fd);
      unsigned long long after_micros = utils::get_microseconds( );
      m_microseconds += (after_micros - before_micros);
  }

  

  m_rate = 1000000.0*((double)(m_offsets.size()*m_buffer_len)/((double)m_microseconds));
  std::cout <<"\r"<<CLR<<"          \r";
  return;

};
