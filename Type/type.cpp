#pragma warning(disable : 4996)

#include "windows.h"
#include "stdio.h"
#include "iostream"
#include "shlwapi.h" 
#include "time.h"
#include "digitalv.h"

#pragma comment(lib,"shlwapi.lib")
#pragma comment(lib,"Winmm.lib")

using namespace std;

static int GetUniStr(wchar_t *pstr, int size);
static void readfile(wchar_t *lrcpath);
static int __cdecl uniprintf(HANDLE hd, const wchar_t * format, ...);
static int GetChar(wchar_t *zi);
static DWORD WINAPI ThreadProc(LPVOID lpParameter);

static wchar_t lrcpath[MAX_PATH];
static DWORD fileSize = 0;

static struct LRC
{
	unsigned int time;
	unsigned int endtime;
	wchar_t word[200];
	wchar_t subtitle[200];
}lrc[11111];

static char sign[11111];
static unsigned int lrccount = 0;
static wchar_t mp3path[MAX_PATH];
static void removerepeat(LRC *);


int main(int argc, char* argv[])
{
	HANDLE hd = ::GetStdHandle(STD_OUTPUT_HANDLE);
	SMALL_RECT src = { 0,0,119,25 };
	SetConsoleWindowInfo(hd, TRUE, &src);
beginning:
	wcout << L"MP3 : ";
	GetUniStr(mp3path, MAX_PATH);
	PathUnquoteSpaces(mp3path);
	wcout << L"LRC : ";
	GetUniStr(lrcpath, MAX_PATH);
	PathUnquoteSpaces(lrcpath);
	WIN32_FIND_DATA fileInfo;
	HANDLE hFind;
	hFind = FindFirstFile(lrcpath, &fileInfo);
	if (hFind != INVALID_HANDLE_VALUE)
		fileSize = fileInfo.nFileSizeLow;
	else
	{
		wcout << L"LRC ERROR" << endl;
		FindClose(hFind);
		goto beginning;
	}
	FindClose(hFind);

	readfile(lrcpath);
	removerepeat(lrc);
	wcout << lrccount << endl;

	DWORD ThreadId;
	HANDLE handle = CreateThread(NULL, 0, ThreadProc, NULL, 0, &ThreadId);
	SetThreadPriority(handle, THREAD_PRIORITY_HIGHEST);

	srand((unsigned)time(NULL));
	unsigned int lastindex, index = rand() % lrccount;
	unsigned int done = 0;
	int i = 0, m = 0;
	wchar_t zi;
	while (1)
	{
		memset(sign, 0, 11111);
		done = 0;
		m = 0;
		while (done < lrccount)
		{
			if (done < (lrccount *0.9))
				do
				{
					index = rand() % lrccount;
				} while (sign[index] == 1);
			else
			{
				for (; m < lrccount; m++)
					if (sign[m] == 0)
					{
						index = m;
						break;
					}
			}
			uniprintf(NULL, L"s", lrc[index].word); wcout << endl;
			i = 0;
			while (lrc[index].word[i] != L'\0')
			{
				switch (GetChar(&zi))
				{
				case 0:
				{
					if (zi == lrc[index].word[i])
					{
						wcout << zi;
						i++;
						continue;
					}
					else
					{
						SetConsoleTextAttribute(hd, BACKGROUND_RED | BACKGROUND_INTENSITY | FOREGROUND_BLUE | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY);
						wcout << zi << L'\b';
						SetConsoleTextAttribute(hd, FOREGROUND_BLUE | FOREGROUND_RED | FOREGROUND_GREEN );
						continue;
					}
				}
				case 1:
				{
					wcout << endl << L"Done:" << done << L" Left:" << lrccount - done << endl;
					continue;
				}
				case 2:
				{
					PostThreadMessage(ThreadId, 3, lastindex, 0);
					continue;
				}
				case 3:
				{
					break;
				}
				case -1: 
				{
					PostThreadMessage(ThreadId, 5, 0, 0);
					Sleep(200);
					return 0;
				}
				}
				break;
			}
			lastindex = index;
			PostThreadMessage(ThreadId, 3, lastindex, 0);
			wcout << endl;
			uniprintf(NULL, L"s", lrc[index].subtitle);
			wcout << endl << endl;
			done++;
			sign[index] = 1;
		}
		SetConsoleTextAttribute(hd, FOREGROUND_RED | FOREGROUND_INTENSITY);
		wcout << L"ALL DONE\n" << endl;
		SetConsoleTextAttribute(hd, FOREGROUND_BLUE | FOREGROUND_RED | FOREGROUND_GREEN );
	}

	return 0;
}

