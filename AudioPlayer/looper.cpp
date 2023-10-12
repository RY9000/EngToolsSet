// repeater.cpp 
//
#include <iostream>
#include "windows.h"
#include "resource.h"
#include "shlobj_core.h"
#include "Pathcch.h"
#include "digitalv.h"
#include "shlwapi.h"
#include "shellscalingapi.h"

#pragma comment(lib,"Pathcch.lib")
#pragma comment(lib,"Winmm.lib")
#pragma comment(lib,"Shlwapi.lib")
#pragma comment(lib,"Shcore.lib")

#define	m_previous		WM_APP
#define	m_next			m_previous+1
#define	m_volup			m_next+1
#define	m_voldown		m_volup+1
#define	m_pp			m_voldown+1
#define	m_repeat		m_pp+1
#define	m_pmode			m_repeat+1
#define	m_rmode			m_pmode+1
#define	m_cmode			m_rmode+1
#define	m_time			m_cmode+1
#define	m_timeNOW		m_time+1
#define	m_subtitle		m_timeNOW+1
#define	m_subtitleNOW	m_subtitle+1
#define	m_refresh		m_subtitleNOW+1
#define	m_startfrom		m_refresh+1
#define	m_addsub		m_startfrom+1
#define	m_delsub		m_addsub+1
#define	m_back2sec		m_delsub+1

HANDLE	hOutPut = GetStdHandle(STD_OUTPUT_HANDLE);
HANDLE	hInPut = GetStdHandle(STD_INPUT_HANDLE);
HANDLE	hThreadPlay;
HWND	hWindow = GetConsoleWindow();
HWND	hDicWindow = NULL;
WORD	nofmp3 = 0;
WORD	startfrom = 0;
WORD	nofsub = 0;
DWORD	consoleID = GetCurrentThreadId();
DWORD	ThreadPlayId;
DWORD	medialength;
const wchar_t* pstate[2] = { L"One|",L"All|" };
const wchar_t* rstate[2] = { L"Once |",L"Twice|" };
const wchar_t* cstate[3] = { L"2sec ",L"LRC  ",L"Auto" };
const wchar_t* krepeating[2] = { L"             ",L"KeepRepeating" };
wchar_t ffmpeg[] = L"C:\\Program Files (x86)\\mkvtoolnix\\ffmpeg.exe";
wchar_t	mp3path[2000][MAX_PATH];

struct {
	DWORD	time;
	wchar_t	text[101];
	BYTE	length;
}subtitle[3000];

DWORD WINAPI ThreadPLay(LPVOID lpParameter);
DWORD WINAPI ThreadTime(LPVOID lpParameter);
DWORD WINAPI ThreadSub(LPVOID lpParameter);
BOOL CALLBACK ListProc(HWND hDlg, UINT msg, WPARAM wPar, LPARAM lPar);
BYTE deletesub(DWORD);
void readlrc();
void auto_cut();
void sendki(INPUT_RECORD);
void finddictionary();
void addsub(DWORD, DWORD);
void savelrc();


