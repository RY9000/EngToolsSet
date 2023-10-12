// dictionary.cpp 
//
#include "stdio.h"
#include "windows.h"
#include "wininet.h"
#include "resource.h"
#include "strsafe.h"

#pragma comment(lib,"Winmm.lib")
#pragma comment(lib,"Wininet.lib")

// Register key: http://open.iciba.com/index.php?c=wiki&t=cc
// Then put the key in the string below. For example: icibakey = L"&key=1234567890ABCDEF";
const wchar_t* icibakey = L"&key=";

#define	hangchangdu	81
#define wm_1		WM_USER
#define wm_2		wm_1+1
#define wm_3		wm_2+1
#define wm_4		wm_3+1
#define wm_0		wm_4+1

typedef char	ByteInt;
void output(wchar_t*);
void result(wchar_t*);
void jinshanjieguo(int n);
void jinshanjieguo2(int n);
void youdaojieguo(wchar_t*, int);
DWORD WINAPI cnresult(_In_ LPVOID lpParameter);
DWORD WINAPI speak(_In_ LPVOID lpParameter);
DWORD WINAPI jdownloadxml(_In_ LPVOID lpParameter);
DWORD WINAPI jdownloadxml2(_In_ LPVOID lpParameter);
DWORD WINAPI ydownloadxml(_In_ LPVOID lpParameter);
BOOL WINAPI HandlerRoutine(_In_ DWORD dwCtrlType);

HANDLE	hOUTPUT = GetStdHandle(STD_OUTPUT_HANDLE);
HANDLE	hINPUT = GetStdHandle(STD_INPUT_HANDLE);
HANDLE ghEvent = CreateEvent(NULL, TRUE, FALSE, L"Event");
DWORD ThreadId;
wchar_t* citiao[26];
wchar_t	ps[100];
wchar_t pronpath[][MAX_PATH] = { L"" , L"" , L"" , L"" };
unsigned int pronerror[4];
bool prondone[4];




