// wakaba/dllmain.cpp
#include <windows.h>
#include <crtdbg.h>

// DLL エントリポイント
BOOL WINAPI DllMain(HINSTANCE, DWORD fdwReason, LPVOID)
{
	// メモリリークのチェックを行う.
	if (fdwReason == DLL_PROCESS_ATTACH) {
		_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	}

	return TRUE;
}
