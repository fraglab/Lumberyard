/*
* All or portions of this file Copyright (c) Amazon.com, Inc. or its affiliates or
* its licensors.
*
* For complete copyright and license terms please see the LICENSE at the root of this
* distribution (the "License"). All use of this software is governed by the License,
* or, if provided, by the license below or the license accompanying this file. Do not
* remove or modify any license notices. This file is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
*
*/
// Original file Copyright Crytek GMBH or its affiliates, used under license.

// Description : Linux/Mac port support for Win32API calls


#include "platform.h" // Note: This should be first to get consistent debugging definitions

#include <CryAssert.h>
    #include <signal.h>

#include <pthread.h>
#include <sys/types.h>
#include <fcntl.h>
#include <AzCore/IO/SystemFile.h>

#ifdef APPLE
#include <mach/mach.h>
#include <mach/mach_time.h>
#include <sys/sysctl.h>                     // for total physical memory on Mac
#include <CoreFoundation/CoreFoundation.h>  // for CryMessageBox
#include <mach/vm_statistics.h>             // host_statistics
#include <mach/mach_types.h>
#include <mach/mach_init.h>
#include <mach/mach_host.h>
#endif

#if defined(ANDROID)
#define FIX_FILENAME_CASE 0 // everything is lower case on android
#elif defined(LINUX) || defined(APPLE)
#define FIX_FILENAME_CASE 1
#endif

#include <sys/time.h>



#if !defined(_RELEASE) || defined(_DEBUG)
#include <set>
unsigned int g_EnableMultipleAssert = 0;//set to something else than 0 if to enable already reported asserts
#endif

#if defined(LINUX) || defined(APPLE)
#include <sys/types.h>
#include <unistd.h>
#include <utime.h>
#include <dirent.h>
#include "CryLibrary.h"
#endif

#if defined(APPLE)
    #include "../CrySystem/SystemUtilsApple.h"
#endif

#include "StringUtils.h"

#if defined(LINUX) || defined(APPLE) || defined(ORBIS) // ACCEPTED_USE
typedef int FS_ERRNO_TYPE;
#if defined(APPLE) || defined(ORBIS) // ACCEPTED_USE
typedef struct stat FS_STAT_TYPE;
#else
typedef struct stat64 FS_STAT_TYPE;
#endif
static const int FS_O_RDWR = O_RDWR;
static const int FS_O_RDONLY = O_RDONLY;
static const int FS_O_WRONLY = O_WRONLY;
static const FS_ERRNO_TYPE FS_EISDIR = EISDIR;

#include <mutex>

#endif

#if (defined(LINUX) || defined(APPLE) || defined(ORBIS)) && (!defined(_RELEASE) || defined(_DEBUG)) // ACCEPTED_USE
struct SAssertData
{
    int line;
    char fileName[256 - sizeof(int)];
    const bool operator==(const SAssertData& crArg) const
    {
        return crArg.line == line && (strcmp(fileName, crArg.fileName) == 0);
    }

    const bool operator<(const SAssertData& crArg) const
    {
        if (line == crArg.line)
        {
            return strcmp(fileName, crArg.fileName) < 0;
        }
        else
        {
            return line < crArg.line;
        }
    }

    SAssertData()
        : line(-1){}
    SAssertData(const int cLine, const char* cpFile)
        : line(cLine)
    {
        strcpy(fileName, cpFile);
    }

    SAssertData(const SAssertData& crAssertData)
    {
        memcpy((void*)this, &crAssertData, sizeof(SAssertData));
    }

    void operator=(const SAssertData& crAssertData)
    {
        memcpy((void*)this, &crAssertData, sizeof(SAssertData));
    }
};


//#define OUTPUT_ASSERT_TO_FILE

void HandleAssert(const char* cpMessage, const char* cpFunc, const char* cpFile, const int cLine)
{
#if defined(OUTPUT_ASSERT_TO_FILE)
    static FILE* pAssertLogFile = fopen("Assert.log", "w+");
#endif
    bool report = true;
    static std::set<SAssertData> assertSet;
    SAssertData assertData(cLine, cpFile);
    if (!g_EnableMultipleAssert)
    {
        std::set<SAssertData>::const_iterator it = assertSet.find(assertData);
        if (it != assertSet.end())
        {
            report = false;
        }
        else
        {
            assertSet.insert(assertData);
        }
    }
    else
    {
        assertSet.insert(assertData);
    }
    if (report)
    {
        //added function to be able to place a breakpoint here or to print out to other consoles
        printf("ASSERT: %s in %s (%s : %d)\n", cpMessage, cpFunc, cpFile, cLine);
#if defined(OUTPUT_ASSERT_TO_FILE)
        if (pAssertLogFile)
        {
            fprintf(pAssertLogFile, "ASSERT: %s in %s (%s : %d)\n", cpMessage, cpFunc, cpFile, cLine);
            fflush(pAssertLogFile);
        }
#endif
    }
}
#endif


bool IsBadReadPtr(void* ptr, unsigned int size)
{
    //too complicated to really support it
    return ptr ? false : true;
}

//////////////////////////////////////////////////////////////////////////
char* _strtime(char* date)
{
    strcpy(date, "0:0:0");
    return date;
}

//////////////////////////////////////////////////////////////////////////
char* _strdate(char* date)
{
    strcpy(date, "0");
    return date;
}

//////////////////////////////////////////////////////////////////////////
char* strlwr (char* str)
{
    char* cp;             /* traverses string for C locale conversion */

    for (cp = str; *cp; ++cp)
    {
        if ('A' <= *cp && *cp <= 'Z')
        {
            *cp += 'a' - 'A';
        }
    }
    return str;
}