int main()
{
	SetConsoleTitle(L"Dictionary");
	SMALL_RECT ConsoleWindow = { 0,0,hangchangdu - 2,30 };
	SetConsoleWindowInfo(hOUTPUT, TRUE, &ConsoleWindow);
	COORD Size = { hangchangdu - 1,1000 };
	SetConsoleScreenBufferSize(hOUTPUT, Size);
	system("color 0F");


	if (GetFileAttributes(L" ") == INVALID_FILE_ATTRIBUTES)
	{
		wchar_t temp[MAX_PATH];
		GetTempPath(MAX_PATH, temp);
		swprintf_s(pronpath[0], MAX_PATH, L"%ls%ls", temp, L"jinpron1.mp3");
		swprintf_s(pronpath[1], MAX_PATH, L"%ls%ls", temp, L"jinpron2.mp3");
		swprintf_s(pronpath[2], MAX_PATH, L"%ls%ls", temp, L"youpron1.mp3");
		swprintf_s(pronpath[3], MAX_PATH, L"%ls%ls", temp, L"youpron2.mp3");
	}

	SetConsoleCtrlHandler(HandlerRoutine, TRUE);

	CreateThread(NULL, 1024 * 1024, speak, 0, 0, &ThreadId);

	unsigned int i, m, n;
	LPVOID lpResLock;
	for (i = 0; i < 26; i++)
	{
		HRSRC hRes = FindResource(NULL, MAKEINTRESOURCE(IDR_CIKU1 + i), L"ciku");
		HGLOBAL hResLoad = LoadResource(NULL, hRes);
		lpResLock = LockResource(hResLoad);
		DWORD size = SizeofResource(NULL, hRes);

		citiao[i] = (wchar_t*)malloc(hangchangdu * 2000 * 2);
		char temp[hangchangdu] = "\0";
		char* p = (char*)lpResLock, * pend = (char*)lpResLock + size - 1;
		m = 0; n = 0;
		while (1)
		{
			switch (*p)
			{
			case 0x0d:
				temp[m] = '\0';
				p = p + 2;
				m = 0;
				MultiByteToWideChar(936, MB_PRECOMPOSED, temp, -1, citiao[i] + n * hangchangdu, hangchangdu);
				n++;
				if (n % 2000 == 0)
					citiao[i] = (wchar_t*)realloc(citiao[i], (n + 2000) * hangchangdu * 2);
				*(citiao[i] + n * hangchangdu) = L'\0';
				break;
			default:
				temp[m] = *p;
				p++;
				m++;
				break;
			}
			if (p > pend)
			{
				temp[m] = '\0';
				MultiByteToWideChar(936, MB_PRECOMPOSED, temp, -1, citiao[i] + n * hangchangdu, hangchangdu);
				n++;
				if (n % 2000 == 0)
					citiao[i] = (wchar_t*)realloc(citiao[i], (n + 2000) * hangchangdu * 2);
				*(citiao[i] + n * hangchangdu) = L'\0';
				break;
			}
		}
	}

	DWORD cNumRead;
	INPUT_RECORD irInBuf;
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	wchar_t word[101] = L"\0";
	wchar_t history[10][101];
	unsigned int hisindex = 0;
	bool zidongdu = 0;
	CONSOLE_CURSOR_INFO ConsoleCursorInfo = { 25,FALSE };
	DWORD NumberOfCharsWritten;
	ByteInt current = hisindex, steps = 0;
	memset(history, '\0', 2 * 10 * 101);
	i = 0;
	while (1)
	{
		ReadConsoleInput(hINPUT, &irInBuf, 1, &cNumRead);
		if (irInBuf.EventType != KEY_EVENT)
			continue;
		if (irInBuf.Event.KeyEvent.bKeyDown == FALSE)
			continue;
		switch (irInBuf.Event.KeyEvent.wVirtualKeyCode)
		{
		case VK_BACK:
			i = i != 0 ? i - 1 : 0;
			word[i] = L'\0';
			ConsoleCursorInfo = { 25,FALSE };
			SetConsoleCursorInfo(hOUTPUT, &ConsoleCursorInfo);
			SetConsoleCursorPosition(hOUTPUT, { 0,0 });
			WriteConsole(hOUTPUT, word, i, &NumberOfCharsWritten, NULL);
			GetConsoleScreenBufferInfo(hOUTPUT, &csbi);
			FillConsoleOutputCharacter(hOUTPUT, L' ', 2, csbi.dwCursorPosition, &NumberOfCharsWritten);
			ConsoleCursorInfo = { 25,TRUE };
			SetConsoleCursorInfo(hOUTPUT, &ConsoleCursorInfo);
			if (i == 0)
				system("cls");
			else
				output(word);
			continue;
		case VK_RETURN:
			system("cls");
			WriteConsoleOutputCharacter(hOUTPUT, word, wcslen(word), { 0,0 }, &NumberOfCharsWritten);
			SetConsoleCursorPosition(hOUTPUT, { 0,1 });
			ResetEvent(ghEvent);
			PostThreadMessage(ThreadId, wm_0, 0, 0);
			WaitForSingleObject(ghEvent, 1000);
			result(word);
			i = 0;
			current = hisindex++ % 10;
			StringCchCopy(history[current], 100, word);
			steps = 1;
			if (zidongdu == 1)
			{
				if (prondone[0])
					PostThreadMessage(ThreadId, wm_1, 0, 0);
				else if (prondone[1])
					PostThreadMessage(ThreadId, wm_2, 0, 0);
				else if (prondone[2])
					PostThreadMessage(ThreadId, wm_3, 0, 0);
				else if (prondone[3])
					PostThreadMessage(ThreadId, wm_4, 0, 0);
			}
			continue;
		case VK_F1:
			if (prondone[0])
				PostThreadMessage(ThreadId, wm_1, 0, 0);
			continue;
		case VK_F2:
			if (prondone[1])
				PostThreadMessage(ThreadId, wm_2, 0, 0);
			continue;
		case VK_F3:
			if (prondone[2])
				PostThreadMessage(ThreadId, wm_3, 0, 0);
			continue;
		case VK_F4:
			if (prondone[3])
				PostThreadMessage(ThreadId, wm_4, 0, 0);
			continue;
		case VK_F5:
			zidongdu = !zidongdu;
			if (zidongdu == 1)
				MessageBox(GetConsoleWindow(), L"自动发音:开", L"自动发音", MB_OK);
			else
				MessageBox(GetConsoleWindow(), L"自动发音:关", L"自动发音", MB_OK);
			continue;
		case VK_F9:
			wchar_t message[100];
			StringCchPrintf(message, 100, L"%s\n0x%x\n0x%x\n0x%x\n0x%x", word, pronerror[0], pronerror[1], pronerror[2], pronerror[3]);
			MessageBox(GetConsoleWindow(), message, NULL, MB_OK);
			continue;
		case VK_LEFT:
			if (current == -1)
				current = (hisindex - 1) % 10;
			else if (history[(current + 9) % 10][0] == L'\0')
				continue;
			else if (steps >= 10)
				continue;
			else
				current = (current + 9) % 10;
			system("cls");
			WriteConsoleOutputCharacter(hOUTPUT, history[current], wcslen(history[current]), { 0,0 }, &NumberOfCharsWritten);
			SetConsoleCursorPosition(hOUTPUT, { 0,1 });
			ResetEvent(ghEvent);
			PostThreadMessage(ThreadId, wm_0, 0, 0);
			WaitForSingleObject(ghEvent, 1000);
			result(history[current]);
			steps++;
			continue;
		case VK_RIGHT:
			if (current == -1)
				continue;
			if ((history[(current + 1) % 10][0] == L'\0') || (steps <= 1))
			{
				if (i != 0)
				{
					system("cls");
					SetConsoleCursorPosition(hOUTPUT, { 0,0 });
					WriteConsole(hOUTPUT, word, i, &NumberOfCharsWritten, NULL);
					current = -1;
					output(word);
					steps = 0;
				}
				continue;
			}
			system("cls");
			current = (current + 1) % 10;
			WriteConsoleOutputCharacter(hOUTPUT, history[current], wcslen(history[current]), { 0,0 }, &NumberOfCharsWritten);
			SetConsoleCursorPosition(hOUTPUT, { 0,1 });
			ResetEvent(ghEvent);
			PostThreadMessage(ThreadId, wm_0, 0, 0);
			WaitForSingleObject(ghEvent, 1000);
			result(history[current]);
			steps--;
			continue;
		default:
			if (irInBuf.Event.KeyEvent.uChar.UnicodeChar != L'\0')
			{
				/*if (i == 0)
				{
					system("cls");
					SetConsoleCursorPosition(hOUTPUT, { 0,0 });
				}*/
				word[i] = irInBuf.Event.KeyEvent.uChar.UnicodeChar;
				i++;
				word[i] = L'\0';
				if (current != -1)
				{
					system("cls");
					SetConsoleCursorPosition(hOUTPUT, { 0,0 });
					WriteConsole(hOUTPUT, word, i, &NumberOfCharsWritten, NULL);
					current = -1;
				}
				else
					WriteConsole(hOUTPUT, &irInBuf.Event.KeyEvent.uChar.UnicodeChar, 1, &NumberOfCharsWritten, NULL);
				output(word);
				steps = 0;
			}
			continue;
		}
	}
	return 0;
}

