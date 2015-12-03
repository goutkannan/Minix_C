/* This file contains the table used to map system call numbers onto the
 * routines that perform them.
 */

#define _TABLE

#include "pm.h"
#include <minix/callnr.h>
#include <signal.h>
#include "mproc.h"

#define CALL(n)	[((n) - PM_BASE)]

int (* const call_vec[NR_PM_CALLS])(void) = {
	CALL(PM_EXIT)		= do_exit,		/* _exit(2) */
	CALL(PM_FORK)		= do_fork,		/* fork(2) */
	CALL(PM_WAIT4)		= do_wait4,		/* wait4(2) */
	CALL(PM_GETPID)		= do_get,		/* get[p]pid(2) */
	CALL(PM_SETUID)		= do_set,		/* setuid(2) */
	CALL(PM_GETUID)		= do_get,		/* get[e]uid(2) */
	CALL(PM_STIME)		= do_stime,		/* stime(2) */
	CALL(PM_PTRACE)		= do_trace,		/* ptrace(2) */
	CALL(PM_SETGROUPS)	= do_set,		/* setgroups(2) */
	CALL(PM_GETGROUPS)	= do_get,		/* getgroups(2) */
	CALL(PM_KILL)		= do_kill,		/* kill(2) */
	CALL(PM_SETGID)		= do_set,		/* setgid(2) */
	CALL(PM_GETGID)		= do_get,		/* get[e]gid(2) */
	CALL(PM_EXEC)		= do_exec,		/* execve(2) */
	CALL(PM_SETSID)		= do_set,		/* setsid(2) */
	CALL(PM_GETPGRP)	= do_get,		/* getpgrp(2) */
	CALL(PM_ITIMER)		= do_itimer,		/* [gs]etitimer(2) */
	CALL(PM_GETMCONTEXT)	= do_getmcontext,	/* getmcontext(2) */
	CALL(PM_SETMCONTEXT)	= do_setmcontext,	/* setmcontext(2) */
	CALL(PM_SIGACTION)	= do_sigaction,		/* sigaction(2) */
	CALL(PM_SIGSUSPEND)	= do_sigsuspend,	/* sigsuspend(2) */
	CALL(PM_SIGPENDING)	= do_sigpending,	/* sigpending(2) */
	CALL(PM_SIGPROCMASK)	= do_sigprocmask,	/* sigprocmask(2) */
	CALL(PM_SIGRETURN)	= do_sigreturn,		/* sigreturn(2) */
	CALL(PM_SYSUNAME)	= do_sysuname,		/* sysuname(2) */
	CALL(PM_GETPRIORITY)	= do_getsetpriority,	/* getpriority(2) */
	CALL(PM_SETPRIORITY)	= do_getsetpriority,	/* setpriority(2) */
	CALL(PM_GETTIMEOFDAY)	= do_time,		/* gettimeofday(2) */
	CALL(PM_SETEUID)	= do_set,		/* geteuid(2) */
	CALL(PM_SETEGID)	= do_set,		/* setegid(2) */
	CALL(PM_ISSETUGID)	= do_get,		/* issetugid */
	CALL(PM_GETSID)		= do_get,		/* getsid(2) */
	CALL(PM_CLOCK_GETRES)	= do_getres,		/* clock_getres(2) */
	CALL(PM_CLOCK_GETTIME)	= do_gettime,		/* clock_gettime(2) */
	CALL(PM_CLOCK_SETTIME)	= do_settime,		/* clock_settime(2) */
	CALL(PM_GETRUSAGE)	= do_getrusage,		/* getrusage(2) */
	CALL(PM_REBOOT)		= do_reboot,		/* reboot(2) */
	CALL(PM_SVRCTL)		= do_svrctl,		/* svrctl(2) */
	CALL(PM_SPROF)		= do_sprofile,		/* sprofile(2) */
	CALL(PM_SRV_FORK)	= do_srv_fork,		/* srv_fork(2) */
	CALL(PM_SRV_KILL)	= do_srv_kill,		/* srv_kill(2) */
	CALL(PM_EXEC_NEW)	= do_newexec,
	CALL(PM_EXEC_RESTART)	= do_execrestart,
	CALL(PM_GETEPINFO)	= do_getepinfo,		/* getepinfo(2) */
	CALL(PM_GETPROCNR)	= do_getprocnr,		/* getprocnr(2) */
	CALL(PM_GETSYSINFO)	= do_getsysinfo,		/* getsysinfo(2) */
	CALL(PM_DEPOSIT)	= do_deposit,			/* deposit message into mailbox*/
	CALL(PM_RETRIEVE)	= do_retrieve,			/* retrieve message into mailbox*/
	CALL(PM_CLEAR)		= do_clear,				/* Clear the messages that are in the mailbox*/
	CALL(PM_SETUP)		= do_setup,				/* admin call to setup mailbox*/
	CALL(PM_CREATE_GROUP)		= create_group, 
	CALL(PM_REMOVE_GROUP)		= remove_group, 
	CALL(PM_ADD_GROUP_LEADER)	= add_group_leader, 
	CALL(PM_REMOVE_GROUP_LEADER)	= remove_group_leader,
	CALL(PM_DENY_SEND)		= deny_send,
	CALL(PM_DENY_RECEIVE)		= deny_receive,
	CALL(PM_CREATE_SECURE_GROUP)	= create_secure_group,
	CALL(PM_CREATE_PUBLIC_GROUP)	= create_public_group,
	CALL(PM_ADD_USER_TO_GROUP)	= add_user_to_group,
	CALL(PM_REMOVE_USER_FROM_GROUP)	= remove_user_from_group
};
