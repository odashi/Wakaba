// wakaba/dllmain.cpp
#include <windows.h>
#include <crtdbg.h>

// DLL �G���g���|�C���g
BOOL WINAPI DllMain(HINSTANCE, DWORD fdwReason, LPVOID)
{
	// ���������[�N�̃`�F�b�N���s��.
	if (fdwReason == DLL_PROCESS_ATTACH) {
		_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	}

	return TRUE;
}