void output(wchar_t* word)
{
	if ((word[0] < L'a') || (word[0] > L'z'))
		return;
	wchar_t* p = citiao[word[0] - L'a'];
	unsigned int m = 0, n;
	short row = 1;
	DWORD NumberOfCharsWritten;
	for (; (p[hangchangdu * m] != L'\0') && (row < 500); m++)
	{
		for (n = 1; word[n] != L'\0'; n++)
		{
			if (word[n] != p[hangchangdu * m + n])
				break;
		}
		if (word[n] == L'\0')
		{
			FillConsoleOutputCharacter(hOUTPUT, L' ', hangchangdu - 1, { 0,row }, &NumberOfCharsWritten);
			FillConsoleOutputAttribute(hOUTPUT, FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED, hangchangdu - 1, 
				{ 0,row }, &NumberOfCharsWritten);
			DWORD nLength = wcslen(p + hangchangdu * m);
			WriteConsoleOutputCharacter(hOUTPUT, p + hangchangdu * m, nLength, { 0,row }, &NumberOfCharsWritten);
			row++;
		}
	}
	if (row <= 500)
	{
		FillConsoleOutputCharacter(hOUTPUT, L' ', (500 - row) * (hangchangdu - 1), { 0,row }, &NumberOfCharsWritten);
	}
	return;
}


HINTERNET jhInternet, jhInternet2, yhInternet, jhFile, jhFile2, yhFile, mp3hInternet, mp3hFile, cnhInternet, cnhFile;
HANDLE hFile;
char xmlj[1024 * 500], xmlj2[1024 * 500], xmly[1024 * 500];
wchar_t wxmlj[1024 * 500], wxmlj2[1024 * 500], wxmly[1024 * 500];
DWORD jNumberOfBytesRead, jNumberOfBytesRead2, yNumberOfBytesRead;
DWORD WINAPI downloadmp3(LPVOID p);
wchar_t pron1[1000], pron2[1000];