int GetUniStr(wchar_t *pstr, int size)
{
	int wsize = size - 1 + 2;
	wchar_t *pw = (wchar_t*)malloc(wsize * sizeof(wchar_t));
	HANDLE hStdin = ::GetStdHandle(STD_INPUT_HANDLE);
	DWORD nCharsToRead = 1024L;
	DWORD cNumRead = 0L;
	DWORD cTotalRead = 0L;
	do
	{
		nCharsToRead = (wsize - cTotalRead) > nCharsToRead ? nCharsToRead : (wsize - cTotalRead);
		ReadConsoleW(hStdin, pw + cTotalRead, nCharsToRead, &cNumRead, NULL);
		cTotalRead = cTotalRead + cNumRead;
		if (cTotalRead >= (DWORD)wsize)
			break;
	} while (!((*(pw + cTotalRead - 2) == 0x0D) && (*(pw + cTotalRead - 1) == 0x0A)));
	*(pw + cTotalRead - 2) = L'\0';
	wcscpy_s(pstr, size, pw);
	free(pw);
	return 0;
}

int __cdecl uniprintf(HANDLE hd, const wchar_t * format, ...)
{
	if (hd == NULL)
		hd = GetStdHandle(STD_OUTPUT_HANDLE);
	DWORD NumberOfCharsWritten = 0;
	void **pformat = (void**)(&format + 1);
	wchar_t number[34] = L"\0";
	WORD cou = 0;
	while (*format != L'\0')
	{
		switch (*format)
		{
		case L'd':
		{
			_ltow(*(long*)pformat, number, 10);
			while (*(number + cou) != L'\0')
				cou++;
			WriteConsole(hd, number, cou, &NumberOfCharsWritten, NULL);
			pformat = (void**)((long*)pformat + 1);
			cou = 0;
			format++;
		}
		break;
		case L's':
		{
			while (*((*(wchar_t**)pformat) + cou) != L'\0')
				cou++;
			WriteConsoleW(hd, *(wchar_t**)pformat, cou,
				&NumberOfCharsWritten, NULL);
			pformat = (void**)((wchar_t**)pformat + 1);
			cou = 0;
			format++;
		}
		break;
		default:
			format++;
			break;
		}
	}
	return 0;
}

void readfile(wchar_t *lrcpath)
{
	char *file = (char*)malloc(fileSize + 1);
	file[fileSize] = '\0';
	FILE *fp;
	_wfopen_s(&fp, lrcpath, L"rb");
	fread_s(file, fileSize, fileSize, 1, fp);
	fclose(fp);

	wchar_t *wfile = (wchar_t*)malloc((fileSize + 1) * sizeof(wchar_t));
	fileSize = MultiByteToWideChar(CP_UTF8, MB_PRECOMPOSED, file, -1, wfile, fileSize + 1);
	free(file);
	unsigned int i = 12, m = 0;
	lrccount = 0;
	wchar_t *tttt;
	while (i < fileSize - 1)
	{
		i++;
		lrc[lrccount].time = (wfile[i] - L'0') * 10 + wfile[i + 1] - L'0';
		lrc[lrccount].time = 60 * lrc[lrccount].time + (wfile[i + 3] - L'0') * 10 + wfile[i + 4] - L'0';
		lrc[lrccount].time = 1000 * lrc[lrccount].time + (wfile[i + 6] - L'0') * 100 + (wfile[i + 7] - L'0') * 10;
		if (lrccount > 0)
			lrc[lrccount - 1].endtime = lrc[lrccount].time - 1;
		i = i + 9;
		m = 0;
		tttt = lrc[lrccount].word;
		while (!(wfile[i] == 0x0d && wfile[i + 1] == 0x0a))
		{
			tttt[m] = wfile[i];
			m++; i++;
			if (wfile[i] == L':')
			{
				tttt[m] = L'\0';
				tttt = lrc[lrccount].subtitle;
				m = 0; i++;
			}
			if (i >= fileSize)
				break;
			else if (m > 198)
			{
				do
				{
					i++;
					if (i >= fileSize)
						break;
				} while (!(wfile[i] == 0x0d && wfile[i + 1] == 0x0a));
				break;
			}
		}
		tttt[m] = L'\0';
		lrccount++;
		i = i + 2;
	}
	lrc[lrccount].word[0] = L'\0';
	lrc[lrccount].subtitle[0] = L'\0';
	lrc[lrccount].time = 999999999;
	lrc[lrccount].endtime = 999999999;
	if (lrccount > 0)
		lrc[lrccount - 1].endtime = 999999999;
	free(wfile);
	return;
}