char* strupr (char* str)
{
    char* cp;             /* traverses string for C locale conversion */

    for (cp = str; *cp; ++cp)
    {
        if ('a' <= *cp && *cp <= 'z')
        {
            *cp += 'A' - 'a';
        }
    }
    return str;
}

char* ltoa (long i, char* a, int radix)
{
    if (a == NULL)
    {
        return NULL;
    }
    strcpy (a, "0");
    if (i && radix > 1 && radix < 37)
    {
        char buf[35];
        unsigned long u = i, p = 34;
        buf[p] = 0;
        if (i < 0 && radix == 10)
        {
            u = -i;
        }
        while (u)
        {
            unsigned int d = u % radix;
            buf[--p] = d < 10 ? '0' + d : 'a' + d - 10;
            u /= radix;
        }
        if (i < 0 && radix == 10)
        {
            buf[--p] = '-';
        }
        strcpy (a, buf + p);
    }
    return a;
}


#if defined(ANDROID) || defined (ORBIS) // ACCEPTED_USE
// For Linux it's redefined to wcscasecmp and wcsncasecmp'
int wcsicmp (const wchar_t* s1, const wchar_t* s2)
{
    wint_t c1, c2;

    if (s1 == s2)
    {
        return 0;
    }

    do
    {
        c1 = towlower(*s1++);
        c2 = towlower(*s2++);
    }
    while (c1 && c1 == c2);

    return (int) (c1 - c2);
}

int wcsnicmp (const wchar_t* s1, const wchar_t* s2, size_t count)
{
    wint_t c1, c2;
    if (s1 == s2 || count == 0)
    {
        return 0;
    }

    do
    {
        c1 = towlower(*s1++);
        c2 = towlower(*s2++);
    }
    while ((--count) && c1 && (c1 == c2));
    return (int) (c1 - c2);
}
#endif

#if defined(ANDROID)
// not defined in android-19 or prior
size_t wcsnlen(const wchar_t* str, size_t maxLen)
{
    size_t length;
    for (length = 0; length < maxLen; ++length, ++str)
    {
        if (!*str)
        {
            break;
        }
    }
    return length;
}

char* stpcpy(char* dest, const char* str)
{
    while (*str != '\0')
    {
        *dest++ = *str++;
    }
    *dest = '\0';

    return dest;
}
#endif

void _makepath(char* path, const char* drive, const char* dir, const char* filename, const char* ext)
{
    char ch;
    char tmp[MAX_PATH];
    if (!path)
    {
        return;
    }
    tmp[0] = '\0';
    if (drive && drive[0])
    {
        tmp[0] = drive[0];
        tmp[1] = ':';
        tmp[2] = 0;
    }
    if (dir && dir[0])
    {
        cry_strcat(tmp, dir);
        ch = tmp[strlen(tmp) - 1];
        if (ch != '/' && ch != '\\')
        {
            cry_strcat(tmp, "\\");
        }
    }
    if (filename && filename[0])
    {
        cry_strcat(tmp, filename);
        if (ext && ext[0])
        {
            if (ext[0] != '.')
            {
                cry_strcat(tmp, ".");
            }
            cry_strcat(tmp, ext);
        }
    }
    strcpy(path, tmp);
}

char* _ui64toa(unsigned long long value,   char* str, int radix)
{
    if (str == 0)
    {
        return 0;
    }

    char buffer[65];
    char* pos;
    int digit;

    pos = &buffer[64];
    *pos = '\0';

    do
    {
        digit = value % radix;
        value = value / radix;
        if (digit < 10)
        {
            *--pos = '0' + digit;
        }
        else
        {
            *--pos = 'a' + digit - 10;
        } /* if */
    } while (value != 0L);

    memcpy(str, pos, &buffer[64] - pos + 1);
    return str;
}

long long _atoi64(const char* str)
{
    if (str == 0)
    {
        return -1;
    }
    unsigned long long RunningTotal = 0;
    char bMinus = 0;
    while (*str == ' ' || (*str >= '\011' && *str <= '\015'))
    {
        str++;
    } /* while */
    if (*str == '+')
    {
        str++;
    }
    else if (*str == '-')
    {
        bMinus = 1;
        str++;
    } /* if */
    while (*str >= '0' && *str <= '9')
    {
        RunningTotal = RunningTotal * 10 + *str - '0';
        str++;
    } /* while */
    return bMinus ? ((long long)-RunningTotal) : (long long)RunningTotal;
}

bool QueryPerformanceCounter(LARGE_INTEGER* counter)
{
#if defined(LINUX)
    // replaced gettimeofday
    // http://fixunix.com/kernel/378888-gettimeofday-resolution-linux.html
    timespec tv;
    clock_gettime(CLOCK_MONOTONIC, &tv);
    counter->QuadPart = (uint64)tv.tv_sec * 1000000 + tv.tv_nsec / 1000;
    return true;
#elif defined(APPLE)
    counter->QuadPart = mach_absolute_time();
    return true;
#else
    return false;
#endif
}

bool QueryPerformanceFrequency(LARGE_INTEGER* frequency)
{
#if defined(LINUX)
    // On Linux we'll use gettimeofday().  The API resolution is microseconds,
    // so we'll report that to the caller.
    frequency->u.LowPart  = 1000000;
    frequency->u.HighPart = 0;
    return true;
#elif defined(APPLE)
    static mach_timebase_info_data_t s_kTimeBaseInfoData;
    if (s_kTimeBaseInfoData.denom == 0)
    {
        mach_timebase_info(&s_kTimeBaseInfoData);
    }
    // mach_timebase_info_data_t expresses the tick period in nanoseconds
    frequency->QuadPart = 1e+9 * (uint64_t)s_kTimeBaseInfoData.denom / (uint64_t)s_kTimeBaseInfoData.numer;
    return true;
#else
    return false;
#endif
}

