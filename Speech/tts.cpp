#pragma warning(disable : 4996)

#include "Urlmon.h"
#include "stdio.h"
#include "Windows.h"
#include "io.h"
#include "Shlwapi.h"
#include "resource.h"
#include "time.h"
#include "sapi.h"
#include "sphelper.h"
#include "Wininet.h"

#pragma comment(lib,"Urlmon.lib")
#pragma comment(lib,"Shlwapi.lib")
#pragma comment(lib,"sapi.lib")
#pragma comment(lib,"Wininet.lib")



#define	cdblock		20

struct LRC
{
	char en[51];
	wchar_t zh[51];
	char bookmark;
};
struct ThreanPar
{
	unsigned int* ptmp3name;
	char** pchar;
	char* pfend;
	char* url;
	char* tmp3path;
	int tp3pthlen;
	LRC** plrcdg;
};

// The path of ffmpeg.exe
const char ffmpeg[] = ".\\ffmpeg.exe";

// Register key: http://open.iciba.com/index.php?c=wiki&t=cc
// Then put the key in the string below. For example: "&key=1234567890ABCDEF";
const char *key[] = { "&key="};///

char youdao[111] = "https://dict.youdao.com/dictvoice?rate=4&from=EN&audio=";
const int youdaolen = strlen(youdao);
char youdaoying[211] = "http://dict.youdao.com/dictvoice?audio=";
const int youdaoyinglen = strlen(youdaoying);
char youdaoying2[] = "&type=1&client=deskdict&id=32d9e0b32943ccd8f&vendor=qiang.youdao&in=&appVer=6.3.69.8341&appZengqiang=1&abTest=7";
char youdaomei[] = "&type=2&client=deskdict&id=32d9e0b32943ccd8f&vendor=qiang.youdao&in=&appVer=6.3.69.8341&appZengqiang=1&abTest=7";
int usebookmark = 0;
int nofffmpeg = 3;
int usetemp = 1;
double timedelay = 0.0;
HINTERNET hin = NULL;
HINTERNET hopen = NULL;


int getansistr(char* pstr, int size);
void run(char* pfile, char* pfend, char* path, char* filename, int mhzjs, char* url, LPVOID pData, DWORD dwSize);
void runTTS(char* pfile, char* pfend, char* path, char* filename, int mhzjs, char* url, LPVOID pData, DWORD dwSize);
void tts(char word[50], char mp3path[MAX_PATH]);
void writelrc(char* pathnn, char* filename, char* mp3path, int mp3name, int p3pthlen, LRC* plrc, int mhzjs);
void deletmp(char* mp3path, int p3pthlen);
DWORD WINAPI ThreadProc(_In_ LPVOID lpParameter);



