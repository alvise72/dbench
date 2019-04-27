#include <cmath>
#include "writer_pool.h"
#include "writer.h"

//--------------------------------------------------------------------------------
writer_pool::writer_pool(int _pool_size, void* _buffer, off_t _buflen, bool _flush )
  : pool_size(_pool_size),
    flush(_flush),
    buflen(_buflen),
    buffer(_buffer),
    p(perf()),
    error_message("")
{
}

//--------------------------------------------------------------------------------
void writer_pool::add_writer( int fd, const std::vector<off_t>& _offsets, const long long block_delay, const std::string& fname ) {

  writers.push_back( new writer( fd, buffer, buflen, flush, _offsets, block_delay, fname ) );
  
}

//--------------------------------------------------------------------------------
void writer_pool::start( ) {
  for( auto it : writers )
    m_threads.push_back( it->spawn() );
}

//--------------------------------------------------------------------------------
void writer_pool::join( ) {
  for(std::list<std::thread>::iterator it = begin(m_threads); it != end(m_threads); ++it)
    it->join();
}

//--------------------------------------------------------------------------------
bool writer_pool::get_status( ) {
  for( auto it : writers ) {
    if(!it->get_status()) {
      error_message = it->get_error_message( );
      return false;
    }
  }
  return true;
}

//--------------------------------------------------------------------------------
void writer_pool::compute_perf( ) {
  if(writers.size() < 1) return;

  p.rate_max    = p.rate_min  = (*writers.begin())->get_rate();
  p.rate_avg    = 0.0;
  p.delta_max   = p.delta_min = (*writers.begin())->get_microseconds();
  p.delta_avg   = 0.0;
  p.rate_stddev = 0.0;
  
  for(auto it : writers ) {
    if(p.rate_max < it->get_rate()) p.rate_max = it->get_rate();
    if(p.rate_min > it->get_rate()) p.rate_min = it->get_rate();
    p.rate_avg += it->get_rate();
    if(p.delta_max < it->get_microseconds()) p.delta_max = it->get_microseconds();
    if(p.delta_min > it->get_microseconds()) p.delta_min = it->get_microseconds();
    p.delta_avg += it->get_microseconds();
  }
  p.rate_avg  /= (float)pool_size;
  p.delta_avg /= (float)pool_size;

  for( auto it : writers )
    p.rate_stddev += pow((it->get_rate() - p.rate_avg),2);
  
  if(pool_size>1)
    p.rate_stddev /= (pool_size-1);
  
  p.rate_stddev = sqrt(p.rate_stddev);
}