int main()
{
	SetConsoleTitle(L"Loop Player");
	SMALL_RECT ConsoleWindow = { 0,0,99,25 };
	SetConsoleWindowInfo(hOutPut, TRUE, &ConsoleWindow);
	SetConsoleScreenBufferSize(hOutPut, { 100,26 });
	SetConsoleMode(hInPut, ENABLE_WINDOW_INPUT | ENABLE_EXTENDED_FLAGS | ENABLE_MOUSE_INPUT);
	CONSOLE_CURSOR_INFO ConsoleCursorInfo = { 1,FALSE };
	SetConsoleCursorInfo(hOutPut, &ConsoleCursorInfo);
	SetProcessDpiAwareness(PROCESS_SYSTEM_DPI_AWARE);


	wchar_t DisplayName[MAX_PATH];
	BROWSEINFO bi;
	memset(&bi, 0, sizeof(bi));
	bi.hwndOwner = hWindow;
	bi.pszDisplayName = DisplayName;
	wchar_t folderpath[MAX_PATH];
	SHGetPathFromIDList(SHBrowseForFolder(&bi), folderpath);


	DWORD	NumberOfCharsWritten;
	HANDLE	hFind = INVALID_HANDLE_VALUE;
	WIN32_FIND_DATA	ffd;
	swprintf_s(mp3path[0], MAX_PATH, L"%ls\\*.mp3", folderpath);
	hFind = FindFirstFile(mp3path[0], &ffd);
	if (INVALID_HANDLE_VALUE == hFind)
	{
		SetConsoleTextAttribute(hOutPut, FOREGROUND_RED | FOREGROUND_INTENSITY);
		WriteConsole(hOutPut, L"NO MP3 HAS BEEN FOUND!", 22, &NumberOfCharsWritten, NULL);
		Sleep(1000 * 60);
		return 0;
	}

	swprintf_s(mp3path[nofmp3], MAX_PATH, L"%ls\\%ls", folderpath, ffd.cFileName);
	nofmp3++;


	while (FindNextFile(hFind, &ffd))
	{
		swprintf_s(mp3path[nofmp3], MAX_PATH, L"%ls\\%ls", folderpath, ffd.cFileName);
		nofmp3++;
		if (nofmp3 >= 2000)
			break;
	}
	FindClose(hFind);


	if (!DialogBox(NULL, MAKEINTRESOURCE(IDD_LISTDIALOG), hWindow, (DLGPROC)ListProc))
		startfrom = 0;


	finddictionary();


	hThreadPlay = CreateThread(NULL, 0, ThreadPLay, NULL, 0, &ThreadPlayId);
	if (NULL == hThreadPlay)
	{
		wchar_t displaystr[101];
		swprintf_s(displaystr, 100, L"Creating Thread Fails. Error Code: %d", GetLastError());
		SetConsoleTextAttribute(hOutPut, FOREGROUND_RED | FOREGROUND_INTENSITY);
		WriteConsole(hOutPut, displaystr, 100, &NumberOfCharsWritten, NULL);
		Sleep(1000 * 60);
		return 0;
	}

	INPUT_RECORD irInBuf;
	DWORD	NumberOfEventsRead;
	while (ReadConsoleInput(hInPut, &irInBuf, 1, &NumberOfEventsRead))
	{
		switch (irInBuf.EventType)
		{
		case KEY_EVENT:
			if (FALSE == irInBuf.Event.KeyEvent.bKeyDown)
				continue;
			switch (irInBuf.Event.KeyEvent.wVirtualKeyCode)
			{
			case VK_LEFT:
				PostThreadMessage(ThreadPlayId, m_previous, 0, 0);
				continue;
			case VK_UP:
				PostThreadMessage(ThreadPlayId, m_volup, 0, 0);
				continue;
			case VK_RIGHT:
				PostThreadMessage(ThreadPlayId, m_next, 0, 0);
				continue;
			case VK_DOWN:
				PostThreadMessage(ThreadPlayId, m_voldown, 0, 0);
				continue;
			case VK_OEM_2:/// /?
				PostThreadMessage(ThreadPlayId, m_pp, 0, 0);
				continue;
			case VK_F5:
				finddictionary();
				continue;
			case VK_INSERT:
				PostThreadMessage(ThreadPlayId, m_addsub, 0, 0);
				continue;
			case VK_DELETE:
				PostThreadMessage(ThreadPlayId, m_delsub, 0, 0);
				continue;
			default:
				if (hDicWindow != NULL)
					sendki(irInBuf);
				continue;
			}
			continue;
		case MOUSE_EVENT:
			switch (irInBuf.Event.MouseEvent.dwEventFlags)
			{
			case DOUBLE_CLICK:
			case 0://a mouse button being pressed or released.
				switch (irInBuf.Event.MouseEvent.dwButtonState)
				{
				case FROM_LEFT_1ST_BUTTON_PRESSED:
					if (irInBuf.Event.MouseEvent.dwMousePosition.Y == 0)
					{
						if (irInBuf.Event.MouseEvent.dwMousePosition.X == 59)
							PostThreadMessage(ThreadPlayId, m_pmode, 0, 0);
						else if (irInBuf.Event.MouseEvent.dwMousePosition.X == 63)
							PostThreadMessage(ThreadPlayId, m_rmode, 0, 0);
						else if (irInBuf.Event.MouseEvent.dwMousePosition.X == 69)
							PostThreadMessage(ThreadPlayId, m_cmode, 0, 0);
						else if (irInBuf.Event.MouseEvent.dwMousePosition.X == 0)
						{
							AttachThreadInput(consoleID, GetWindowThreadProcessId(hDicWindow, NULL), FALSE);
							if (DialogBox(NULL, MAKEINTRESOURCE(IDD_LISTDIALOG), hWindow, (DLGPROC)ListProc))
								PostThreadMessage(ThreadPlayId, m_startfrom, 0, 0);
							finddictionary();
						}
					}
					else
						PostThreadMessage(ThreadPlayId, m_pp, 0, 0);
					continue;
				case RIGHTMOST_BUTTON_PRESSED:
					PostThreadMessage(ThreadPlayId, m_repeat, 0, 0);
					continue;
				case FROM_LEFT_2ND_BUTTON_PRESSED:
					PostThreadMessage(ThreadPlayId, m_back2sec, 0, 0);
					continue;
				default:continue;
				}
				continue;
			case MOUSE_WHEELED:
				if (HIWORD(irInBuf.Event.MouseEvent.dwButtonState) < 30000)
					PostThreadMessage(ThreadPlayId, m_previous, 0, 0);
				else
					PostThreadMessage(ThreadPlayId, m_next, 0, 0);
				continue;
			default:continue;
			}
			continue;
		case WINDOW_BUFFER_SIZE_EVENT:
			if ((irInBuf.Event.WindowBufferSizeEvent.dwSize.X != 100) || (irInBuf.Event.WindowBufferSizeEvent.dwSize.Y != 26))
			{
				SetConsoleWindowInfo(hOutPut, TRUE, &ConsoleWindow);
				SetConsoleScreenBufferSize(hOutPut, { 100,26 });
				PostThreadMessage(ThreadPlayId, m_refresh, 0, 0);
			}
			continue;
		default:continue;
		}
	}
	return 0;
}


