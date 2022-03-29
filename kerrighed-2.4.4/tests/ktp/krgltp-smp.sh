#!/bin/bash

# Default values for arguments

#onetest=sendfile01
let nr_ps=16
let nr_run=1

BASETEST=${libdir}/debian-test/tests/linux/testcases/bin
PATH=${PATH}:${libdir}/debian-test/tests/linux/tools:$BASETEST


###############################
############################### FUNCTIONS DEFINITION
###############################


# Check for possible prerequisites
function check_environment(){
	# creat08
	if ! grep -qs ^nobody: /etc/group ; then
		echo 'Unable to find "nobody" in /etc/group.'
		echo 'Needed for at least: creat08'
		exit 1
	fi

	# getdtablesize01
	if [ ! -f /etc/hosts ]; then
		echo 'Unable to find the /etc/hosts file.'
		echo 'Needed for at least: getdtablesize01'
		exit 1
	fi
}

# Tell which loop of all tests is currently running
function print_progress(){
	local run_nr

	nr_arg=
	if [ -z "$onetest" ]; then
		let run_nr=nr+1
		nr_arg="run $run_nr/$nr_run "
	fi
	echo
	echo -n "$nr_arg"
}

# Start all concurrent instances of a given test command in separate
# working directories
# The pids of the commands are stored in the pid[] array.
function start_all_instances() {
	local i

	# Load any background pipe in its own pgrp
	set -m
	for i in `seq 1 $nr_ps`; do
		$BASETEST/$@ &
		pid[$i]=$!
	done
	# Restore default behavior in non-interactive shell
	set +m
}

# run one cmd
# runtest cmd cmdopt1 cmdopt2 ...
function runtest(){
	local i

	print_progress
	echo "*** $@ ***"
	start_all_instances $@
	for i in `seq 1 $nr_ps`; do
		wait ${pid[$i]} || exit 0
	done
	let nbpassed=nbpassed+1
}

#run a set of cmd (cmd01, cmd02, ...) using the same options
# runtests valmin valmax cmd cmdopt1 cmdopt2 ...
function runtests(){
	local i

	for i in `seq -f "%02g" $1 $2`; do
		runtest $3$i `echo $@ | cut -d' ' -f4-`
	done
}

# Run a test and accept a warning. This is mainly used for tests which warn
# on a .nfs### file. This comes from the behavior of NFS in the presence of
# a file removed since still being opened. Some tests do that...
function rwarn(){
	local i

	print_progress
	echo "*** $@ *** expecting WARN due to NFS"
	start_all_instances $@
	for i in `seq 1 $nr_ps`; do
		wait ${pid[$i]}
		res=$?
		if [ "$res" -ne 4 ] && [ "$res" -ne 0 ]; then
		    exit 0
		fi
	done

	let nbpassed=nbpassed+1
}

#failing test: test known to fail
function rfail(){
	local i

	print_progress
	echo "*** $@ *** expecting FAIL"
	start_all_instances $@
	for i in `seq 1 $nr_ps`; do
		wait ${pid[$i]}
	done
	let nbfailed=nbfailed+1
}

# Test only running if launched alone.
function rsingle(){
	echo "*** $@ *** SINGLE run"

	# Load any background pipe in its own pgrp
	set -m
	$BASETEST/$@ || exit 0
	# Restore default behavior in non-interactive shell
	set +m
	let nbpassed=nbpassed+1
}

#crashing test: test known to crash
function rcrash(){
	print_progress
	echo "*** $@ *** expecting CRASH ! Test not executed"
	let nbcrash=nbcrash+1
}

#running test: test supposed to be ok
function r(){
	runtest $@
}

# Cleanup some directory not correctly removed by LTP
function cleanup_dirs () {
	for pat in dlinkattestdir drenameattestdir faccessattestdir fchmodattestdir fchownattestdir fstatattestdir futimesattestdir linkattestdir open10.testdir. openattestdir readlinkattestdir symlinkattestdir testdir. unlinkattestdir waitpid14.; do
		rm -rf $pat*
	done
}

function usage(){
	name=`basename $0`
	echo "usage: $name [-t] [-n <nr_loops>] [-p <nr_process>]]"
	echo "           Run all tests <nr_loops> times (default $nr_run), each test being run with <nr_process> concurrent instances (default $nr_ps)."
	echo
	echo "       $name [-n <nr_loops>] -s <testname>"
	echo "           Run test <testname> (<nr_loops> times, default is 1)"
	echo
	echo "       $name -b <testname>"
	echo "           Run test <testname> in an infinite loop"
	echo
	echo "       $name -t"
	echo "           Run test with no distant action to mesure basic execution time"
	echo
	echo "       $name -h"
	echo "           Show this help message"

}

