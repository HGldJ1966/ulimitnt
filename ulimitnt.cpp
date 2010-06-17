//UlimitNT v2.7 - Create a job object with processes
//Copyright 2010 Christopher Smith

//This program is free software; you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation; version 2 of the License.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

//You should have received a copy of the GNU General Public License
//along with this program; if not, write to the Free Software
//Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

#define _WIN32_WINNT 0x0500

#include <windows.h>
#include <iostream>
#include <string>
#include <vector>
#include <sstream>

#define NT_ERROR(Status) ((ULONG)(Status) >> 30 == 3)

using namespace std;

enum PROCESS_INFORMATION_CLASS
{
	ProcessBasicInformation = 0
};

//typedef DWORD NTSTATUS;

struct PROCESS_BASIC_INFORMATION
{
	NTSTATUS ExitStatus;
	PVOID PebBase;
	KAFFINITY AffinityMask;
	int BasePriority;
	ULONG ProcessId;
	ULONG InheritedId;
};

//There's no Win32 api to query a process's parent for some reason...
extern "C" NTSYSAPI NTSTATUS NTAPI NtQueryInformationProcess(HANDLE Process, PROCESS_INFORMATION_CLASS InfoClass, PVOID Info, ULONG InfoLength, PULONG ReturnLength);

void ShowHelp()
{
	wcout << L"UlimitNT v2.7 - Associate new or existing processes with a job," << endl;
	wcout << L"\trestricted token and/or desktop." << endl;
	wcout << L"Copyright (c) 2006 Christopher Smith" << endl;
	wcout << L"This program is distributed under the GNU General Public License" << endl << endl;
	wcout << L"Usage: UlimitNT [-option <parameter> ...] [executable ...]" << endl;
	wcout << L"Options (not case sensitive):" << endl;
	wcout << L"-UseDesktop\t\tUse the named [window station\\]desktop" << endl;
	wcout << L"-UseOpenDesktop\t\tUse the existing named desktop" << endl;
	wcout << L"-SwitchDesktop\t\tSwitch to the previously named desktop" << endl;
	wcout << L"-LimitDesktop\t\tPrevents desktop switching" << endl;
	wcout << L"-DisplaySettings\tPrevents display changes" << endl;
	wcout << L"-ExitWindows\t\tPrevents shutdown/logoff" << endl;
	wcout << L"-GlobalAtoms\t\tForces a private atom table" << endl;
	wcout << L"-Handles\t\tPrevents access of USER handles outside the job" << endl;
	wcout << L"-AllowCursor\t\tAllows access to the mouse cursor despite -Handles" << endl;
	wcout << L"-ReadClipboard\t\tPrevents reading of the clipboard" << endl;
	wcout << L"-WriteClipboard\t\tPrevents writing of the clipboard" << endl;
	wcout << L"-SystemParameters\tPrevents changing of USER parameters" << endl;
	wcout << L"-BreakawayOk\t\tChild processes can break away from the job upon parent request" << endl;
	wcout << L"-BreakawaySilent\tChild processes always break away from the job" << endl;
	wcout << L"-Breakaway\t\tRequest that new processes be outside the pre-existing job" << endl;
	wcout << L"-DieOnUnException\tProcesses will die on unhandled exception (no debugging)" << endl;	
	wcout << L"-NoAdmin\t\tProcesses cannot use any tokens that include local admins" << endl;
	wcout << L"-Restricted\t\tProcesses cannot use tokens that aren't restricted" << endl;
	wcout << L"-PrcLimit num\t\tMaximum number of processes that can be in the job" << endl;
	wcout << L"-JobTime num\t\tUser mode time limit (in 100ns units) for entire job" << endl;
	wcout << L"-PrcTime num\t\tUser mode time limit (in 100ns units) for individual processes" << endl;
	wcout << L"-JobMem num\t\tMaximum memory (in bytes) the entire job can commit" << endl;
	wcout << L"-PrcMem num\t\tMaximum memory (in bytes) any process can commit" << endl;
	wcout << L"-Sched num\t\tScheduling class, from 0 to 9 where 9 is highest" << endl;
	wcout << L"-WorkingSet min max\tA fixed working set for each process, in bytes" << endl;
	wcout << L"-DSid\t\t\tMakes the specified SID deny-only." << endl;
	wcout << L"-RSid\t\t\tMakes the job/processes restricted to these SIDs only" << endl;
	wcout << L"-[D|R]SidLogon\t\tDisables or restrictes the logon SID" << endl;
	wcout << L"-[D|R]SidUser\t\tDisables or restrictes the user SID" << endl;
	wcout << L"-DPriv\t\t\tDeletes the named privelege." << endl;
	wcout << L"-DPrivMax\t\tDeletes all priveleges." << endl;
	wcout << L"-JobToken\t\tForces creation of a job with the token." << endl;
	wcout << L"-DupJob\t\t\tGives new processes a no-access handle to job to keep job open" << endl;
	wcout << L"-NoDupJob\t\tPrevents -DupJob. Use with -Name" << endl;
	wcout << L"-Name text\t\tGives the job a name. Implies -DupJob" << endl;
	wcout << L"-Reset\t\t\tCauses reset of job limits. Use with -Name" << endl;
	wcout << L"-Parent num\t\tAdds the parent process of ulimitnt num levels up to the job" << endl;
	wcout << L"-ProcessID num\t\tAdds the process with the specified ID to the job" << endl;
	exit(0);
}