DWORD WINAPI ThreadPLay(LPVOID lpParameter)
{
	MCI_OPEN_PARMS	mciOpenParms;
	MCI_PLAY_PARMS	mciPlayParms;
	MCI_STATUS_PARMS		Status, track;
	MCI_DGV_SETAUDIO_PARMS	thevolume;
	MCI_SEEK_PARMS	Seek;
	DWORD	fdwError;
	wchar_t	mciError[100];

	thevolume.dwItem = MCI_DGV_SETAUDIO_VOLUME;
	thevolume.dwValue = 50;

	track.dwItem = MCI_STATUS_POSITION;

	BYTE	pmode = 0, rmode = 0, cmode = 1, keeprepeating = 0, pp = 0, nofrepeat = 0;
	MSG		msg;
	DWORD	index = 0;
	DWORD	NumberOfCharsWritten;
	DWORD	ThreadTimeId, ThreadSubId;
	HANDLE	hThreadTime, hThreadSub;


	hThreadTime = CreateThread(NULL, 0, ThreadTime, NULL, 0, &ThreadTimeId);
	hThreadSub = CreateThread(NULL, 0, ThreadSub, NULL, 0, &ThreadSubId);


beginning:
	FillConsoleOutputCharacter(hOutPut, L' ', 100 * 26, { 0,0 }, &NumberOfCharsWritten);


	mciOpenParms.lpstrElementName = mp3path[startfrom];
	fdwError = mciSendCommand(0, MCI_OPEN, MCI_OPEN_ELEMENT | MCI_WAIT, (DWORD)&mciOpenParms);
	if (fdwError)
	{
		mciGetErrorString(fdwError, mciError, 100);
		MessageBox(hWindow, mciError, L"OpenError", 0);
		Sleep(1000000);
	}


	WriteConsoleOutputCharacter(hOutPut, PathFindFileName(mp3path[startfrom]), wcslen(PathFindFileName(mp3path[startfrom])),
		{ 0,0 }, &NumberOfCharsWritten);
	FillConsoleOutputCharacter(hOutPut, L' ', 60, { 37,0 }, &NumberOfCharsWritten);


	mciSendCommand(mciOpenParms.wDeviceID, MCI_SETAUDIO, MCI_DGV_SETAUDIO_ITEM | MCI_DGV_SETAUDIO_VALUE, (DWORD)&thevolume);


	Status.dwItem = MCI_STATUS_LENGTH;
	mciSendCommand(mciOpenParms.wDeviceID, MCI_STATUS, MCI_STATUS_ITEM | MCI_WAIT, (DWORD)&Status);
	medialength = Status.dwReturn;


	pp = 1;
	index = 0;
	mciPlayParms.dwFrom = 0;

	if (1 == cmode)
	{
		readlrc();
		if (nofsub == 0)
			cmode = 2;
	}
	if (2 == cmode)
	{
		auto_cut();
		if (nofsub == 0)
			cmode = 0;
	}

	if ((rmode == 0) && (keeprepeating == 0))
		mciPlayParms.dwTo = medialength;
	else
	{
		if (cmode == 0)
			mciPlayParms.dwTo = 2000;
		else
			mciPlayParms.dwTo = subtitle[1].time;
		nofrepeat = 2;
	}
	fdwError = mciSendCommand(mciOpenParms.wDeviceID, MCI_PLAY, MCI_FROM | MCI_TO, (DWORD)&mciPlayParms);
	if (fdwError)
	{
		mciGetErrorString(fdwError, mciError, 100);
		MessageBox(hWindow, mciError, L"PlayError", 0);
		Sleep(1000000);
	}

	WriteConsoleOutputCharacter(hOutPut, pstate[pmode], 4, { 59,0 }, &NumberOfCharsWritten);
	WriteConsoleOutputCharacter(hOutPut, rstate[rmode], 6, { 63,0 }, &NumberOfCharsWritten);
	WriteConsoleOutputCharacter(hOutPut, cstate[cmode], 5, { 69,0 }, &NumberOfCharsWritten);
	WriteConsoleOutputCharacter(hOutPut, krepeating[keeprepeating], 13, { 77,0 }, &NumberOfCharsWritten);


	BYTE PreOrNext = 0;
	while (1)
	{
		mciSendCommand(mciOpenParms.wDeviceID, MCI_STATUS, MCI_WAIT | MCI_STATUS_ITEM, (DWORD)&track);
		PostThreadMessage(ThreadTimeId, m_time, track.dwReturn, medialength);
		if (cmode != 0)
		{
			while (track.dwReturn > subtitle[index + 1].time)
				index = (index + 1) > (nofsub - 1) ? (nofsub - 1) : (index + 1);
			PostThreadMessage(ThreadSubId, m_subtitle, cmode, index);
		}
		else
			PostThreadMessage(ThreadSubId, m_subtitle, cmode, track.dwReturn);


	PeekMsg:
		if (PeekMessage(&msg, (HWND)-1, 0, 0, PM_REMOVE))
		{
			switch (msg.message)
			{
			case m_addsub:
				if (1 == cmode)
				{
					mciSendCommand(mciOpenParms.wDeviceID, MCI_STATUS, MCI_WAIT | MCI_STATUS_ITEM, (DWORD)&track);
					addsub(index, track.dwReturn);
					PostThreadMessage(ThreadSubId, m_subtitleNOW, cmode, index);
					if ((rmode == 1) || (keeprepeating == 1))
					{
						mciPlayParms.dwFrom = subtitle[index].time;
						mciPlayParms.dwTo = subtitle[index + 1].time;
						nofrepeat = 2;
						if (pp == 1)
							mciSendCommand(mciOpenParms.wDeviceID, MCI_PLAY, MCI_TO, (DWORD)&mciPlayParms);
						else
						{
							Seek.dwTo = mciPlayParms.dwFrom;
							mciSendCommand(mciOpenParms.wDeviceID, MCI_SEEK, MCI_TO, (DWORD)&Seek);
						}
					}
				}
				break;
			case m_delsub:
				if (1 == cmode)
				{
					if (!deletesub(index))
						break;
					index--;
					PostThreadMessage(ThreadSubId, m_subtitleNOW, cmode, index);
					if ((rmode == 1) || (keeprepeating == 1))
					{
						mciPlayParms.dwFrom = subtitle[index].time;
						mciPlayParms.dwTo = subtitle[index + 1].time;
						nofrepeat = 2;
					}
				}
				break;
			case m_previous:
				if (0 == cmode)
				{
					if (track.dwReturn <= 2000)
					{
						mciPlayParms.dwFrom = 0;
						if ((rmode == 1) || (keeprepeating == 1))
							mciPlayParms.dwTo = 2000;
					}
					else
					{
						mciPlayParms.dwFrom = (track.dwReturn / 2000 - 1) * 2000;
						if ((rmode == 1) || (keeprepeating == 1))
							mciPlayParms.dwTo = mciPlayParms.dwFrom + 2000;
					}
					track.dwReturn = mciPlayParms.dwFrom;
					PostThreadMessage(ThreadSubId, m_subtitleNOW, cmode, track.dwReturn);
				}
				else
				{
					if (index > 0)
						index--;
					mciPlayParms.dwFrom = subtitle[index].time;
					if ((rmode == 1) || (keeprepeating == 1))
						mciPlayParms.dwTo = subtitle[index + 1].time;
					PostThreadMessage(ThreadSubId, m_subtitleNOW, cmode, index);
				}
				PreOrNext = 1;
				nofrepeat = 2;
				PostThreadMessage(ThreadTimeId, m_timeNOW, mciPlayParms.dwFrom, medialength);
				goto PeekMsg;
			case m_next:
				if (0 == cmode)
				{
					mciPlayParms.dwFrom = (track.dwReturn / 2000 + 1) * 2000;
					if (mciPlayParms.dwFrom >= medialength)
						mciPlayParms.dwFrom = medialength - 1;

					if ((rmode == 1) || (keeprepeating == 1))
					{
						mciPlayParms.dwTo = mciPlayParms.dwFrom + 2000;
						if (mciPlayParms.dwTo > medialength)
							mciPlayParms.dwTo = medialength;
					}
					track.dwReturn = mciPlayParms.dwFrom;
					PostThreadMessage(ThreadSubId, m_subtitleNOW, cmode, track.dwReturn);
				}
				else
				{
					if (index < (nofsub - 1))
						index++;
					mciPlayParms.dwFrom = subtitle[index].time;
					if ((rmode == 1) || (keeprepeating == 1))
						mciPlayParms.dwTo = subtitle[index + 1].time;
					PostThreadMessage(ThreadSubId, m_subtitleNOW, cmode, index);
				}
				PreOrNext = 1;
				nofrepeat = 2;
				PostThreadMessage(ThreadTimeId, m_timeNOW, mciPlayParms.dwFrom, medialength);
				goto PeekMsg;
			case m_volup:
				thevolume.dwValue = (thevolume.dwValue + 50) > 1000 ? 1000 : (thevolume.dwValue + 50);
				mciSendCommand(mciOpenParms.wDeviceID, MCI_SETAUDIO, MCI_DGV_SETAUDIO_ITEM | MCI_DGV_SETAUDIO_VALUE,
					(DWORD)&thevolume);
				break;
			case m_voldown:
				thevolume.dwValue = thevolume.dwValue == 0 ? 0 : (thevolume.dwValue - 50);
				mciSendCommand(mciOpenParms.wDeviceID, MCI_SETAUDIO, MCI_DGV_SETAUDIO_ITEM | MCI_DGV_SETAUDIO_VALUE,
					(DWORD)&thevolume);
				break;
			case m_pp:
				pp = pp ^ 0x01;
				MCI_PLAY_PARMS	mciPreviousPlayParms;
				DWORD	pauseposition;
				if (0 == pp)
				{
					mciSendCommand(mciOpenParms.wDeviceID, MCI_PAUSE, MCI_WAIT, NULL);
					Status.dwItem = MCI_STATUS_POSITION;
					mciSendCommand(mciOpenParms.wDeviceID, MCI_STATUS, MCI_WAIT | MCI_STATUS_ITEM, (DWORD)&Status);
					pauseposition = Status.dwReturn + 1;
					mciPreviousPlayParms = mciPlayParms;
				}
				else
				{
					if (mciPlayParms.dwFrom == mciPreviousPlayParms.dwFrom)
					{
						mciPreviousPlayParms = mciPlayParms;
						mciPreviousPlayParms.dwFrom = pauseposition;
						mciSendCommand(mciOpenParms.wDeviceID, MCI_PLAY, MCI_FROM | MCI_TO, (DWORD)&mciPreviousPlayParms);
					}
					else
						mciSendCommand(mciOpenParms.wDeviceID, MCI_PLAY, MCI_FROM | MCI_TO, (DWORD)&mciPlayParms);
				}
				break;
			case m_pmode:
				pmode = pmode ^ 0x01;
				WriteConsoleOutputCharacter(hOutPut, pstate[pmode], 4, { 59,0 }, &NumberOfCharsWritten);
				break;
			case m_rmode:
				rmode = rmode ^ 0x01;
				if (rmode == 0)
					mciPlayParms.dwTo = medialength;
				else
				{
					if (0 == cmode)
					{
						mciPlayParms.dwFrom = (track.dwReturn / 2000) * 2000;
						mciPlayParms.dwTo = mciPlayParms.dwFrom + 2000;
					}
					else
					{
						mciPlayParms.dwFrom = subtitle[index].time;
						mciPlayParms.dwTo = subtitle[index + 1].time;
					}
					nofrepeat = 2;
				}
				if (pp == 1)
					mciSendCommand(mciOpenParms.wDeviceID, MCI_PLAY, MCI_TO, (DWORD)&mciPlayParms);
				else
				{
					Seek.dwTo = mciPlayParms.dwFrom;
					mciSendCommand(mciOpenParms.wDeviceID, MCI_SEEK, MCI_TO, (DWORD)&Seek);
				}
				WriteConsoleOutputCharacter(hOutPut, rstate[rmode], 6, { 63,0 }, &NumberOfCharsWritten);
				break;
			case m_cmode:
				cmode = (++cmode) % 3;
				if (1 == cmode)
				{
					readlrc();
					if (nofsub == 0)
						cmode = 2;
					else
					{
						for (index = 0; track.dwReturn >= subtitle[index + 1].time; )
							index++;
						PostThreadMessage(ThreadSubId, m_subtitleNOW, cmode, index);
					}
				}
				if (2 == cmode)
				{
					auto_cut();
					if (nofsub == 0)
						cmode = 0;
					else
					{
						for (index = 0; track.dwReturn >= subtitle[index + 1].time; )
							index++;
						PostThreadMessage(ThreadSubId, m_subtitleNOW, cmode, index);
					}
				}
				if ((rmode == 1) || (keeprepeating == 1))
				{
					if (cmode == 0)
					{
						mciPlayParms.dwFrom = (track.dwReturn / 2000) * 2000;
						mciPlayParms.dwTo = mciPlayParms.dwFrom + 2000;
					}
					else
					{
						mciPlayParms.dwFrom = subtitle[index].time;
						mciPlayParms.dwTo = subtitle[index + 1].time;
					}
					nofrepeat = 2;
					if (pp == 1)
						mciSendCommand(mciOpenParms.wDeviceID, MCI_PLAY, MCI_TO, (DWORD)&mciPlayParms);
					else
					{
						Seek.dwTo = mciPlayParms.dwFrom;
						mciSendCommand(mciOpenParms.wDeviceID, MCI_SEEK, MCI_TO, (DWORD)&Seek);
					}
				}
				WriteConsoleOutputCharacter(hOutPut, cstate[cmode], 5, { 69,0 }, &NumberOfCharsWritten);
				break;
			case m_repeat:
				keeprepeating = keeprepeating ^ 0x01;
				if (keeprepeating == 1)
				{
					if (0 == cmode)
					{
						mciPlayParms.dwFrom = (track.dwReturn / 2000) * 2000;
						mciPlayParms.dwTo = mciPlayParms.dwFrom + 2000;
					}
					else
					{
						mciPlayParms.dwFrom = subtitle[index].time;
						mciPlayParms.dwTo = subtitle[index + 1].time;
					}
				}
				else
				{
					if (rmode == 0)
						mciPlayParms.dwTo = medialength;
					else
					{
						if (0 == cmode)
						{
							mciPlayParms.dwFrom = (track.dwReturn / 2000) * 2000;
							mciPlayParms.dwTo = mciPlayParms.dwFrom + 2000;
						}
						else
						{
							mciPlayParms.dwFrom = subtitle[index].time;
							mciPlayParms.dwTo = subtitle[index + 1].time;
						}
					}
				}
				if (pp == 1)
				{
					mciSendCommand(mciOpenParms.wDeviceID, MCI_PLAY, MCI_TO, (DWORD)&mciPlayParms);
					nofrepeat = 1;
				}
				else
				{
					Seek.dwTo = mciPlayParms.dwFrom;
					mciSendCommand(mciOpenParms.wDeviceID, MCI_SEEK, MCI_TO, (DWORD)&Seek);
				}
				WriteConsoleOutputCharacter(hOutPut, krepeating[keeprepeating], 13, { 77,0 }, &NumberOfCharsWritten);
				break;
			case m_refresh:
				FillConsoleOutputCharacter(hOutPut, L' ', 100 * 2, { 0,0 }, &NumberOfCharsWritten);
				WriteConsoleOutputCharacter(hOutPut, PathFindFileName(mp3path[startfrom]),
					wcslen(PathFindFileName(mp3path[startfrom])), { 0,0 }, &NumberOfCharsWritten);
				FillConsoleOutputCharacter(hOutPut, L' ', 60, { 40,0 }, &NumberOfCharsWritten);
				WriteConsoleOutputCharacter(hOutPut, pstate[pmode], 4, { 59,0 }, &NumberOfCharsWritten);
				WriteConsoleOutputCharacter(hOutPut, rstate[rmode], 6, { 63,0 }, &NumberOfCharsWritten);
				WriteConsoleOutputCharacter(hOutPut, cstate[cmode], 5, { 69,0 }, &NumberOfCharsWritten);
				WriteConsoleOutputCharacter(hOutPut, krepeating[keeprepeating], 13, { 77,0 }, &NumberOfCharsWritten);
				FillConsoleOutputAttribute(hOutPut, FOREGROUND_GREEN | FOREGROUND_INTENSITY, 100, { 0,22 }, &NumberOfCharsWritten);
				break;
			case m_back2sec:
				if (cmode == 0)
					break;
				MCI_PLAY_PARMS	mciPlayParms_2 = mciPlayParms;
				mciPlayParms_2.dwFrom = track.dwReturn > 2000 ? track.dwReturn - 2000 : 0;
				if (mciPlayParms_2.dwFrom < subtitle[index].time)
				{
					index--;
					PostThreadMessage(ThreadSubId, m_subtitleNOW, cmode, index);
				}
				PostThreadMessage(ThreadTimeId, m_timeNOW, mciPlayParms_2.dwFrom, medialength);
				mciSendCommand(mciOpenParms.wDeviceID, MCI_PLAY, MCI_FROM | MCI_TO, (DWORD)&mciPlayParms_2);
				break;
			case m_startfrom:
				mciSendCommand(mciOpenParms.wDeviceID, MCI_CLOSE, MCI_WAIT, NULL);
				goto beginning;
			}
		}

		if (PreOrNext == 1)
		{
			if (1 == pp)
				mciSendCommand(mciOpenParms.wDeviceID, MCI_PLAY, MCI_FROM | MCI_TO, (DWORD)&mciPlayParms);
			else
			{
				Seek.dwTo = mciPlayParms.dwFrom;
				mciSendCommand(mciOpenParms.wDeviceID, MCI_SEEK, MCI_TO, (DWORD)&Seek);
			}
			PreOrNext = 0;
		}


		Status.dwItem = MCI_STATUS_MODE;
		mciSendCommand(mciOpenParms.wDeviceID, MCI_STATUS, MCI_WAIT | MCI_STATUS_ITEM, (DWORD)&Status);

		if ((MCI_MODE_STOP == Status.dwReturn) && (pp == 1))
		{
			if (keeprepeating == 1)
			{
				mciSendCommand(mciOpenParms.wDeviceID, MCI_PLAY, MCI_FROM | MCI_TO, (DWORD)&mciPlayParms);
				continue;
			}


			if (rmode == 1)
			{
				nofrepeat--;
				if (nofrepeat == 0)
				{
					nofrepeat = 2;
					if (mciPlayParms.dwTo == medialength)
					{
						if (pmode == 1)
							startfrom = (startfrom + 1) > (nofmp3 - 1) ? 0 : (startfrom + 1);
						mciSendCommand(mciOpenParms.wDeviceID, MCI_CLOSE, MCI_WAIT, NULL);
						goto beginning;
					}

					if (cmode == 0)
					{
						mciPlayParms.dwFrom = mciPlayParms.dwTo;
						mciPlayParms.dwTo = (mciPlayParms.dwFrom + 2000) > medialength ? medialength : (mciPlayParms.dwFrom + 2000);
					}
					else
					{
						index++;
						mciPlayParms.dwFrom = subtitle[index].time;
						mciPlayParms.dwTo = subtitle[index + 1].time;
					}
				}
				mciSendCommand(mciOpenParms.wDeviceID, MCI_PLAY, MCI_FROM | MCI_TO, (DWORD)&mciPlayParms);
			}
			else
			{
				if (pmode == 1)
					startfrom = (startfrom + 1) > (nofmp3 - 1) ? 0 : (startfrom + 1);
				mciSendCommand(mciOpenParms.wDeviceID, MCI_CLOSE, MCI_WAIT, NULL);
				goto beginning;
			}
		}
		Sleep(1);
	}
	return 0;
}


