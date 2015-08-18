<pre>
ulimitnt: a program for starting processes in a new NT job object.<br>
Copyright (c) 2010 Christopher Smith<br>
<br>
http://code.google.com/p/ulimitnt/<br>
mailto:csmith32@gmail.com<br>
<br>
Version 2.7<br>
<br>
Files that should be in the package:<br>
<br>
ulimitnt.exe	The program binary<br>
ulimitnt.cpp	The source code<br>
readme.txt	This file<br>
copying		The licence<br>
<br>
Licence:<br>
--------<br>
This program is licened under the GNU General Public Licence, which<br>
should be included in the package as COPYING. See the file for<br>
further details.<br>
<br>
This program is free software; you can redistribute it and/or modify<br>
it under the terms of the GNU General Public License as published by<br>
the Free Software Foundation; version 2 of the License.<br>
<br>
This program is distributed in the hope that it will be useful,<br>
but WITHOUT ANY WARRANTY; without even the implied warranty of<br>
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the<br>
GNU General Public License for more details.<br>
<br>
You should have received a copy of the GNU General Public License<br>
along with this program; if not, write to the Free Software<br>
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA<br>
<br>
Overview:<br>
---------<br>
This program creates or opens processes, and associates them with a job object and/or restricted token.<br>
<br>
Job objects are a feature of Windows 2000 (and later) that are used to enforce quotas. A job has a set of processes associated with it and a set of limitations. These limitations are applied to all the processes in the job. Once a process is in a job, it cannot leave. A process can belong to at most one job. Normally, any child processes created will be associated with the parent's job. (See options below)<br>
<br>
Job limits include UI restrictions, such as reading/writing to the clipboard, memory limits, secuirty token limits, process count limits, and total CPU time limits. They can be used to provide extra security and DoS resistance to your computer.<br>
<br>
Restricted tokens provide a means to give a new process less access than its parent process, but without involving the account of another user.<br>
<br>
Almost all of ulimitnt's functions require no special privileges to operate since they represent only a reduction in available privileges and resources for processes.<br>
<br>
As usual, Microsoft puts nice features into the base system and then doesn't provide a good way to take advantage of them; ulimitnt provides a convenient command line interface to these features.<br>
<br>
Requirements:<br>
-------------<br>
This program requires Microsoft Windows 2000 (NT 5.0) or later, as this is the first version that supports both jobs and restricted tokens.<br>
<br>
Standard usage summary:<br>
--------------<br>
ulimitnt [-option <parameters>...] executable [executable ...]<br>
<br>
Options are not case sensitive<br>
-UseDesktop		Use the named [window station\]desktop<br>
-UseOpenDesktop		Use the existing named desktop<br>
-SwitchDesktop		Switch to the previously named desktop (don't create)<br>
-LimitDesktop 		Prevents desktop switching<br>
-DisplaySettings	Prevents display changes<br>
-ExitWindows		Prevents shutdown/logoff<br>
-GlobalAtoms		Forces a private atom table<br>
-Handles		Prevents access of USER handles outside the job<br>
-ReadClipboard		Prevents reading of the clipboard<br>
-WriteClipboard		Prevents writing of the clipboard<br>
-SystemParameters	Prevents changing of USER parameters<br>
-BreakawayOk		Child processes can break away from the job upon parent request<br>
-BreakawaySilent	Child processes always break away from the job<br>
-Breakaway		Request that new processes be outside the pre-existing job<br>
-DieOnUnException	Processes will die on unhandled exception (no debugging)<br>
-NoAdmin		Processes cannot use any tokens that include local admins<br>
-Restricted		Processes cannot use tokens that aren't restricted<br>
-PrcLimit num		Maximum number of processes that can be in the job<br>
-JobTime num		User mode time limit (in 100ns units) for entire job<br>
-PrcTime num		User mode time limit (in 100ns units) for individual processes<br>
-JobMem num		Maximum memory (in bytes) the entire job can commit<br>
-PrcMem num		Maximum memory (in bytes) any process can commit<br>
-Sched num		Scheduling class, from 0 to 9 where 9 is highest<br>
-WorkingSet min, max 	A fixed working set for each process, in bytes<br>
-DSid                   Makes the specified SID deny-only.<br>
-RSid                   Makes the job/processes restricted to these SIDs only<br>
-[D|R]SidLogon		Adds the logon sid to the restricted list.<br>
-[D|R]SidUser		Disables or restrictes the user SID<br>
-DPriv                  Deletes the named privelege.<br>
-DPrivMax               Deletes all priveleges.<br>
-DupJob			Gives new processes a no-access handle to job to keep job open<br>
-NoDupJob		Prevents -DupJob. Use with -Name<br>
-Name text		Gives the job a name. Implies -DupJob<br>
-Reset			Causes reset of existing job's limits. Use with -Name<br>
-Parent num		Adds the parent process of ulimitnt num levels up to the job<br>
-ProcessID num		Adds the process with the specified ID to the job<br>
<br>
Later options override earlier options, where a conflict would exist<br>
<br>
Examples:<br>
<br>
ulimitnt iexplore -prclimit 32 -readclipboard -writeclipboard -handles<br>
<br>
Starts Internet Explorer in a job with a 32 process limit, unable to read or write the clipboard and prevented from accessing windows outside the job.<br>
<br>
ulimitnt -prctime 600000000 -prclimit 1 notepad<br>
<br>
Starts notepad, limited to a maximum of 60 seconds of CPU time and unable to start any child processes.<br>
<br>
ulimitnt notepad -handles -atoms -dsid administrators -dprivmax<br>
<br>
Starts notepad without access to other windows, prevented from accessing global atoms, with no privileges and without the access the administrator's group brings (if it was previously considered).<br>
<br>
ulimitnt -usedesktop newdesk -switchdesktop cmd<br>
<br>
Creates a new desktop named "newdesk", switches to it and starts a command prompt in it.<br>
<br>
ulimitnt -usedesktop default -switchdesktop<br>
<br>
Switches back to the default desktop (created when the user logged on).<br>
<br>
Option notes:<br>
-------------<br>
Options are case insensitive.<br>
<br>
The order of options and executables is meaningless, except that later options can override earlier ones.<br>
<br>
All time values are expressed in kernel time format: an int64 in 100ns units. In other words, the number of seconds you want times 10000000.<br>
<br>
Desktop objects seperate windows into visible display surfaces. A process is normally associated with one desktop and can't access windows on other desktops.<br>
-Usedesktop will use the named desktop when creating the processes.<br>
-SwitchDesktop will make the named desktop visible.<br>
Winlogon automatically creates a desktop called "Default" that is visible by default.<br>
<br>
-LimitDesktop is a job limit that prevents processes in the job from creating new desktop objects and from switching the current desktop.<br>
<br>
-DisplaySettings prevents the job's processes from changing the current resolution/color depth.<br>
<br>
-SystemParameters prevents changes to display metrics like the font and size used for title bars.<br>
<br>
-Handles prevents processes in the job from getting handles to USER objects (like windows) from processes outside of the job. This limit is very effective in preventing shatter attacks from unprivileged processes by preventing them from getting window handles to send malicious messages to. The only handles available are for those processes already in the job, which wouldn't be too useful to compromise.<br>
<br>
-BreakawayOk allows child processes to exist outside of the job if the parent specifies the CREATE_BREAKAWAY_FROM_JOB flag in the call to CreateProcess.<br>
<br>
-BreakawaySilent causes all children of the associated processes to be outside of the job.<br>
<br>
-Breakaway will cause ulimitnt to request that the new processes break away from the old job that ulimitnt is running in. The job that ulimitnt currently runs in must have the -BreakawayOk option. The children will be created in a new job. This option is moot if ulimitnt is not already in a job.<br>
<br>
-PrcLimit sets the maximum number of active processes that can exist inside of a job at one time. Useful for avoiding DoS by forkbomb.<br>
<br>
-DupJob gives the child processes a handle to the job object with no access. This is done to prevent the job object from closing; when all handles to the job close, the object is closed. When the job object is closed, the limits are still enforced but the name is destroyed and the job cannot be re-opened. -DupJob is implied by -Name to preserve the job's name. This way, the job's name will survive as long as the processes directly created by ulimitnt survive.<br>
<br>
-NoDupJob causes the job handle to not be duplicated to child processes. This is default without specifying -Name or -DupJob. Using this after -Name will make -Name ineffective because later options override earlier ones.<br>
<br>
-Name gives the job object a name. Use this option to create a named job; use this name later to reopen a previously created job to modify its properties oradd more processes to the job. Implies -DupJob to keep the object open and name alive.<br>
Note that changing any UILimit overrides the entire UILimit state. Not all job properties can be modified after the job is created.<br>
<br>
-Reset resets all the variable "basic" limits.<br>
<br>
-DSid makes SIDs from the parent process deny-only. The identity provided by these SIDs can't be used to gain access to anything-- only to deny access. Note that only the named SID is looked up, not any groups that the SID the account/group represents would normally belong to.<br>
<br>
-RSid starts or adds to a list of restricted SIDs for the new processes. Before such a restricted process can access anything, the normal list of SIDs must grant access, AND the restricted list of SIDs must also be able to independently grant access on an object. Restricted SIDs make access the intersection of normal access and this list of restricted SIDs. The SID "RESTRICTED" should be in this list is most cases, esp. for GUI applications. Restricted SIDs can be rendered ineffective if the process also belongs to high-privilege groups such as Administrator. Use with -DSid on such groups.<br>
See restricted tokens in the See Also section.<br>
<br>
-[D|R]SidLogon disables or adds the logon SID to the list of restricted SIDs, repsectively. A unique logon SID is assigned to each logon session. It has access to the session's desktop and window station.<br>
<br>
-[D|R]SidUser disables or adds the user's own SID to the list of restricted SIDs, repsectively. This is the primary SID of the user's identity. For example, if you are logged on as Bob, then Bob is the user sid.<br>
<br>
-DPrivMax deletes all privileges, except for SeChangeNotifyPrivilege, which isn't a security risk (except in rare configurations) to have.<br>
<br>
-Parent num adds every process num levels up the process hierarchy to the job. For example, if process A created B which created C which started ulimitnt, then -Parent 1 adds process C and -Parent 2 adds both processes C and B. Note that every process up the chain must still exist; if process B had exited before ulimitnt was executed, -Parent 3 will cause ulimitnt fail instead of adding A. This option can only be used once per use of ulimitnt.<br>
<br>
Known Issues:<br>
-------------<br>
-NoAdmin doesn't seem to work correctly if you launch from an admin process.<br>
-DupJob is ineffective when all the processes created directly by ulimitnt for that job have terminated.<br>
<br>
Changelog:<br>
----------<br>
1.0	Initial release<br>
<br>
1.1	Added -BreakawayNoJob, -DupJob, -NoDupJob, -Reset, improved -Name<br>
<br>
2.0	Added -DSid, -RSid, -DPriv, -DPrivMax, documented -NoJob<br>
<br>
2.1	Added -RSidLogon<br>
<br>
2.2	Added -RSidUser, -DSidUser, -DSidLogon, -JobToken, source overhaul, removed -NoJob<br>
<br>
2.3	Added -UseDesktop, -UseOpenDesktop, -SwitchDesktop<br>
<br>
2.5	Fixed many internal issues, renamed to ulimitnt<br>
<br>
Known privileges:<br>
-----------------<br>
SeAssignPrimaryTokenPrivilege	Replace a process-level token.<br>
SeAuditPrivilege		Generate security audits.<br>
SeBackupPrivilege		Back up files and directories.<br>
SeChangeNotifyPrivilege		Bypass traverse checking.<br>
SeCreatePagefilePrivilege	Create a pagefile.<br>
SeCreatePermanentPrivilege	Create permanent shared objects.<br>
SeCreateTokenPrivilege		Create a token object.<br>
SeDebugPrivilege		Debug programs.<br>
SeEnableDelegationPrivilege	Enable accounts to be trusted for delegation.<br>
SeIncreaseBasePriorityPrivilege	Increase scheduling priority.<br>
SeIncreaseQuotaPrivilege	Adjust memory quotas for a process.<br>
SeLoadDriverPrivilege		Load and unload device drivers.<br>
SeLockMemoryPrivilege		Lock pages in memory.<br>
SeMachineAccountPrivilege	Add workstations to domain.<br>
SeManageVolumePrivilege		Manage the files on a volume.<br>
SeProfileSingleProcessPrivilege	Profile single process.<br>
SeRemoteShutdownPrivilege	Force shutdown from a remote system.<br>
SeRestorePrivilege		Restore files and directories.<br>
SeSecurityPrivilege		Manage auditing and security log.<br>
SeShutdownPrivilege		Shut down the system.<br>
SeSyncAgentPrivilege		Synchronize directory service data.<br>
SeSystemEnvironment		Modify firmware environment values.<br>
SeSystemProfilePrivilege	Profile system performance.<br>
SeSystemtimePrivilege		Change the system time.<br>
SeTakeOwnershipPrivilege	Forcibly take ownership of files or other objects.<br>
SeTcbPrivilege			Trusted computing base privileges.<br>
SeUndockPrivilege		Remove computer from docking station.<br>
SeUnsolicitedInputPrivilege	Read unsolicited input from a terminal device.<br>
<br>
New in XP SP2, 2000 SP4 and Server 2003:<br>
SeImpersonatePrivilege		Impersonate a client after authentication.<br>
SeCreateGlobalPrivilege		Create global objects.<br>
<br>
See also:<br>
-----------<br>
<br>
See http://msdn.microsoft.com/library/en-us/dllproc/base/setinformationjobobject.asp<br>
for Microsoft's descripton of job limits, and the functions that ulimitnt uses.<br>
<br>
See http://msdn.microsoft.com/library/default.asp?url=/library/en-us/secauthz/security/authorization_constants.asp<br>
for a list of current privilege names and meanings.<br>
<br>
See http://msdn.microsoft.com/library/en-us/secauthz/security/restricted_tokens.asp<br>
for a description of restricted tokens (rsid, dsid, dpriv)<br>
<br>
See http://msdn.microsoft.com/library/en-us/dllproc/base/desktops.asp<br>
for a description of desktop objects.<br>
</pre>