void _splitpath(const char* inpath, char* drv, char* dir, char* fname, char* ext)
{
    if (drv)
    {
        drv[0] = 0;
    }

    typedef CryStackStringT<char, AZ_MAX_PATH_LEN> path_stack_string;

    const path_stack_string inPath(inpath);
    string::size_type s = inPath.rfind('/', inPath.size());//position of last /
    path_stack_string fName;
    if (s == string::npos)
    {
        if (dir)
        {
            dir[0] = 0;
        }
        fName = inpath; //assign complete string as rest
    }
    else
    {
        if (dir)
        {
            strcpy(dir, (inPath.substr((string::size_type)0, (string::size_type)(s + 1))).c_str());    //assign directory
        }
        fName = inPath.substr((string::size_type)(s + 1));                    //assign remaining string as rest
    }
    if (fName.size() == 0)
    {
        if (ext)
        {
            ext[0] = 0;
        }
        if (fname)
        {
            fname[0] = 0;
        }
    }
    else
    {
        //dir and drive are now set
        s = fName.find(".", (string::size_type)0);//position of first .
        if (s == string::npos)
        {
            if (ext)
            {
                ext[0] = 0;
            }
            if (fname)
            {
                strcpy(fname, fName.c_str());   //assign filename
            }
        }
        else
        {
            if (ext)
            {
                strcpy(ext, (fName.substr(s)).c_str());     //assign extension including .
            }
            if (fname)
            {
                if (s == 0)
                {
                    fname[0] = 0;
                }
                else
                {
                    strcpy(fname, (fName.substr((string::size_type)0, s)).c_str());  //assign filename
                }
            }
        }
    }
}

//////////////////////////////////////////////////////////////////////////
int memicmp(LPCSTR s1, LPCSTR s2, DWORD len)
{
    int ret = 0;
    while (len--)
    {
        if ((ret = tolower(*s1) - tolower(*s2)))
        {
            break;
        }
        s1++;
        s2++;
    }
    return ret;
}

//////////////////////////////////////////////////////////////////////////
int strcmpi(const char* str1, const char* str2)
{
    for (;; )
    {
        int ret = tolower(*str1) - tolower(*str2);
        if (ret || !*str1)
        {
            return ret;
        }
        str1++;
        str2++;
    }
}

//-----------------------------------------other stuff-------------------------------------------------------------------

void GlobalMemoryStatus(LPMEMORYSTATUS lpmem)
{
    //not complete implementation
#if   defined(APPLE)

    // Retrieve dwTotalPhys
    int kMIB[] = {CTL_HW, HW_MEMSIZE};
    uint64_t dwTotalPhys;
    size_t ulength = sizeof(dwTotalPhys);
    if (sysctl(kMIB, 2, &dwTotalPhys, &ulength, NULL, 0) != 0)
    {
        gEnv->pLog->LogError("sysctl failed\n");
    }
    else
    {
        lpmem->dwTotalPhys = static_cast<SIZE_T>(dwTotalPhys);
    }

    // Get the page size
    mach_port_t kHost(mach_host_self());
    vm_size_t uPageSize;
    if (host_page_size(kHost, &uPageSize) != 0)
    {
        gEnv->pLog->LogError("host_page_size failed\n");
    }
    else
    {
        // Get memory statistics
        vm_statistics_data_t kVMStats;
        mach_msg_type_number_t uCount(sizeof(kVMStats) / sizeof(natural_t));
        if (host_statistics(kHost, HOST_VM_INFO, (host_info_t)&kVMStats, &uCount) != 0)
        {
            gEnv->pLog->LogError("host_statistics failed\n");
        }
        else
        {
            // Calculate dwAvailPhys
            lpmem->dwAvailPhys = uPageSize * kVMStats.free_count;
        }
    }
#else
    FILE* f;
    lpmem->dwMemoryLoad    = 0;
    lpmem->dwTotalPhys     = 16 * 1024 * 1024;
    lpmem->dwAvailPhys     = 16 * 1024 * 1024;
    lpmem->dwTotalPageFile = 16 * 1024 * 1024;
    lpmem->dwAvailPageFile = 16 * 1024 * 1024;
    f = ::fopen("/proc/meminfo", "r");
    if (f)
    {
        char buffer[256];
        memset(buffer, '0', 256);
        int total, used, free, shared, buffers, cached;

        lpmem->dwLength = sizeof(MEMORYSTATUS);
        lpmem->dwTotalPhys = lpmem->dwAvailPhys = 0;
        lpmem->dwTotalPageFile = lpmem->dwAvailPageFile = 0;
        while (fgets(buffer, sizeof(buffer), f))
        {
            if (sscanf(buffer, "Mem: %d %d %d %d %d %d", &total, &used, &free, &shared, &buffers, &cached))
            {
                lpmem->dwTotalPhys += total;
                lpmem->dwAvailPhys += free + buffers + cached;
            }
            if (sscanf(buffer, "Swap: %d %d %d", &total, &used, &free))
            {
                lpmem->dwTotalPageFile += total;
                lpmem->dwAvailPageFile += free;
            }
            if (sscanf(buffer, "MemTotal: %d", &total))
            {
                lpmem->dwTotalPhys = total * 1024;
            }
            if (sscanf(buffer, "MemFree: %d", &free))
            {
                lpmem->dwAvailPhys = free * 1024;
            }
            if (sscanf(buffer, "SwapTotal: %d", &total))
            {
                lpmem->dwTotalPageFile = total * 1024;
            }
            if (sscanf(buffer, "SwapFree: %d", &free))
            {
                lpmem->dwAvailPageFile = free * 1024;
            }
            if (sscanf(buffer, "Buffers: %d", &buffers))
            {
                lpmem->dwAvailPhys += buffers * 1024;
            }
            if (sscanf(buffer, "Cached: %d", &cached))
            {
                lpmem->dwAvailPhys += cached * 1024;
            }
        }
        fclose(f);
        if (lpmem->dwTotalPhys)
        {
            DWORD TotalPhysical = lpmem->dwTotalPhys + lpmem->dwTotalPageFile;
            DWORD AvailPhysical = lpmem->dwAvailPhys + lpmem->dwAvailPageFile;
            lpmem->dwMemoryLoad = (TotalPhysical - AvailPhysical)  / (TotalPhysical / 100);
        }
    }
#endif
}

