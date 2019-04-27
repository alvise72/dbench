/**
 * C++/STL includes
 */
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cerrno>
#include <string>
#include <vector>
#include <iostream>
#include <list>
#include <numeric>
#include <thread>
#include <chrono>
#include <regex>

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
#include <boost/program_options.hpp>

/**
 * Local includes
 */
#include "writer_pool.h"
#include "logger.h"
#include "utils.h"

namespace po = boost::program_options;

#define ONEKILO 1024
#define ONEMEGA ONEKILO*ONEKILO
#define ONEGIGA ONEKILO*ONEMEGA
#define ONETERA ONEKILO*ONEGIGA

#define RANGEull(a, b) unsigned long long a=0; a<b; a++
#define RANGEll(a, b) long long a=0; a<b; a++
#define RANGEui(a, b) unsigned int a=0; a<b; a++
#define RANGEi(a, b) int a=0; a<b; a++

void manglefd( int, bool );

unsigned long long bslen    = ONEMEGA;
unsigned long long segcount = ONEKILO;
unsigned long long filesize = ONEGIGA;
int flags = 0;

void killhandler(int sig)
{
   std::cout << "Caught signal=" << sig << std::endl;
};

//------------------------------------------------------------------------------------------------------------------
int main( int argc, char** argv ) {
    
  char*          	   bs                   = 0;
  bool           	   use_direct           = false;
  bool           	   flush                = false;
  bool           	   keep                 = false;
  std::list<perf>          time_info;
  long long      	   iterations           = 1;
  bool           	   write_randomly       = false;
  std::string         	   bslen_s              = std::to_string( ONEMEGA );// "1048576"
  bool           	   osync                = false;
  unsigned int   	   num_of_threads       = 1;
  bool           	   threads_on_same_file = false;
  std::string         	   filesize_s      	= std::to_string( ONEGIGA );//"1073741824";
  std::vector<std::string> to_unlink;
  std::vector<int>         to_close;
  std::string              basetestfilename	= "./testfile";
  std::string              absolute_filename    = "";
  int 			   iteration_delay      = 100;
  long long   		   block_delay          = 0;
  bool                     quiet                = false;
  bool                     existing             = false;
  
  /*
   *
   * Default Filesize to write is 1.0 GBytes
   *
   */
  
  po::options_description desc("Usage: ");
  desc.add_options()
    ("help,h", "")
    ("block-size,b", po::value<std::string>(&bslen_s), "" )
    ("block-delay,B", po::value<long long>(&block_delay),"")
    ("count,c", po::value<unsigned long long>(&segcount),"")
    ("flush,F","")
    ("quiet,Q","")
    ("use-direct,d","")
    ("osync,s","")
    ("test-filename,f", po::value<std::string>(&basetestfilename), "")
    ("iterations,i", po::value<long long>(&iterations),"")
    ("iteration-delay,I", po::value<int>(&iteration_delay), "")
    ("random,R", "")
    ("num-threads,n", po::value<unsigned int>(&num_of_threads), "" )
    ("read-after,r", "")
    ("same-file,S","" )
    ("filesize,D", po::value<std::string>(&filesize_s), "")
    ("existing-file,e", "")
    ("absolute-filename,a", po::value<std::string>(&absolute_filename), "")
    ("remove-testfile,k","");

  po::positional_options_description p;

  po::variables_map vm;
  try {
    po::store(po::command_line_parser(argc, argv).
              options(desc).positional(p).run(), vm);
    po::notify( vm );
  } catch( std::exception& ex ) {
    std::cerr << "ERROR: There was an error parsing the command line. "
              << "Error was: " << ex.what() << std::endl
              << "Type " << argv[0] 
              << " --help for the list of available options"
              << std::endl;
    return 1;
  }

  if(vm.count("help")) {
      std::cout << "Usage: " << argv[0] << " [OPTIONS]" << std::endl;
      std::cout << "\nOptions:\n" << std::endl;
      std::cout << "--block-size|b #\t\tSpecifies the size of a block which is written at once. Can use k|K|m|M|g|G suffixes"<<std::endl;
      std::cout << "--block-delay|B #\t\tSpecifies the number of microseconds to sleep between a syscall pwrite and the next one (usefull to artificially slow down the writing speed, incompatible with the options -S and -n used together)"<<std::endl;
      std::cout << "--filesize|D #\t\t\tSpecifies the file's size. Can use k|K|m|M|g|G suffixes"<<std::endl;
      std::cout << "--existing-file|e\t\tIf want to write to an existing file use this flag" << std::endl;
      std::cout << "--count|c #\t\t\tSpecifies the number of blocks to be written"<<std::endl;
      std::cout << "--flush|F\t\t\tSpecifies if take into accout the fsync() syscall in the time calculation for the throughput"<<std::endl;
      std::cout << "--use-direct|d\t\t\tSpecifies if open the file with O_DIRECT flag"<<std::endl;
      std::cout << "--test-filename|f <path>\tSpecifes path and filename to be written for the test"<<std::endl;
      std::cout << "--absolute-filename|a <path>\tSpecifies the abolute path and name for file to be written; nothing will be appended to the filename"<< std::endl;
      std::cout << "--iterations|i #\t\tSpecifies the number of tests on which the final value will be averaged" <<std::endl;
      std::cout << "--iteration-delay|I #\t\tSpecifies the number of milliseconds to sleep between an iteration and the next one" <<std::endl;
      std::cout << "--pre-allocate|p #\t\tPre-allocate disk space before start writing" <<std::endl;
      std::cout << "--random|R\t\t\tPerform random writing instead of sequential" << std::endl;
      std::cout << "--osync|s\t\t\tUse O_SYNC flag when opening the file " << std::endl;
      std::cout << "--num-threads|n #\t\tSpawn a number of thread, each one writing its own file" << std::endl;
      std::cout << "--same-file|S\t\t\tMultiple threads write on the same file" << std::endl;
      std::cout << "--remove-testfile|k\t\tRemove the testfile"<<std::endl;
      std::cout << "--quiet|Q\t\tQuiet log mode" << std::endl<<std::endl;
      return 0;
  }

  existing             = vm.count("existing-file") != 0;
  
  char *hname = getenv("HOSTNAME");
  char *cname = getenv("COMPUTERNAME");
  if(hname != 0 && !existing)
    basetestfilename += "-" + std::string(hname) ;
  else {
    if(cname!=0 && !existing)
      basetestfilename += "-" + std::string(cname);
  }
  char* uname  = getenv("USER");
  char* uname2 = getenv("USERNAME");
  char* uname3 = getenv("LOGNAME");
  if(uname != 0 && !existing)
    basetestfilename += "-" + std::string(uname);
  else {
    if(uname2!=0 && !existing)
      basetestfilename += "-" + std::string(uname2);
    else {
      if(uname3!=0 && !existing)
      basetestfilename += "-" + std::string(uname3);
    }
  }
  
  flush                = vm.count("flush") != 0;
  use_direct           = vm.count("use-direct") != 0;
  keep                 = vm.count("remove-testfile") == 0;
  write_randomly       = vm.count("random") != 0 && (vm.count("same-file")==0 && vm.count("num-thread")==0);
  osync                = vm.count("osync") != 0;
  threads_on_same_file = vm.count("same-file") != 0;
  quiet                = vm.count("quiet") != 0;

  // if we want to rewrite a file, we specify only one (not multiple files)
  // then in multithreaded mode (-n #) we must switch to "all threads writing on the same file"-mode
  if(existing)
    if(num_of_threads>1)
      threads_on_same_file = true;

  if(threads_on_same_file && num_of_threads==1) {
    std::cout << "Warning: specified 1 thread and --same-file, which doesn't make sense with only one thread. Ignoring it..." << std::endl;
    threads_on_same_file = false;
  }
  if(vm.count("filesize")!=0 && vm.count("count")!=0 && vm.count("block-size")!=0) {
    std::cerr << "Must specify block-size & count, or block-size & filesize, or filesize & count. Not them all. Stop."<< std::endl;
    return 1;
  }

  if(filesize_s.back()=='k' || filesize_s.back()=='K') {
    std::string tmp = filesize_s.substr(0, filesize_s.length()-1);
    if(!utils::is_digits(tmp)) {
      std::cerr << "Specified filesize \"" << tmp << "\" is not a number "<< std::endl;
      return 1;
    }
    filesize = stoll(filesize_s.substr(0, filesize_s.length()-1)) * ONEKILO;
  }
  if(filesize_s.back()=='m' || filesize_s.back()=='M') {
    std::string tmp = filesize_s.substr(0, filesize_s.length()-1);
    if(!utils::is_digits(tmp)) {
      std::cerr << "Specified filesize \"" << tmp << "\" is not a number "<< std::endl;
      return 1;
    }
    
    filesize = stoll(filesize_s.substr(0, filesize_s.length()-1)) * ONEMEGA;
  }
  if(filesize_s.back()=='g' || filesize_s.back()=='G') {
    std::string tmp = filesize_s.substr(0, filesize_s.length()-1);
    if(!utils::is_digits(tmp)) {
      std::cerr << "Specified filesize \"" << tmp << "\" is not a number "<< std::endl;
      return 1;
    }
    filesize = stoll(filesize_s.substr(0, filesize_s.length()-1)) * ONEGIGA;
  }
  if(filesize_s.back()=='t' || filesize_s.back()=='T') {
    std::string tmp = filesize_s.substr(0, filesize_s.length()-1);
    if(!utils::is_digits(tmp)) {
      std::cerr << "Specified filesize \"" << tmp << "\" is not a number "<< std::endl;
      return 1;
    }
    filesize = stoll(filesize_s.substr(0, filesize_s.length()-1)) * ONETERA;
  }
  if(isdigit(filesize_s.back())) {
    if(!utils::is_digits(filesize_s)) {
      std::cerr << "Specified filesize \"" << filesize_s << "\" is not a number "<< std::endl;
      return 1;
    }
    filesize = stoll(filesize_s);
  }
    
    
  std::regex bslen_regex("^([0-9]+)([kmgt])$", std::regex_constants::ECMAScript | std::regex_constants::icase);
  std::smatch match;
  if (!std::regex_search(bslen_s, match, bslen_regex)) {
    std::cerr << "Specified bslen (--block-size|-b) " << bslen << " is not correct. Please see help." << std::endl;
    return 1;
  }
    
    
  if( bslen_s.back() == 'k' || bslen_s.back() == 'K' ) {
    std::string tmp = bslen_s.substr(0, bslen_s.length()-1);
    if(!utils::is_digits(tmp)) {
      std::cerr << "Specified blocksize \"" << tmp << "\" is not a number "<< std::endl;
      return 1;
    }
    bslen = stoll(tmp) * ONEKILO;
  }
  if( bslen_s.back() == 'm' || bslen_s.back() == 'M' ){
    std::string tmp = bslen_s.substr(0, bslen_s.length()-1);
    if(!utils::is_digits(tmp)) {
      std::cerr << "Specified blocksize \"" << tmp << "\" is not a number "<< std::endl;
      return 1;
    }
    bslen = stoll(tmp) * ONEMEGA;
  }
  if( bslen_s.back() == 'g' || bslen_s.back() == 'G' ) {
    std::string tmp = bslen_s.substr(0, bslen_s.length()-1);
    if(!utils::is_digits(tmp)) {
      std::cerr << "Specified blocksize \"" << tmp << "\" is not a number "<< std::endl;
      return 1;
    }
    bslen = stoll(tmp) * ONEGIGA;
  }
  if( bslen_s.back() == 't' || bslen_s.back() == 'T' ) {
    std::string tmp = bslen_s.substr(0, bslen_s.length()-1);
    if(!utils::is_digits(tmp)) {
      std::cerr << "Specified blocksize \"" << tmp << "\" is not a number "<< std::endl;
      return 1;
    }
    bslen = stoll(bslen_s.substr(0, bslen_s.length()-1)) * ONETERA;
  }
  if( isdigit(bslen_s.back()) ) {
    if(!utils::is_digits(bslen_s)) {
      std::cerr << "Specified blocksize \"" << bslen_s << "\" is not a number "<< std::endl;
      return 1;
    }
    bslen = stoll(bslen_s);
  }
  
  if(vm.count("filesize")!=0 && vm.count("count")!=0) {
    bslen = filesize / segcount;
  }
  
  if(vm.count("filesize")!=0 && vm.count("block-size")!=0) {
    segcount = filesize / bslen;
  }
  
  if(vm.count("count")!=0 && vm.count("block-size")!=0) {
    filesize = bslen * segcount;
  }
  
  if(!utils::check_power_of_two( bslen )) {
    std::cerr << "Block size must be a power of 2. Stop" << std::endl;
    return 1;
  }

  if(segcount < num_of_threads && threads_on_same_file) {
    std::cerr << "In same-file mode (all threads writing on the same file), count cannot be smaller than the number of threads. Stop" << std::endl;
    return 1;
  }

  if ( segcount % num_of_threads != 0 ) {
    std::cerr << "Please specify a filesize (or an explicit count) in such a way that filesize/blocksize (or the explicit count number) is integral multiple of the number of threads. Stop." << std::endl; 
    return 1;
  }
  
  if(!utils::check_multiple_of_512(bslen)) {
    std::cerr << "Block size must be multiple of 512. Stop." << std::endl;
    return 1;
  }

  if(block_delay>0 && threads_on_same_file && num_of_threads > 1) {
    std::cerr << "Multiple threads on same file (-S -n #) are incompatible with -B, because pre-emption makes threads operate in an random interleaved way that makes useless to wait between two subsequent block writes. Stop." << std::endl;
    return 1;
  }
  
  signal(20, killhandler);
  
  /**************************************************************
   *
   * Prepare memory buffer
   * 
   **************************************************************/
  char *name;
  name = (char*)malloc(1024);
  memset((void*)name, 0, 1024);
  gethostname(name, 1024);

  logger::init();
  logger::get()->set_quiet( quiet);
  logger::get()->log( std::string("Starting benchmark on host ") + name );
  logger::get()->log( "Malloc-ing " + std::to_string(bslen) + " bytes of memory for the block..." );
  posix_memalign((void**)&bs, 512, bslen);

  logger::get()->log( "Filling " + std::to_string(bslen) + " bytes of memory for the block..." );
  unsigned long long bef = utils::get_microseconds( );
  memset((void*)bs, ' ', bslen);
  unsigned long long aft = utils::get_microseconds( );
  unsigned long long deltamicros  = aft-bef;//aft.tv_sec * 1000000 + aft.tv_usec;
  logger::get()->log( std::to_string(deltamicros) + " microseconds spent to fill the memory block" );
  
  /**************************************************************
   *
   * Add required flag if selected by the user
   * 
   **************************************************************/
  flags = O_WRONLY;
  if(!existing)
    flags |= O_CREAT|O_TRUNC;
  if(osync)
    flags |= O_SYNC;
#ifndef __APPLE__
   if(use_direct) {
     flags |= O_DIRECT;
     logger::get()->log("Will bypass OS's cache (O_DIRECT).");
   }  
#endif
  /**************************************************************
   *
   * Prepare a std::vector of offsets
   * 
   **************************************************************/
   std::vector<off_t> offsets(segcount);
   off_t offt = 0;
   std::iota( offsets.begin(), offsets.end(), offt);

  /***********************************************************************
   *
   * Create a pool of writers that will span threads for parallel writing
   * 
   ***********************************************************************/
   logger::get()->log("Creating writer pool composed of " + std::to_string( num_of_threads ) + " thread(s)...");
   long long it = 0;
   if(threads_on_same_file)
     logger::get()->log("File size: " + utils::prettyPrintSize( filesize ) ) ;
   else
     logger::get()->log("Single file size: " + utils::prettyPrintSize( filesize ) + ", aggregated file size: " + utils::prettyPrintSize(filesize*num_of_threads)) ;
   while( it< iterations ) {
     logger::get()->log("Iteration #" + std::to_string(it+1) + " of " + std::to_string(iterations));
     ++it;
     writer_pool WP( num_of_threads, bs, bslen, flush );
     
     if(threads_on_same_file) {
       std::string filename;
       if(!existing)
	 filename = basetestfilename + "-" + std::to_string( getpid() ) + "-" + std::to_string(it);
       else
	 filename = basetestfilename;
       if(!absolute_filename.empty()) {
	 filename = absolute_filename;
       }
       if(!existing)
	 unlink(filename.c_str()); // let's remove it at the second and subsequent iterations
       logger::get()->log("Opening file [" + filename + "]");
       int fd = open(filename.c_str(), flags, S_IRUSR|S_IWUSR);
       logger::get()->log("Success: file [" + filename + "] -> descriptor [" + std::to_string(fd) + "]");
       if(fd<0) {
	 logger::get()->log("Error opening file [" + filename + "] for writing:" + strerror(errno));
	 return 1;
       }
       if(!existing)
	 to_unlink.push_back( filename );
       to_close.push_back( fd );
       manglefd(fd, use_direct);
       int chunks_per_thread = segcount/num_of_threads;
       for( RANGEui(j, num_of_threads)) {
	 std::vector<off_t> chunks( chunks_per_thread );
	 std::iota( begin(chunks), end(chunks), j*chunks_per_thread );
	 if( write_randomly )
	   utils::shuffle_vector( chunks );
	 WP.add_writer ( fd, chunks, block_delay, filename );
       }
      
    } else {
      /**
       * Each thread writes on its own file
       */
      for( RANGEui(j, num_of_threads)) {
	std::string filename;
	if(!existing)
	  filename = basetestfilename + "-" + std::to_string( getpid() ) + "-" + std::to_string(it) + "-" + std::to_string(j);
	else
	  filename = basetestfilename;
	if(!absolute_filename.empty()) {
	  filename = absolute_filename;
	}	
	if(!existing)
	  unlink(filename.c_str()); // let's remove it at the second and subsequent iterations
	logger::get()->log("Opening file [" + filename + "]");
      	int fd = open(filename.c_str(), flags, S_IRUSR|S_IWUSR);
	logger::get()->log("Success: file [" + filename + "] -> descriptor [" + std::to_string(fd) + "]");
      	if(fd<0) {
	  logger::get()->err("Error opening file [" + filename + "]:" + strerror(errno));
      	  return 1;
      	}
	if(!existing)
	  to_unlink.push_back( filename );
	to_close.push_back(fd);
	manglefd(fd, use_direct);
	if( write_randomly )
	  utils::shuffle_vector( offsets );
      	WP.add_writer ( fd, offsets, block_delay, filename );
      }
    }
    WP.start();
    WP.join();
    
    if(!WP.get_status()) {
      logger::get()->err("Error: " + WP.get_error_message( ));
      return 1;
    }
    
    WP.compute_perf();
    time_info.push_back( WP.get_perf() );
    for(auto f : to_close) {
      logger::get()->log("Closing file descriptor [" + std::to_string(f) + "]");
      close(f);
    }
    to_close.clear();
    std::this_thread::sleep_for(std::chrono::milliseconds(iteration_delay));
    if(!keep)
      for(auto cit : to_unlink)
	unlink(cit.c_str());
      
  } // loop over iterations
  
  /******************************************************************
   *
   * Final report
   * 
   * time_info contains as many elements as the number of iterations
   *
   ******************************************************************/
  float max_of_max = time_info.begin()->rate_max;
  float min_of_min = time_info.begin()->rate_min;
  float avg_of_avg = 0.0;

  for(auto perfObj: time_info) {
    if(max_of_max < perfObj.rate_max) max_of_max = perfObj.rate_max;
    if(min_of_min > perfObj.rate_min) min_of_min = perfObj.rate_min;
    avg_of_avg += perfObj.rate_avg;
  }
  avg_of_avg /= time_info.size();
  
  double stddev = 0.0;

  for(auto perfObj : time_info)
    stddev+=pow(perfObj.rate_avg - avg_of_avg,2);
  
  stddev /= (iterations-1);
  stddev = sqrt(stddev);

  if(iterations==1) {
    if(num_of_threads==1) { // 1 iteration, 1 thread
      std::cout << "Results for the write of " << utils::prettyPrintSize(bslen*segcount) << std::endl;
      std::cout << " - Speed: " << utils::prettyPrintSize(time_info.begin()->rate_avg) +"/s" << std::endl;
      std::cout << " - Time : " << (time_info.begin()->delta_avg/1000) << " milli-seconds" << std::endl;
    } else { // 1 iteration, more threads
      std::cout << "Results for the write of "
		<<  utils::prettyPrintSize(bslen*segcount)
		<< " ("
		<< utils::prettyPrintSize(threads_on_same_file ? bslen*segcount : bslen*segcount*num_of_threads)
		<< " aggregated from " << num_of_threads <<" threads)" << std::endl;
      
      std::cout << " - Max speed   : " << utils::prettyPrintSize(max_of_max) +"/s" << std::endl;
      std::cout << " - Min speed   : " << utils::prettyPrintSize(min_of_min) +"/s" << std::endl;
      std::cout << " - Avg speed   : " << utils::prettyPrintSize(avg_of_avg) +"/s" << std::endl;
      
      std::cout << " - StdDev speed: " << utils::prettyPrintSize(time_info.begin()->rate_stddev) +"/s" << std::endl;
      std::cout << std::endl 
		<< " - Aggregated avg speed      : " << utils::prettyPrintSize(avg_of_avg*num_of_threads) +"/s" << std::endl;
    }
  } else {
    if(num_of_threads==1) { // more iterations, 1 thread
      std::cout << "Average result for the write of "
		<< utils::prettyPrintSize(segcount*bslen)
		<< std::endl;
      
      std::cout << " - Max speed   : " << utils::prettyPrintSize(max_of_max) +"/s" << std::endl;
      std::cout << " - Min speed   : " << utils::prettyPrintSize(min_of_min) +"/s" << std::endl;
      std::cout << " - Avg speed   : " << utils::prettyPrintSize(avg_of_avg) +"/s" << std::endl;
      std::cout << " - StdDev speed: " << utils::prettyPrintSize(stddev) +"/s" << std::endl;
    } else { // more iterations, more threads
      std::cout << "Results for the write of "
		<< utils::prettyPrintSize(segcount*bslen)
		<< " ("
		<< utils::prettyPrintSize(segcount*bslen*num_of_threads)
		<< " aggregated from " << num_of_threads <<" threads)" << std::endl;
      
      std::cout << " - Max speed   : " << utils::prettyPrintSize(max_of_max) +"/s" << std::endl;
      std::cout << " - Min speed   : " << utils::prettyPrintSize(min_of_min) +"/s" << std::endl;
      std::cout << " - Avg speed   : " << utils::prettyPrintSize(avg_of_avg) +"/s" << std::endl;
      std::cout << " - StdDev speed: " << utils::prettyPrintSize(stddev) +"/s" << std::endl;
      std::cout << std::endl 
		<< " - Aggregated avg speed      : " << utils::prettyPrintSize(avg_of_avg*num_of_threads) +"/s" << std::endl;
    }
  }
  
  // if(!keep)
  //   for(auto cit : to_unlink)
  //     unlink(cit.c_str());
}

//------------------------------------------------------------------------------------------------------------------
void manglefd( int fd, bool use_direct ) {
#ifdef  __APPLE__
    if(use_direct) {
      fcntl(fd, F_NOCACHE, 1);
      logger::get()->log("Will not use OS's buffer (F_NOCACHE) - Apple specific.");
    } else fcntl(fd, F_NOCACHE, 0);
#endif
}