DWORD String2DWORD(wstring &x)
{
	WCHAR LastChar = x[x.length() - 1];
	DWORD Magnitude;

	switch(LastChar)
	{
	case 'K':
		Magnitude = 10;
		break;

	case 'M':
		Magnitude = 20;
		break;

	case 'G':
		Magnitude = 30;
		break;

	default:
		Magnitude = 0;
	};

	wstringstream ss(x);
	DWORD r;
	ss >> r;
	return r << Magnitude;
}

LARGE_INTEGER String2LINT(wstring &x)
{
	wstringstream ss(x);
	__int64 r0;
	ss >> r0;
	LARGE_INTEGER r1;
	r1.QuadPart = r0;
	return r1;
}

wstring GetNext(int argc, wchar_t *argv[], int &i)
{
	if(argc > i)
		return argv[++i];
	else
		return wstring();
}

BOOL HandleError(BOOL x, PCWSTR Operation = NULL, PCWSTR Object = NULL)
{
	if(!x)
	{
		LPTSTR pb;
		FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), 0, (LPTSTR)&pb, 0, NULL);
		wcout << pb;
		LocalFree(pb);
		if(Operation)
			wcout << L"while " << Operation;
		if(Object)
			wcout << L" " << Object;
		wcout << endl;

		exit(GetLastError());
	}
	return x;
}

bool CompareNoCase(wstring x, wstring y)
{
	return CompareString(LOCALE_USER_DEFAULT, NORM_IGNORECASE | NORM_IGNOREWIDTH, x.data(), (int)x.length(), y.data(), (int)y.length()) == CSTR_EQUAL;
}

template <class T> size_t auto_heap_block_length(T const *x);