############################### MAIN TEST FUNCTION

# Run all the tests
# main <nr_run>
function main()
{
let nr=0
while [ "$nr" -lt "$1" ]; do

echo "KRGLTP: run #$nr" >> /tmp/krgltp-slabinfo.dat
cat /proc/slabinfo >> /tmp/krgltp-slabinfo.dat

cleanup_dirs

let nbpassed=0
let nbfailed=0
let nbcrash=0

# flat list of test

############################ Abort

ulimit -c 1024; r abort01

############################ Accept

r accept01

############################ Access

runtests 1 5 access

############################ Acct

rwarn acct01
r acct02

############################ Adjtimex

runtests 1 2 adjtimex

############################ Alarm

runtests 1 3 alarm
runtests 5 7 alarm

############################ Asyncio

r asyncio02

############################ Bind

runtests 1 2 bind

############################ Brk

r brk01

############################ Capget

runtests 1 2 capget

############################ Capset

runtests 1 2 capset

############################ Chdir

r chdir01
r symlink01 -T chdir01
runtests 2 4 chdir

############################ Chmod

runtests 1 4 chmod
cp -p $BASETEST/change_owner $PWD; r chmod05
cp -p $BASETEST/change_owner $PWD; r chmod06
r chmod07
r symlink01 -T chmod01

############################ Chown

runtests 1 2 chown
export change_owner=$BASETEST/change_owner ; r chown03
cp -p $BASETEST/change_owner $PWD; r chown04
r chown05

############################ Chroot

runtests 1 4 chroot

############################ Clone

runtests 1 7 clone

############################ Close

runtests 1 2 close
r close08

############################ Confstr

r confstr01

############################ Connect

r connect01

############################ Creat

r creat01
runtests 3 6 creat
cp -p $BASETEST/test1 $PWD; rfail creat07 -F test1 # NFS problem.
                            # Don't know remotely that a file is being executed
runtests 8 9 creat

############################ Dup

runtests 1 7 dup

############################ Dup2

runtests 1 5 dup2

############################ Exec

r execl01
r execle01
r execlp01
r execv01

r execve01
r execve02 -F $BASETEST/test3
r execve03
cp -f $BASETEST/test3 $PWD; rwarn execve04 -F test3  # Unknown problem.
rfail execve05 -F $BASETEST/test3                    # NFS problem
                          # Don't know remotely that a file is open on write
r execve06 20 $BASETEST/execve06 $BASETEST/execve06 4

r execvp01

############################ Exit

runtests 1 2 exit

############################ Faccessat

r faccessat01

############################ Fadvise (posix)

runtests posix_fadvise 1 4
r posix_fadvise01_64
r posix_fadvise02_64
r posix_fadvise03_64
r posix_fadvise04_64

############################ Fallocate

rfail fallocate01                # Need kernel >= 2.6.23
rfail fallocate02                # Need kernel >= 2.6.23
rfail fallocate03                # Need kernel >= 2.6.23

############################ Fchdir

runtests 1 3 fchdir

############################ Fchmod

runtests 1 4 fchmod
cp -p $BASETEST/change_owner $PWD; r fchmod05
cp -p $BASETEST/change_owner $PWD; r fchmod06
r fchmod07

############################ Fchmodat

r fchmodat01

############################ Fchown

runtests 1 2 fchown
cp -p $BASETEST/change_owner $PWD; r fchown03
export change_owner=$BASETEST/change_owner; r fchown04
r fchown05

############################ Fchownat

r fchownat01

############################ Fcntl

runtests 1 10 fcntl
rfail fcntl11          # NFS locking problem.
r fcntl12
r fcntl13
rfail fcntl14          # NFS locking problem.
rfail fcntl15          # NFS locking problem.

# BUG(LTP) r fcntl16
rfail fcntl17          # NFS locking problem.
r fcntl18
rfail fcntl19          # NFS locking problem.
rfail fcntl20          # NFS locking problem.
rfail fcntl21          # NFS locking problem.
rfail fcntl22          # NFS locking problem.
r fcntl23
rfail fcntl24          # NFS locking problem.
rfail fcntl25          # NFS locking problem.
rfail fcntl26          # NFS locking problem.
# runtests 27 28 fcntl	Disabled by ltp
r fcntl07B

############################ Fdatasync

runtests 1 2 fdatasync

############################ Flock

runtests 1 2 flock
rfail flock03     # NFS locking problem.
rfail flock04     # NFS locking problem.
rfail flock05     # NFS locking problem.
r flock06

############################ Fmtmsg

r fmtmsg01

############################ Fork

runtests 1 11 fork
# BUG(Linux) r fork12

############################ Fpathconf

r fpathconf01

############################ Fstat

runtests 1 5 fstat

############################ Fstatat

r fstatat01

############################ Fstatfs

runtests 1 2 fstatfs

############################ Fsync

runtests 1 3 fsync

############################ Ftruncate

runtests 1 3 ftruncate
rfail ftruncate04         # Cannot be run over NFS

############################ Futimesat

r futimesat01

############################ Getcontext

r getcontext01

############################ Getcwd

runtests 1 3 getcwd

############################ Getdents

runtests 1 4 getdents

############################ Getdomainname

r getdomainname01

############################ Getdtablesize

r getdtablesize01

############################ Getegid

r getegid01

############################ Geteuid

r geteuid01

############################ Getgid

runtests 1 3 getgid

############################ Getgroups

runtests 1 4 getgroups

############################ Gethostid

rfail gethostid01        # Kerrighed issue. Host id not global.

############################ Gethostname

r gethostname01

############################ Getitimer

runtests 1 3 getitimer

############################ Getpagesize

r getpagesize01

############################ Getpeername

r getpeername01

############################ Getpgid

runtests 1 2 getpgid

############################ Getpgrp

r getpgrp01

############################ Getpid

runtests 1 2 getpid

############################ Getppid

runtests 1 2 getppid

############################ Getpriority

runtests 1 2 getpriority

############################ Getresgid

runtests 1 3 getresgid

############################ Getresuid

runtests 1 3 getresuid

############################ Getrlimit

runtests 1 2 getrlimit

############################ Getrusage

runtests 1 2 getrusage

############################ Ggetsid

runtests 1 2 getsid

############################ Getsockname

r getsockname01

############################ Getsockopt

r getsockopt01

############################ Gettimeofday

runtests 1 2 gettimeofday

############################ Gettid

r gettid01

############################ Getuid

runtests 1 3 getuid


############################ Ioctl

r ioctl01 -D /dev/tty0  # NFS .something file remaning in the dir to remove
r ioctl02 -D /dev/tty0

############################ Inotify

r inotify01
rfail inotify02         # Broken in LTP

############################ Ioperm

runtests 1 2 ioperm

############################ Iopl

runtests 1 2 iopl

############################ IPC

# Check sub-directories

############################ Kill

runtests 1 10 kill

ulimit -c 1024; r kill11
r kill12

############################ Lchown

r lchown01
cp -p $BASETEST/create_link $PWD; chmod u+s $PWD/create_link; r lchown02

############################ Link

r symlink01 -T link01
runtests 2 7 link

############################ Linkat

rfail linkat01            # Unknown problem.

############################ Listen

r listen01

############################ Llseek

runtests 1 2 llseek

############################ Lseek

runtests 1 10 lseek

############################ Lstat

r symlink01 -T lstat01
runtests 1 3 lstat

############################ madvise

runtests 1 2 madvise

############################ Mallopt

r mallopt01

############################ Memcmp

r memcmp01

############################ Memcpy

r memcpy01

############################ Memmap

r mem03

############################ Memset

r memset01

############################ Mincore

runtests 1 2 mincore

############################ Mkdir

runtests 1 5 mkdir
r symlink01 -T mkdir05
runtests 8 9 mkdir

############################ Mkdirat

r mkdirat01

############################ Mknod

runtests 1 9 mknod

############################ Mlock

runtests 1 2 mlock

############################ Mlockall

runtests 1 3 mlockall

############################ Mmap

r mmap001 -m 1
runtests 1 9 mmap
# Have a look at mmapstress

############################ Modify_ldt

runtests 1 2 modify_ldt

############################ Mount

# Check more

############################ Move_pages

rfail move_pages.sh 01          # Need NUMA support in the kernel
rfail move_pages.sh 02          # Need NUMA support in the kernel
rfail move_pages.sh 03          # Need NUMA support in the kernel
rfail move_pages.sh 04          # Need NUMA support in the kernel
rfail move_pages.sh 05          # Need NUMA support in the kernel
rfail move_pages.sh 06          # Need NUMA support in the kernel
rfail move_pages.sh 07          # Need NUMA support in the kernel
rfail move_pages.sh 08          # Need NUMA support in the kernel
rfail move_pages.sh 09          # Need NUMA support in the kernel
rfail move_pages.sh 10          # Need NUMA support in the kernel
rfail move_pages.sh 11          # Need NUMA support in the kernel

############################ Mprotect

runtests 1 3 mprotect

############################ Mremap

runtests 1 4 mremap

############################ Msgctl

runtests 1 2 msgctl
rfail msgctl03       # Kerrighed issue.
r msgctl04
rfail msgctl05       # caused by usage of current
runtests 6 7 msgctl
rsingle msgctl08     # This test is already a SMP stress test
rsingle msgctl09     # This test is already a SMP stress test

############################ Msgget

runtests 1 2 msgget
rsingle msgget03     # LTP issue. Does not support concurency by design.
r msgget04

############################ Msgrcv

runtests 1 6 msgrcv

############################ Msgsnd

runtests 1 6 msgsnd

############################ Msync

runtests 1 5 msync

############################ Munlock

runtests 1 2 munlock

############################ Munlockall

r munlockall01
# CHECK r munlockall02

############################ Munmap

runtests 1 3 munmap

############################ Nanosleep

r nanosleep01
rfail nanosleep02      # May fail due to weak sync with hard-coded sleep of 1s.
runtests 3 4 nanosleep

############################ Nftw

r nftw01
r nftw6401

############################ Nice

runtests 1 5 nice

############################ Open

r symlink01 -T open01
runtests 1 9 open
rfail open10           # NFS V3 problem

############################ Openat

r openat01

############################ Paging

# Check this

############################ Pathconf

r pathconf01

############################ Pause

runtests 1 3 pause    # I had casual issues with pause02 and pause03

############################ Personality

runtests 1 2 personality

############################ Pipe, etc

runtests 1 6 pipe
rfail pipe07      # Kerrighed issue.
                  # Fails because /proc/<pid>/fd in not filled on remote nodes.
runtests 8 11 pipe

############################ Poll

r poll01

############################ Prctl

runtests 1 2 prctl

############################ Pread

runtests 1 3 pread

############################ Profil

r profil01

############################ Pselect

r pselect01

############################ Ptrace

rfail ptrace01  # Kerrighed issue. Does not support ptrace.
rfail ptrace02  # Kerrighed issue. Does not support ptrace.
rfail ptrace03  # Kerrighed issue. Does not support ptrace.

############################ Pwrite

runtests 1 4 pwrite

############################ Read

runtests 1 4 read

############################ Readdir

runtests 1 2 readdir

############################ Readlink

r symlink01 -T readlink01
runtests 1 3 readlink
cp -f $BASETEST/creat_slink $PWD; r readlink04

############################ Readlinkat

r readlinkat01

############################ Readv

runtests 1 3 readv

############################ Reboot

runtests 1 2 reboot

############################ Recv

rfail recv01         # Unknown problem...

############################ Recvfrom

rfail recvfrom01     # Unknown problem...

############################ Recvmsg

rcrash recvmsg01       # User level deadlock

############################ Remap_file_pages

r remap_file_pages01
r remap_file_pages02

############################ Rename

r symlink01 -T rename01
runtests 1 10 rename
runtests 12 14 rename

############################ Renameat

r renameat01

############################ Rmdir

runtests 1 2 rmdir
r symlink01 -T rmdir03
runtests 3 5 rmdir

############################ Sbrk

r sbrk01

############################ Sched_getparam

runtests 1 3 sched_getparam

############################ Sched_get_priority_max

runtests 1 2 sched_get_priority_max

############################ Sched_get_priority_min

runtests 1 2 sched_get_priority_min

############################ Sched_getscheduler

runtests 1 2 sched_getscheduler

############################ Sched_rr_get_interval

runtests 1 3 sched_rr_get_interval

############################ Sched_setparam

runtests 1 5 sched_setparam

############################ Sched_setscheduler

runtests 1 2 sched_setscheduler

############################ Sched_yield

r sched_yield01

############################ Select

runtests 1 3 select

############################ Semctl

rfail semctl01
runtests 2 7 semctl

############################ Semget

runtests 1 3 semget
rsingle semget05      # Does not support concurrency by design
r semget06

############################ Semop

runtests 1 5 semop

############################
# the following tests are part of LTP but not syscalls part
# the display format is not equal but it's interesting to test it too
#r sem01
#r sem02
#r semaphore_test_01
#r semaphore_test_02
#r semaphore_test_03

############################ Send

r send01

############################ Sendfile

r sendfile02
r sendfile03
r sendfile04
r sendfile05
r sendfile06
r sendfile07

r sendfile02_64
r sendfile03_64
r sendfile04_64
r sendfile05_64
r sendfile06_64
r sendfile07_64

############################ Sendmsg

rwarn sendmsg01

############################ Sendto

r sendto01

############################ Setdomainname

runtests 1 3 setdomainname

############################ Setegid

r setegid01

############################ Setfsgid

runtests 1 3 setfsgid

############################ Setfsuid

runtests 1 4 setfsuid

############################ Setgid

runtests 1 3 setgid

############################ Setgroups

runtests 1 4 setgroups

############################ Sethostname

runtests 1 3 sethostname

############################ Setitimer

runtests 1 3 setitimer

############################ Setpgid

runtests 1 3 setpgid

############################ Setpgrp

runtests 1 2 setpgrp

############################ Setpriority

runtests 1 5 setpriority

############################ Setregid

runtests 1 4 setregid

############################ Setresgid

runtests 1 3 setresgid

############################ Setresuid

runtests 1 4 setresuid

############################ Setreuid

runtests 1 7 setreuid

############################ Setrlimit

runtests 1 3 setrlimit

############################ Setsid

rfail setsid01	  # Kerrighed issue. Limitation of global setpgid

############################ Setsockopt

r setsockopt01

############################ Settimeofday

runtests 1 2 settimeofday

############################ Setuid

runtests 1 4 setuid

############################ Shmat

runtests 1 3 shmat

############################ Shmctl

runtests 1 4 shmctl

############################ Shmdt

runtests 1 2 shmdt

############################ Shmget

runtests 1 2 shmget
rsingle shmget03   # Test does not support concurrency by design.
runtests 4 5 shmget

ipcs # Just check ipcs does not crash.

############################ Sigaction

runtests 1 2 sigaction

############################ Sigaltstack

runtests 1 2 sigaltstack

############################ Sighold

r sighold02

############################ Signal

runtests 1 5 signal

############################ Sigpending

r sigpending02

############################ Sigprocmask

r sigprocmask01

############################ Sigrelse

r sigrelse01

############################ Sigsuspend

r sigsuspend01

############################ Socket

r socket01

############################ Socketcall

runtests 1 4 socketcall

############################ Socketpair

r socketpair01

############################ Sockioctl

r sockioctl01

############################ Splice

rfail splice01        # Unknown problem.
rfail tee01           # Unknown problem.

############################ Stat

runtests 1 3 stat
r symlink01 -T stat04
runtests 5 6 stat

############################ Statfs

runtests 1 3 statfs

############################ Statvfs

r statvfs01

############################ String

r string01

############################ Swapoff

# WARN runtests 1 2 swapoff

############################ Swapon

rfail swapon01	           # Cannot run on top of NFS
rfail swapon02	           # Cannot run on top of NFS
rfail swapon03             # Cannot run on top of NFS

############################ Symlink

runtests 1 5 symlink

############################ Symlinkat

r symlinkat01

############################ Sync

runtests 1 2 sync

############################ Syscall

r syscall01

############################ Sysconf

r sysconf01

############################ Sysctl

r sysctl01
runtests 3 5 sysctl

############################ Sysinfo

runtests 1 2 sysinfo

############################ Syslog

rfail syslog01
rfail syslog02
rfail syslog03
rfail syslog04
rfail syslog05
rfail syslog06
rfail syslog07
rfail syslog08
rfail syslog09
rfail syslog10
runtests 11 12 syslog

############################ Timerfd

rfail timerfd01            # Need kernel >= 2.6.25

############################ Times

r times01
r times03

############################ Truncate

runtests 1 4 truncate

############################ Umask

runtests 1 3 umask

############################ Uname

runtests 1 3 uname

############################ Unlink

r symlink01 -T unlink01
runtests 5 8 unlink

############################ Unlinkat

r unlinkat01

############################ Ustat

runtests 1 2 ustat

############################ Utime

r symlink01 -T utime01
# BUG runtests 1 3 utime	NFS
runtests 4 6 utime

############################ Utimenstat

rfail utimenstat_tests.sh        # Need kernel >= 2.6.22

############################ Vfork

runtests 1 2 vfork

############################ Vhangup

runtests 1 2 vhangup

############################ Vmsplice

rfail vmsplice01   # NFS problem.

############################ Wait

r wait02

runtests 1 2 wait4

runtests 1 9 waitpid
r waitpid10 5
runtests 11 12 waitpid
rcrash waitpid13              # User level dead-lock

############################ Write

runtests 1 5 write

############################ Writev

runtests 1 6 writev

############################
# tests created for Kerrighed
############################

############################ Checkpoint/Restart

rsingle cr-pre # fake test to prepare C/R tests
r cr01
#rsingle cr02 #NFS...
#r cr03 # scenario included in cr06
r cr04
#r cr05 # scenario included in cr10
r cr06
#r cr07 # scenario included in cr09 and cr10
runtests 8 10 cr
######## following tests are running single mode currently
#rsingle cr11 # scenario included in cr14 and cr15
#rsingle cr12 # scenario included in cr16
rsingle cr13
rsingle cr14
rsingle cr15
rsingle cr16
#rsingle cr17 # scenario included in cr23
rsingle cr18
#rsingle cr19 # scenario included in cr21
#rsingle cr20 # scenario included in cr22
rsingle cr21
rsingle cr22
rsingle cr23
rsingle cr24
r cr_abort01
r cr_abort02
r cr_tree01
rsingle cr_tree02
r cr_signal01
r cr_clone_files01
r cr_clone_fs01
r cr_clone_semundo01
r cr_thread01
########

rsingle cr-post # fake test to clean results of C/R tests

let nr=nr+1
done

echo "KRGLTP: run #$nr_run" >> /tmp/krgltp-slabinfo.dat
cat /proc/slabinfo >> /tmp/krgltp-slabinfo.dat

cleanup_dirs
}