void result(wchar_t* word)
{
	if (word[0] > 0x2E7F)
	{
		HANDLE cnThreadHandle = CreateThread(NULL, 0, cnresult, word, 0, NULL);
		while (WaitForSingleObject(cnThreadHandle, 4444) != WAIT_OBJECT_0)
		{
			TerminateThread(cnThreadHandle, 0);
			CloseHandle(cnThreadHandle);
			InternetCloseHandle(cnhFile);
			InternetCloseHandle(cnhInternet);
			cnThreadHandle = CreateThread(NULL, 0, cnresult, word, 0, NULL);
		}
		return;
	}

	wchar_t jinurl[200] = L"http://dict-co.iciba.com/api/dictionary.php?w=";
	static unsigned char ikey = 0;
	StringCbCat(jinurl, 200 * sizeof(wchar_t), word);
	StringCbCat(jinurl, 200 * sizeof(wchar_t), icibakey);

	wchar_t youurl[300] = L"http://dict.youdao.com/jsonapi?q=";
	const wchar_t you[] = L"&keyfrom=deskdict.main&dogVersion=1.0&dogui=json&client=deskdict&id=32d9e0b32943ccd8f&vendor=unknown&in=&appVer=6.3.69.8341&appZengqiang=1&abTest=7&le=eng&LTH=31";
	StringCbCat(youurl, 300 * sizeof(wchar_t), word);
	StringCbCat(youurl, 300 * sizeof(wchar_t), you);

	wchar_t jinurl2[200] = L"http://dict-co.iciba.com/api/dictionary.php?type=json&w=";
	StringCbCat(jinurl2, 200 * sizeof(wchar_t), word);
	StringCbCat(jinurl2, 200 * sizeof(wchar_t), icibakey);
	HANDLE ThreadHandlej2 = CreateThread(NULL, 0, jdownloadxml2, jinurl2, 0, NULL);

	memset(prondone, 0, 4 * sizeof(bool));
	HANDLE ThreadHandle[2];
	DWORD index;
	while (1)
	{
		ThreadHandle[0] = CreateThread(NULL, 0, jdownloadxml, jinurl, 0, NULL);
		ThreadHandle[1] = CreateThread(NULL, 0, ydownloadxml, youurl, 0, NULL);
		index = WaitForMultipleObjects(2, ThreadHandle, FALSE, 4444);
		switch (index)
		{
		case WAIT_OBJECT_0:
			jinshanjieguo(jNumberOfBytesRead);
			break;
		case WAIT_OBJECT_0 + 1:
			youdaojieguo(word, yNumberOfBytesRead);
			break;
		default:
			TerminateThread(ThreadHandle[0], 0);
			TerminateThread(ThreadHandle[1], 0);
			CloseHandle(ThreadHandle[0]);
			CloseHandle(ThreadHandle[1]);
			InternetCloseHandle(jhFile);
			InternetCloseHandle(jhInternet);
			InternetCloseHandle(yhFile);
			InternetCloseHandle(yhInternet);
			continue;
		}
		break;
	}

	if (index == WAIT_OBJECT_0)
	{
		if (WaitForSingleObject(ThreadHandle[1], 4444) == WAIT_OBJECT_0)
			youdaojieguo(word, yNumberOfBytesRead);
		else
		{
			TerminateThread(ThreadHandle[1], 0);
			CloseHandle(ThreadHandle[1]);
		}
		InternetCloseHandle(yhFile);
		InternetCloseHandle(yhInternet);
	}
	else
	{
		if (WaitForSingleObject(ThreadHandle[0], 4444) == WAIT_OBJECT_0)
			jinshanjieguo(jNumberOfBytesRead);
		else
		{
			TerminateThread(ThreadHandle[0], 0);
			CloseHandle(ThreadHandle[0]);
		}
		InternetCloseHandle(jhFile);
		InternetCloseHandle(jhInternet);
	}

	if (WaitForSingleObject(ThreadHandlej2, 3333) == WAIT_OBJECT_0)
		jinshanjieguo2(jNumberOfBytesRead2);
	else
	{
		TerminateThread(ThreadHandlej2, 0);
		CloseHandle(ThreadHandlej2);
	}
	InternetCloseHandle(jhFile2);
	InternetCloseHandle(jhInternet2);

	printf("\n\n");
	DWORD NumberOfCharsWritten;
	WriteConsole(hOUTPUT, ps, wcslen(ps), &NumberOfCharsWritten, NULL);

	printf("\n");
	wchar_t laba[] = { 0x2208,L':' };
	WriteConsole(hOUTPUT, laba, 2, &NumberOfCharsWritten, NULL);
	for (unsigned char i = 0; i < 4; i++)
	{
		if (prondone[i])
			printf("F%d  ", i + 1);
	}
	return;
}