template <class T> class auto_heap_block
{
	union
	{
		void *Ptr; //a pointer to T, treated as an opaque blob
		T *TypedPtr; //for debugging convenience
	};
	void FreePtrIf()
	{
		if(Ptr)
			delete Ptr;
	}
	void AllocCopySetPtr(T const *x)
	{
		size_t Length = auto_heap_block_length(x);
		Ptr = new byte[Length];
		RtlCopyMemory(Ptr, x, Length);
	}
public:
	auto_heap_block() :
		Ptr(NULL)
	{}
	auto_heap_block(auto_heap_block const &Src) :
		Ptr(NULL)
	{
		AllocCopySetPtr(reinterpret_cast<T *>(Src.Ptr));
	}
	auto_heap_block(T const *Ptr) :
		Ptr(NULL)
	{
		AllocCopySetPtr(Ptr);
	}
	explicit auto_heap_block(size_t Length) :
		Ptr(NULL)
	{
		Ptr = new byte[Length];
	}
	void realloc(size_t Length)
	{
		FreePtrIf();
		Ptr = new byte[Length];
	}
	operator T *()
	{
		return reinterpret_cast<T *>(Ptr);
	}
	T *operator->()
	{
		return reinterpret_cast<T *>(Ptr);
	}
	T operator *()
	{
		return *reinterpret_cast<T *>(Ptr);
	}
	auto_heap_block<T> &operator =(auto_heap_block<T> const &x)
	{
		FreePtrIf();
		AllocCopySetPtr(reinterpret_cast<T *>(x.Ptr));
		return *this;
	}
	auto_heap_block<T> &operator =(T const *x)
	{
		FreePtrIf();
		AllocCopySetPtr(x);
		return *this;
	}
	~auto_heap_block()
	{
		FreePtrIf();
	}
};

template <> size_t auto_heap_block_length(SID const *x)
{
	return GetLengthSid(const_cast<SID *>(x)); //GetLengthSid won't be modifying the length
}

template <> size_t auto_heap_block_length(TOKEN_GROUPS const *x)
{
	return FIELD_OFFSET(TOKEN_GROUPS, Groups) + sizeof(SID_AND_ATTRIBUTES) * x->GroupCount; //header + variable
}

template <> size_t auto_heap_block_length(wchar_t const *x)
{
	return wcslen(x) * sizeof(wchar_t) + sizeof(wchar_t);
}

auto_heap_block<SID> LookupSidByName(wstring name)
{
	DWORD cbSid = 128;
	DWORD cbDomain = 256;
	SID_NAME_USE use;
	auto_heap_block<wchar_t> domain(cbDomain);
	auto_heap_block<SID> sid(cbSid);
	HandleError(LookupAccountName(NULL, name.c_str(), sid, &cbSid, domain, &cbDomain, &use), L"looking up account name", name.c_str());
	return sid;
}

struct AUTO_SID_AND_ATTRIBUTES
{
	auto_heap_block<SID> Sid;
	DWORD Attributes;
};

struct ulimitnt
{
	vector<wstring> proclist;

	vector<AUTO_SID_AND_ATTRIBUTES> rsidlist;
	vector<AUTO_SID_AND_ATTRIBUTES> dsidlist;
	vector<LUID_AND_ATTRIBUTES> dprivs;
	vector<DWORD> procidlist;
	DWORD parents;
	wstring jobname;
	wstring desktopname;
	DWORD ProcessFlags;
	JOBOBJECT_BASIC_UI_RESTRICTIONS UILimit;
	JOBOBJECT_EXTENDED_LIMIT_INFORMATION BasicLimit;
	JOBOBJECT_SECURITY_LIMIT_INFORMATION SecLimit;
	bool DupJob;
	bool HasJob;
	bool HasToken;
	bool HasDesktop;
	bool OpenDesktopOnly;
	bool HasSwitchDesktop;
	bool AllowCursor;
	DWORD TokenFlags;
	HANDLE hJob;
	HANDLE hToken;
	HANDLE hCurrentToken;
	HDESK hDesktop;

	ulimitnt() :
		ProcessFlags(CREATE_SUSPENDED | CREATE_NEW_CONSOLE),
		DupJob(false),
		HasJob(false),
		HasToken(false),
		HasDesktop(false),
		OpenDesktopOnly(false),
		HasSwitchDesktop(false),
		AllowCursor(false),
		TokenFlags(0),
		hJob(0),
		hToken(0),
		hCurrentToken(0),
		hDesktop(0),
		parents(0)
	{
		RtlZeroMemory(&UILimit, sizeof(JOBOBJECT_BASIC_UI_RESTRICTIONS));
		RtlZeroMemory(&BasicLimit, sizeof(JOBOBJECT_EXTENDED_LIMIT_INFORMATION));
		RtlZeroMemory(&SecLimit, sizeof(JOBOBJECT_SECURITY_LIMIT_INFORMATION));
	}