# Run all the tests and display a report
# run_tests <nr_run>
function run_tests(){
	check_environment
	time main $1

	let total=nbpassed+nbfailed+nbcrash

	echo "*** krgcapset options: none"
	echo "*** nr loops: $nr"
	echo "*** "
	echo "*** Execution report"
	echo "***     - passed: $nbpassed"
	echo "***     - failed: $nbfailed"
	echo "***     - crash:  $nbcrash"
	echo "***     - total:  $total"
}

###############################
############################### SCRIPT CORE
###############################


# Parse args
while getopts 'b:ts:n:p:h' name; do
	case "$name" in
	b)	onetest="$OPTARG"
		infiniteloop="yes"
		;;
	t)
		basictime="yes"
		;;
	s)
		onetest="$OPTARG"
		;;
	n)
		nr_run=$OPTARG
		;;
	p)
		nr_ps=$OPTARG
		;;
	h)
		usage; exit 0
		;;
	*)
		usage; exit 1
		;;
	esac
done

let nr_shift=$OPTIND-1
shift $nr_shift

# Fix broken installs
chmod u+s $BASETEST/change_owner
chmod u+s $BASETEST/create_link

rm -f /tmp/krgltp-slabinfo.dat
echo "KRGLTP: nbrun=$nr_run" > /tmp/krgltp-slabinfo.dat

TIMEFORMAT=$'\n\n*** Total execution time: %lR (%R seconds)'

# Check if we want to mesure basic execution time
if [ -n "$basictime" ]; then
	run_tests 1
	exit 0
fi

slab-check

# Use the distant fork feature
krgcapset -d +DISTANT_FORK || exit 0

export KTP_NR_PS=$nr_ps

# Check if we just want a single test
if [ ! -z "$onetest" ]; then
	if [ -z "$infiniteloop" ]; then
		for i in $(seq 1 $nr_run); do
			echo
			echo "[ITERATION $i/$nr_run starts...]"
			r $onetest $@
		done
		exit 0
	else
		i=0
		while /bin/true; do
			i=$((i+1))
			echo
			echo "[ITERATION $i/infinity starts...]"
			r $onetest $@
		done
	fi
else
	run_tests $nr_run
	echo "***"
	echo "*** Memory leak checking"
	slab-check
fi