static const int YearLengths[2] = {DAYSPERNORMALYEAR, DAYSPERLEAPYEAR};
static const int MonthLengths[2][MONSPERYEAR] =
{
    { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 },
    { 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 }
};

static int IsLeapYear(int Year)
{
    return Year % 4 == 0 && (Year % 100 != 0 || Year % 400 == 0) ? 1 : 0;
}

static void NormalizeTimeFields(short* FieldToNormalize, short* CarryField, int Modulus)
{
    *FieldToNormalize = (short) (*FieldToNormalize - Modulus);
    *CarryField = (short) (*CarryField + 1);
}

bool TimeFieldsToTime(PTIME_FIELDS tfTimeFields, PLARGE_INTEGER Time)
{
    #define SECSPERMIN         60
    #define MINSPERHOUR        60
    #define HOURSPERDAY        24
    #define MONSPERYEAR        12

    #define EPOCHYEAR          1601

    #define SECSPERDAY         86400
    #define TICKSPERMSEC       10000
    #define TICKSPERSEC        10000000
    #define SECSPERHOUR        3600

    int CurYear, CurMonth;
    LONGLONG rcTime;
    TIME_FIELDS TimeFields = *tfTimeFields;

    rcTime = 0;
    while (TimeFields.Second >= SECSPERMIN)
    {
        NormalizeTimeFields(&TimeFields.Second, &TimeFields.Minute, SECSPERMIN);
    }
    while (TimeFields.Minute >= MINSPERHOUR)
    {
        NormalizeTimeFields(&TimeFields.Minute, &TimeFields.Hour, MINSPERHOUR);
    }
    while (TimeFields.Hour >= HOURSPERDAY)
    {
        NormalizeTimeFields(&TimeFields.Hour, &TimeFields.Day, HOURSPERDAY);
    }
    while (TimeFields.Day > MonthLengths[IsLeapYear(TimeFields.Year)][TimeFields.Month - 1])
    {
        NormalizeTimeFields(&TimeFields.Day, &TimeFields.Month, SECSPERMIN);
    }
    while (TimeFields.Month > MONSPERYEAR)
    {
        NormalizeTimeFields(&TimeFields.Month, &TimeFields.Year, MONSPERYEAR);
    }
    for (CurYear = EPOCHYEAR; CurYear < TimeFields.Year; CurYear++)
    {
        rcTime += YearLengths[IsLeapYear(CurYear)];
    }
    for (CurMonth = 1; CurMonth < TimeFields.Month; CurMonth++)
    {
        rcTime += MonthLengths[IsLeapYear(CurYear)][CurMonth - 1];
    }
    rcTime += TimeFields.Day - 1;
    rcTime *= SECSPERDAY;
    rcTime += TimeFields.Hour * SECSPERHOUR + TimeFields.Minute * SECSPERMIN + TimeFields.Second;

    rcTime *= TICKSPERSEC;
    rcTime += TimeFields.Milliseconds * TICKSPERMSEC;

    Time->QuadPart = rcTime;

    return true;
}

BOOL SystemTimeToFileTime(const SYSTEMTIME* syst, LPFILETIME ft)
{
    TIME_FIELDS tf;
    LARGE_INTEGER t;

    tf.Year = syst->wYear;
    tf.Month = syst->wMonth;
    tf.Day = syst->wDay;
    tf.Hour = syst->wHour;
    tf.Minute = syst->wMinute;
    tf.Second = syst->wSecond;
    tf.Milliseconds = syst->wMilliseconds;

    TimeFieldsToTime(&tf, &t);
    ft->dwLowDateTime = t.u.LowPart;
    ft->dwHighDateTime = t.u.HighPart;
    return TRUE;
}

void adaptFilenameToLinux(string& rAdjustedFilename)
{
    //first replace all \\ by /
    string::size_type loc = 0;
    while ((loc = rAdjustedFilename.find("\\", loc)) != string::npos)
    {
        rAdjustedFilename.replace(loc, 1, "/");
    }
    loc = 0;
    //remove /./
    while ((loc = rAdjustedFilename.find("/./", loc)) != string::npos)
    {
        rAdjustedFilename.replace(loc, 3, "/");
    }
}

void replaceDoublePathFilename(char* szFileName)
{
    //replace "\.\" by "\"
    string s(szFileName);
    string::size_type loc = 0;
    //remove /./
    while ((loc = s.find("/./", loc)) != string::npos)
    {
        s.replace(loc, 3, "/");
    }
    loc = 0;
    //remove "\.\"
    while ((loc = s.find("\\.\\", loc)) != string::npos)
    {
        s.replace(loc, 3, "\\");
    }
    strcpy((char*)szFileName, s.c_str());
}