	~ulimitnt()
	{
		if(hJob)
			CloseHandle(hJob);
		if(hToken)
			CloseHandle(hToken);
		if(hCurrentToken)
			CloseHandle(hCurrentToken);
		if(hDesktop)
			CloseDesktop(hDesktop);
	}

	HANDLE DemandCurrentToken()
	{
		if(!hCurrentToken)
			HandleError(OpenProcessToken(GetCurrentProcess(), TOKEN_ALL_ACCESS, &hCurrentToken), L"opening current token");
		return hCurrentToken;
	}

	auto_heap_block<SID> GetLogonSid()
	{
		HANDLE CurrentToken = DemandCurrentToken();
		DWORD gsize = 0;
		auto_heap_block<SID> ret;
		GetTokenInformation(CurrentToken, TokenGroups, 0, 0, &gsize);
		auto_heap_block<TOKEN_GROUPS> groups(gsize);
		HandleError(GetTokenInformation(CurrentToken, TokenGroups, groups, gsize, &gsize), L"getting logon token information");
		for(int i = groups->GroupCount; i--;)
			if((groups->Groups[i].Attributes & SE_GROUP_LOGON_ID) == SE_GROUP_LOGON_ID)
			{
				DWORD cbSid = 128;
				ret.realloc(cbSid);
				HandleError(CopySid(cbSid, ret, groups->Groups[i].Sid), L"copying a SID");
				break;
			}
		return ret;
	}

	auto_heap_block<SID> GetUserSid()
	{
		HANDLE CurrentToken = DemandCurrentToken();
		DWORD cbSid = 128;
		auto_heap_block<SID> ret(cbSid);
		DWORD tusersize = 0;
		GetTokenInformation(CurrentToken, TokenUser, 0, 0, &tusersize);
		auto_heap_block<TOKEN_USER> tu(tusersize);
		HandleError(GetTokenInformation(CurrentToken, TokenUser, tu, tusersize, &tusersize), L"getting token information");
		HandleError(CopySid(cbSid, ret, tu->User.Sid), L"copying a SID");
		return ret;
	}

