//
//          Tracy profiler
//         ----------------
//
// For fast integration, compile and
// link with this source file (and none
// other) in your executable (or in the
// main DLL / shared object on multi-DLL
// projects).
//

// Define TRACY_ENABLE to enable profiler.

#include "common/TracySystem.cpp"

#ifdef TRACY_ENABLE

#ifdef _MSC_VER
#  pragma warning(push, 0)
#define TRACY_DBGHELP_LOCK DbgHelp
extern "C"
{
static HANDLE dbgHelpLock;

void DbgHelpInit() { dbgHelpLock = CreateMutex(nullptr, FALSE, nullptr); }
void DbgHelpLock() { WaitForSingleObject(dbgHelpLock, INFINITE); }
void DbgHelpUnlock() { ReleaseMutex(dbgHelpLock); }
}
#endif

#include "common/tracy_lz4.cpp"
#include "client/TracyProfiler.cpp"
#include "client/TracyCallstack.cpp"
#include "client/TracySysTime.cpp"
#include "client/TracySysTrace.cpp"
#include "common/TracySocket.cpp"
#include "client/tracy_rpmalloc.cpp"
#include "client/TracyDxt1.cpp"
#include "client/TracyAlloc.cpp"
#include "client/TracyOverride.cpp"

#if TRACY_HAS_CALLSTACK == 2 || TRACY_HAS_CALLSTACK == 3 || TRACY_HAS_CALLSTACK == 4 || TRACY_HAS_CALLSTACK == 6
#  include "libbacktrace/alloc.cpp"
#  include "libbacktrace/dwarf.cpp"
#  include "libbacktrace/fileline.cpp"
#  include "libbacktrace/mmapio.cpp"
#  include "libbacktrace/posix.cpp"
#  include "libbacktrace/sort.cpp"
#  include "libbacktrace/state.cpp"
#  if TRACY_HAS_CALLSTACK == 4
#    include "libbacktrace/macho.cpp"
#  else
#    include "libbacktrace/elf.cpp"
#  endif
#  include "common/TracyStackFrames.cpp"
#endif

#ifdef _MSC_VER
#  pragma comment(lib, "ws2_32.lib")
#  pragma comment(lib, "dbghelp.lib")
#  pragma comment(lib, "advapi32.lib")
#  pragma comment(lib, "user32.lib")
#  pragma warning(pop)
#endif

#if PLATFORM_POSIX
#define CONSTRUCT_EARLY			__attribute__((init_priority(111)))
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#else
#define CONSTRUCT_EARLY
#endif

#ifdef TRACY_DELAYED_INIT
#pragma warning( disable:4075 ) // warning C4075: initializers put in unrecognized initialization area
#pragma init_seg( ".CRT$XCB" )
static struct TracyInitializer
{
	TracyInitializer()
	{
#ifdef WIN32
		bool bTrack = !!strstr( GetCommandLineA(), "-tracy" );
#else
		bool bTrack = false;
		int fd = open( "/proc/self/cmdline", O_RDONLY );
		if ( fd > 0 )
		{
			char data[4096];
			auto r = read( fd, data, 4096 );
			auto start = data;
			auto end = start + r;
			for ( auto p = start; p < end; )
			{
				if ( strstr( p, "-tracy" ) )
				{
					bTrack = true;
					break;
				}
				while ( *p++ );
			}
			close( fd );
		}
#endif
		if ( bTrack )
			tracy::StartupProfiler();
	}
} s_init CONSTRUCT_EARLY;
#endif

#endif