const int comparePathNames(const char* cpFirst, const char* cpSecond, unsigned int len)
{
    //create two strings and replace the \\ by / and /./ by /
    string first(cpFirst);
    string second(cpSecond);
    adaptFilenameToLinux(first);
    adaptFilenameToLinux(second);
    if (strlen(cpFirst) < len || strlen(cpSecond) < len)
    {
        return -1;
    }
    unsigned int length = std::min(std::min(first.size(), second.size()), (size_t)len);    //make sure not to access invalid memory
    return memicmp(first.c_str(), second.c_str(), length);
}

#if defined(LINUX) || defined(APPLE)
static bool FixOnePathElement(char* path)
{
    if (*path == '\0')
    {
        return true;
    }

    if ((path[0] == '/') && (path[1] == '\0'))
    {
        return true;  // root dir always exists.
    }
    if (strchr(path, '*') || strchr(path, '?'))
    {
        return true; // wildcard...stop correcting path.
    }
    struct stat statbuf;
    if (stat(path, &statbuf) != -1)      // current case exists.
    {
        return true;
    }

    char* name = path;
    char* ptr = strrchr(path, '/');
    if (ptr)
    {
        name = ptr + 1;
        *ptr = '\0';
    }

    if (*name == '\0')      // trailing '/' ?
    {
        *ptr = '/';
        return true;
    }

    const char* parent;
    if (ptr == path)
    {
        parent = "/";
    }
    else if (ptr == NULL)
    {
        parent = ".";
    }
    else
    {
        parent = path;
    }

    DIR* dirp = opendir(parent);
    if (ptr)
    {
        *ptr = '/';
    }

    if (dirp == NULL)
    {
        return false;
    }

    struct dirent* dent;
    bool found = false;
    while ((dent = readdir(dirp)) != NULL)
    {
        if (strcasecmp(dent->d_name, name) == 0)
        {
            strcpy(name, dent->d_name);
            found = true;
            break;
        }
    }

    closedir(dirp);
    return found;
}
#endif

#define Int32x32To64(a, b) ((uint64)((uint64)(a)) * (uint64)((uint64)(b)))

//////////////////////////////////////////////////////////////////////////
threadID GetCurrentThreadId()
{
    return threadID(pthread_self());
}

//////////////////////////////////////////////////////////////////////////
HANDLE CreateEvent
(
    LPSECURITY_ATTRIBUTES lpEventAttributes,
    BOOL bManualReset,
    BOOL bInitialState,
    LPCSTR lpName
)
{
    //TODO: implement
    CRY_ASSERT_MESSAGE(0, "CreateEvent not implemented yet");
    return 0;
}


//////////////////////////////////////////////////////////////////////////
DWORD Sleep(DWORD dwMilliseconds)
{
#if defined(LINUX) || defined(APPLE)
    timespec req;
    timespec rem;

    memset(&req, 0, sizeof(req));
    memset(&rem, 0, sizeof(rem));

    time_t sec = (int)(dwMilliseconds / 1000);
    req.tv_sec = sec;
    req.tv_nsec = (dwMilliseconds - (sec * 1000)) * 1000000L;
    if (nanosleep(&req, &rem) == -1)
    {
        nanosleep(&rem, 0);
    }

    return 0;
#else
    timeval tv, start, now;
    uint64 tStart;

    memset(&tv, 0, sizeof tv);
    memset(&start, 0, sizeof start);
    memset(&now, 0, sizeof now);
    gettimeofday(&now, NULL);
    start = now;
    tStart = (uint64)start.tv_sec * 1000000 + start.tv_usec;
    while (true)
    {
        uint64 tNow, timePassed, timeRemaining;
        tNow = (uint64)now.tv_sec * 1000000 + now.tv_usec;
        timePassed = tNow - tStart;
        if (timePassed >= dwMilliseconds)
        {
            break;
        }
        timeRemaining = dwMilliseconds * 1000 - timePassed;
        tv.tv_sec = timeRemaining / 1000000;
        tv.tv_usec = timeRemaining % 1000000;
        select(1, NULL, NULL, NULL, &tv);
        gettimeofday(&now, NULL);
    }
    return 0;
#endif
}

//////////////////////////////////////////////////////////////////////////
DWORD SleepEx(DWORD dwMilliseconds, BOOL bAlertable)
{
    //TODO: implement
    //  CRY_ASSERT_MESSAGE(0, "SleepEx not implemented yet");
    printf("SleepEx not properly implemented yet\n");
    Sleep(dwMilliseconds);
    return 0;
}

//////////////////////////////////////////////////////////////////////////
DWORD WaitForSingleObjectEx(HANDLE hHandle, DWORD dwMilliseconds,   BOOL bAlertable)
{
    //TODO: implement
    CRY_ASSERT_MESSAGE(0, "WaitForSingleObjectEx not implemented yet");
    return 0;
}

#if 0
//////////////////////////////////////////////////////////////////////////
DWORD WaitForMultipleObjectsEx(
    DWORD nCount,
    const HANDLE* lpHandles,
    BOOL bWaitAll,
    DWORD dwMilliseconds,
    BOOL bAlertable)
{
    //TODO: implement
    return 0;
}
#endif

//////////////////////////////////////////////////////////////////////////
DWORD WaitForSingleObject(HANDLE hHandle, DWORD dwMilliseconds)
{
    //TODO: implement
    CRY_ASSERT_MESSAGE(0, "WaitForSingleObject not implemented yet");
    return 0;
}

//////////////////////////////////////////////////////////////////////////
BOOL SetEvent(HANDLE hEvent)
{
    //TODO: implement
    CRY_ASSERT_MESSAGE(0, "SetEvent not implemented yet");
    return TRUE;
}

//////////////////////////////////////////////////////////////////////////
BOOL ResetEvent(HANDLE hEvent)
{
    //TODO: implement
    CRY_ASSERT_MESSAGE(0, "ResetEvent not implemented yet");
    return TRUE;
}

