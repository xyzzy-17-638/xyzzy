// updater.cpp : アプリケーションのエントリ ポイントを定義します。
//

#include "stdafx.h"
#include "updater.h"

#define WM_UPDATE_DONE (WM_USER + 1)
#define WM_UPDATE_PROGESS (WM_USER + 2)


#define MAX_LOADSTRING 100

// グローバル変数:
HINSTANCE hInst;								// 現在のインターフェイス
TCHAR szTitle[MAX_LOADSTRING];					// タイトル バーのテキスト
TCHAR szWindowClass[MAX_LOADSTRING];			// メイン ウィンドウ クラス名

// このコード モジュールに含まれる関数の宣言を転送します:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);


// dir should end with \\. relativeDir not start from \\ or .\\ but start from roman character.
static errno_t
ResolveRelativePath(LPCTSTR root_path,  LPCTSTR relativeDir, LPCTSTR fileName, LPTSTR dest, int destSize)
{
    TCHAR drive[_MAX_DRIVE];
    TCHAR dir[_MAX_DIR];

	_tsplitpath_s(root_path, drive,  _MAX_DRIVE, dir, _MAX_DIR,  NULL, 0,  NULL, 0);
	errno_t err;
	if(relativeDir != 0)
	{
		err = _tcscat_s(dir, _MAX_DIR, relativeDir);
		if(err != 0) {
			return err;
		}
	}
	err = _tmakepath_s(dest, destSize, drive, dir, fileName, NULL);
	return err;
}

static errno_t
ConcatPath(LPCTSTR root_path, LPCTSTR relativeDir, LPTSTR dest, int destSize)
{
	return ResolveRelativePath(root_path, relativeDir, NULL, dest, destSize);
}


static errno_t
ResolveModuleRelativePath(LPCTSTR fileName, LPCTSTR relativeDir, LPTSTR dest, int destSize)
{
    TCHAR module_path_name[_MAX_PATH];

	GetModuleFileName (0, module_path_name, sizeof module_path_name);
	return ResolveRelativePath(module_path_name, relativeDir, fileName, dest, destSize);
}

static void
ResolveModulePath(LPCTSTR fileName, LPTSTR dest, int destSize)
{
	ResolveModuleRelativePath(fileName, NULL, dest, destSize);
}



static bool
FileExists(LPCTSTR fullPath)
{
	DWORD res = GetFileAttributes(fullPath);
	return res != -1;
}

#define OLD_XYZZY_EXE_NAME _T("xyzzy_old.exe")
#define XYZZY_EXE_NAME _T("xyzzy.exe")
#define XYZZY_NEW_FOLDER_NAME _T("xyzzy_new")

static void
ErrorBox(LPCTSTR msg)
{
	MessageBox(NULL, msg, _T("Error"), MB_ICONERROR);
}

static bool
NeedUpdate()
{
	TCHAR xyzzy_new_path[MAX_PATH];
	ResolveModuleRelativePath(NULL, XYZZY_NEW_FOLDER_NAME, xyzzy_new_path, MAX_PATH);
	if(!FileExists(xyzzy_new_path))
	{
		return false;
	}
	return true;
}

static bool
PreSetup()
{
	TCHAR xyzzy_path[MAX_PATH];
	TCHAR old_path[MAX_PATH];
	TCHAR xyzzy_new_path[MAX_PATH];
	ResolveModulePath(OLD_XYZZY_EXE_NAME, old_path, MAX_PATH);
	ResolveModulePath(XYZZY_EXE_NAME, xyzzy_path, MAX_PATH);
	ResolveModuleRelativePath(NULL, XYZZY_NEW_FOLDER_NAME, xyzzy_new_path, MAX_PATH);

	if(!FileExists(xyzzy_new_path))
	{
		MessageBox(NULL, _T("folder xyzzy_new does not exists."), _T("Error"), MB_ICONERROR);
		return false;
	}
	if(FileExists(old_path))
	{
		if(!DeleteFile(old_path))
		{
			MessageBox(NULL, _T("can't delete xyzzy_old.exe"), _T("Error"), MB_ICONERROR);
			return false;
		}
	}
	if(!FileExists(xyzzy_path))
	{
		MessageBox(NULL, _T("xyzzy.exe does not exist."), _T("Error"), MB_ICONERROR);
		return false;
	}

	if(!MoveFileEx(xyzzy_path, old_path, MOVEFILE_REPLACE_EXISTING))
	{
		ErrorBox(_T("fail to rename xyzzy.exe to xyzzy_old.exe"));
		return false;
	}

	return true;
}

#include <Shellapi.h>
#include <string>
#include <vector>
#include <map>
using std::vector;
using std::map;
typedef std::basic_string<TCHAR> tstring;