int main(int argc, char* argv[])
{
	HANDLE hd = GetStdHandle(STD_OUTPUT_HANDLE);
	HRSRC hrc = FindResourceA(NULL, MAKEINTRESOURCEA(IDR_MP31), "MP3");
	HGLOBAL hGlobal = LoadResource(NULL, hrc);
	LPVOID pData = LockResource(hGlobal);
	DWORD dwSize = SizeofResource(NULL, hrc);
	char cmd[1024 * 1024] = { '\0' };
	char* pcmd = cmd;
	char path[MAX_PATH];
	char* ppath = path;
	char filename[MAX_PATH] = { '\0' };
	char url[300] = "http://dict-co.iciba.com/api/dictionary.php?w=";///[46]
	FILE* fp = NULL;
	char* pfile = NULL, * pfend = NULL;
	long fsize;
	int mhzjs = 0;
	char cmhzjs[10];
	void(*prun)(char* pfile, char* pfend, char* path, char* filename, int mhzjs, char* url, LPVOID pData, DWORD dwSize) = run;

	while ((mhzjs < 1) || (mhzjs > 99))
	{
		printf("The max length of each line of LRC files (<100) = ");
		getansistr(cmhzjs, 10);
		mhzjs = atoi(cmhzjs);
	}
	while (1)
	{
		pcmd = cmd;
		printf("TXT files:");
		getansistr(cmd, 1024 * 1024);
		if ((*pcmd) == '\0')
			continue;
		while ((*pcmd) != '\0')
		{
			ppath = path;
			(*ppath) = '\0';
			while ((*pcmd) != '\0')
			{
				if ((*pcmd) == '\"')
				{
					pcmd++;
					continue;
				}
				else if ((*pcmd) == '*')
				{
					pcmd++;
					char ct[10] = { '\0' };
					char* pct;
					pct = ct;
					switch (*pcmd)
					{
					case 't':
					{
						pcmd++;
						while (((*pcmd) == '+') || ((*pcmd) == '-') || ((*pcmd) == '.') ||
							(((*pcmd) >= '0') && ((*pcmd) <= '9')))
						{
							(*pct) = (*pcmd);
							pct++;
							pcmd++;
							if ((pct - ct) >= 9)
								break;
						}
						(*pct) = '\0';
						timedelay = atof(ct);
						break;
					}
					case 's':prun = runTTS; pcmd++; break;
					case 'w':prun = run; pcmd++; break;
					case 'n':nofffmpeg = 1; pcmd++; break;
					case 'b':usebookmark = 1; pcmd++; break;
					case '.':usetemp = 0; pcmd++; break;
					case 'h':printf("t:调时间\ns:读整篇\nw:读单词\nn:一个ffmpeg\n.:程序所在目录存放临时文件\ne:返回\nf:文件夹\nb:书签\n\n"); pcmd++; break;
					case 'e':return 0;
					case 'f':
					{
						pcmd++;
						char fpath[MAX_PATH + 1] = { '\0' };
						strcpy(fpath, pcmd);
						PathUnquoteSpacesA(fpath);
						PathAddBackslashA(fpath);
						int fpathlen = strlen(fpath);
						strcpy(fpath + fpathlen, "*.txt");
						cmd[0] = '\0';
						WIN32_FIND_DATAA fd;
						HANDLE hffile = FindFirstFileA(fpath, &fd);
						do
						{
							if ((strcmp(fd.cFileName, ".") == 0) || (strcmp(fd.cFileName, "..") == 0))
								continue;
							strcpy(fpath + fpathlen, fd.cFileName);
							if ((strlen(cmd) + strlen(fpath)) < 1024 * 1024)
								strcat(cmd, fpath);
							else
								break;
						} while (FindNextFileA(hffile, &fd));
						FindClose(hffile);
						pcmd = cmd;  break;
					}
					}
					continue;
				}
				else if (((*pcmd) == '.') && (*(pcmd + 1) == 't') && (*(pcmd + 2) == 'x') && (*(pcmd + 3) == 't'))
				{
					strcat(path, ".txt");
					pcmd = pcmd + 4;
					break;
				}
				else
				{
					(*ppath) = (*pcmd);
					ppath++;
					(*ppath) = '\0';
					pcmd++;
				}
			}
			if (strcmp(path, "\0") == 0)
				continue;
			SetConsoleTextAttribute(hd,  FOREGROUND_GREEN | FOREGROUND_INTENSITY );
			printf("%s\n", path);
			SetConsoleTextAttribute(hd, FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_BLUE );
			fp = fopen(path, "rb");
			if (fp == NULL)
			{
				printf(" (ERROR)\n");
				Beep(99, 110);
				continue;
			}
			fsize = filelength(fileno(fp)) + 1L;
			if (fsize == 1L)
				continue;
			pfile = (char*)malloc(fsize);
			pfend = pfile + fsize - 1L;
			fseek(fp, 0, SEEK_SET);
			fread(pfile, fsize - 1L, 1, fp);
			(*pfend) = '.';
			fclose(fp);
			fp = NULL;
			strcpy(filename, path);
			PathStripPathA(filename);
			CoInitialize(NULL);
			hin = InternetOpenA(NULL, INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
			(*prun)(pfile, pfend, path, filename, mhzjs, url, pData, dwSize);
			CoUninitialize();
			InternetCloseHandle(hin);
			free(pfile);
		}
	}
	return 0;
}

void run(char* pfile, char* pfend, char* path, char* filename, int mhzjs, char* url, LPVOID pData, DWORD dwSize)
{
	char* pchar = pfile;
	LRC* plrc = NULL;
	LRC* plrcdg = NULL;
	char mp3path[MAX_PATH + 1];
	int p3pthlen;
	int mp3name = 0;
	char tmp3path[MAX_PATH + 1];
	int tp3pthlen;
	int tp3pthlen2;
	unsigned int tmp3name = 0;
	char pathnn[MAX_PATH + 1];
	char par[1024 * 1024];
	long cnt;
	HANDLE htmp;
	FILE* fp = NULL;
	STARTUPINFOA si;
	PROCESS_INFORMATION pi;
	HANDLE* hdf = (HANDLE*)malloc(nofffmpeg * sizeof(HANDLE));
	memset(hdf, NULL, nofffmpeg * sizeof(HANDLE));
	int index = 0;
	char cr[] = { 0x0D,0x0A,'\0' };
	strcpy(pathnn, path);  //////////////  
	PathRemoveFileSpecA(pathnn);
	PathAddBackslashA(pathnn);
	if (usetemp == 1)
	{
		GetTempPathA(MAX_PATH + 1, mp3path);
		GetLongPathNameA(mp3path, mp3path, MAX_PATH + 1);
	}
	else
	{
		GetModuleFileNameA(NULL, mp3path, MAX_PATH + 1);
		PathRemoveFileSpecA(mp3path);
		PathAddBackslashA(mp3path);
	}
	strcat(mp3path, filename);
	PathRemoveExtensionA(mp3path);
	strcat(mp3path, "_tmp");
	for (; strchr(mp3path, '\'') != NULL; )
		*strchr(mp3path, '\'') = ' ';
	CreateDirectoryA(mp3path, NULL);
	PathAddBackslashA(mp3path);
	p3pthlen = strlen(mp3path);
	strcpy(tmp3path, mp3path);
	strcat(tmp3path, "tmpmp3");
	tp3pthlen2 = strlen(tmp3path);
	strcat(tmp3path, "0");
	tp3pthlen = strlen(tmp3path);
	_itoa(mp3name, mp3path + p3pthlen, 10);/////////////////
	mp3name++;
	strcat(mp3path, ".mp3");
	htmp = CreateFileA(mp3path, GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	DWORD NumberOfBytesWritten;
	WriteFile(htmp, pData, dwSize, &NumberOfBytesWritten, NULL);
	CloseHandle(htmp);
	for (cnt = 0L, pchar = pfile; pchar <= pfend; pchar++)////////////
	{
		if ((((*pchar) >= '0') && ((*pchar) <= '9')) || (((*pchar) >= 'a') && ((*pchar) <= 'z')) ||
			(((*pchar) >= 'A') && ((*pchar) <= 'Z')))
			continue;
		else
			cnt++;
	}
	plrc = (LRC*)malloc((cnt + 1) * sizeof(LRC));
	memset(plrc, '\0', (cnt + 1) * sizeof(LRC));
	plrcdg = plrc;
	plrcdg->bookmark = 1;
	for (pchar = pfile; pchar <= pfend; )
	{
		ThreanPar threadpar;
		threadpar.pchar = &pchar;
		threadpar.pfend = pfend;
		threadpar.plrcdg = &plrcdg;
		threadpar.ptmp3name = &tmp3name;
		threadpar.tmp3path = tmp3path;
		threadpar.tp3pthlen = tp3pthlen;
		threadpar.url = url;
		HANDLE ThreadHandle;
		ThreadHandle = CreateThread(NULL, 0, ThreadProc, &threadpar, 0, NULL);
		while (WAIT_TIMEOUT == WaitForSingleObject(ThreadHandle, 5000 * cdblock))
		{
			InternetCloseHandle(hopen);
			InternetCloseHandle(hin);
			TerminateThread(ThreadHandle, 0);
			CloseHandle(ThreadHandle);
			Sleep(3000);
			hin = InternetOpenA(NULL, INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
			ThreadHandle = CreateThread(NULL, 0, ThreadProc, &threadpar, 0, NULL);
		}
		CloseHandle(ThreadHandle);
		strcpy(par, " -y ");
		for (cnt = 0L; cnt < tmp3name; cnt++)
		{
			strcat(par, " -i \"");
			_itoa(cnt, tmp3path + tp3pthlen, 10);
			strcat(tmp3path, ".mp3");
			if (PathFileExistsA(tmp3path) != TRUE)
				PathRenameExtensionA(tmp3path, ".wav");
			strcat(par, tmp3path);
			strcat(par, "\" -map ");
			_itoa(cnt, par + strlen(par), 10);
			//////strcat(par, ":a ");
			strcat(par,
				":a -af silenceremove=start_periods=1:start_threshold=-70dB:stop_periods=1:stop_duration=0.03:stop_threshold=-70dB:window=3 -acodec libmp3lame -ar 48K -ab 196K -minrate 196K -maxrate 196K -compression_level 0 -ac 1 -write_xing 0 -id3v2_version 0 \"");
			_itoa(mp3name, mp3path + p3pthlen, 10);
			strcat(mp3path, ".mp3");
			mp3name++;
			strcat(par, mp3path);
			strcat(par, "\" ");
		}
		GetStartupInfoA(&si);
		si.cb = sizeof(STARTUPINFO);
		CreateProcessA(ffmpeg, par, NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi);
		hdf[index] = pi.hProcess;
		CloseHandle(pi.hThread);
		index = WaitForMultipleObjects(nofffmpeg, hdf, FALSE, INFINITE);
		if (index == -1)
		{
			int ii;
			DWORD flags;
			ii = 0;
			while (0 != GetHandleInformation(hdf[ii], &flags))
				ii = (ii + 1) % nofffmpeg;
			index = ii;
		}
		CloseHandle(hdf[index]);
		hdf[index] = 0;
		_itoa(index, tmp3path + tp3pthlen2, 10);
	}
	_itoa(mp3name, mp3path + p3pthlen, 10);/////////////////
	mp3name++;
	strcat(mp3path, ".mp3");
	htmp = CreateFileA(mp3path, GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	WriteFile(htmp, pData, dwSize, &NumberOfBytesWritten, NULL);
	CloseHandle(htmp);
	strcpy(par, " -n -f concat -safe 0 -i \""); /////////////////////
	strcpy(mp3path + p3pthlen, "cat.txt");
	strcat(par, mp3path);
	strcat(par, "\" -acodec copy -write_xing 0 -id3v2_version 0 \"");
	strcat(par, pathnn);
	PathRenameExtensionA(filename, ".mp3");
	strcat(par, filename);
	strcat(par, "\" ");
	fp = fopen(mp3path, "wb+");
	for (cnt = 0; cnt < mp3name; cnt++)
	{
		fwrite("file \'", 6, 1, fp);
		_itoa(cnt, mp3path + p3pthlen, 10);
		strcat(mp3path, ".mp3\'");
		strcat(mp3path, cr);
		fwrite(mp3path, strlen(mp3path), 1, fp);
	}
	fflush(fp);
	fclose(fp);
	printf("\n");
	for (index = 0; index < nofffmpeg; index++)
	{
		WaitForSingleObject(hdf[index], INFINITE);
		CloseHandle(hdf[index]);
	}
	GetStartupInfoA(&si);
	si.cb = sizeof(STARTUPINFO);
	CreateProcessA(ffmpeg, par, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);
	WaitForSingleObject(pi.hProcess, INFINITE);
	CloseHandle(pi.hThread);
	CloseHandle(pi.hProcess);
	writelrc(pathnn, filename, mp3path, mp3name, p3pthlen, plrc, mhzjs);
end:
	free(plrc);
	free(hdf);
	deletmp(mp3path, p3pthlen);
	printf("\n");
	return;
}

void tts(char word[51], char mp3path[MAX_PATH + 1])
{
	char wavpath[MAX_PATH + 1];
	strcpy(wavpath, mp3path);
	ISpVoice* pVoice = NULL;
	CoCreateInstance(CLSID_SpVoice, NULL, CLSCTX_ALL, IID_ISpVoice, (void**)&pVoice);
	pVoice->SetVolume(100);
	CComPtr<ISpAudio>m_cpOutAudio; //声音输出接口
	SpCreateDefaultObjectFromCategoryId(SPCAT_AUDIOOUT, &m_cpOutAudio); //创建接口	
	SPSTREAMFORMAT eFmt = SPSF_48kHz16BitMono;
	CSpStreamFormat Fmt;
	Fmt.AssignFormat(eFmt);
	HRESULT hr = m_cpOutAudio->SetFormat(Fmt.FormatId(), Fmt.WaveFormatExPtr());
	pVoice->SetOutput(m_cpOutAudio, FALSE);
	CSpStreamFormat OriginalFmt;
	CComPtr<ISpStream>cpWavStream;
	CComPtr<ISpStreamFormat>cpOldStream;
	hr = pVoice->GetOutputStream(&cpOldStream);
	if (hr == S_OK)
		hr = OriginalFmt.AssignFormat(cpOldStream);
	else
		hr = E_FAIL;
	if (SUCCEEDED(hr))         // 使用sphelper.h中提供的函数创建 wav文件
	{
		PathRenameExtensionA(wavpath, ".wav");
		hr = SPBindToFile(wavpath, SPFM_CREATE_ALWAYS, &cpWavStream,
			&OriginalFmt.FormatId(), OriginalFmt.WaveFormatExPtr());
	}
	if (SUCCEEDED(hr))
	{
		pVoice->SetOutput(cpWavStream, TRUE);   //设置声音的输出到 wav 文件，而不是 speakers
	}
	wchar_t wword[51];
	MultiByteToWideChar(CP_UTF8, 0, word, -1, wword, 51);
	pVoice->Speak(wword, SPF_ASYNC | SPF_IS_NOT_XML, 0); //开始朗读
	pVoice->WaitUntilDone(INFINITE);  //等待朗读结束
	pVoice->SetOutput(cpOldStream, FALSE);//把输出重新定位到原来的流
	pVoice->Release();
	m_cpOutAudio.Release();
	cpWavStream.Release();
	cpOldStream.Release();
	Fmt.Clear();
	OriginalFmt.Clear();
	return;
}

void writelrc(char* pathnn, char* filename, char* mp3path, int mp3name, int p3pthlen, LRC* plrc, int mhzjs)
{
	char lrcfile[MAX_PATH];
	char enfile[MAX_PATH];
	char bookmarkfile[MAX_PATH];
	FILE* lrcfp = NULL;
	FILE* enfp = NULL;
	FILE* tp3fp = NULL;
	FILE* bookmark = NULL;
	LRC* plrcdg = plrc;
	long cnt = 0L;
	char zhansi[51 * 4] = { '\0' };
	wchar_t wzh[51] = { L'\0' };
	double time = 2.328 + timedelay;
	char ctime[11] = { '\0' };
	char bmtime[21] = { '\0' };
	char cr[] = { 0x0D,0x0A,'\0' };
	strcpy(lrcfile, pathnn);///////////////
	strcat(lrcfile, filename);
	PathRenameExtensionA(lrcfile, ".lrc");
	lrcfp = fopen(lrcfile, "wb+");
	strcpy(enfile, pathnn);
	strcat(enfile, filename);
	PathRenameExtensionA(enfile, ".xls");
	enfp = fopen(enfile, "wb+");
	if (usebookmark == 1)
	{
		strcpy(bookmarkfile, pathnn);
		strcat(bookmarkfile, filename);
		PathRenameExtensionA(bookmarkfile, ".pbf");
		bookmark = fopen(bookmarkfile, "wb+");
		fwrite("[Bookmark]", 10, 1, bookmark);
		fwrite(cr, 2, 1, bookmark);
	}
	int mm;////////////
	int zhky;
	unsigned long ibookmark = 0;
	fwrite("[00:00.00]", 10, 1, lrcfp);
	fwrite(cr, 2, 1, lrcfp);
	for (cnt = 0L; cnt < mp3name - 2; cnt++)
	{
		mm = ((int)time) / 60;
		sprintf(ctime, "[%02d:%05.2f]", mm, time - mm * 60);
		fwrite(ctime, 10, 1, lrcfp);
		fwrite(plrcdg->en, strlen(plrcdg->en), 1, lrcfp);///////////////
		fwrite(":", 1, 1, lrcfp);
		int i, m;////////////////////
		zhky = mhzjs - strlen(plrcdg->en) - 1;
		for (i = 0, m = 0; (zhky - 2 >= 0) && (plrcdg->zh[m] != L'\0'); i++, m++)
		{
			switch (plrcdg->zh[m])
			{
			case L' ':i--; continue;
			case L';':
			case L'，':
			case L'。':
			case L'；':
			case L'：':
			case L'、':
			case L'！':wzh[i] = L'.'; zhky--; continue;
			case L'“':
			case L'‘':
			case L'（':
			case L'[':
			case L'【':
			case L'〈':
			case L'(':
			case L'《':wzh[i] = L'('; zhky--; continue;
			case L'”':
			case L'’':
			case L'）':
			case L'》':
			case L'〉':
			case L']':
			case L')':
			case L'】':wzh[i] = L')'; zhky--; continue;
			case L'\'':
			case L'\"':wzh[i] = L'\''; zhky--; continue;
			case L'&':wzh[i] = L'&'; zhky--; continue;
			case L'=':wzh[i] = L'='; zhky--; continue;
			default:
			{
				wzh[i] = (plrcdg->zh[m]);
				zhky = zhky - 2;
				if (((wzh[i] >= L'0') && (wzh[i] <= L'9')) ||
					((wzh[i] >= L'a') && (wzh[i] <= L'z')))
					zhky++;
			}
			}
		}
		wzh[i] = L'\0';
		WideCharToMultiByte(CP_UTF8, 0, wzh, -1, zhansi, 51 * 4, NULL, NULL);
		fwrite(zhansi, strlen(zhansi), 1, lrcfp);
		fwrite(cr, 2, 1, lrcfp);
		fwrite(plrcdg->en, strlen(plrcdg->en), 1, enfp);//////////////////////
		fwrite("\t\t", 2, 1, enfp);
		if (wcscmp(plrcdg->zh, L"\0") == 0)  ////////////////////
		{
			plrcdg->zh[0] = L'~';
			plrcdg->zh[1] = L'\0';
		}
		WideCharToMultiByte(CP_UTF8, 0, plrcdg->zh, -1, zhansi, 51 * 4, NULL, NULL);
		fwrite(zhansi, strlen(zhansi), 1, enfp);
		fwrite(cr, 2, 1, enfp);
		if ((bookmark != NULL) && (plrcdg->bookmark == 1))
		{
			_itoa(ibookmark, bmtime, 10);
			fwrite(bmtime, strlen(bmtime), 1, bookmark);
			fwrite("=", 1, 1, bookmark);
			_itoa((int)(time * 1000), bmtime, 10);
			fwrite(bmtime, strlen(bmtime), 1, bookmark);
			fwrite("*", 1, 1, bookmark);
			fwrite(plrcdg->en, strlen(plrcdg->en), 1, bookmark);
			fwrite("*", 1, 1, bookmark);
			fwrite(cr, 2, 1, bookmark);
			ibookmark++;
		}
		_itoa((cnt + 1), mp3path + p3pthlen, 10);////////////////
		strcat(mp3path, ".mp3");
		tp3fp = fopen(mp3path, "rb");
		time = time + (((double)filelength(fileno(tp3fp))) * 8 / 1000 / 192);
		fclose(tp3fp);
		plrcdg++;////////////////////////
	}
	fflush(lrcfp);
	fflush(enfp);
	fclose(lrcfp);
	fclose(enfp);
	if (bookmark != NULL)
	{
		fflush(bookmark);
		fclose(bookmark);
	}
	return;
}

void deletmp(char* mp3path, int p3pthlen)
{
	char path[MAX_PATH + 1] = { '\0' };
	strcpy(path, mp3path);
	strcpy(path + p3pthlen, "*");
	WIN32_FIND_DATAA fd;
	HANDLE hffile = FindFirstFileA(path, &fd);
	for (; FindNextFileA(hffile, &fd); )
	{
		if ((strcmp(fd.cFileName, ".") == 0) || (strcmp(fd.cFileName, "..") == 0))
			continue;
		strcpy(path + p3pthlen, fd.cFileName);
		DeleteFileA(path);
	}
	FindClose(hffile);
	strcpy(path + p3pthlen, "\0");
	RemoveDirectoryA(path);
	return;
}

void runTTS(char* pfile, char* pfend, char* path, char* filename, int mhzjs, char* url, LPVOID pData, DWORD dwSize)
{
	char wavpath[MAX_PATH + 1];
	strcpy(wavpath, path);
	PathRemoveExtensionA(wavpath);
	strcat(wavpath, "_TTS.wav");
	ISpVoice* pVoice = NULL;
	CoCreateInstance(CLSID_SpVoice, NULL, CLSCTX_ALL, IID_ISpVoice, (void**)&pVoice);
	pVoice->SetVolume(100);
	CComPtr<ISpAudio>m_cpOutAudio; //声音输出接口
	SpCreateDefaultObjectFromCategoryId(SPCAT_AUDIOOUT, &m_cpOutAudio); //创建接口	
	SPSTREAMFORMAT eFmt = SPSF_32kHz16BitMono;
	CSpStreamFormat Fmt;
	Fmt.AssignFormat(eFmt);
	HRESULT hr = m_cpOutAudio->SetFormat(Fmt.FormatId(), Fmt.WaveFormatExPtr());
	pVoice->SetOutput(m_cpOutAudio, FALSE);
	CSpStreamFormat OriginalFmt;
	CComPtr<ISpStream>cpWavStream;
	CComPtr<ISpStreamFormat>cpOldStream;
	hr = pVoice->GetOutputStream(&cpOldStream);
	if (hr == S_OK)
		hr = OriginalFmt.AssignFormat(cpOldStream);
	else
		hr = E_FAIL;
	if (SUCCEEDED(hr))         // 使用sphelper.h中提供的函数创建 wav文件
	{
		hr = SPBindToFile(wavpath, SPFM_CREATE_ALWAYS, &cpWavStream,
			&OriginalFmt.FormatId(), OriginalFmt.WaveFormatExPtr());
	}
	if (SUCCEEDED(hr))
	{
		pVoice->SetOutput(cpWavStream, TRUE);   //设置声音的输出到 wav 文件，而不是 speakers
	}
	wchar_t* pwfile = (wchar_t*)malloc(_msize(pfile) * sizeof(wchar_t));
	wmemset(pwfile, L'\0', _msize(pfile));
	MultiByteToWideChar(CP_UTF8, 0, pfile, pfend - pfile, pwfile, _msize(pfile));
	pVoice->SetVolume(100);
	pVoice->Speak(pwfile, SPF_ASYNC | SPF_IS_NOT_XML, 0); //开始朗读
	printf("正在保存.");
	while (S_FALSE == pVoice->WaitUntilDone(500))	//等待朗读结束
		printf(".");
	printf("\n\n");
	pVoice->SetOutput(cpOldStream, FALSE);//把输出重新定位到原来的流
	pVoice->Release();
	m_cpOutAudio.Release();
	cpWavStream.Release();
	cpOldStream.Release();
	Fmt.Clear();
	OriginalFmt.Clear();
	free(pwfile);
	return;
}

int getansistr(char* pstr, int size)
{
	int wsize = size - 1 + 2;
	wchar_t* pw = (wchar_t*)malloc(wsize * sizeof(wchar_t));
	HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
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
	int cNumWritten = WideCharToMultiByte(CP_UTF8, 0, pw, -1, pstr, size, NULL, NULL);
	free(pw);
	return cNumWritten;
}

DWORD WINAPI ThreadProc(_In_ LPVOID lpParameter)
{
	char* pchar = *(((ThreanPar*)lpParameter)->pchar);
	char* pfend = ((ThreanPar*)lpParameter)->pfend;
	char* url = ((ThreanPar*)lpParameter)->url;
	char* tmp3path = ((ThreanPar*)lpParameter)->tmp3path;
	int tp3pthlen = ((ThreanPar*)lpParameter)->tp3pthlen;
	LRC* plrcdg = *(((ThreanPar*)lpParameter)->plrcdg);
	char word[51] = { '\0' };
	int bookmarknextword = 0;
	unsigned int keycount = 0;
	char tmpxml[4096];
	char* pxmlend = tmpxml + 4096 - 1;
	char* p = tmpxml;
	char mp3url[300];
	wchar_t wtmpxml[4096];
	wchar_t* wpxmlend = wtmpxml + 4096 - 1;
	wchar_t* wp = wtmpxml;
	long cnt;
	unsigned int tmp3name;
	CoInitialize(NULL);
	for (cnt = 0L, tmp3name = 0; (tmp3name < cdblock) && (pchar <= pfend); pchar++)
	{
		if (((((*pchar) >= '0') && ((*pchar) <= '9')) || (((*pchar) >= 'a') && ((*pchar) <= 'z'))) && (cnt < 50))
		{
			word[cnt] = (*pchar);
			cnt++;
			continue;
		}
		else if ((((*pchar) >= 'A') && ((*pchar) <= 'Z')) && (cnt < 50))
		{
			word[cnt] = (*pchar) + 32;
			cnt++;
			continue;
		}
		else if ((((*pchar) == ',') || ((*pchar) == '.')) && ((*(pchar - 1) >= '0') && (*(pchar - 1) <= '9')) && ((*(pchar + 1) >= '0') && (*(pchar + 1) <= '9')))
		{
			word[cnt] = (*pchar);
			cnt++;
			continue;
		}
		else if (((*pchar) == '%') && ((*(pchar - 1) >= '0') && (*(pchar - 1) <= '9')))
		{
			word[cnt] = (*pchar);
			cnt++;
			continue;
		}
		else if (((*pchar) == '\'') && ((*(pchar + 1) == 't') || (*(pchar + 1) == 'T') || (*(pchar + 1) == 'R') || (*(pchar + 1) == 'r')) && (cnt > 0))
		{
			word[cnt] = (*pchar);
			cnt++;
			continue;
		}
		else
		{
			if (((*pchar) == 0x0A) && (*(pchar - 1) == 0x0D))
				bookmarknextword = 1;
			if (cnt == 0L)
				continue;
			word[cnt] = '\0';
			printf("%s ", word);
			strcpy(url + 46, word);
			strcat(url, key[0]);
			memset(tmpxml, '\0', 4096);
			int i, re;
			i = 1;
			DWORD NumberOfBytesRead;
			while (1)
			{
				hopen = InternetOpenUrlA(hin, url, NULL, 0, INTERNET_FLAG_HYPERLINK, NULL);
				re = InternetReadFile(hopen, tmpxml, 4096, &NumberOfBytesRead);
				InternetCloseHandle(hopen);
				if (TRUE == re)
				{
					break;
				}
				else
				{
					if ((i % 10) == 0)
						Beep(300, 1000);
					InternetCloseHandle(hin);
					Sleep(2000);
					hin = InternetOpenA(NULL, INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
					i++;
				}
			}
			p = tmpxml;		////////////
			while (((*p) != '\0') && (p <= pxmlend) && (!(((*p) == '<') && (*(p + 1) == 'p') &&
				(*(p + 2) == 'r') && (*(p + 3) == 'o') && (*(p + 4) == 'n') && (*(p + 5) == '>'))))
				p++;
			p = p + 6;
			if (((*p) == 0x0D) && (*(p + 1) == 0x0A))
				p = p + 2;
			for (cnt = 0L; ((*p) != '\0') && (p <= pxmlend) && (!(((*p) == '<') && (*(p + 1) == '/') &&
				(*(p + 2) == 'p') && (*(p + 3) == 'r') && (*(p + 4) == 'o') && (*(p + 5) == 'n') &&
				(*(p + 6) == '>'))) && (!(((*p) == 0x0D) && (*(p + 1) == 0x0A))) && (cnt < 499); cnt++, p++)
				mp3url[cnt] = (*p);
			mp3url[cnt] = '\0';
			HRESULT hr;      ///////////////////
			hr = INET_E_DOWNLOAD_FAILURE;
			_itoa(tmp3name, tmp3path + tp3pthlen, 10);
			tmp3name++;
			strcat(tmp3path, ".mp3");
			if ((strstr(mp3url, ".mp3") != NULL) || (strstr(mp3url, ".MP3") != NULL))
			{
				hr = URLDownloadToFileA(NULL, mp3url, tmp3path, 0, NULL);
				for (i = 0; (hr != S_OK) && (i < 3); i++)
				{
					Sleep(2000);
					hr = URLDownloadToFileA(NULL, mp3url, tmp3path, 0, NULL);
				}
			}
			int wordlen;
			if (hr != S_OK)
			{
				wordlen = strlen(word);
				if (word[wordlen - 1] == '%')
				{
					word[wordlen] = '2';
					word[wordlen + 1] = '5';
					word[wordlen + 2] = '\0';
				}
				strcpy(youdaoying + youdaoyinglen, word);
				strcat(youdaoying, youdaoying2);
				hr = URLDownloadToFileA(NULL, youdaoying, tmp3path, 0, NULL);
				for (i = 0; (hr != S_OK) && (i < 3); i++)
				{
					Sleep(2000);
					hr = URLDownloadToFileA(NULL, youdaoying, tmp3path, 0, NULL);
				}
			}
			if (hr != S_OK)
			{
				strcpy(youdaoying + youdaoyinglen, word);
				strcat(youdaoying, youdaomei);
				hr = URLDownloadToFileA(NULL, youdaoying, tmp3path, 0, NULL);
				for (i = 0; (hr != S_OK) && (i < 3); i++)
				{
					Sleep(2000);
					hr = URLDownloadToFileA(NULL, youdaoying, tmp3path, 0, NULL);
				}
			}
			if (hr != S_OK)
			{
				strcpy(youdao + youdaolen, word);
				hr = URLDownloadToFileA(NULL, youdao, tmp3path, 0, NULL);
				for (i = 0; (hr != S_OK) && (i < 3); i++)
				{
					Sleep(2000);
					hr = URLDownloadToFileA(NULL, youdao, tmp3path, 0, NULL);
				}
			}
			if (hr != S_OK)
			{
				wordlen = strlen(word);
				if (word[wordlen - 3] == '%')
					word[wordlen - 2] = '\0';
				remove(tmp3path);
				tts(word, tmp3path);
			}
			strcpy(plrcdg->en, word);/////////////////////////
			wmemset(wtmpxml, L'\0', 4096);    ////////////////
			MultiByteToWideChar(CP_UTF8, 0, tmpxml, 4096, wtmpxml, 4096);
			wp = wtmpxml;
			cnt = 0L;
			while ((wp <= wpxmlend) && (cnt < 50))
			{
				while ((wp <= wpxmlend) && (!(((*wp) == L'<') && (*(wp + 1) == L'a') && (*(wp + 2) == L'c') && (*(wp + 3) == L'c') && (*(wp + 4) == L'e') && (*(wp + 5) == L'p') && (*(wp + 6) == L't') && (*(wp + 7) == L'a') && (*(wp + 8) == L't') && (*(wp + 9) == L'i') && (*(wp + 10) == L'o') && (*(wp + 11) == L'n') && (*(wp + 12) == L'>'))))
					wp++;
				wp = wp + 13;
				if (((*wp) == 0x0DL) && (*(wp + 1) == 0x0AL))
					wp = wp + 2;
				for (; (!(((*wp) == 0x0DL) && (*(wp + 1) == 0x0AL))) && (!(((*wp) == L'<') && (*(wp + 1) == L'/') && (*(wp + 2) == L'a') && (*(wp + 3) == L'c') && (*(wp + 4) == L'c') && (*(wp + 5) == L'e') && (*(wp + 6) == L'p') && (*(wp + 7) == L't') && (*(wp + 8) == L'a') && (*(wp + 9) == L't') && (*(wp + 10) == L'i') && (*(wp + 11) == L'o') && (*(wp + 12) == L'n') && (*(wp + 13) == L'>'))) && (cnt < 50) && (wp <= wpxmlend); cnt++, wp++)
				{
					switch (*wp)
					{
					case L' ':cnt--; continue;
					case L'&':
					{
						if ((*(wp + 1) == L'l') && (*(wp + 2) == L't') && (*(wp + 3) == L';'))
						{
							(plrcdg->zh[cnt]) = L'('; wp = wp + 3;
						}
						else if ((*(wp + 1) == L'g') && (*(wp + 2) == L't') && (*(wp + 3) == L';'))
						{
							(plrcdg->zh[cnt]) = L')'; wp = wp + 3;
						}
						else if ((*(wp + 1) == L'a') && (*(wp + 2) == L'p') && (*(wp + 3) == L'o') &&
							(*(wp + 4) == L's') && (*(wp + 5) == L';'))
						{
							(plrcdg->zh[cnt]) = L'\''; wp = wp + 5;
						}
						else if ((*(wp + 1) == L'q') && (*(wp + 2) == L'u') && (*(wp + 3) == L'o') &&
							(*(wp + 4) == L't') && (*(wp + 5) == L';'))
						{
							(plrcdg->zh[cnt]) = L'\"'; wp = wp + 5;
						}
						else if ((*(wp + 1) == L'a') && (*(wp + 2) == L'm') && (*(wp + 3) == L'p') &&
							(*(wp + 4) == L';'))
						{
							(plrcdg->zh[cnt]) = L'&'; wp = wp + 4;
						}
						else
							(plrcdg->zh[cnt]) = L'&';
						continue;
					}
					default:(plrcdg->zh[cnt]) = (*wp);
					}
				}
			}
			(plrcdg->zh[cnt]) = L'\0';
			cnt = 0L;
			if (bookmarknextword == 1)
			{
				plrcdg->bookmark = 1;
				bookmarknextword = 0;
			}
			plrcdg++;////////////////
		}
	}
	CoUninitialize();
	*(((ThreanPar*)lpParameter)->ptmp3name) = tmp3name;
	*(((ThreanPar*)lpParameter)->pchar) = pchar;
	*(((ThreanPar*)lpParameter)->plrcdg) = plrcdg;
	return 0;
}