int GetChar(wchar_t *zi)
{
	HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
	DWORD cNumRead;
	INPUT_RECORD irInBuf;
	int fkey = 0;
	while (1)
	{
		ReadConsoleInput(hStdin, &irInBuf, 1, &cNumRead);
		if ((KEY_EVENT == irInBuf.EventType) && (TRUE == irInBuf.Event.KeyEvent.bKeyDown))
		{
			switch (irInBuf.Event.KeyEvent.wVirtualKeyCode)
			{
			case VK_F1:fkey = 1; break;
			case VK_OEM_MINUS:fkey = 2; break;
			case VK_OEM_PLUS:fkey = 3; break;
			case VK_F4:fkey = 4; break;
			case VK_DELETE:fkey = VK_DELETE; break;
			case VK_ESCAPE:fkey = -1; break;
			case VK_BACK:continue;
			default:
				if (irInBuf.Event.KeyEvent.uChar.UnicodeChar != L'\0')
				{
					*zi = irInBuf.Event.KeyEvent.uChar.UnicodeChar;
					return 0;
				}
				else
					continue;
			}
			if ((irInBuf.Event.KeyEvent.dwControlKeyState & 0x001F) == 0x00)
				return fkey;
		}
	}
	return 0;
}

DWORD WINAPI ThreadProc(LPVOID lpParameter)
{
	UINT DeviceID;
	MCI_PLAY_PARMS mciPlay;
	MCI_OPEN_PARMS mciOpen;
	MCIERROR mciError;
	wchar_t ErrorString[128] = { L'\0' };
	mciOpen.lpstrDeviceType = L"mpegvideo";
	mciOpen.lpstrElementName = mp3path;
	mciError = mciSendCommand(0, MCI_OPEN, MCI_OPEN_TYPE | MCI_OPEN_ELEMENT, (DWORD)&mciOpen);
	if (mciError)
	{
		mciGetErrorString(mciError, ErrorString, 128);
		wcout << L"OpenErrorString = " << ErrorString << endl;
		return 1;
	}
	DeviceID = mciOpen.wDeviceID;

	

	MSG msg;
	msg.wParam = 0;
	while (1)
	{
		GetMessage(&msg, (HWND)-1, 0, 0);
		if (msg.message == 3)
		{
			mciPlay.dwFrom = lrc[msg.wParam].time;
			mciPlay.dwTo = lrc[msg.wParam].endtime; 
			mciSendCommand(DeviceID, MCI_STOP, MCI_WAIT, NULL);
			mciSendCommand(DeviceID, MCI_PLAY, MCI_FROM | MCI_TO, (DWORD)&mciPlay);
		}
		else if (msg.message == 5)
		{
			mciSendCommand(DeviceID, MCI_CLOSE, MCI_WAIT, (DWORD)&mciPlay);
		}
	}
	return 0;
}

void removerepeat(LRC *lrc)
{
	int i, m, n;
	for (i = 0; i < lrccount - 1; i++)
	{
		for (m = i + 1; m < lrccount; m++)
		{
			if (wcscmp(lrc[i].word, lrc[m].word) == 0)
			{
				n = m;
				do
				{
					lrc[n].time = lrc[n + 1].time;
					lrc[n].endtime = lrc[n + 1].endtime;
					wcscpy_s(lrc[n].word, 200, lrc[n + 1].word);
					wcscpy_s(lrc[n].subtitle, 200, lrc[n + 1].subtitle);
					n++;
				} while (n < lrccount);
				lrccount--; m--;
			}
		}
	}

	return;
}