DWORD WINAPI ThreadSub(LPVOID lpParameter)
{
	wchar_t	display[100] = L"\0";
	MSG		msg;
	DWORD	NumberOfCharsWritten, index;
	BYTE	row;
	LPARAM	lPar = -1;

	FillConsoleOutputAttribute(hOutPut, FOREGROUND_GREEN | FOREGROUND_INTENSITY, 100, { 0,22 }, &NumberOfCharsWritten);

	while (GetMessage(&msg, (HWND)-1, 0, 0))
	{
		switch (msg.message)
		{
		case m_subtitle:
			if ((0 == msg.wParam) && (lPar == (msg.lParam / 2000) * 2000))
				continue;
			else if ((0 != msg.wParam) && (lPar == msg.lParam))
				continue;
		case m_subtitleNOW:
			if (0 == msg.wParam)
			{
				index = (msg.lParam / 2000) * 2000;
				lPar = index;
				for (row = 22; row >= 2; )
				{
					FillConsoleOutputCharacter(hOutPut, L' ', 100, { 0,row }, &NumberOfCharsWritten);
					swprintf_s(display, 100, L"[%02d:%02d]", index / 1000 / 60, index / 1000 % 60);
					WriteConsoleOutputCharacter(hOutPut, display, wcslen(display), { 0,row }, &NumberOfCharsWritten);
					row--;
					if (index == 0)
						break;
					index = index - 2000;
				}
				for (; row >= 2; row--)
					FillConsoleOutputCharacter(hOutPut, L' ', 100, { 0,row }, &NumberOfCharsWritten);

				index = (msg.lParam / 2000) * 2000 + 2000;
				for (row = 23; row <= 25; )
				{
					if (index >= medialength)
						break;
					FillConsoleOutputCharacter(hOutPut, L' ', 100, { 0,row }, &NumberOfCharsWritten);
					swprintf_s(display, 100, L"[%02d:%02d]", index / 1000 / 60, index / 1000 % 60);
					WriteConsoleOutputCharacter(hOutPut, display, wcslen(display), { 0,row }, &NumberOfCharsWritten);
					row++;
					index = index + 2000;
				}
				for (; row <= 25; row++)
					FillConsoleOutputCharacter(hOutPut, L' ', 100, { 0,row }, &NumberOfCharsWritten);
			}
			else
			{
				index = msg.lParam;
				lPar = index;
				for (row = 22; row >= 2; )
				{
					FillConsoleOutputCharacter(hOutPut, L' ', 100, { 0,row }, &NumberOfCharsWritten);
					WriteConsoleOutputCharacter(hOutPut, subtitle[index].text, subtitle[index].length, { 0,row },
						&NumberOfCharsWritten);
					if (0 == subtitle[index].length)
					{
						swprintf_s(display, 100, L"<%02d:%02d>", subtitle[index].time / 1000 / 60,
							subtitle[index].time / 1000 % 60);
						WriteConsoleOutputCharacter(hOutPut, display, wcslen(display), { 0,row }, &NumberOfCharsWritten);
					}
					row--;
					if (index == 0)
						break;
					index--;
				}
				for (; row >= 2; row--)
					FillConsoleOutputCharacter(hOutPut, L' ', 100, { 0,row }, &NumberOfCharsWritten);

				index = msg.lParam;
				for (row = 23; row <= 25; row++)
				{
					if (index == nofsub - 1)
						break;
					index++;
					FillConsoleOutputCharacter(hOutPut, L' ', 100, { 0,row }, &NumberOfCharsWritten);
					if (0 != subtitle[index].length)
						WriteConsoleOutputCharacter(hOutPut, subtitle[index].text, subtitle[index].length,
							{ 0,row }, &NumberOfCharsWritten);
					else
					{
						swprintf_s(display, 100, L"<%02d:%02d>", subtitle[index].time / 1000 / 60,
							subtitle[index].time / 1000 % 60);
						WriteConsoleOutputCharacter(hOutPut, display, wcslen(display), { 0,row }, &NumberOfCharsWritten);
					}
				}
				for (; row <= 25; row++)
					FillConsoleOutputCharacter(hOutPut, L' ', 100, { 0,row }, &NumberOfCharsWritten);
			}
			continue;
		}
	}
	return 0;
}


