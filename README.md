WORK IN PROGRESS

This is a tool to bench filesystem with standard posix calls for write; 
pwrite is used in order to be thread-safe, so please check your system does 
support it.

To build it you need to install cmake, g++, make and boost-devel packages:
```
git clone git@git.psi.ch:dorigo_a/dbench.git
cd dbench
cmake .
make
make install
```
it will install the executable dbench in `/usr/local/bin`.

The accepted options are:
```
dbench -h
Usage: /usr/local/bin/dbench [OPTIONS]

Options:

--block-size|b #		Specifies the size of a block which is written at once. Can use k|K|m|M|g|G suffixes
--block-delay|B #		Specifies the number of microseconds to sleep between a syscall pwrite and the next one (usefull to artificially slow down the writing speed)
--filesize|D #			Specifies the file's size. Can use k|K|m|M|g|G suffixes
--existing-file|e		If want to write to an existing file use this flag
--count|c #			    Specifies the number of blocks to be written
--flush|F			    Specifies if take into accout the fsync() syscall in the time calculation for the throughput
--use-direct|d			Specifies if open the file with O_DIRECT flag
--test-filename|f <path> Specifes path and filename to be written for the test
--iterations|i #		Specifies the number of tests on which the final value will be averaged
--iteration-delay|I #	Specifies the number of milliseconds to sleep between an iteration and the next one
--pre-allocate|p #		Pre-allocate disk space before start writing
--random|R			    Perform random writing instead of sequential
--osync|s			    Use O_SYNC flag when opening the file 
--num-threads|n #		Spawn a number of thread, each one writing its own file
--same-file|S			Multiple threads write on the same file
--remove-testfile|k		Remove the testfile
--quiet|Q		        Quiet log mode
```

By default the tool creates a file in the current working directory with this 
schema for filename:
```
<BASENAME>-<HOSTNAME>-<USERNAME>-<PROCESS_ID>-<ITERATION>-<THREAD_NUMBER>
```
By default `<BASENAME>=./testfile` and can be overwritten by specifying a 
custom argument with `--test-filename` (or `-f`). E.g.:
```
dbench --test-filename /mnt/NFS/nfs_test-myfirsttest
```
that will generate a test file like
```
/mnt/NFS/nfs_test-myfirsttest-mylaptop.myhouse.ch-dorigo_a-4571-1-0
```
This option can be used to test different filesystems mounted on different paths 
 without necessarily change directory.

