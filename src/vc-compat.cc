#include <windows.h>

//
//	VC++2010で作るバイナリを2000で
//	http://recyclebin.jugem.jp/?eid=341
//
//	VC++ 2010 再頒布可能パッケージ と Windows 2000
//	http://blog.livedoor.jp/blackwingcat/archives/1192179.html
//
//	Can I use Visual Studio 2010's C++ compiler with Visual Studio 2008's C++ Runtime Library?
//	http://stackoverflow.com/questions/2484511/can-i-use-visual-studio-2010s-c-compiler-with-visual-studio-2008s-c-runtim/3502056#3502056
//

#define IS_VC2010_OR_LATER (_MSC_VER >= 1600)
#define PROC_PTR_UNINITIALIZED ((void*)1)


static void *
getModuleProcAddress (void *procPtr, const char *szModule, const char *szProc)
{
	if (PROC_PTR_UNINITIALIZED == procPtr) {
		void *p = 0;
		HMODULE h = GetModuleHandleA (szModule);
		if (h) {
			p = GetProcAddress (h, szProc);
		}
		procPtr = p;
	}
	return procPtr;
}


//
// SetDllDirectory
// http://msdn.microsoft.com/en-us/library/windows/desktop/ms686203(v=vs.85).aspx
//
extern "C" BOOL __stdcall
xyzzySetDllDirectoryA (LPCSTR lpPathName)
{
  static const char szModule[] = "KERNEL32";
  static const char szProc[] = "SetDllDirectoryA";
  typedef BOOL (WINAPI *FUNC)(LPCSTR);
  static void *procPtr = PROC_PTR_UNINITIALIZED;

  procPtr = getModuleProcAddress (procPtr, szModule, szProc);
  BOOL ret = 0;
  if (procPtr) {
    ret = ((FUNC)procPtr)(lpPathName);
  }
  return ret;
}


#if IS_VC2010_OR_LATER
//
// EncodePointer
// http://msdn.microsoft.com/en-us/library/bb432254(v=vs.85).aspx
//
extern "C" PVOID __stdcall
xyzzyEncodePointer (PVOID ptr)
{
  static const char szModule[] = "KERNEL32";
  static const char szProc[] = "EncodePointer";
  typedef PVOID (WINAPI *FUNC)(PVOID);
  static void *procPtr = PROC_PTR_UNINITIALIZED;

  procPtr = getModuleProcAddress (procPtr, szModule, szProc);
  if (procPtr) {
    ptr = ((FUNC)procPtr)(ptr);
  }
  return ptr;
}
#endif // IS_VC2010_OR_LATER


#if IS_VC2010_OR_LATER
//
// DecodePointer
// http://msdn.microsoft.com/en-us/library/bb432242(v=VS.85).aspx
//
extern "C" PVOID __stdcall
xyzzyDecodePointer (PVOID ptr)
{
  static const char szModule[] = "KERNEL32";
  static const char szProc[] = "DecodePointer";
  typedef PVOID (WINAPI *FUNC)(PVOID);
  static void *procPtr = PROC_PTR_UNINITIALIZED;

  procPtr = getModuleProcAddress (procPtr, szModule, szProc);
  if (procPtr) {
    ptr = ((FUNC)procPtr)(ptr);
  }
  return ptr;
}
#endif // IS_VC2010_OR_LATER