DWORD WINAPI jdownloadxml(LPVOID url)
{
	jhInternet = InternetOpen(NULL, INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
	jhFile = InternetOpenUrl(jhInternet, (wchar_t*)url, NULL, 0, INTERNET_FLAG_NO_CACHE_WRITE, 0);
	memset(xmlj, '\0', 1024 * 500);
	InternetReadFile(jhFile, xmlj, 1024 * 500, &jNumberOfBytesRead);
	InternetCloseHandle(jhFile);
	InternetCloseHandle(jhInternet);
	jNumberOfBytesRead = MultiByteToWideChar(CP_UTF8, MB_PRECOMPOSED, xmlj, jNumberOfBytesRead, wxmlj, 1024 * 500);
	return 0;
}

DWORD WINAPI jdownloadxml2(LPVOID url)
{
	jhInternet2 = InternetOpen(NULL, INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
	jhFile2 = InternetOpenUrl(jhInternet2, (wchar_t*)url, NULL, 0, INTERNET_FLAG_NO_CACHE_WRITE, 0);
	memset(xmlj2, '\0', 1024 * 500);
	InternetReadFile(jhFile2, xmlj2, 1024 * 500, &jNumberOfBytesRead2);
	InternetCloseHandle(jhFile2);
	InternetCloseHandle(jhInternet2);
	jNumberOfBytesRead2 = MultiByteToWideChar(CP_UTF8, MB_PRECOMPOSED, xmlj2, jNumberOfBytesRead2, wxmlj2, 1024 * 500);
	return 0;
}

DWORD WINAPI ydownloadxml(LPVOID url)
{
	yhInternet = InternetOpen(L"Youdao Desktop Dict (Windows NT 10.0)", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
	yhFile = InternetOpenUrl(yhInternet, (wchar_t*)url, NULL, 0, INTERNET_FLAG_NO_CACHE_WRITE, 0);
	memset(xmly, '\0', 1024 * 500);
	InternetReadFile(yhFile, xmly, 1024 * 500, &yNumberOfBytesRead);
	InternetCloseHandle(yhFile);
	InternetCloseHandle(yhInternet);
	yNumberOfBytesRead = MultiByteToWideChar(CP_UTF8, MB_PRECOMPOSED, xmly, yNumberOfBytesRead, wxmly, 1024 * 500);
	return 0;
}

void jinshanjieguo(int n)
{
	DWORD NumberOfCharsWritten;
	WriteConsole(hOUTPUT, L"\n金山:\n", 5, &NumberOfCharsWritten, NULL);

	wchar_t* p = wxmlj, * pend = wxmlj + n - 1;
	unsigned int i = 0;
	while ((!((p[0] == L'<') && (p[1] == L'p') && (p[2] == L'r') && (p[3] == L'o') && (p[4] == L'n') && (p[5] == L'>'))) && p < pend)
		p++;
	p = p + 6;
	if (p < pend)
		while ((!((p[0] == L'<') && (p[1] == L'/') && (p[2] == L'p') && (p[3] == L'r') && (p[4] == L'o') && (p[5] == L'n') && (p[6] == L'>'))) && p < pend)
		{
			pron1[i] = *p;
			p++; i++;
		}
	pron1[i] = L'\0';
	p = p + 7;

	i = 0;
	while ((!((p[0] == L'<') && (p[1] == L'p') && (p[2] == L'r') && (p[3] == L'o') && (p[4] == L'n') && (p[5] == L'>'))) && p < pend)
		p++;
	p = p + 6;
	if (p < pend)
		while ((!((p[0] == L'<') && (p[1] == L'/') && (p[2] == L'p') && (p[3] == L'r') && (p[4] == L'o') && (p[5] == L'n') && (p[6] == L'>'))) && p < pend)
		{
			pron2[i] = *p;
			p++; i++;
		}
	pron2[i] = L'\0';

	HANDLE ThreadHandle = CreateThread(NULL, 1024 * 1024, downloadmp3, 0, 0, NULL);

	wchar_t cao[1000];
	i = 0;
	p = wxmlj;
	while (p < pend)
	{
		while ((!((p[0] == L'<') && (p[1] == L'p') && (p[2] == L'o') && (p[3] == L's') && (p[4] == L'>'))) && p < pend)
			p++;
		p = p + 5;
		if (p < pend)
			while ((!((p[0] == L'<') && (p[1] == L'/') && (p[2] == L'p') && (p[3] == L'o') && (p[4] == L's') && (p[5] == L'>'))) && p < pend)
			{
				if (*p != L'&')
				{
					cao[i] = *p;
					p++;
				}
				else
				{
					switch (p[1])
					{
					case L'l':
						cao[i] = L'<'; p = p + 4;
						break;
					case L'g':
						cao[i] = L'>'; p = p + 4;
						break;
					case L'q':
						cao[i] = L'\"'; p = p + 6;
						break;
					default:
						if (p[2] == L'm')
						{
							cao[i] = L'&'; p = p + 5;
						}
						else
						{
							cao[i] = L'\''; p = p + 6;
						}
					}
				}
				i++;
			}
		cao[i] = L' ';
		i++; p = p + 6;

		while ((!((p[0] == L'<') && (p[1] == L'a') && (p[2] == L'c') && (p[3] == L'c') && (p[4] == L'e') && (p[5] == L'p') && (p[6] == L't') && (p[7] == L'a') && (p[8] == L't') && (p[9] == L'i') && (p[10] == L'o') && (p[11] == L'n') && (p[12] == L'>'))) && p < pend)
			p++;
		p = p + 13;
		if (p < pend)
			while ((!((p[0] == L'<') && (p[1] == L'/') && (p[2] == L'a') && (p[3] == L'c') && (p[4] == L'c') && (p[5] == L'e') && (p[6] == L'p') && (p[7] == L't') && (p[8] == L'a') && (p[9] == L't') && (p[10] == L'i') && (p[11] == L'o') && (p[12] == L'n') && (p[13] == L'>'))) && p < pend)
			{
				if (*p != L'&')
				{
					cao[i] = *p;
					p++;
				}
				else
				{
					switch (p[1])
					{
					case L'l':
						cao[i] = L'<'; p = p + 4;
						break;
					case L'g':
						cao[i] = L'>'; p = p + 4;
						break;
					case L'q':
						cao[i] = L'\"'; p = p + 6;
						break;
					default:
						if (p[2] == L'm')
						{
							cao[i] = L'&'; p = p + 5;
						}
						else
						{
							cao[i] = L'\''; p = p + 6;
						}
					}
				}
				i++;
			}
		cao[i] = L'\0';
		p = p + 14;
		if (i > 1)
			WriteConsole(hOUTPUT, cao, i, &NumberOfCharsWritten, NULL);
		i = 0;
	}

	i = 0;
	p = wxmlj;
	while (p < pend)
	{
		while ((!((p[0] == L'<') && (p[1] == L'p') && (p[2] == L's') && (p[3] == L'>'))) && p < pend)
			p++;
		p = p + 4;
		if (p < pend)
		{
			ps[i] = L'[';
			i++;
		}
		while (p < pend && (!((p[0] == L'<') && (p[1] == L'/') && (p[2] == L'p') && (p[3] == L's') && (p[4] == L'>'))))
		{
			ps[i] = *p;
			p++; i++;
		}
		if (p < pend)
		{
			ps[i] = L']';
			i++;
			p = p + 5;
		}
	}
	ps[i] = L'\0';

	if (WAIT_OBJECT_0 != WaitForSingleObject(ThreadHandle, 3333))
	{
		TerminateThread(ThreadHandle, 0);
		CloseHandle(ThreadHandle);
		InternetCloseHandle(mp3hFile);
		InternetCloseHandle(mp3hInternet);
		CloseHandle(hFile);
		ThreadHandle = CreateThread(NULL, 1024 * 1024, downloadmp3, 0, 0, NULL);
		if (WAIT_OBJECT_0 != WaitForSingleObject(ThreadHandle, 3333))
		{
			TerminateThread(ThreadHandle, 0);
			CloseHandle(ThreadHandle);
			InternetCloseHandle(mp3hFile);
			InternetCloseHandle(mp3hInternet);
			CloseHandle(hFile);
		}
	}
	return;
}

void youdaojieguo(wchar_t* word, int n)
{
	DWORD NumberOfCharsWritten;
	WriteConsole(hOUTPUT, L"\n有道:\n", 5, &NumberOfCharsWritten, NULL);

	const wchar_t youpronurl[] = L"http://dict.youdao.com/dictvoice?audio=";
	const wchar_t* you[] = { L"&type=1&client=deskdict&id=32d9e0b32943ccd8f&vendor=qiang.youdao&in=&appVer=6.3.69.8341&appZengqiang=1&abTest=7",
		L"&type=2&client=deskdict&id=32d9e0b32943ccd8f&vendor=qiang.youdao&in=&appVer=6.3.69.8341&appZengqiang=1&abTest=7" };
	StringCbCopy(pron1, 1000 * sizeof(wchar_t), youpronurl);
	StringCbCat(pron1, 1000 * sizeof(wchar_t), word);
	StringCbCat(pron1, 1000 * sizeof(wchar_t), you[0]);
	StringCbCopy(pron2, 1000 * sizeof(wchar_t), youpronurl);
	StringCbCat(pron2, 1000 * sizeof(wchar_t), word);
	StringCbCat(pron2, 1000 * sizeof(wchar_t), you[1]);
	HANDLE ThreadHandle = CreateThread(NULL, 1024 * 1024, downloadmp3, (LPVOID)2, 0, NULL);

	wchar_t* p = wxmly, * pend = wxmly + n - 1;

	wchar_t cao[1000];
	unsigned int i = 0;
	while ((!((p[0] == L'\"') && (p[1] == L'e') && (p[2] == L'c') && (p[3] == L'\"'))) && p < pend)
		p++;
	p = p + 4;
	while ((!((p[0] == L'\"') && (p[1] == L'w') && (p[2] == L'o') && (p[3] == L'r') && (p[4] == L'd') && (p[5] == L'\"'))) && p < pend)
		p++;
	p = p + 6;
	while ((!((p[0] == L'\"') && (p[1] == L't') && (p[2] == L'r') && (p[3] == L's') && (p[4] == L'\"'))) && p < pend)
		p++;
	p = p + 5;
	while (p < pend)
	{
		while ((!((p[0] == L'\"') && (p[1] == L'i') && (p[2] == L'\"'))) && p < pend)
			p++;
		p = p + 6;
		if (p < pend)
			while ((*p != L'\"') && (p < pend))
			{
				cao[i] = *p;
				p++; i++;
			}
		cao[i] = L'\0';
		p++;

		WriteConsole(hOUTPUT, cao, i, &NumberOfCharsWritten, NULL);
		i = 0;
		printf("\n");

		if (p[5] == L']')
			break;
	}

	if (WAIT_OBJECT_0 != WaitForSingleObject(ThreadHandle, 3333))
	{
		TerminateThread(ThreadHandle, 0);
		CloseHandle(ThreadHandle);
		InternetCloseHandle(mp3hFile);
		InternetCloseHandle(mp3hInternet);
		CloseHandle(hFile);
		ThreadHandle = CreateThread(NULL, 1024 * 1024, downloadmp3, (LPVOID)2, 0, NULL);
		if (WAIT_OBJECT_0 != WaitForSingleObject(ThreadHandle, 3333))
		{
			TerminateThread(ThreadHandle, 0);
			CloseHandle(ThreadHandle);
			InternetCloseHandle(mp3hFile);
			InternetCloseHandle(mp3hInternet);
			CloseHandle(hFile);
		}
	}
	return;
}

DWORD WINAPI downloadmp3(LPVOID url)
{
	char mp3[1024 * 128];
	DWORD mp3NumberOfBytesRead;

	mp3hInternet = InternetOpen(NULL, INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
	mp3hFile = InternetOpenUrl(mp3hInternet, pron1, NULL, 0, INTERNET_FLAG_NO_CACHE_WRITE, 0);
	if (InternetReadFile(mp3hFile, mp3, 1024 * 128, &mp3NumberOfBytesRead))
	{
		hFile = CreateFile(pronpath[(int)url], GENERIC_READ | GENERIC_WRITE, FILE_SHARE_DELETE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if (WriteFile(hFile, mp3, mp3NumberOfBytesRead, &mp3NumberOfBytesRead, NULL))
			prondone[(int)url] = 1;
		CloseHandle(hFile);
	}
	else
		pronerror[(int)url] = GetLastError();
	InternetCloseHandle(mp3hFile);
	InternetCloseHandle(mp3hInternet);


	mp3hInternet = InternetOpen(NULL, INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
	mp3hFile = InternetOpenUrl(mp3hInternet, pron2, NULL, 0, INTERNET_FLAG_NO_CACHE_WRITE, 0);
	if (InternetReadFile(mp3hFile, mp3, 1024 * 128, &mp3NumberOfBytesRead))
	{
		hFile = CreateFile(pronpath[(int)url + 1], GENERIC_READ | GENERIC_WRITE, FILE_SHARE_DELETE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if (WriteFile(hFile, mp3, mp3NumberOfBytesRead, &mp3NumberOfBytesRead, NULL))
			prondone[(int)url + 1] = 1;
		CloseHandle(hFile);
	}
	else
		pronerror[(int)url + 1] = GetLastError();
	InternetCloseHandle(mp3hFile);
	InternetCloseHandle(mp3hInternet);
	return 0;
}

void jinshanjieguo2(int n)
{
	printf("\n");
	wchar_t* p = wxmlj2, * pend = wxmlj2 + n - 1;
	while ((!((p[0] == L'\"') && (p[1] == L'e') && (p[2] == L'x') && (p[3] == L'c') && (p[4] == L'h') && (p[5] == L'a') && (p[6] == L'n') && (p[7] == L'g') && (p[8] == L'e') && (p[9] == L'\"'))) && p < pend)
		p++;
	p = p + 13;

	DWORD NumberOfCharsWritten;
	wchar_t cao[1000];
	unsigned int i = 0;
	CONSOLE_SCREEN_BUFFER_INFO ConsoleScreenBufferInfo;
	while (p < pend)
	{
		while ((!(p[0] == L'\"')) && p < pend)
			p++;

		GetConsoleScreenBufferInfo(hOUTPUT, &ConsoleScreenBufferInfo);

		switch (p[-1])
		{
		case L'l':
			WriteConsole(hOUTPUT, L"复数:", 3, &NumberOfCharsWritten, NULL);
			break;
		case L'e':
			WriteConsole(hOUTPUT, L"过去分词:", 5, &NumberOfCharsWritten, NULL);
			break;
		case L'g':
			WriteConsole(hOUTPUT, L"现在分词:", 5, &NumberOfCharsWritten, NULL);
			break;
		case L'd':
			WriteConsole(hOUTPUT, L"第三人称单数:", 7, &NumberOfCharsWritten, NULL);
			break;
		case L'r':
			WriteConsole(hOUTPUT, L"比较级:", 4, &NumberOfCharsWritten, NULL);
			break;
		default:
			if (p[-3] == L'a')
				WriteConsole(hOUTPUT, L"过去式:", 4, &NumberOfCharsWritten, NULL);
			else
				WriteConsole(hOUTPUT, L"最高级:", 4, &NumberOfCharsWritten, NULL);
		}
		p = p + 2;

		if (p[0] != L'[')
		{
			const wchar_t* space = L"                                  ";
			SetConsoleCursorPosition(hOUTPUT, ConsoleScreenBufferInfo.dwCursorPosition);
			WriteConsole(hOUTPUT, space, NumberOfCharsWritten * 2, &NumberOfCharsWritten, NULL);
			SetConsoleCursorPosition(hOUTPUT, ConsoleScreenBufferInfo.dwCursorPosition);
			while ((!(p[0] == L',')) && p < pend)
			{
				if (*p == L'}')
					goto end;
				p++;
			}
			p = p + 2;
			continue;
		}
		else
		{
			p++;
			while ((*p != L']') && p < pend)
			{
				if (*p == L'\"')
				{
					p++;
					continue;
				}
				cao[i] = *p;
				p++; i++;
			}
			cao[i] = L'\0';
			WriteConsole(hOUTPUT, cao, i, &NumberOfCharsWritten, NULL);
			WriteConsole(hOUTPUT, L"  ", 2, &NumberOfCharsWritten, NULL);
			if (p[1] == L'}')
				goto end;
			p = p + 3;
			i = 0;
		}
	}
end:
	return;
}

DWORD WINAPI speak(LPVOID lpParameter)
{
	MSG msg;
	unsigned char i = 0;

	MCI_OPEN_PARMS mciOpen;
	mciOpen.lpstrDeviceType = NULL;
	mciOpen.lpstrElementName = pronpath[i];
	mciSendCommand(0, MCI_OPEN, MCI_OPEN_ELEMENT, (DWORD)&mciOpen);

	MCI_PLAY_PARMS mciPlay;
	mciPlay.dwFrom = 0;

	while (GetMessage(&msg, (HWND)-1, 0, 0))
	{
		mciSendCommand(mciOpen.wDeviceID, MCI_CLOSE, MCI_WAIT, (DWORD)&mciOpen);
		SetEvent(ghEvent);

		switch (msg.message)
		{
		case wm_1: i = 0; break;
		case wm_2: i = 1; break;
		case wm_3: i = 2; break;
		case wm_4: i = 3; break;
		default:continue;
		}
		mciOpen.lpstrElementName = pronpath[i];
		mciSendCommand(0, MCI_OPEN, MCI_OPEN_ELEMENT | MCI_WAIT, (DWORD)&mciOpen);
		mciSendCommand(mciOpen.wDeviceID, MCI_PLAY, MCI_FROM, (DWORD)&mciPlay);
	}
	return 0;
}

BOOL WINAPI HandlerRoutine(DWORD dwCtrlType)
{
	if (dwCtrlType == CTRL_CLOSE_EVENT)
	{
		ResetEvent(ghEvent);
		PostThreadMessage(ThreadId, wm_0, 0, 0);
		WaitForSingleObject(ghEvent, 1000);
		return TRUE;
	}
	return FALSE;
}

DWORD WINAPI cnresult(LPVOID lpParameter)
{
	wchar_t jinurl[200] = L"http://dict-co.iciba.com/api/dictionary.php?type=json&w=";

	char wordutf8[100], * pp = wordutf8;
	int cao = wcslen(jinurl), gun;
	gun = WideCharToMultiByte(CP_UTF8, 0, (wchar_t*)lpParameter, -1, wordutf8, 100, NULL, NULL);
	while (pp < wordutf8 + gun)
	{
		jinurl[cao++] = L'%';
		StringCbPrintf(jinurl + cao, 200, L"%X", (*pp) >> 4 & 0x0f);
		cao++;
		StringCbPrintf(jinurl + cao, 200, L"%X", (*pp) & 0x0f);
		cao++;
		pp++;
	}
	jinurl[cao++] = L'\0';

	StringCbCat(jinurl, 200 * sizeof(wchar_t), icibakey);

	cnhInternet = InternetOpen(NULL, INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
	cnhFile = InternetOpenUrl(cnhInternet, jinurl, NULL, 0, INTERNET_FLAG_NO_CACHE_WRITE, 0);
	memset(xmlj, '\0', 1024 * 500);
	InternetReadFile(cnhFile, xmlj, 1024 * 500, &jNumberOfBytesRead);
	InternetCloseHandle(cnhFile);
	InternetCloseHandle(cnhInternet);

	jNumberOfBytesRead = MultiByteToWideChar(CP_UTF8, MB_PRECOMPOSED, xmlj, jNumberOfBytesRead, wxmlj, 1024 * 500);

	DWORD NumberOfCharsWritten;
	WriteConsole(hOUTPUT, L"\n金山:\n", 5, &NumberOfCharsWritten, NULL);

	memset(prondone, 0, 4 * sizeof(bool));
	wchar_t sdf[1000];
	int i = 0;
	wchar_t* p = wxmlj, * pend = wxmlj + jNumberOfBytesRead - 1;
	while (p <= pend)
	{
		////////////"word_mean"
		while ((!((p[0] == L'\"') && (p[1] == L'w') && (p[2] == L'o') && (p[3] == L'r') && (p[4] == L'd') && (p[5] == L'_') && (p[6] == L'm') && (p[7] == L'e') && (p[8] == L'a') && (p[9] == L'n') && (p[10] == L'\"'))) && p < pend)
			p++;
		p += 13;
		if (p >= pend)
			return 0;

		i = 0;
		while (*p != L'\"')
		{
			if ((p[0] == L'\\') && (p[1] == L'u'))
			{
				p += 2;
				int mabi = 16 * 16 * 16;
				sdf[i] = 0;
				for (int jjjj = 0; jjjj < 4; jjjj++)
				{
					if ((p[jjjj] >= L'0') && (p[jjjj] <= L'9'))
						sdf[i] += (p[jjjj] - L'0') * mabi;
					else if ((p[jjjj] >= L'a') && (p[jjjj] <= L'z'))
						sdf[i] += (p[jjjj] - L'a' + 10) * mabi;
					else
						sdf[i] += (p[jjjj] - L'A' + 10) * mabi;
					mabi /= 16;
				}
				i++; p += 4;
			}
			else
			{
				sdf[i] = *p;
				p++; i++;
			}
		}
		sdf[i] = L'\0';
		p++;
		WriteConsole(hOUTPUT, sdf, i, &NumberOfCharsWritten, NULL);
		printf("\n");
	}
	return 0;
}