template<class T> class FileMover
{
public:
	FileMover(LPCTSTR in_src_root, LPCTSTR in_dest_root, T in_listener) : src_root(in_src_root), dest_root (in_dest_root), listener(in_listener)
	{
		dir_skip_list[_T("etc\\")] = true;
	}
	void EnsureDir(LPCTSTR fullPath)
	{
		if(!FileExists(fullPath))
		{
			if(!CreateDirectory(fullPath, 0))
				throw std::exception("fail to create dir");
		}
	}
	void MoveFiles(LPCTSTR srcFolder, LPCTSTR destFolder, vector<tstring>& files)
	{
		EnsureDir(destFolder);
		for(vector<tstring>::iterator itr = files.begin(); itr != files.end(); itr++)
		{
			if(file_skip_list[*itr])
				continue;
			tstring srcPath(srcFolder);
			srcPath += *itr;
			tstring destPath(destFolder);
			destPath += *itr;
			if(!MoveFileEx(srcPath.c_str(), destPath.c_str(), MOVEFILE_REPLACE_EXISTING))
				throw std::exception("move file fail");
		}
	}

	void Move(LPCTSTR relative)
	{
		if(relative && dir_skip_list[relative])
			return;

		TCHAR dest[MAX_PATH];
		TCHAR src[MAX_PATH];
		TCHAR src_pat[MAX_PATH];
		
		ConcatPath(src_root, relative, src, MAX_PATH);
		ConcatPath(dest_root, relative, dest, MAX_PATH);


		vector<tstring> files;
		vector<tstring> dirs;

		WIN32_FIND_DATA fd;
		ResolveRelativePath(src, NULL, _T("*"), src_pat, MAX_PATH);
		HANDLE hFind = FindFirstFile(src_pat, &fd);
		if( INVALID_HANDLE_VALUE == hFind )
		{
			throw std::exception("no src folder.");
		}

		do
		{
			if(_T('.') == fd.cFileName[0])
				continue;
			if( FILE_ATTRIBUTE_DIRECTORY & fd.dwFileAttributes )
			{
				dirs.push_back(fd.cFileName);
			}
			else
			{
				files.push_back(fd.cFileName);
			}

		}while( FindNextFile(hFind, &fd));
		FindClose(hFind);
		MoveFiles(src, dest, files);

		for(vector<tstring>::iterator itr = dirs.begin(); itr != dirs.end(); itr++)
		{
			tstring path_with_delim(_T(""));
			if(relative)
				path_with_delim += relative;
			path_with_delim += *itr;
			path_with_delim.push_back(_T('\\'));
			Move(path_with_delim.c_str());
		}
	}
	void DeleteFolder(LPCTSTR inputPath)
	{
		TCHAR path[MAX_PATH+1];
		_tcscpy_s(path, MAX_PATH, inputPath);
		int len = _tcslen(path);
		if(len > MAX_PATH-1)
			throw std::exception("too long delete folder path");
		path[len+1] = _T('\0');

		SHFILEOPSTRUCT ss;
		ZeroMemory(&ss, sizeof(SHFILEOPSTRUCT));
		ss.hwnd = NULL;
		ss.wFunc = FO_DELETE;
		ss.fFlags = FOF_NOCONFIRMATION;
		ss.pFrom = path;
		int ret = SHFileOperation(&ss);
		if(ret != 0)
			throw std::exception("Shell file operation fail");
	}
	bool MoveAll()
	{
		try
		{
			listener.Notify(_T("Moving files..."));
			Move(NULL);
			listener.Notify(_T("Deleting files..."));
			DeleteFolder(src_root);
			listener.Notify(_T("DONE"));
			return true;
		}catch(std::exception& e)
		{
			WCHAR msg[256];	
			mbstowcs_s(NULL, msg, 256, e.what(), strlen(e.what()));
			ErrorBox(msg);
			return false;
		}
	}
private:
	map<tstring, bool> dir_skip_list;
	map<tstring, bool> file_skip_list;

	// xyzzy root
	LPCTSTR dest_root;

	// xyzzy_new folder
	LPCTSTR src_root;
	T& listener;
};


#ifdef _DEBUG
#include <assert.h>

void VerifyResolveModuleRelativePath(LPCTSTR inputFileName, LPCTSTR inputDir, LPCTSTR expect)
{
	TCHAR path[MAX_PATH];
	ResolveModuleRelativePath(inputFileName, inputDir, path, MAX_PATH);
	assert(_tcscmp(path, expect) == 0);
}

void VerifyConcat(LPCTSTR inputRoot, LPCTSTR inputDir, LPCTSTR expect)
{
	TCHAR path[MAX_PATH];
	ConcatPath(inputRoot, inputDir, path, MAX_PATH);
	assert(_tcscmp(path, expect) == 0);
}

void UnitTest()
{
	/*
	VerifyResolveModuleRelativePath(XYZZY_EXE_NAME, NULL, _T("C:\\Users\\xxx\\Documents\\tools\\xyzzy\\Debug\\xyzzy.exe"));
	VerifyResolveModuleRelativePath(NULL, XYZZY_NEW_FOLDER_NAME, _T("C:\\Users\\xxx\\Documents\\tools\\xyzzy\\Debug\\xyzzy_new\\"));
	*/
	VerifyConcat(_T("C:\\hoge\\ika\\"), _T("fuga\\bar\\"), _T("C:\\hoge\\ika\\fuga\\bar\\"));
}
#endif
const int MAX_RETRY_COUNT = 100;

