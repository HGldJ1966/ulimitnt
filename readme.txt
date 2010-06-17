ulimitnt: a program for starting processes in a new NT job object.
Copyright (c) 2010 Christopher Smith

http://code.google.com/p/ulimitnt/
mailto:csmith32@gmail.com

Version 2.7

Files that should be in the package:

ulimitnt.exe	The program binary
ulimitnt.cpp	The source code
readme.txt	This file
copying		The licence

Licence:
--------
This program is licened under the GNU General Public Licence, which
should be included in the package as COPYING. See the file for
further details.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; version 2 of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

Overview:
---------
This program creates or opens processes, and associates them with a job object and/or restricted token.

Job objects are a feature of Windows 2000 (and later) that are used to enforce quotas. A job has a set of processes associated with it and a set of limitations. These limitations are applied to all the processes in the job. Once a process is in a job, it cannot leave. A process can belong to at most one job. Normally, any child processes created will be associated with the parent's job. (See options below)

Job limits include UI restrictions, such as reading/writing to the clipboard, memory limits, secuirty token limits, process count limits, and total CPU time limits. They can be used to provide extra security and DoS resistance to your computer.

Restricted tokens provide a means to give a new process less access than its parent process, but without involving the account of another user.

Almost all of ulimitnt's functions require no special privileges to operate since they represent only a reduction in available privileges and resources for processes.

As usual, Microsoft puts nice features into the base system and then doesn't provide a good way to take advantage of them; ulimitnt provides a convenient command line interface to these features.

Requirements:
-------------
This program requires Microsoft Windows 2000 (NT 5.0) or later, as this is the first version that supports both jobs and restricted tokens.

Standard usage summary:
--------------
ulimitnt [-option <parameters>...] executable [executable ...]