//////////////////////////////////////////////////////////////////////////
HANDLE CreateMutex
(
    LPSECURITY_ATTRIBUTES lpMutexAttributes,
    BOOL bInitialOwner,
    LPCSTR lpName
)
{
    //TODO: implement
    CRY_ASSERT_MESSAGE(0, "CreateMutex not implemented yet");
    return 0;
}

//////////////////////////////////////////////////////////////////////////
BOOL ReleaseMutex(HANDLE hMutex)
{
    //TODO: implement
    CRY_ASSERT_MESSAGE(0, "ReleaseMutex not implemented yet");
    return TRUE;
}

//////////////////////////////////////////////////////////////////////////


typedef DWORD (* PTHREAD_START_ROUTINE)(LPVOID lpThreadParameter);
typedef PTHREAD_START_ROUTINE LPTHREAD_START_ROUTINE;

//////////////////////////////////////////////////////////////////////////
HANDLE CreateThread
(
    LPSECURITY_ATTRIBUTES lpThreadAttributes,
    SIZE_T dwStackSize,
    LPTHREAD_START_ROUTINE lpStartAddress,
    LPVOID lpParameter,
    DWORD dwCreationFlags,
    LPDWORD lpThreadId
)
{
    //TODO: implement
    CRY_ASSERT_MESSAGE(0, "CreateThread not implemented yet");
    return 0;
}

#if defined(LINUX) || defined(APPLE)
BOOL GetComputerName(LPSTR lpBuffer, LPDWORD lpnSize)
{
    if (!lpBuffer || !lpnSize)
    {
        return FALSE;
    }

    int err = gethostname(lpBuffer, *lpnSize);

    if (-1 == err)
    {
        CryLog("GetComputerName falied [%d]\n", errno);
        return FALSE;
    }
    return TRUE;
}
#endif

#if defined(LINUX) || defined(APPLE) || defined(ORBIS) // ACCEPTED_USE
DWORD GetCurrentProcessId(void)
{
    return (DWORD)getpid();
}
#endif

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
void CrySleep(unsigned int dwMilliseconds)
{
    Sleep(dwMilliseconds);
}

