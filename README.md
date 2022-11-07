
# log_mt_test

This app is designed to be able to reproduce a problem that should be
fixed with pull the request [#464](https://github.com/KjellKod/g3log/pull/464)
of [g3log](https://github.com/KjellKod/g3log).  
The first goal was to be able to reproduce the same execution conditions as the real
app in terms of multi-threading, but it appears that is not necessary. The
problem is reproducable with 3 threads:

* the main thread,
* the g3log active thread
* a thread that calls **abort()**.

## Fetch sources

Execute:
`$ git clone --recurse-submodules https://github.com/ablangy/log_mt_test.git`  

By default, g3log & g3sinks repositories are on *master* branch.  

## Build sources

Go to *log_mt_test* directory.  
Execute:

* `$ cmake -S . -B ./build`  
* `$ cmake --build ./build -j 6`

That can be adapted to your needs.  

## Reproduce the issue

*log_mt_test* must be compiled from scratch with g3log & g3sinks pointing to `master` branch  
Execute:  
`$ ./build/Debug/bin/logMtApp -f tapp -d /tmp`

The app should terminate with similar output:  
```
2022/11/07 12:42:23 230086

***** FATAL SIGNAL RECEIVED *******
Received fatal signal: SIGABRT(6)       PID: 24986

***** SIGNAL SIGABRT(6)

******* STACKDUMP *******
        stack dump [1]  /home/ablangy/workspace-github/log_mt_test/build/Debug/g3log/lib/libg3log.so.1+0x1630a [0x7f57597de30a]
        stack dump [2]  /lib64/libc.so.6+0x3ea30 [0x7f575903ea30]
        stack dump [3]  /lib64/libc.so.6+0x8ec0c [0x7f575908ec0c]
        stack dump [4]  /lib64/libc.so.6raise+0x16 [0x7f575903e986]
        stack dump [5]  /lib64/libc.so.6abort+0xcf [0x7f57590287f4]
        stack dump [6]  ./build/Debug/bin/logMtApp() [0x407a97]
        stack dump [7]  ./build/Debug/bin/logMtApp() [0x408bce]
        stack dump [8]  ./build/Debug/bin/logMtApp() [0x408ab6]
        stack dump [9]  ./build/Debug/bin/logMtApp() [0x4089b9]
        stack dump [10]  ./build/Debug/bin/logMtApp() [0x40aaac]
        stack dump [11]  ./build/Debug/bin/logMtApp() [0x419f45]
        stack dump [12]  ./build/Debug/bin/logMtApp() [0x419c35]
        stack dump [13]  ./build/Debug/bin/logMtApp() [0x41a2a8]
        stack dump [14]  ./build/Debug/bin/logMtApp() [0x41a26b]
        stack dump [15]  ./build/Debug/bin/logMtApp() [0x41a218]
        stack dump [16]  ./build/Debug/bin/logMtApp() [0x41a1ec]
        stack dump [17]  ./build/Debug/bin/logMtApp() [0x41a1d0]
        stack dump [18]  /lib64/libstdc++.so.6+0xdbb03 [0x7f57594dbb03]
        stack dump [19]  /lib64/libc.so.6+0x8cded [0x7f575908cded]
        stack dump [20]  /lib64/libc.so.6+0x112370 [0x7f5759112370]

Exiting after fatal event  (FATAL_SIGNAL). Fatal type:  SIGABRT
Log content flushed successfully to sink



Exiting, log location: /tmp/tapp.log


exitWithDefaultSignalHandler:230. Exiting due to FATAL_SIGNAL, 6

Abandon (core dumped)
```

Open the core, you should see 3 threads:  
```
gdb$ info threads
  Id   Target Id                         Frame
* 1    Thread 0x7f4c1c240740 (LWP 25989) 0x00007f4c1bc3f6ea in sigtimedwait () from /lib64/libc.so.6
  2    Thread 0x7f4c1bbff640 (LWP 25990) 0x00007f4c1c249980 in __do_global_dtors_aux () from /lib64/libgcc_s.so.1
  3    Thread 0x7f4c1abfd640 (LWP 25992) 0x00007f4c1bcd8635 in clock_nanosleep@GLIBC_2.2.5 () from /lib64/libc.so.6
```
#1 is the main thread that waits for SIGTERM or SIGQUIT.  
#3 is the thread from which we call abort().  
#2 is the **kjellkod::Active::run()** thread. This is the one that calls **kill()**
syscall that do not block the running thread which continues until calling **exit()**.  

The exit() call can be seen directly:
```
gdb$ info threads
  Id   Target Id                         Frame
* 1    Thread 0x7f4c1c240740 (LWP 25989) 0x00007f4c1bc3f6ea in sigtimedwait () from /lib64/libc.so.6
  2    Thread 0x7f4c1bbff640 (LWP 25990) 0x00007f4c1c249980 in __do_global_dtors_aux () from /lib64/libgcc_s.so.1
  3    Thread 0x7f4c1abfd640 (LWP 25992) 0x00007f4c1bcd8635 in clock_nanosleep@GLIBC_2.2.5 () from /lib64/libc.so.6
gdb$ thread 2
[Switching to thread 2 (Thread 0x7f4c1bbff640 (LWP 25990))]
#0  0x00007f4c1c249980 in __do_global_dtors_aux () from /lib64/libgcc_s.so.1
gdb$ bt
#0  0x00007f4c1c249980 in __do_global_dtors_aux () from /lib64/libgcc_s.so.1
#1  0x00007f4c1c363d3e in _dl_fini () at dl-fini.c:142
#2  0x00007f4c1bc41045 in __run_exit_handlers () from /lib64/libc.so.6
#3  0x00007f4c1bc411c0 in exit () from /lib64/libc.so.6
#4  0x00007f4c1c31ca15 in g3::internal::exitWithDefaultSignalHandler (level=..., fatal_signal_id=0x6) at /home/ablangy/workspace-github/log_mt_test/extern/g3log/src/crashhandler_unix.cpp:240
#5  0x00007f4c1c330aa1 in g3::LogWorkerImpl::bgFatal (this=0xcec090, msgPtr=...) at /home/ablangy/workspace-github/log_mt_test/extern/g3log/src/logworker.cpp:67
#6  0x00007f4c1c330d77 in operator() (__closure=0x7f4c10001b30) at /home/ablangy/workspace-github/log_mt_test/extern/g3log/src/logworker.cpp:109
#7  0x00007f4c1c331c20 in std::__invoke_impl<void, g3::LogWorker::fatal(g3::FatalMessagePtr)::<lambda()>&>(std::__invoke_other, struct {...} &) (__f=...) at /usr/include/c++/12/bits/invoke.h:61
#8  0x00007f4c1c3319ae in std::__invoke_r<void, g3::LogWorker::fatal(g3::FatalMessagePtr)::<lambda()>&>(struct {...} &) (__fn=...) at /usr/include/c++/12/bits/invoke.h:111
#9  0x00007f4c1c331605 in std::_Function_handler<void(), g3::LogWorker::fatal(g3::FatalMessagePtr)::<lambda()> >::_M_invoke(const std::_Any_data &) (__functor=...) at /usr/include/c++/12/bits/std_function.h:290
#10 0x0000000000423aac in std::function<void ()>::operator()() const (this=0x7f4c1bbfed40) at /usr/include/c++/12/bits/std_function.h:591
#11 0x0000000000422bb0 in kjellkod::Active::run (this=0xcec0c0) at /home/ablangy/workspace-github/log_mt_test/build/Debug/g3log/include/g3log/active.hpp:40
#12 0x0000000000430d71 in std::__invoke_impl<void, void (kjellkod::Active::*)(), kjellkod::Active*> (__f=@0xcec400: (void (kjellkod::Active::*)(kjellkod::Active * const)) 0x422b72 <kjellkod::Active::run()>, __t=@0xcec3f8: 0xcec0c0) at /usr/include/c++/12/bits/invoke.h:74
#13 0x0000000000430aaa in std::__invoke<void (kjellkod::Active::*)(), kjellkod::Active*> (__fn=@0xcec400: (void (kjellkod::Active::*)(kjellkod::Active * const)) 0x422b72 <kjellkod::Active::run()>) at /usr/include/c++/12/bits/invoke.h:96
#14 0x0000000000430553 in std::thread::_Invoker<std::tuple<void (kjellkod::Active::*)(), kjellkod::Active*> >::_M_invoke<0ul, 1ul> (this=0xcec3f8) at /usr/include/c++/12/bits/std_thread.h:252
#15 0x00000000004300e0 in std::thread::_Invoker<std::tuple<void (kjellkod::Active::*)(), kjellkod::Active*> >::operator() (this=0xcec3f8) at /usr/include/c++/12/bits/std_thread.h:259
#16 0x000000000042fcb6 in std::thread::_State_impl<std::thread::_Invoker<std::tuple<void (kjellkod::Active::*)(), kjellkod::Active*> > >::_M_run (this=0xcec3f0) at /usr/include/c++/12/bits/std_thread.h:210
#17 0x00007f4c1c0dbb03 in execute_native_thread_routine () from /lib64/libstdc++.so.6
#18 0x00007f4c1bc8cded in start_thread () from /lib64/libc.so.6
#19 0x00007f4c1bd12370 in clone3 () from /lib64/libc.so.6
```

or indirectly (from another execution)
```
gdb$ info threads
  Id   Target Id                         Frame
* 1    Thread 0x7fbaee058740 (LWP 25886) 0x00007fbaeda3f6ea in sigtimedwait () from /lib64/libc.so.6
  2    Thread 0x7fbaed9ff640 (LWP 25887) std::_Rb_tree<int, std::pair<int const, std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > >, std::_Select1st<std::pair<int const, std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > > >, std::less<int>, std::allocator<std::pair<int const, std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > > > >::_S_right (__x=0x557fa0) at /usr/include/c++/12/bits/stl_tree.h:788
  3    Thread 0x7fbaec9fd640 (LWP 25889) 0x00007fbaedad8635 in clock_nanosleep@GLIBC_2.2.5 () from /lib64/libc.so.6
gdb$ thread 2
[Switching to thread 2 (Thread 0x7fbaed9ff640 (LWP 25887))]
#0  std::_Rb_tree<int, std::pair<int const, std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > >, std::_Select1st<std::pair<int const, std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > > >, std::less<int>, std::allocator<std::pair<int const, std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > > > >::_S_right (__x=0x557fa0) at /usr/include/c++/12/bits/stl_tree.h:788
788           { return static_cast<_Link_type>(__x->_M_right); }
gdb$ bt
#0  std::_Rb_tree<int, std::pair<int const, std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > >, std::_Select1st<std::pair<int const, std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > > >, std::less<int>, std::allocator<std::pair<int const, std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > > > >::_S_right (__x=0x557fa0) at /usr/include/c++/12/bits/stl_tree.h:788
#1  0x0000000000426686 in std::_Rb_tree<int, std::pair<int const, std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > >, std::_Select1st<std::pair<int const, std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > > >, std::less<int>, std::allocator<std::pair<int const, std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > > > >::_M_erase (this=0x7fbaee18c7c0 <(anonymous namespace)::kSignals>, __x=0x557fa0) at /usr/include/c++/12/bits/stl_tree.h:1935
#2  0x0000000000426698 in std::_Rb_tree<int, std::pair<int const, std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > >, std::_Select1st<std::pair<int const, std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > > >, std::less<int>, std::allocator<std::pair<int const, std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > > > >::_M_erase (this=0x7fbaee18c7c0 <(anonymous namespace)::kSignals>, __x=0x557eb0) at /usr/include/c++/12/bits/stl_tree.h:1935
#3  0x0000000000424bea in std::_Rb_tree<int, std::pair<int const, std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > >, std::_Select1st<std::pair<int const, std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > > >, std::less<int>, std::allocator<std::pair<int const, std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > > > >::~_Rb_tree (this=0x7fbaee18c7c0 <(anonymous namespace)::kSignals>, __in_chrg=<optimized out>) at /usr/include/c++/12/bits/stl_tree.h:984
#4  0x0000000000423588 in std::map<int, std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >, std::less<int>, std::allocator<std::pair<int const, std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > > > >::~map (this=0x7fbaee18c7c0 <(anonymous namespace)::kSignals>, __in_chrg=<optimized out>) at /usr/include/c++/12/bits/stl_map.h:312
#5  0x00007fbaeda41537 in __cxa_finalize () from /lib64/libc.so.6
#6  0x00007fbaee14db67 in __do_global_dtors_aux () from /home/ablangy/workspace-github/log_mt_test/build/Debug/g3log/lib/libg3log.so.1
#7  0x00007fbaed9fe8b0 in ?? ()
#8  0x00007fbaee195d3e in _dl_fini () at dl-fini.c:142
Backtrace stopped: frame did not save the PC
```

## Check the fix

*log_mt_test* must be compiled from scratch with **g3sinks** pointing to `master` branch and **g3log** pointing to `linux_multithreadFix_exitWithDefaultSignalHandler`.  
Execute:  
`$ rm -rf ./build`  
`$ cd extern/g3log`  
`$ git switch linux_multithreadFix_exitWithDefaultSignalHandler`  
`$ cd ../../`  
`$ cmake -S . -B ./build`  
`$ cmake --build ./build -j 6`  
`$ ./build/Debug/bin/logMtApp -f tapp -d /tmp`

The app should terminate with similar output:  
```
2022/11/07 13:09:27 191200

***** FATAL SIGNAL RECEIVED ******* 
Received fatal signal: SIGABRT(6)       PID: 28633

***** SIGNAL SIGABRT(6)

******* STACKDUMP *******
        stack dump [1]  /home/ablangy/workspace-github/log_mt_test/build/Debug/g3log/lib/libg3log.so.1+0x58c33 [0x7f9faf324c33]
        stack dump [2]  /lib64/libc.so.6+0x3ea30 [0x7f9faec3ea30]
        stack dump [3]  /lib64/libc.so.6+0x8ec0c [0x7f9faec8ec0c]
        stack dump [4]  /lib64/libc.so.6raise+0x16 [0x7f9faec3e986]
        stack dump [5]  /lib64/libc.so.6abort+0xcf [0x7f9faec287f4]
        stack dump [6]  ./build/Debug/bin/logMtApp() [0x420a97]
        stack dump [7]  ./build/Debug/bin/logMtApp() [0x421bce]
        stack dump [8]  ./build/Debug/bin/logMtApp() [0x421ab6]
        stack dump [9]  ./build/Debug/bin/logMtApp() [0x4219b9]

        stack dump [10]  ./build/Debug/bin/logMtApp : std::function<void ()>::operator()() const+0x32 [0x423aac]
        stack dump [11]  ./build/Debug/bin/logMtApp() [0x432f45]
        stack dump [12]  ./build/Debug/bin/logMtApp() [0x432c35]
        stack dump [13]  ./build/Debug/bin/logMtApp() [0x4332a8]
        stack dump [14]  ./build/Debug/bin/logMtApp() [0x43326b]
        stack dump [15]  ./build/Debug/bin/logMtApp() [0x433218]
        stack dump [16]  ./build/Debug/bin/logMtApp() [0x4331ec]
        stack dump [17]  ./build/Debug/bin/logMtApp() [0x4331d0]
        stack dump [18]  /lib64/libstdc++.so.6+0xdbb03 [0x7f9faf0dbb03]
        stack dump [19]  /lib64/libc.so.6+0x8cded [0x7f9faec8cded]
        stack dump [20]  /lib64/libc.so.6+0x112370 [0x7f9faed12370]

Exiting after fatal event  (FATAL_SIGNAL). Fatal type:  SIGABRT
Log content flushed successfully to sink



Exiting, log location: /tmp/tapp.log


exitWithDefaultSignalHandler:230. Exiting due to FATAL_SIGNAL, 6   

Abandon (core dumped)
```

Open the core, you should see the 3 threads as described earlier:  
```
gdb$ info threads
  Id   Target Id                         Frame 
* 1    Thread 0x7f9faebff640 (LWP 28634) 0x00007f9faec8ec0c in __pthread_kill_implementation () from /lib64/libc.so.6
  2    Thread 0x7f9faef1d740 (LWP 28633) 0x00007f9faec3f6ea in sigtimedwait () from /lib64/libc.so.6
  3    Thread 0x7f9fadbfd640 (LWP 28636) 0x00007f9faecd8635 in clock_nanosleep@GLIBC_2.2.5 () from /lib64/libc.so.6
```
#2 is the main thread that waits for SIGTERM or SIGQUIT  
#3 is the thread from which we call abort()  
#1 is the **kjellkod::Active::run()** thread. With the fix, this one calls
**raise()** that do block the execution of the calling thread.  
```
gdb$ info threads
  Id   Target Id                         Frame
* 1    Thread 0x7f9faebff640 (LWP 28634) 0x00007f9faec8ec0c in __pthread_kill_implementation () from /lib64/libc.so.6
  2    Thread 0x7f9faef1d740 (LWP 28633) 0x00007f9faec3f6ea in sigtimedwait () from /lib64/libc.so.6
  3    Thread 0x7f9fadbfd640 (LWP 28636) 0x00007f9faecd8635 in clock_nanosleep@GLIBC_2.2.5 () from /lib64/libc.so.6
gdb$ bt
#0  0x00007f9faec8ec0c in __pthread_kill_implementation () from /lib64/libc.so.6
#1  0x00007f9faec3e986 in raise () from /lib64/libc.so.6
#2  0x00007f9faf3259ee in g3::internal::exitWithDefaultSignalHandler (level=..., fatal_signal_id=0x6)
    at /home/ablangy/workspace-github/log_mt_test/extern/g3log/src/crashhandler_unix.cpp:233
#3  0x00007f9faf339a97 in g3::LogWorkerImpl::bgFatal (this=0x49e090, msgPtr=...)
    at /home/ablangy/workspace-github/log_mt_test/extern/g3log/src/logworker.cpp:67
#4  0x00007f9faf339d6d in operator() (__closure=0x7f9fa4001b30)
    at /home/ablangy/workspace-github/log_mt_test/extern/g3log/src/logworker.cpp:109
#5  0x00007f9faf33ac16 in std::__invoke_impl<void, g3::LogWorker::fatal(g3::FatalMessagePtr)::<lambda()>&>(std::__invoke_other, struct {...} &) (__f=...) at /usr/include/c++/12/bits/invoke.h:61
#6  0x00007f9faf33a9a4 in std::__invoke_r<void, g3::LogWorker::fatal(g3::FatalMessagePtr)::<lambda()>&>(struct {...} &) (__fn=...) at /usr/include/c++/12/bits/invoke.h:111
#7  0x00007f9faf33a5fb in std::_Function_handler<void(), g3::LogWorker::fatal(g3::FatalMessagePtr)::<lambda()> >::_M_invoke(const std::_Any_data &) (__functor=...) at /usr/include/c++/12/bits/std_function.h:290
#8  0x0000000000423aac in std::function<void ()>::operator()() const (this=0x7f9faebfed40)
    at /usr/include/c++/12/bits/std_function.h:591
#9  0x0000000000422bb0 in kjellkod::Active::run (this=0x49e0c0)
    at /home/ablangy/workspace-github/log_mt_test/build/Debug/g3log/include/g3log/active.hpp:40
#10 0x0000000000430d71 in std::__invoke_impl<void, void (kjellkod::Active::*)(), kjellkod::Active*> (
    __f=@0x49e400: (void (kjellkod::Active::*)(kjellkod::Active * const)) 0x422b72 <kjellkod::Active::run()>,
    __t=@0x49e3f8: 0x49e0c0) at /usr/include/c++/12/bits/invoke.h:74
#11 0x0000000000430aaa in std::__invoke<void (kjellkod::Active::*)(), kjellkod::Active*> (
    __fn=@0x49e400: (void (kjellkod::Active::*)(kjellkod::Active * const)) 0x422b72 <kjellkod::Active::run()>)
    at /usr/include/c++/12/bits/invoke.h:96
#12 0x0000000000430553 in std::thread::_Invoker<std::tuple<void (kjellkod::Active::*)(), kjellkod::Active*> >::_M_invoke<0ul, 1ul> (this=0x49e3f8) at /usr/include/c++/12/bits/std_thread.h:252
#13 0x00000000004300e0 in std::thread::_Invoker<std::tuple<void (kjellkod::Active::*)(), kjellkod::Active*> >::operator() (this=0x49e3f8) at /usr/include/c++/12/bits/std_thread.h:259
#14 0x000000000042fcb6 in std::thread::_State_impl<std::thread::_Invoker<std::tuple<void (kjellkod::Active::*)(), kjellkod::Active*> > >::_M_run (this=0x49e3f0) at /usr/include/c++/12/bits/std_thread.h:210
#15 0x00007f9faf0dbb03 in execute_native_thread_routine () from /lib64/libstdc++.so.6
#16 0x00007f9faec8cded in start_thread () from /lib64/libc.so.6
#17 0x00007f9faed12370 in clone3 () from /lib64/libc.so.6
```

The global variables are left intact, the core file is not corrupted.  