DWORD WINAPI ThreadTime(LPVOID lpParameter)
{
	wchar_t	displaytime[16] = L"\0";
	MSG		msg;
	DWORD	NumberOfCharsWritten;
	BYTE	delay = 1;

	while (GetMessage(&msg, (HWND)-1, 0, 0))
	{
		switch (msg.message)
		{
		case m_time:
			if (((--delay) & 0x7F) != 0)
				continue;
		case m_timeNOW:
			swprintf_s(displaytime, 16, L"%02d:%02d - %02d:%02d",
				msg.wParam / 1000 / 60, msg.wParam / 1000 % 60, msg.lParam / 1000 / 60, msg.lParam / 1000 % 60);
			WriteConsoleOutputCharacter(hOutPut, displaytime, 13, { 41,0 }, &NumberOfCharsWritten);
		}
	}
	return 0;
}


void readlrc()
{
	wchar_t lrcpath[MAX_PATH];
	swprintf_s(lrcpath, MAX_PATH, L"%ls", mp3path[startfrom]);
	PathCchRenameExtension(lrcpath, MAX_PATH, L"lrc");

	HANDLE	hFind = INVALID_HANDLE_VALUE;
	WIN32_FIND_DATA	ffd;
	hFind = FindFirstFile(lrcpath, &ffd);
	if (INVALID_HANDLE_VALUE == hFind)
	{
		nofsub = 0;
		return;
	}
	FindClose(hFind);
	DWORD filesize = ffd.nFileSizeLow;


	char* lrcfile = (char*)malloc(filesize);
	HANDLE hFile = CreateFile(lrcpath, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	DWORD NumberOfBytesRead;
	ReadFile(hFile, lrcfile, filesize, &NumberOfBytesRead, NULL);
	CloseHandle(hFile);


	wchar_t* wlrcfile = (wchar_t*)malloc(filesize * 2);
	MultiByteToWideChar(CP_UTF8, MB_PRECOMPOSED, lrcfile, filesize, wlrcfile, filesize);
	free(lrcfile);


	wchar_t* p = wlrcfile, * pend = wlrcfile + filesize - 1;
	nofsub = 0;
	BYTE i = 0;


	subtitle[nofsub].time = 0;
	subtitle[nofsub].text[0] = L'\0';
	subtitle[nofsub].length = 0;
	nofsub++;
	while (p <= pend)
	{
		if (p[0] != L'[')
			break;


		subtitle[nofsub].time = (p[1] - L'0') * 10 * 60 * 1000 +
			(p[2] - L'0') * 60 * 1000 + (p[4] - L'0') * 10 * 1000 +
			(p[5] - L'0') * 1000 + (p[7] - L'0') * 100 + (p[8] - L'0') * 10;
		p = p + 10;


		if (subtitle[nofsub].time == 0)
			nofsub--;


		while ((p[0] != 0x0D) && (p <= pend))
		{
			subtitle[nofsub].text[i] = *p;
			p++; i++;
			if (i > 99)
				break;
		}
		subtitle[nofsub].text[i] = L'\0';
		subtitle[nofsub].length = i;
		nofsub++;

		if (nofsub >= 2999)
			break;
		i = 0;
		p = p + 2;
	}
	free(wlrcfile);
	subtitle[nofsub].time = medialength;
	subtitle[nofsub].text[0] = L'\0';
	subtitle[nofsub].length = 0;
	return;
}


void auto_cut()
{
	wchar_t szCmdline[1000];

	swprintf_s(szCmdline, 1000, L" -i \"%ls\"  -af silencedetect=noise=-51dB:duration=0.15 -f null -", mp3path[startfrom]);

	HANDLE	hReadPipe, hWritePipe;
	SECURITY_ATTRIBUTES saAttr;
	saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
	saAttr.bInheritHandle = TRUE;
	saAttr.lpSecurityDescriptor = NULL;
	CreatePipe(&hReadPipe, &hWritePipe, &saAttr, 1024 * 1024);

	PROCESS_INFORMATION piProcInfo;
	STARTUPINFO siStartInfo;
	ZeroMemory(&piProcInfo, sizeof(PROCESS_INFORMATION));
	ZeroMemory(&siStartInfo, sizeof(STARTUPINFO));
	siStartInfo.cb = sizeof(STARTUPINFO);
	siStartInfo.hStdError = hWritePipe;
	siStartInfo.hStdOutput = hWritePipe;
	siStartInfo.dwFlags |= STARTF_USESTDHANDLES;

	if (!CreateProcess(ffmpeg, szCmdline, NULL, NULL, TRUE, 0, NULL, NULL, &siStartInfo, &piProcInfo))
	{
		nofsub = 0;
		return;
	}
	
	WaitForSingleObject(piProcInfo.hProcess, 1000 * 60);

	DWORD dwRead;
	char result[1024 * 1024 * 2];
	ZeroMemory(result, 1024 * 1024 * 2);
	ReadFile(hReadPipe, result, 1024 * 1024 * 2, &dwRead, NULL);

	char* p = result;
	char digital[22];
	int i;
	float endtime_1 = 0.0, durationtime_1 = 0.0, starttime_2 = 0.0;
	nofsub = 0;

	while (p != NULL)
	{
		subtitle[nofsub].time = (endtime_1 - durationtime_1 / 2) * 1000 + 0.6;
		subtitle[nofsub].text[0] = L'\0';
		subtitle[nofsub].length = 0;
		nofsub++;
		if (nofsub >= 2999)
			break;

		do
		{
			p = strstr(++p, "silence_start");
			if (p == NULL)
				goto lastone;
			for (i = 0; i < 21; i++)
			{
				if (p[15 + i] == 0x0D)
					break;
				digital[i] = p[15 + i];
			}
			digital[i] = '\0';
			starttime_2 = atof(digital);
		} while ((starttime_2 - endtime_1) < 1);
		//////////////////////
		p = strstr(p, "silence_end");
		if (p == NULL)
			break;
		for (i = 0; i < 21; i++)
		{
			if (p[13 + i] == ' ')
				break;
			digital[i] = p[13 + i];
		}
		digital[i] = '\0';
		endtime_1 = atof(digital);
		/////////////////durationtime_1
		p = strstr(p, "silence_duration");
		if (p == NULL)
			break;
		for (i = 0; i < 21; i++)
		{
			if (p[18 + i] == 0x0D)
				break;
			digital[i] = p[18 + i];
		}
		digital[i] = '\0';
		durationtime_1 = atof(digital);
	}
lastone:
	subtitle[nofsub].time = medialength;
	subtitle[nofsub].text[0] = L'\0';
	subtitle[nofsub].length = 0;
	return;
}


void sendki(INPUT_RECORD irInBuf)
{
	INPUT input;
	input.type = INPUT_KEYBOARD;

	switch (irInBuf.Event.KeyEvent.wVirtualKeyCode)
	{
	case VK_BACK:
	case VK_RETURN:
	case VK_F1:
	case VK_F2:
	case VK_F3:
	case VK_F4:
		input.ki.wVk = irInBuf.Event.KeyEvent.wVirtualKeyCode;
		input.ki.wScan = irInBuf.Event.KeyEvent.wVirtualScanCode;
		input.ki.dwFlags = 0;
		break;
	default:
		input.ki.wVk = 0;
		input.ki.wScan = irInBuf.Event.KeyEvent.uChar.UnicodeChar;
		input.ki.dwFlags = KEYEVENTF_UNICODE;
		break;
	}

	input.ki.time = 0;
	input.ki.dwExtraInfo = GetMessageExtraInfo();

	ShowWindow(hDicWindow, SW_SHOWNORMAL);
	SetWindowPos(hDicWindow, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
	SetWindowPos(hDicWindow, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
	SetForegroundWindow(hDicWindow);

	SendInput(1, &input, sizeof(input));

	ShowWindow(hWindow, SW_SHOWNORMAL);
	SetWindowPos(hWindow, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
	SetWindowPos(hWindow, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
	SetForegroundWindow(hWindow);
	return;
}


void finddictionary()
{
	hDicWindow = FindWindow(L"ConsoleWindowClass", L"Dictionary");
	if (hDicWindow != NULL)
		AttachThreadInput(consoleID, GetWindowThreadProcessId(hDicWindow, NULL), TRUE);
	return;
}


BOOL CALLBACK ListProc(HWND hDlg, UINT msg, WPARAM wPar, LPARAM lPar)
{
	switch (msg)
	{
	case WM_INITDIALOG:
		for (int i = 0; i < nofmp3; i++)
			SendDlgItemMessage(hDlg, IDC_PLAYLIST, LB_ADDSTRING, 0, (LPARAM)PathFindFileName(mp3path[i]));
		SendDlgItemMessage(hDlg, IDC_PLAYLIST, LB_SETCURSEL, startfrom, 0);
		return TRUE;
	case WM_COMMAND:
		switch (LOWORD(wPar))
		{
		case IDC_PLAYLIST:
			if (HIWORD(wPar) == LBN_DBLCLK)
			{
				startfrom = SendMessage((HWND)lPar, LB_GETCURSEL, 0, 0);
				EndDialog(hDlg, 1);
				return TRUE;
			}
			else
				return FALSE;
		case IDCANCEL:
			EndDialog(hDlg, 0);
			return TRUE;
		}
	}
	return FALSE;
}


BYTE deletesub(DWORD index)
{
	if (index > 0)
	{
		do
		{
			subtitle[index].time = subtitle[index + 1].time;
			wcscpy_s(subtitle[index].text, 100, subtitle[index + 1].text);
			subtitle[index].length = subtitle[index + 1].length;
		} while (++index < nofsub);
		nofsub--;
		savelrc();
		return 1;
	}
	return 0;
}


void savelrc()
{
	wchar_t lrcpath[MAX_PATH];
	swprintf_s(lrcpath, MAX_PATH, L"%ls", mp3path[startfrom]);
	PathCchRenameExtension(lrcpath, MAX_PATH, L"lrc");

	wchar_t mp3name[MAX_PATH];
	swprintf_s(mp3name, MAX_PATH, L"%ls", mp3path[startfrom]);
	PathRemoveExtension(mp3name);

	FILE* fp;
	_wfopen_s(&fp, lrcpath, L"w");

	for (int i = 0; i < nofsub; i++)
		fprintf_s(fp, "[%02d:%02d.%02d]%ls %.2f\n",
			subtitle[i].time / 1000 / 60,
			subtitle[i].time / 1000 % 60,
			(subtitle[i].time % 1000) / 10,
			PathFindFileName(mp3name),
			((float)(subtitle[i].time)) / 1000);

	fflush(fp);
	fclose(fp);
	return;
}


void addsub(DWORD index, DWORD current)
{
	nofsub++;
	wchar_t mp3name[MAX_PATH];
	swprintf_s(mp3name, MAX_PATH, L"%ls", mp3path[startfrom]);
	PathRemoveExtension(mp3name);
	WORD ii;
	for (ii = nofsub; ii > index + 1; ii--)
	{
		subtitle[ii].time = subtitle[ii - 1].time;
		wcscpy_s(subtitle[ii].text, 100, subtitle[ii - 1].text);
		subtitle[ii].length = subtitle[ii - 1].length;
	}
	subtitle[ii].time = current;
	swprintf_s(subtitle[ii].text, 100, L"%ls %.2f", PathFindFileName(mp3name), ((float)(subtitle[ii].time)) / 1000);
	subtitle[ii].length = wcslen(subtitle[ii].text);
	savelrc();
	return;
}