Options are not case sensitive
-UseDesktop		Use the named [window station\]desktop
-UseOpenDesktop		Use the existing named desktop
-SwitchDesktop		Switch to the previously named desktop (don't create)
-LimitDesktop 		Prevents desktop switching
-DisplaySettings	Prevents display changes
-ExitWindows		Prevents shutdown/logoff
-GlobalAtoms		Forces a private atom table
-Handles		Prevents access of USER handles outside the job
-ReadClipboard		Prevents reading of the clipboard
-WriteClipboard		Prevents writing of the clipboard
-SystemParameters	Prevents changing of USER parameters
-BreakawayOk		Child processes can break away from the job upon parent request
-BreakawaySilent	Child processes always break away from the job
-Breakaway		Request that new processes be outside the pre-existing job
-DieOnUnException	Processes will die on unhandled exception (no debugging)
-NoAdmin		Processes cannot use any tokens that include local admins
-Restricted		Processes cannot use tokens that aren't restricted
-PrcLimit num		Maximum number of processes that can be in the job
-JobTime num		User mode time limit (in 100ns units) for entire job
-PrcTime num		User mode time limit (in 100ns units) for individual processes
-JobMem num		Maximum memory (in bytes) the entire job can commit
-PrcMem num		Maximum memory (in bytes) any process can commit
-Sched num		Scheduling class, from 0 to 9 where 9 is highest
-WorkingSet min, max 	A fixed working set for each process, in bytes
-DSid                   Makes the specified SID deny-only.
-RSid                   Makes the job/processes restricted to these SIDs only
-[D|R]SidLogon		Adds the logon sid to the restricted list.
-[D|R]SidUser		Disables or restrictes the user SID
-DPriv                  Deletes the named privelege.
-DPrivMax               Deletes all priveleges.
-DupJob			Gives new processes a no-access handle to job to keep job open
-NoDupJob		Prevents -DupJob. Use with -Name
-Name text		Gives the job a name. Implies -DupJob
-Reset			Causes reset of existing job's limits. Use with -Name
-Parent num		Adds the parent process of ulimitnt num levels up to the job
-ProcessID num		Adds the process with the specified ID to the job

Later options override earlier options, where a conflict would exist

Examples:

 ulimitnt iexplore -prclimit 32 -readclipboard -writeclipboard -handles

Starts Internet Explorer in a job with a 32 process limit, unable to read or write the clipboard and prevented from accessing windows outside the job.

 ulimitnt -prctime 600000000 -prclimit 1 notepad

Starts notepad, limited to a maximum of 60 seconds of CPU time and unable to start any child processes.

 ulimitnt notepad -handles -atoms -dsid administrators -dprivmax

Starts notepad without access to other windows, prevented from accessing global atoms, with no privileges and without the access the administrator's group brings (if it was previously considered).

 ulimitnt -usedesktop newdesk -switchdesktop cmd

Creates a new desktop named "newdesk", switches to it and starts a command prompt in it.

 ulimitnt -usedesktop default -switchdesktop

Switches back to the default desktop (created when the user logged on).

Option notes:
-------------
Options are case insensitive.

The order of options and executables is meaningless, except that later options can override earlier ones.

All time values are expressed in kernel time format: an int64 in 100ns units. In other words, the number of seconds you want times 10000000.

Desktop objects seperate windows into visible display surfaces. A process is normally associated with one desktop and can't access windows on other desktops.
-Usedesktop will use the named desktop when creating the processes.
-SwitchDesktop will make the named desktop visible.
Winlogon automatically creates a desktop called "Default" that is visible by default.

-LimitDesktop is a job limit that prevents processes in the job from creating new desktop objects and from switching the current desktop.

-DisplaySettings prevents the job's processes from changing the current resolution/color depth.

-SystemParameters prevents changes to display metrics like the font and size used for title bars.

-Handles prevents processes in the job from getting handles to USER objects (like windows) from processes outside of the job. This limit is very effective in preventing shatter attacks from unprivileged processes by preventing them from getting window handles to send malicious messages to. The only handles available are for those processes already in the job, which wouldn't be too useful to compromise.

-BreakawayOk allows child processes to exist outside of the job if the parent specifies the CREATE_BREAKAWAY_FROM_JOB flag in the call to CreateProcess.

-BreakawaySilent causes all children of the associated processes to be outside of the job.

-Breakaway will cause ulimitnt to request that the new processes break away from the old job that ulimitnt is running in. The job that ulimitnt currently runs in must have the -BreakawayOk option. The children will be created in a new job. This option is moot if ulimitnt is not already in a job.

-PrcLimit sets the maximum number of active processes that can exist inside of a job at one time. Useful for avoiding DoS by forkbomb.

-DupJob gives the child processes a handle to the job object with no access. This is done to prevent the job object from closing; when all handles to the job close, the object is closed. When the job object is closed, the limits are still enforced but the name is destroyed and the job cannot be re-opened. -DupJob is implied by -Name to preserve the job's name. This way, the job's name will survive as long as the processes directly created by ulimitnt survive.

-NoDupJob causes the job handle to not be duplicated to child processes. This is default without specifying -Name or -DupJob. Using this after -Name will make -Name ineffective because later options override earlier ones.

-Name gives the job object a name. Use this option to create a named job; use this name later to reopen a previously created job to modify its properties oradd more processes to the job. Implies -DupJob to keep the object open and name alive.
Note that changing any UILimit overrides the entire UILimit state. Not all job properties can be modified after the job is created.

-Reset resets all the variable "basic" limits.

-DSid makes SIDs from the parent process deny-only. The identity provided by these SIDs can't be used to gain access to anything-- only to deny access. Note that only the named SID is looked up, not any groups that the SID the account/group represents would normally belong to.

-RSid starts or adds to a list of restricted SIDs for the new processes. Before such a restricted process can access anything, the normal list of SIDs must grant access, AND the restricted list of SIDs must also be able to independently grant access on an object. Restricted SIDs make access the intersection of normal access and this list of restricted SIDs. The SID "RESTRICTED" should be in this list is most cases, esp. for GUI applications. Restricted SIDs can be rendered ineffective if the process also belongs to high-privilege groups such as Administrator. Use with -DSid on such groups.
See restricted tokens in the See Also section.

-[D|R]SidLogon disables or adds the logon SID to the list of restricted SIDs, repsectively. A unique logon SID is assigned to each logon session. It has access to the session's desktop and window station.

-[D|R]SidUser disables or adds the user's own SID to the list of restricted SIDs, repsectively. This is the primary SID of the user's identity. For example, if you are logged on as Bob, then Bob is the user sid.

-DPrivMax deletes all privileges, except for SeChangeNotifyPrivilege, which isn't a security risk (except in rare configurations) to have.

-Parent num adds every process num levels up the process hierarchy to the job. For example, if process A created B which created C which started ulimitnt, then -Parent 1 adds process C and -Parent 2 adds both processes C and B. Note that every process up the chain must still exist; if process B had exited before ulimitnt was executed, -Parent 3 will cause ulimitnt fail instead of adding A. This option can only be used once per use of ulimitnt.

Known Issues:
-------------
-NoAdmin doesn't seem to work correctly if you launch from an admin process.
-DupJob is ineffective when all the processes created directly by ulimitnt for that job have terminated.

Changelog:
----------
1.0	Initial release

1.1	Added -BreakawayNoJob, -DupJob, -NoDupJob, -Reset, improved -Name

2.0	Added -DSid, -RSid, -DPriv, -DPrivMax, documented -NoJob

2.1	Added -RSidLogon

2.2	Added -RSidUser, -DSidUser, -DSidLogon, -JobToken, source overhaul, removed -NoJob

2.3	Added -UseDesktop, -UseOpenDesktop, -SwitchDesktop

2.5	Fixed many internal issues, renamed to ulimitnt

Known privileges:
-----------------
SeAssignPrimaryTokenPrivilege	Replace a process-level token.
SeAuditPrivilege		Generate security audits.
SeBackupPrivilege		Back up files and directories.
SeChangeNotifyPrivilege		Bypass traverse checking.
SeCreatePagefilePrivilege	Create a pagefile.
SeCreatePermanentPrivilege	Create permanent shared objects.
SeCreateTokenPrivilege		Create a token object.
SeDebugPrivilege		Debug programs.
SeEnableDelegationPrivilege	Enable accounts to be trusted for delegation.
SeIncreaseBasePriorityPrivilege	Increase scheduling priority.
SeIncreaseQuotaPrivilege	Adjust memory quotas for a process.
SeLoadDriverPrivilege		Load and unload device drivers.
SeLockMemoryPrivilege		Lock pages in memory.
SeMachineAccountPrivilege	Add workstations to domain.
SeManageVolumePrivilege		Manage the files on a volume.
SeProfileSingleProcessPrivilege	Profile single process.
SeRemoteShutdownPrivilege	Force shutdown from a remote system.
SeRestorePrivilege		Restore files and directories.
SeSecurityPrivilege		Manage auditing and security log.
SeShutdownPrivilege		Shut down the system.
SeSyncAgentPrivilege		Synchronize directory service data.
SeSystemEnvironment		Modify firmware environment values.
SeSystemProfilePrivilege	Profile system performance.
SeSystemtimePrivilege		Change the system time.
SeTakeOwnershipPrivilege	Forcibly take ownership of files or other objects.
SeTcbPrivilege			Trusted computing base privileges.
SeUndockPrivilege		Remove computer from docking station.
SeUnsolicitedInputPrivilege	Read unsolicited input from a terminal device.

New in XP SP2, 2000 SP4 and Server 2003:
SeImpersonatePrivilege		Impersonate a client after authentication.
SeCreateGlobalPrivilege		Create global objects.

See also:
-----------

See http://msdn.microsoft.com/library/en-us/dllproc/base/setinformationjobobject.asp
for Microsoft's descripton of job limits, and the functions that ulimitnt uses.

See http://msdn.microsoft.com/library/default.asp?url=/library/en-us/secauthz/security/authorization_constants.asp
for a list of current privilege names and meanings.

See http://msdn.microsoft.com/library/en-us/secauthz/security/restricted_tokens.asp
for a description of restricted tokens (rsid, dsid, dpriv)

See http://msdn.microsoft.com/library/en-us/dllproc/base/desktops.asp
for a description of desktop objects.