static bool
WaitParentFinish(DWORD pid)
{
	DWORD exitCode;
	HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, pid);
	int retryNum = 0;
	while(!GetExitCodeProcess(hProcess, &exitCode) ||
		exitCode == STILL_ACTIVE)
	{
		retryNum++;
		if(retryNum > MAX_RETRY_COUNT)
		{
			return false;
		}
		Sleep(500);
	}
	CloseHandle(hProcess);
	return true;
}

bool
StartXyzzy()
{
	TCHAR path[MAX_PATH];
	ResolveModulePath(XYZZY_EXE_NAME, path, MAX_PATH);
	tstring cmdline(path);
	cmdline += _T(" -redump dummy");

	PROCESS_INFORMATION pi;
	STARTUPINFO si;
	memset (&si, 0, sizeof si);
	si.cb = sizeof si;
	// why not const!
	if (!CreateProcess (0, (LPTSTR)cmdline.c_str(), 0, 0, 0, CREATE_NEW_PROCESS_GROUP, 0, 0, &si, &pi))
		return false;
	CloseHandle (pi.hProcess);
	CloseHandle (pi.hThread);
	return true;
}


int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);


#ifdef _DEBUG
	UnitTest();
#endif

	if(__argc == 2)
	{
		DWORD pid = _wtoi(__wargv[1]);
		if(pid == 0)
		{
			ErrorBox(_T("Invalid process id."));
			return 0;
		}
		if(!WaitParentFinish(pid))
		{
			ErrorBox(_T("retry reached max count. never finish parent process"));
			return 0;
		}
		// fall through
	} else if(__argc != 1)
	{
		ErrorBox(_T("invalid arguments."));
		return 0;
	}


	if(!NeedUpdate())
	{
		// just reload.
		StartXyzzy();
		return 1;
	}


	if(!PreSetup())
		return 1;

	MSG msg;
	HACCEL hAccelTable;

	// グローバル文字列を初期化しています。
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_UPDATER, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// アプリケーションの初期化を実行します:
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_UPDATER));

	// メイン メッセージ ループ:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int) msg.wParam;
}



//
//  関数: MyRegisterClass()
//
//  目的: ウィンドウ クラスを登録します。
//
//  コメント:
//
//    この関数および使い方は、'RegisterClassEx' 関数が追加された
//    Windows 95 より前の Win32 システムと互換させる場合にのみ必要です。
//    アプリケーションが、関連付けられた
//    正しい形式の小さいアイコンを取得できるようにするには、
//    この関数を呼び出してください。
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_UPDATER));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_UPDATER);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

//
//   関数: InitInstance(HINSTANCE, int)
//
//   目的: インスタンス ハンドルを保存して、メイン ウィンドウを作成します。
//
//   コメント:
//
//        この関数で、グローバル変数でインスタンス ハンドルを保存し、
//        メイン プログラム ウィンドウを作成および表示します。
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   HWND hWnd;

   hInst = hInstance; // グローバル変数にインスタンス処理を格納します。

   hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, 320, 90, NULL, NULL, hInstance, NULL);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

class Listener
{
public:
	Listener(HWND in_hwnd) : hwnd(in_hwnd)
	{
	}
	void Notify(LPCTSTR msg)
	{
		SendMessage(hwnd, WM_UPDATE_PROGESS, 0, (LPARAM)msg); 
	}
private:
	HWND hwnd;
};

static DWORD WINAPI UpdateWoker(LPVOID data)
{
	HWND parent = (HWND) data;

	Listener listener(parent);

	TCHAR destRoot[_MAX_PATH];
	TCHAR srcRoot[_MAX_PATH];
	ResolveModulePath(NULL, destRoot, _MAX_PATH);
	ResolveModuleRelativePath(NULL, XYZZY_NEW_FOLDER_NAME, srcRoot, _MAX_PATH);


	FileMover<Listener> mover(srcRoot, destRoot, listener);
	bool res = mover.MoveAll();

	PostMessage(parent, WM_UPDATE_DONE, res, 0);
	return 0;
}



//
//  関数: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  目的:  メイン ウィンドウのメッセージを処理します。
//
//  WM_COMMAND	- アプリケーション メニューの処理
//  WM_PAINT	- メイン ウィンドウの描画
//  WM_DESTROY	- 中止メッセージを表示して戻る
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static HANDLE hThread;
	static DWORD tid;
	static TCHAR buf[256];

	PAINTSTRUCT ps;
	HDC hdc;

	switch (message)
	{
	case WM_CREATE:
		_tcscpy_s(buf, _T("update start..."));
		hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE) UpdateWoker, (LPVOID)hWnd, 0, &tid);
		return DefWindowProc(hWnd, message, wParam, lParam);
		break;
	case WM_UPDATE_DONE:
		if(wParam)
			StartXyzzy();
		PostQuitMessage(1);
		break;
	case WM_UPDATE_PROGESS:
		{
			LPTSTR msg = (LPTSTR)lParam;
			_tcscpy_s(buf, msg);
			InvalidateRect(hWnd, NULL, TRUE);
		}
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		TextOut(hdc, 10, 20, buf, _tcslen(buf));
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