//////////////////////////////////////////////////////////////////////////
void CryLowLatencySleep(unsigned int dwMilliseconds)
{
    CrySleep(dwMilliseconds);
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
int CryMessageBox(const char* lpText, const char* lpCaption, unsigned int uType)
{
#ifdef WIN32
#   error WIN32 is defined in WinBase.cpp (it is a non-Windows file)
#elif defined(MAC)
    CFStringRef strText = CFStringCreateWithCString(NULL, lpText, strlen(lpText));
    CFStringRef strCaption = CFStringCreateWithCString(NULL, lpCaption, strlen(lpCaption));

    CFOptionFlags kResult;
    CFUserNotificationDisplayAlert(
        0, // no timeout
        kCFUserNotificationNoteAlertLevel, //change it depending message_type flags ( MB_ICONASTERISK.... etc.)
        NULL, //icon url, use default, you can change it depending message_type flags
        NULL, //not used
        NULL, //localization of strings
        strText, //header text
        strCaption, //message text
        NULL, //default "ok" text in button
        CFSTR("Cancel"), //alternate button title
        NULL, //other button title, null--> no other button
        &kResult //response flags
        );

    CFRelease(strCaption);
    CFRelease(strText);

    if (kResult == kCFUserNotificationDefaultResponse)
    {
        return 1;   // IDOK on Win32
    }
    else
    {
        return 2;   // IDCANCEL on Win32
    }
#else
    printf("Messagebox: cap: %s  text:%s\n", lpCaption ? lpCaption : " ", lpText ? lpText : " ");
    return 0;
#endif
}

//////////////////////////////////////////////////////////////////////////
DWORD GetCurrentDirectory(DWORD nBufferLength, char* lpBuffer)
{
    return 0;
}

//////////////////////////////////////////////////////////////////////////
void CryGetCurrentDirectory(unsigned int nBufferLength, char* lpBuffer)
{
    if (nBufferLength > 0 && lpBuffer)
    {
        *lpBuffer = 0;
    }
}

//////////////////////////////////////////////////////////////////////////
short CryGetAsyncKeyState(int vKey)
{
    //TODO: implement
    CRY_ASSERT_MESSAGE(0, "CryGetAsyncKeyState not implemented yet");
    return 0;
}

#if defined(LINUX) || defined(APPLE)
//[K01]: http://www.memoryhole.net/kyle/2007/05/atomic_incrementing.html
//http://forums.devx.com/archive/index.php/t-160558.html
//////////////////////////////////////////////////////////////////////////
DLL_EXPORT LONG  CryInterlockedIncrement(LONG volatile* lpAddend)
{
    /*int r;
    __asm__ __volatile__ (
        "lock ; xaddl %0, (%1) \n\t"
        : "=r" (r)
        : "r" (lpAddend), "0" (1)
        : "memory"
    );
    return (LONG) (r + 1); */// add, since we get the original value back.
    return __sync_fetch_and_add(lpAddend, 1) + 1;
}

//////////////////////////////////////////////////////////////////////////
DLL_EXPORT LONG  CryInterlockedDecrement(LONG volatile* lpAddend)
{
    /*int r;
    __asm__ __volatile__ (
        "lock ; xaddl %0, (%1) \n\t"
        : "=r" (r)
        : "r" (lpAddend), "0" (-1)
        : "memory"
    );
    return (LONG) (r - 1);  */// subtract, since we get the original value back.
    return __sync_fetch_and_sub(lpAddend, 1) - 1;
}

//////////////////////////////////////////////////////////////////////////
DLL_EXPORT LONG      CryInterlockedExchangeAdd(LONG  volatile* lpAddend, LONG  Value)
{
    /*  LONG r;
        __asm__ __volatile__ (
        #if defined(LINUX64) || defined(APPLE)  // long is 64 bits on amd64.
            "lock ; xaddq %0, (%1) \n\t"
        #else
            "lock ; xaddl %0, (%1) \n\t"
        #endif
            : "=r" (r)
            : "r" (lpAddend), "0" (Value)
            : "memory"
        );
        return r;*/
    return __sync_fetch_and_add(lpAddend, Value);
}

DLL_EXPORT LONG     CryInterlockedOr(LONG volatile* Destination, LONG Value)
{
    return __sync_fetch_and_or(Destination, Value);
}

DLL_EXPORT LONG     CryInterlockedCompareExchange(LONG  volatile* dst, LONG  exchange, LONG comperand)
{
    return __sync_val_compare_and_swap(dst, comperand, exchange);
    /*LONG r;
    __asm__ __volatile__ (
    #if defined(LINUX64) || defined(APPLE)  // long is 64 bits on amd64.
        "lock ; cmpxchgq %2, (%1) \n\t"
    #else
        "lock ; cmpxchgl %2, (%1) \n\t"
    #endif
        : "=a" (r)
        : "r" (dst), "r" (exchange), "0" (comperand)
        : "memory"
    );
    return r;*/
}


DLL_EXPORT void*     CryInterlockedCompareExchangePointer(void* volatile* dst, void* exchange, void* comperand)
{
    return __sync_val_compare_and_swap(dst, comperand, exchange);
    //return (void*)CryInterlockedCompareExchange((long volatile*)dst, (long)exchange, (long)comperand);
}

DLL_EXPORT void*     CryInterlockedExchangePointer(void* volatile* dst, void* exchange)
{
    __sync_synchronize();
    return __sync_lock_test_and_set(dst, exchange);
    //return (void*)CryInterlockedCompareExchange((long volatile*)dst, (long)exchange, (long)comperand);
}

#if defined(LINUX64) || defined(MAC) || defined(IOS_SIMULATOR)
DLL_EXPORT unsigned char _InterlockedCompareExchange128(int64 volatile* dst, int64 exchangehigh, int64 exchangelow, int64* comperand)
{
    bool bEquals;
    __asm__ __volatile__
    (
        "lock cmpxchg16b %1\n\t"
        "setz %0"
        : "=q" (bEquals), "+m" (*dst), "+d" (comperand[1]), "+a" (comperand[0])
        : "c" (exchangehigh), "b" (exchangelow)
        : "cc"
    );
    return (char)bEquals;
}
#elif defined(INTERLOCKED_COMPARE_EXCHANGE_128_NOT_SUPPORTED)
DLL_EXPORT unsigned char _InterlockedCompareExchange128(int64 volatile* dst, int64 exchangehigh, int64 exchangelow, int64* comperand)
{
    // Here there be dragons...
    //
    // arm64 processors do not provide a cmpxchg16b (or equivalent) instruction,
    // so _InterlockedCompareExchange128 is not implemented on arm64 platforms.
    //
    // Various attempts were made to emulate this in a thread safe manner, but
    // all ultimately failed as there is nothing stopping systems doing things
    // in other threads with the memory that gets passed into this function to
    // be compared and swapped.
    //
    // Ideally we simply wouldn't define this function at all for arm64 platforms,
    // but the cry job system still compiles it in, although it is never actually
    // called because we set JOBMANAGER_DISABLED 1 in IJobManager.h for arm64.
    AZ_Assert(false, "_InterlockedCompareExchange128 called on an arm64 platform, which is not supported.");
    return 0;
}
#endif

threadID CryGetCurrentThreadId()
{
    return GetCurrentThreadId();
}

void CryDebugBreak()
{
    __builtin_trap();
}
#endif//LINUX APPLE

#if defined(APPLE) || defined(LINUX)
// WinAPI debug functions.
DLL_EXPORT void OutputDebugString(const char* outputString)
{
#if _DEBUG
    // There is no such thing as a debug console on XCode
    fprintf(stderr, "debug: %s\n", outputString);
#endif
}


DLL_EXPORT void DebugBreak()
{
    CryDebugBreak();
}

#endif

// This code does not have a long life span and will be replaced soon
#if defined(APPLE) || defined(LINUX)

typedef DIR* FS_DIR_TYPE;
typedef dirent FS_DIRENT_TYPE;
static const FS_ERRNO_TYPE FS_ENOENT = ENOENT;
static const FS_ERRNO_TYPE FS_EINVAL = EINVAL;
static const FS_DIR_TYPE FS_DIR_NULL = NULL;
static const unsigned char FS_TYPE_DIRECTORY = DT_DIR;

typedef int FS_ERRNO_TYPE;

#if defined(APPLE)
typedef struct stat FS_STAT_TYPE;
#else
typedef struct stat64 FS_STAT_TYPE;
#endif

#include <mutex>

bool CrySetFileAttributes(const char* lpFileName, uint32 dwFileAttributes)
{
    //TODO: implement
    printf("CrySetFileAttributes not properly implemented yet\n");
    return false;
}


ILINE void FS_OPEN(const char* szFileName, int iFlags, int& iFileDesc, mode_t uMode, FS_ERRNO_TYPE& rErr)
{
    rErr = ((iFileDesc = open(szFileName, iFlags, uMode)) != -1) ? 0 : errno;
}

ILINE void FS_CLOSE(int iFileDesc, FS_ERRNO_TYPE& rErr)
{
    rErr = close(iFileDesc) != -1 ? 0 : errno;
}

ILINE void FS_CLOSE_NOERR(int iFileDesc)
{
    close(iFileDesc);
}

ILINE void FS_OPENDIR(const char* szDirName, FS_DIR_TYPE& pDir, FS_ERRNO_TYPE& rErr)
{
    rErr = (pDir = opendir(szDirName)) != NULL ? 0 : errno;
}

ILINE void FS_READDIR(FS_DIR_TYPE pDir, FS_DIRENT_TYPE& kEnt, uint64_t& uEntSize, FS_ERRNO_TYPE& rErr)
{
    errno = 0;  // errno is used to determine if readdir succeeds after
    FS_DIRENT_TYPE* pDirent(readdir(pDir));
    if (pDirent == NULL)
    {
        uEntSize = 0;
        rErr = (errno == FS_ENOENT) ? 0 : errno;
    }
    else
    {
        kEnt = *pDirent;
        uEntSize = static_cast<uint64_t>(sizeof(FS_DIRENT_TYPE));
        rErr = 0;
    }
}

ILINE void FS_STAT(const char* szFileName, FS_STAT_TYPE& kStat, FS_ERRNO_TYPE& rErr)
{
#if defined(APPLE)
    rErr = stat(szFileName, &kStat) != -1 ? 0 : errno;
#else
    rErr = stat64(szFileName, &kStat) != -1 ? 0 : errno;
#endif
}

ILINE void FS_FSTAT(int iFileDesc, FS_STAT_TYPE& kStat, FS_ERRNO_TYPE& rErr)
{
#if defined(APPLE)
    rErr = fstat(iFileDesc, &kStat) != -1 ? 0 : errno;
#else
    rErr = fstat64(iFileDesc, &kStat) != -1 ? 0 : errno;
#endif
}

ILINE void FS_CLOSEDIR(FS_DIR_TYPE pDir, FS_ERRNO_TYPE& rErr)
{
    errno = 0;
    rErr = closedir(pDir) == 0 ? 0 : errno;
}

ILINE void FS_CLOSEDIR_NOERR(FS_DIR_TYPE pDir)
{
    closedir(pDir);
}

const bool GetFilenameNoCase
(
    const char* file,
    char* pAdjustedFilename,
    const bool cCreateNew
)
{
    assert(file);
    assert(pAdjustedFilename);
    strcpy(pAdjustedFilename, file);

    // Fix the dirname case.
    const int cLen = strlen(file);
    for (int i = 0; i < cLen; ++i)
    {
        if (pAdjustedFilename[i] == '\\')
        {
            pAdjustedFilename[i] = '/';
        }
    }

    char* slash;
    const char* dirname;
    char* name;
    FS_ERRNO_TYPE fsErr = 0;
    FS_DIRENT_TYPE dirent;
    uint64_t direntSize = 0;
    FS_DIR_TYPE fd = FS_DIR_NULL;

    if (
        (pAdjustedFilename) == (char*)-1)
    {
        return false;
    }

    slash = strrchr(pAdjustedFilename, '/');
    if (slash)
    {
        dirname = pAdjustedFilename;
        name = slash + 1;
        *slash = 0;
    }
    else
    {
        dirname = ".";
        name = pAdjustedFilename;
    }

#if !defined(LINUX) && !defined(APPLE)      // fix the parent path anyhow.
    // Check for wildcards. We'll always return true if the specified filename is
    // a wildcard pattern.
    if (strchr(name, '*') || strchr(name, '?'))
    {
        if (slash)
        {
            *slash = '/';
        }
        return true;
    }
#endif

    // Scan for the file.
    bool found = false;
    bool skipScan = false;

    if (slash)
    {
        *slash = '/';
    }

#if FIX_FILENAME_CASE
    char* path = pAdjustedFilename;
    char* sep;
    while ((sep = strchr(path, '/')) != NULL)
    {
        *sep = '\0';
        const bool exists = FixOnePathElement(pAdjustedFilename);
        *sep = '/';
        if (!exists)
        {
            return false;
        }

        path = sep + 1;
    }
    if (!FixOnePathElement(pAdjustedFilename)) // catch last filename.
    {
        return false;
    }

#else
    for (char* c = pAdjustedFilename; *c; ++c)
    {
        *c = tolower(*c);
    }
#endif

    return true;
}

DWORD GetFileAttributes(LPCSTR lpFileName)
{
    struct stat fileStats;
    const int success = stat(lpFileName, &fileStats);
    if (success == -1)
    {
        char adjustedFilename[MAX_PATH];
        GetFilenameNoCase(lpFileName, adjustedFilename);
        if (stat(adjustedFilename, &fileStats) == -1)
        {
            return (DWORD)INVALID_FILE_ATTRIBUTES;
        }
    }
    DWORD ret = 0;

    const int acc = (fileStats.st_mode & S_IWRITE);

    if (acc != 0)
    {
        if (S_ISDIR(fileStats.st_mode) != 0)
        {
            ret |= FILE_ATTRIBUTE_DIRECTORY;
        }
    }
    return (ret == 0) ? FILE_ATTRIBUTE_NORMAL : ret;//return file attribute normal as the default value, must only be set if no other attributes have been found
}

uint32 CryGetFileAttributes(const char* lpFileName)
{
    
    string fn = lpFileName;
    adaptFilenameToLinux(fn);
    const char* buffer = fn.c_str();
    return GetFileAttributes(buffer);
    
}

__finddata64_t::~__finddata64_t()
{
    if (m_Dir != FS_DIR_NULL)
    {
        FS_CLOSEDIR_NOERR(m_Dir);
        m_Dir = FS_DIR_NULL;
    }
}
#endif //defined(APPLE) || defined(LINUX)