	void PushParentProcesses()
	{
		HANDLE Current = GetCurrentProcess();
		PROCESS_BASIC_INFORMATION ProcessInfo;
		ULONG ActualLength;

		for(DWORD i = parents; parents--;)
		{
			NTSTATUS Status = NtQueryInformationProcess(Current, ProcessBasicInformation, &ProcessInfo, sizeof ProcessInfo, &ActualLength);
			if(NT_ERROR(Status))
				printf("Error finding a parent process. Status:%X", Status);
			wcout << L"Parent:" << ProcessInfo.InheritedId << endl;
			CloseHandle(Current);
			Current = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, ProcessInfo.InheritedId);
			HandleError(Current != NULL, L"opening a parent process");
			procidlist.push_back(ProcessInfo.InheritedId);
		}
	}

	void AddProcessToJob(HANDLE process)
	{
		if(hJob)//if there actually is a job
			HandleError(AssignProcessToJobObject(hJob, process), L"adding a process to the job");

		if(DupJob) //give child a job handle to keep job alive, but with no access
			HandleError(DuplicateHandle(GetCurrentProcess(), hJob, process, NULL, 0, false, 0), L"duplicating the job handle to the attached process");
	}

	void OpenAddProcessToJob(DWORD ProcessId)
	{
		HANDLE Process;

		Process = OpenProcess(PROCESS_DUP_HANDLE | PROCESS_SET_QUOTA | PROCESS_TERMINATE | PROCESS_SET_INFORMATION, FALSE, ProcessId);

		HandleError(Process != NULL, L"opening a process by ID");

		AddProcessToJob(Process);

		CloseHandle(Process);
	}

	void CreateProcessWithJob(wstring fn)
	{
		PROCESS_INFORMATION PI;
		STARTUPINFO SI;
		RtlZeroMemory(&SI, sizeof(STARTUPINFO));
		SI.cb = sizeof(STARTUPINFO);
		auto_heap_block<wchar_t> fname, desktopnamech;
		if(HasDesktop)
		{
			desktopnamech = desktopname.c_str();
			SI.lpDesktop = desktopnamech;
		}
		fname = fn.c_str();
		
		if(HasToken)
			HandleError(CreateProcessAsUser(hToken, NULL, fname, NULL, NULL, FALSE, ProcessFlags, NULL, NULL, &SI, &PI), L"creating a process");
		else
			HandleError(CreateProcess(NULL, fname, NULL, NULL, FALSE, ProcessFlags, NULL, NULL, &SI, &PI), L"creating a process");

		AddProcessToJob(PI.hProcess);

		HandleError(ResumeThread(PI.hThread), L"starting a process");
		
		if(HasDesktop)
			WaitForInputIdle(PI.hProcess, 500);

		CloseHandle(PI.hProcess);
		CloseHandle(PI.hThread);
	}

	void ParseArgs(int argc, wchar_t *argv[])
	{
		for(int i = 1; i < argc; i++)
		{
			wstring c(argv[i]);
			//wcout << i << L":" << c << endl;
			if(CompareNoCase(c, L"-h") || CompareNoCase(c, L"-?") || CompareNoCase(c, L"/?") || CompareNoCase(c, L"-Help"))
				ShowHelp();
			else if(CompareNoCase(c, L"-LimitDesktop"))
			{
				UILimit.UIRestrictionsClass |= JOB_OBJECT_UILIMIT_DESKTOP;
				HasJob = true;
			}
			else if(CompareNoCase(c, L"-DisplaySettings"))
			{
				UILimit.UIRestrictionsClass |= JOB_OBJECT_UILIMIT_DISPLAYSETTINGS;
				HasJob = true;
			}
			else if(CompareNoCase(c, L"-ExitWindows"))
			{
				UILimit.UIRestrictionsClass |= JOB_OBJECT_UILIMIT_EXITWINDOWS;
				HasJob = true;
			}
			else if(CompareNoCase(c, L"-GlobalAtoms"))
			{
				UILimit.UIRestrictionsClass |= JOB_OBJECT_UILIMIT_GLOBALATOMS;
				HasJob = true;
			}
			else if(CompareNoCase(c, L"-Handles"))
			{
				UILimit.UIRestrictionsClass |= JOB_OBJECT_UILIMIT_HANDLES;
				HasJob = true;
			}
			else if(CompareNoCase(c, L"-ReadClipboard"))
			{
				UILimit.UIRestrictionsClass |= JOB_OBJECT_UILIMIT_READCLIPBOARD;
				HasJob = true;
			}
			else if(CompareNoCase(c, L"-SystemParameters"))
			{
				UILimit.UIRestrictionsClass |= JOB_OBJECT_UILIMIT_SYSTEMPARAMETERS;
				HasJob = true;
			}
			else if(CompareNoCase(c, L"-WriteClipboard"))
			{
				UILimit.UIRestrictionsClass |= JOB_OBJECT_UILIMIT_WRITECLIPBOARD;
				HasJob = true;
			}
			else if(CompareNoCase(c, L"-BreakawayOk"))
			{
				BasicLimit.BasicLimitInformation.LimitFlags |= JOB_OBJECT_LIMIT_BREAKAWAY_OK;
				HasJob = true;
			}
			else if(CompareNoCase(c, L"-BreakawayAlways"))
			{
				BasicLimit.BasicLimitInformation.LimitFlags |= JOB_OBJECT_LIMIT_SILENT_BREAKAWAY_OK;
				HasJob = true;
			}
			else if(CompareNoCase(c, L"-DieOnUnException"))
			{
				BasicLimit.BasicLimitInformation.LimitFlags |= JOB_OBJECT_LIMIT_DIE_ON_UNHANDLED_EXCEPTION;
				HasJob = true;
			}
			else if(CompareNoCase(c, L"-NoAdmin"))
			{
				SecLimit.SecurityLimitFlags |= JOB_OBJECT_SECURITY_NO_ADMIN;
				HasJob = true;
			}
			else if(CompareNoCase(c, L"-Restricted"))
			{
				SecLimit.SecurityLimitFlags |= JOB_OBJECT_SECURITY_RESTRICTED_TOKEN;
				HasJob = true;
			}
			else if(CompareNoCase(c, L"-Breakaway"))
			{
				ProcessFlags |= CREATE_BREAKAWAY_FROM_JOB;
				HasJob = true;
			}
			else if(CompareNoCase(c, L"-PrcLimit"))
			{
				BasicLimit.BasicLimitInformation.LimitFlags |= JOB_OBJECT_LIMIT_ACTIVE_PROCESS;
				BasicLimit.BasicLimitInformation.ActiveProcessLimit = String2DWORD(GetNext(argc, argv, i));
				HasJob = true;
			}
			else if(CompareNoCase(c, L"-JobTime"))
			{
				BasicLimit.BasicLimitInformation.LimitFlags |= JOB_OBJECT_LIMIT_JOB_TIME;
				BasicLimit.BasicLimitInformation.PerJobUserTimeLimit = String2LINT(GetNext(argc, argv, i));
				HasJob = true;
			}
			else if(CompareNoCase(c, L"-PrcTime"))
			{
				BasicLimit.BasicLimitInformation.LimitFlags |= JOB_OBJECT_LIMIT_PROCESS_TIME;
				BasicLimit.BasicLimitInformation.PerProcessUserTimeLimit = String2LINT(GetNext(argc, argv, i));
				HasJob = true;
			}
			else if(CompareNoCase(c, L"-JobMem"))
			{
				BasicLimit.BasicLimitInformation.LimitFlags |= JOB_OBJECT_LIMIT_JOB_MEMORY;
				BasicLimit.JobMemoryLimit = String2DWORD(GetNext(argc, argv, i));
				HasJob = true;
			}
			else if(CompareNoCase(c, L"-PrcMem"))
			{
				BasicLimit.BasicLimitInformation.LimitFlags |= JOB_OBJECT_LIMIT_PROCESS_MEMORY;
				BasicLimit.ProcessMemoryLimit = String2DWORD(GetNext(argc, argv, i));
				HasJob = true;
			}
			else if(CompareNoCase(c, L"-Sched"))
			{
				BasicLimit.BasicLimitInformation.LimitFlags |= JOB_OBJECT_LIMIT_SCHEDULING_CLASS;
				BasicLimit.BasicLimitInformation.SchedulingClass = String2DWORD(GetNext(argc, argv, i));
				HasJob = true;
			}
			else if(CompareNoCase(c, L"-WorkingSet"))
			{
				BasicLimit.BasicLimitInformation.LimitFlags |= JOB_OBJECT_LIMIT_WORKINGSET;
				BasicLimit.BasicLimitInformation.MinimumWorkingSetSize = String2DWORD(GetNext(argc, argv, i));
				BasicLimit.BasicLimitInformation.MaximumWorkingSetSize = String2DWORD(GetNext(argc, argv, i));
				HasJob = true;
			}
			else if (CompareNoCase(c, L"-RSid"))
			{
				AUTO_SID_AND_ATTRIBUTES SidAndAttributes;
				SidAndAttributes.Sid = LookupSidByName(GetNext(argc, argv, i));
				SidAndAttributes.Attributes = 0;
				rsidlist.push_back(SidAndAttributes);
				HasToken = true;
			}
			else if (CompareNoCase(c, L"-RSidLogon"))
			{
				AUTO_SID_AND_ATTRIBUTES SidAndAttributes;
				SidAndAttributes.Sid = GetLogonSid();
				SidAndAttributes.Attributes = 0;
				rsidlist.push_back(SidAndAttributes);
				HasToken = true;
			}
			else if (CompareNoCase(c, L"-RSidUser"))
			{
				AUTO_SID_AND_ATTRIBUTES SidAndAttributes;
				SidAndAttributes.Sid = GetUserSid();
				SidAndAttributes.Attributes = 0;
				rsidlist.push_back(SidAndAttributes);
				HasToken = true;
			}
			else if (CompareNoCase(c, L"-DSid"))
			{
				AUTO_SID_AND_ATTRIBUTES SidAndAttributes;
				SidAndAttributes.Sid = LookupSidByName(GetNext(argc, argv, i));
				SidAndAttributes.Attributes = 0;
				dsidlist.push_back(SidAndAttributes);
				HasToken = true;
			}
			else if (CompareNoCase(c, L"-DSidLogon"))
			{
				AUTO_SID_AND_ATTRIBUTES SidAndAttributes;
				SidAndAttributes.Sid = GetLogonSid();
				SidAndAttributes.Attributes = 0;
				dsidlist.push_back(SidAndAttributes);
				HasToken = true;
			}
			else if (CompareNoCase(c, L"-DSidUser"))
			{
				AUTO_SID_AND_ATTRIBUTES SidAndAttributes;
				SidAndAttributes.Sid = GetUserSid();
				SidAndAttributes.Attributes = 0;
				dsidlist.push_back(SidAndAttributes);
				HasToken = true;
			}
			else if (CompareNoCase(c, L"-DPriv"))
			{
				HasToken = true;
				LUID_AND_ATTRIBUTES nl;
				nl.Attributes = 0;
				HandleError(LookupPrivilegeValue(NULL, GetNext(argc, argv, i).c_str(), &nl.Luid), L"looking up a privilege");
				dprivs.push_back(nl);
			}
			else if (CompareNoCase(c, L"-DPrivMax"))
			{
				HasToken = true;
				TokenFlags |= DISABLE_MAX_PRIVILEGE;
			}
			else if(CompareNoCase(c, L"-JobToken"))
			{
				HasToken = true;
				HasJob = true;
			}
			else if(CompareNoCase(c, L"-UseDesktop"))
			{
				desktopname = GetNext(argc, argv, i);
				OpenDesktopOnly = false;
				HasDesktop = true;
			}
			else if(CompareNoCase(c, L"-UseOpenDesktop"))
			{
				desktopname = GetNext(argc, argv, i);
				OpenDesktopOnly = true;
				HasDesktop = true;
			}
			else if(CompareNoCase(c, L"-SwitchDesktop"))
			{
				HasSwitchDesktop = true;
			}
			else if(CompareNoCase(c, L"-Name"))
			{
				jobname = GetNext(argc, argv, i);
				DupJob = true;
				HasJob = true;
			}
			else if(CompareNoCase(c, L"-NoDupJob"))
				DupJob = false;
			else if(CompareNoCase(c, L"-DupJob"))
				DupJob = true;
			else if(CompareNoCase(c, L"-Parent"))
				parents = String2DWORD(GetNext(argc, argv, i));
			else if(CompareNoCase(c, L"-ProcessId"))
				procidlist.push_back(String2DWORD(GetNext(argc, argv, i)));
			else if(CompareNoCase(c, L"-Reset"))
			{
				BasicLimit.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_WORKINGSET |
					JOB_OBJECT_LIMIT_PROCESS_TIME |
					JOB_OBJECT_LIMIT_JOB_TIME |
					JOB_OBJECT_LIMIT_ACTIVE_PROCESS |
					JOB_OBJECT_LIMIT_AFFINITY |
					JOB_OBJECT_LIMIT_PRIORITY_CLASS |
					JOB_OBJECT_LIMIT_PRESERVE_JOB_TIME |
					JOB_OBJECT_LIMIT_SCHEDULING_CLASS |
					JOB_OBJECT_LIMIT_PROCESS_MEMORY |
					JOB_OBJECT_LIMIT_JOB_MEMORY;
			}
			else if(CompareNoCase(c, L"-AllowCursor"))
				AllowCursor = true;
			else //otherwise, assume it's an executable
				proclist.push_back(c);
		}
	}
	void CreateProcesses()
	{
		if(HasDesktop)
		{
			if(OpenDesktopOnly)
				hDesktop = OpenDesktop(desktopname.c_str(), 0, FALSE, MAXIMUM_ALLOWED);
			else
				hDesktop = CreateDesktop(desktopname.c_str(), NULL, NULL, 0, MAXIMUM_ALLOWED, NULL);
			HandleError(hDesktop != 0, L"creating/opening a desktop");

			if(HasSwitchDesktop)
				HandleError(SwitchDesktop(hDesktop), L"switching to a desktop");
		}
		if(HasToken)
		{
			HANDLE CurrentToken;
			HandleError(OpenProcessToken(GetCurrentProcess(), TOKEN_ALL_ACCESS, &CurrentToken), L"opening the current token");
			SID_AND_ATTRIBUTES *DSids, *RSids;
			LUID_AND_ATTRIBUTES *DPrivs;
			DSids = dsidlist.size() == 0 ? NULL : reinterpret_cast<SID_AND_ATTRIBUTES *>(&dsidlist[0]);
			RSids = rsidlist.size() == 0 ? NULL : reinterpret_cast<SID_AND_ATTRIBUTES *>(&rsidlist[0]);
			DPrivs = dprivs.size() == 0 ? NULL : &dprivs[0];
			HandleError(CreateRestrictedToken(CurrentToken, TokenFlags, (DWORD)dsidlist.size(), DSids, (DWORD)dprivs.size(), DPrivs, (DWORD)rsidlist.size(), RSids, &hToken), L"creating a restricted token");
			SecLimit.JobToken = hToken;
			SecLimit.SecurityLimitFlags |= JOB_OBJECT_SECURITY_ONLY_TOKEN;
		}
		if(HasJob)
		{
			hJob = CreateJobObject(NULL, jobname.c_str());
			HandleError(hJob != 0, L"creating/opening the job");	

			if(UILimit.UIRestrictionsClass) //only set limits if limits specified
				HandleError(SetInformationJobObject(hJob, JobObjectBasicUIRestrictions, &UILimit, sizeof(JOBOBJECT_BASIC_UI_RESTRICTIONS)), L"setting UI restrictions");

			if(BasicLimit.BasicLimitInformation.LimitFlags)
				HandleError(SetInformationJobObject(hJob, JobObjectExtendedLimitInformation, &BasicLimit, sizeof(JOBOBJECT_EXTENDED_LIMIT_INFORMATION)), L"setting basic restrictions");
			
			if(SecLimit.SecurityLimitFlags)
				HandleError(SetInformationJobObject(hJob, JobObjectSecurityLimitInformation, &SecLimit, sizeof(JOBOBJECT_SECURITY_LIMIT_INFORMATION)), L"setting security restrictions");
			
			if(AllowCursor)
			{
				CURSORINFO ci;
				ZeroMemory(&ci, sizeof ci);
				ci.cbSize = sizeof ci;
				HandleError(GetCursorInfo(&ci), L"getting cursor information");
				HandleError(UserHandleGrantAccess(ci.hCursor, hJob, TRUE), L"granting cursor access");
			}
		}

		PushParentProcesses();

		for(vector<DWORD>::iterator it(procidlist.begin()); it != procidlist.end();)
			OpenAddProcessToJob(*it++);

		for(vector<wstring>::iterator it(proclist.begin()); it != proclist.end();)
			CreateProcessWithJob(*it++);
	}
	int main(int argc, wchar_t *argv[])
	{
		ParseArgs(argc, argv);
		CreateProcesses();
		return 0;
	}
};

int __cdecl wmain(int argc, wchar_t *argv[])
{
	ulimitnt ulnt;
	return ulnt.main(argc, argv);
}