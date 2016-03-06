/* Miscellaneous system calls.				Author: Kees J. Bot
 *								31 Mar 2000
 * The entry points into this file are:
 *   do_reboot: kill all processes, then reboot system
 *   do_getsysinfo: request copy of PM data structure  (Jorrit N. Herder)
 *   do_getprocnr: lookup endpoint by process ID
 *   do_getepinfo: get the pid/uid/gid of a process given its endpoint
 *   do_getsetpriority: get/set process priority
 *   do_svrctl: process manager control
 *   do_getrusage: obtain process resource usage information
 */

#include "pm.h"
#include <minix/callnr.h>
#include <signal.h>
#include <sys/svrctl.h>
#include <sys/reboot.h>
#include <sys/resource.h>
#include <sys/utsname.h>
#include <minix/com.h>
#include <minix/config.h>
#include <minix/sysinfo.h>
#include <minix/type.h>
#include <minix/ds.h>
#include <machine/archtypes.h>
#include <lib.h>
#include <assert.h>
#include "mproc.h"
#include "kernel/proc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
struct utsname uts_val = {
  OS_NAME,		/* system name */
  "noname",		/* node/network name */
  OS_RELEASE,		/* O.S. release (e.g. 3.3.0) */
  OS_VERSION,		/* O.S. version (e.g. Minix 3.3.0 (GENERIC)) */
#if defined(__i386__)
  "i386",		/* machine (cpu) type */
  "i386",		/* architecture */
#elif defined(__arm__)
  "arm",		/* machine (cpu) type */
  "arm",		/* architecture */
#else
#error			/* oops, no 'uname -mk' */
#endif
};

/*==== MailBox Declaration =====*/
struct mail
{
	int group_id;
	char secure_mailbox[16][30];
	char public_mailbox[16][30];
	int secure_ids[16];
	int public_ids[16];
	int members[16][2];
	int secure_flag, public_flag;
	int subscribe_ids[16];
	}mailbox[16];

int init_mailbox()
{
int j,i = 0;
	while (i<16)
	{
		mailbox[i].group_id = -1;
	   for(j=0;j<16;j++)
		   mailbox[i].secure_ids[j]=0;
		   mailbox[i].public_ids[j]=0;
		   mailbox[i].subscribe_ids[j]=-1;
		i++;
	}
	return (1);
}
int init_mailbox_flag, global_deny_send[30]={-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1} ;
char m[16][30] ;
int counter =0, rec[16], global_deny_rec[30]={-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1};
static int prev;
/*==== MailBox =================*/


static char *uts_tbl[] = {
  uts_val.arch,
  NULL,			/* No kernel architecture */
  uts_val.machine,
  NULL,			/* No hostname */
  uts_val.nodename,
  uts_val.release,
  uts_val.version,
  uts_val.sysname,
  NULL,			/* No bus */			/* No bus */
};

#if ENABLE_SYSCALL_STATS
unsigned long calls_stats[NR_PM_CALLS];
#endif

/*===========================================================================*
 *				do_sysuname				     *
 *===========================================================================*/
int do_sysuname()
{
/* Set or get uname strings. */
  int r;
  size_t n;
  char *string;
#if 0 /* for updates */
  char tmp[sizeof(uts_val.nodename)];
  static short sizes[] = {
	0,	/* arch, (0 = read-only) */
	0,	/* kernel */
	0,	/* machine */
	0,	/* sizeof(uts_val.hostname), */
	sizeof(uts_val.nodename),
	0,	/* release */
	0,	/* version */
	0,	/* sysname */
  };
#endif

  if (m_in.m_lc_pm_sysuname.field >= _UTS_MAX) return(EINVAL);

  string = uts_tbl[m_in.m_lc_pm_sysuname.field];
  if (string == NULL)
	return EINVAL;	/* Unsupported field */

  switch (m_in.m_lc_pm_sysuname.req) {
  case _UTS_GET:
	/* Copy an uname string to the user. */
	n = strlen(string) + 1;
	if (n > m_in.m_lc_pm_sysuname.len) n = m_in.m_lc_pm_sysuname.len;
	r = sys_datacopy(SELF, (vir_bytes)string, mp->mp_endpoint,
		m_in.m_lc_pm_sysuname.value, (phys_bytes)n);
	if (r < 0) return(r);
	break;

#if 0	/* no updates yet */
  case _UTS_SET:
	/* Set an uname string, needs root power. */
	len = sizes[m_in.m_lc_pm_sysuname.field];
	if (mp->mp_effuid != 0 || len == 0) return(EPERM);
	n = len < m_in.m_lc_pm_sysuname.len ? len : m_in.m_lc_pm_sysuname.len;
	if (n <= 0) return(EINVAL);
	r = sys_datacopy(mp->mp_endpoint, m_in.m_lc_pm_sysuname.value, SELF,
		(phys_bytes)tmp, (phys_bytes)n);
	if (r < 0) return(r);
	tmp[n-1] = 0;
	strcpy(string, tmp);
	break;
#endif

  default:
	return(EINVAL);
  }
  /* Return the number of bytes moved. */
  return(n);
}


/*===========================================================================*
 *				do_getsysinfo			       	     *
 *===========================================================================*/
int do_getsysinfo()
{
  vir_bytes src_addr, dst_addr;
  size_t len;

  /* This call leaks important information. In the future, requests from
   * non-system processes should be denied.
   */
  if (mp->mp_effuid != 0)
  {
	printf("PM: unauthorized call of do_getsysinfo by proc %d '%s'\n",
		mp->mp_endpoint, mp->mp_name);
	sys_diagctl_stacktrace(mp->mp_endpoint);
	return EPERM;
  }

  switch(m_in.m_lsys_getsysinfo.what) {
  case SI_PROC_TAB:			/* copy entire process table */
        src_addr = (vir_bytes) mproc;
        len = sizeof(struct mproc) * NR_PROCS;
        break;
#if ENABLE_SYSCALL_STATS
  case SI_CALL_STATS:
  	src_addr = (vir_bytes) calls_stats;
  	len = sizeof(calls_stats);
  	break; 
#endif
  default:
  	return(EINVAL);
  }

  if (len != m_in.m_lsys_getsysinfo.size)
	return(EINVAL);

  dst_addr = m_in.m_lsys_getsysinfo.where;
  return sys_datacopy(SELF, src_addr, who_e, dst_addr, len);
}

/*===========================================================================*
 *				do_getprocnr			             *
 *===========================================================================*/
int do_getprocnr(void)
{
  register struct mproc *rmp;

  /* This check should be replaced by per-call ACL checks. */
  if (who_e != RS_PROC_NR) {
	printf("PM: unauthorized call of do_getprocnr by %d\n", who_e);
	return EPERM;
  }

  if ((rmp = find_proc(m_in.m_lsys_pm_getprocnr.pid)) == NULL)
	return(ESRCH);

  mp->mp_reply.m_pm_lsys_getprocnr.endpt = rmp->mp_endpoint;
  return(OK);
}

/*===========================================================================*
 *				do_getepinfo			             *
 *===========================================================================*/
int do_getepinfo(void)
{
  struct mproc *rmp;
  endpoint_t ep;
  int slot;

  ep = m_in.m_lsys_pm_getepinfo.endpt;
  if (pm_isokendpt(ep, &slot) != OK)
	return(ESRCH);

  rmp = &mproc[slot];
  mp->mp_reply.m_pm_lsys_getepinfo.uid = rmp->mp_effuid;
  mp->mp_reply.m_pm_lsys_getepinfo.gid = rmp->mp_effgid;
  return(rmp->mp_pid);
}

/*===========================================================================*
 *				do_reboot				     *
 *===========================================================================*/
int do_reboot()
{
  message m;

  /* Check permission to abort the system. */
  if (mp->mp_effuid != SUPER_USER) return(EPERM);

  /* See how the system should be aborted. */
  abort_flag = m_in.m_lc_pm_reboot.how;

  /* notify readclock (some arm systems power off via RTC alarms) */
  if (abort_flag & RB_POWERDOWN) {
	endpoint_t readclock_ep;
	if (ds_retrieve_label_endpt("readclock.drv", &readclock_ep) == OK) {
		message m; /* no params to set, nothing we can do if it fails */
		_taskcall(readclock_ep, RTCDEV_PWR_OFF, &m);
	}
  }

  /* Order matters here. When VFS is told to reboot, it exits all its
   * processes, and then would be confused if they're exited again by
   * SIGKILL. So first kill, then reboot. 
   */

  check_sig(-1, SIGKILL, FALSE /* ksig*/); /* kill all users except init */
  sys_stop(INIT_PROC_NR);		   /* stop init, but keep it around */

  /* Tell VFS to reboot */
  memset(&m, 0, sizeof(m));
  m.m_type = VFS_PM_REBOOT;

  tell_vfs(&mproc[VFS_PROC_NR], &m);

  return(SUSPEND);			/* don't reply to caller */
}

/*===========================================================================*
 *				do_getsetpriority			     *
 *===========================================================================*/
int do_getsetpriority()
{
	int r, arg_which, arg_who, arg_pri;
	struct mproc *rmp;

	arg_which = m_in.m_lc_pm_priority.which;
	arg_who = m_in.m_lc_pm_priority.who;
	arg_pri = m_in.m_lc_pm_priority.prio;	/* for SETPRIORITY */

	/* Code common to GETPRIORITY and SETPRIORITY. */

	/* Only support PRIO_PROCESS for now. */
	if (arg_which != PRIO_PROCESS)
		return(EINVAL);

	if (arg_who == 0)
		rmp = mp;
	else
		if ((rmp = find_proc(arg_who)) == NULL)
			return(ESRCH);

	if (mp->mp_effuid != SUPER_USER &&
	   mp->mp_effuid != rmp->mp_effuid && mp->mp_effuid != rmp->mp_realuid)
		return EPERM;

	/* If GET, that's it. */
	if (call_nr == PM_GETPRIORITY) {
		return(rmp->mp_nice - PRIO_MIN);
	}

	/* Only root is allowed to reduce the nice level. */
	if (rmp->mp_nice > arg_pri && mp->mp_effuid != SUPER_USER)
		return(EACCES);
	
	/* We're SET, and it's allowed.
	 *
	 * The value passed in is currently between PRIO_MIN and PRIO_MAX.
	 * We have to scale this between MIN_USER_Q and MAX_USER_Q to match
	 * the kernel's scheduling queues.
	 */

	if ((r = sched_nice(rmp, arg_pri)) != OK) {
		return r;
	}

	rmp->mp_nice = arg_pri;
	return(OK);
}

/*===========================================================================*
 *				do_svrctl				     *
 *===========================================================================*/
int do_svrctl(void)
{
  unsigned long req;
  int s;
  vir_bytes ptr;
#define MAX_LOCAL_PARAMS 2
  static struct {
  	char name[30];
  	char value[30];
  } local_param_overrides[MAX_LOCAL_PARAMS];
  static int local_params = 0;

  req = m_in.m_lc_svrctl.request;
  ptr = m_in.m_lc_svrctl.arg;

  /* Is the request indeed for the PM? ('M' is old and being phased out) */
  if (IOCGROUP(req) != 'P' && IOCGROUP(req) != 'M') return(EINVAL);

  /* Control operations local to the PM. */
  switch(req) {
  case OPMSETPARAM:
  case OPMGETPARAM:
  case PMSETPARAM:
  case PMGETPARAM: {
      struct sysgetenv sysgetenv;
      char search_key[64];
      char *val_start;
      size_t val_len;
      size_t copy_len;

      /* Copy sysgetenv structure to PM. */
      if (sys_datacopy(who_e, ptr, SELF, (vir_bytes) &sysgetenv, 
              sizeof(sysgetenv)) != OK) return(EFAULT);  

      /* Set a param override? */
      if (req == PMSETPARAM || req == OPMSETPARAM) {
  	if (local_params >= MAX_LOCAL_PARAMS) return ENOSPC;
  	if (sysgetenv.keylen <= 0
  	 || sysgetenv.keylen >=
  	 	 sizeof(local_param_overrides[local_params].name)
  	 || sysgetenv.vallen <= 0
  	 || sysgetenv.vallen >=
  	 	 sizeof(local_param_overrides[local_params].value))
  		return EINVAL;
  		
          if ((s = sys_datacopy(who_e, (vir_bytes) sysgetenv.key,
            SELF, (vir_bytes) local_param_overrides[local_params].name,
               sysgetenv.keylen)) != OK)
               	return s;
          if ((s = sys_datacopy(who_e, (vir_bytes) sysgetenv.val,
            SELF, (vir_bytes) local_param_overrides[local_params].value,
              sysgetenv.vallen)) != OK)
               	return s;
            local_param_overrides[local_params].name[sysgetenv.keylen] = '\0';
            local_param_overrides[local_params].value[sysgetenv.vallen] = '\0';

  	local_params++;

  	return OK;
      }

      if (sysgetenv.keylen == 0) {	/* copy all parameters */
          val_start = monitor_params;
          val_len = sizeof(monitor_params);
      } 
      else {				/* lookup value for key */
      	  int p;
          /* Try to get a copy of the requested key. */
          if (sysgetenv.keylen > sizeof(search_key)) return(EINVAL);
          if ((s = sys_datacopy(who_e, (vir_bytes) sysgetenv.key,
                  SELF, (vir_bytes) search_key, sysgetenv.keylen)) != OK)
              return(s);

          /* Make sure key is null-terminated and lookup value.
           * First check local overrides.
           */
          search_key[sysgetenv.keylen-1]= '\0';
          for(p = 0; p < local_params; p++) {
          	if (!strcmp(search_key, local_param_overrides[p].name)) {
          		val_start = local_param_overrides[p].value;
          		break;
          	}
          }
          if (p >= local_params && (val_start = find_param(search_key)) == NULL)
               return(ESRCH);
          val_len = strlen(val_start) + 1;
      }

      /* See if it fits in the client's buffer. */
      if (val_len > sysgetenv.vallen)
      	return E2BIG;

      /* Value found, make the actual copy (as far as possible). */
      copy_len = MIN(val_len, sysgetenv.vallen); 
      if ((s=sys_datacopy(SELF, (vir_bytes) val_start, 
              who_e, (vir_bytes) sysgetenv.val, copy_len)) != OK)
          return(s);

      return OK;
  }

  default:
	return(EINVAL);
  }
}

/*===========================================================================*
 *				do_getrusage				     *
 *===========================================================================*/
int
do_getrusage(void)
{
	clock_t user_time, sys_time;
	struct rusage r_usage;
	int r, children;

	if (m_in.m_lc_pm_rusage.who != RUSAGE_SELF &&
	    m_in.m_lc_pm_rusage.who != RUSAGE_CHILDREN)
		return EINVAL;

	/*
	 * TODO: first relay the call to VFS.  As is, VFS does not have any
	 * fields it can fill with meaningful values, but this may change in
	 * the future.  In that case, PM would first have to use the tell_vfs()
	 * system to get those values from VFS, and do the rest here upon
	 * getting the response.
	 */

	memset(&r_usage, 0, sizeof(r_usage));

	children = (m_in.m_lc_pm_rusage.who == RUSAGE_CHILDREN);

	/*
	 * Get system times.  For RUSAGE_SELF, get the times for the calling
	 * process from the kernel.  For RUSAGE_CHILDREN, we already have the
	 * values we should return right here.
	 */
	if (!children) {
		if ((r = sys_times(who_e, &user_time, &sys_time, NULL,
		    NULL)) != OK)
			return r;
	} else {
		user_time = mp->mp_child_utime;
		sys_time = mp->mp_child_stime;
	}

	/* In both cases, convert from clock ticks to microseconds. */
	set_rusage_times(&r_usage, user_time, sys_time);

	/* Get additional fields from VM. */
	if ((r = vm_getrusage(who_e, &r_usage, children)) != OK)
		return r;

	/* Finally copy the structure to the caller. */
	return sys_datacopy(SELF, (vir_bytes)&r_usage, who_e,
	    m_in.m_lc_pm_rusage.addr, (vir_bytes)sizeof(r_usage));
}
int create_group()
{
int j = 0, i = 0;
if(init_mailbox_flag == 0)
{
init_mailbox();
init_mailbox_flag = 1;
}
//check if group id already exists
if(mp->mp_realuid == 0)
{
	while(mailbox[i].group_id != m_in.m1_i1 && i<16)
	i++;
	if(i<16)
	{
	printf("error: group id already exists\n");
	return(-1);
	}
	i=0;
	while(mailbox[i].group_id != -1 && i<16)
	i++;
	mailbox[i].group_id = m_in.m1_i1;
	while(j<16)
	{
	mailbox[i].members[j][0] = -1 ;
	j++;
	}
	mailbox[i].secure_flag = 0;
	mailbox[i].public_flag = 0;
	mailbox[i].members[0][0] = m_in.m1_i2;
	mailbox[i].members[0][1] = 1;
	return (1);
	}
	else
	printf("\nerror: only root can create a group\n");	
	return (-2);
}
int remove_group()
{
int i = 0;

if(mp->mp_realuid == 0)
{
	while(mailbox[i].group_id != m_in.m1_i1 && i<16)
	i++;
	if(i<16)
	{
	mailbox[i].group_id = -1;
	return(1);
	}
	else
	{
	printf("error: No group id exists\n");
	return(-1);
	}
	}
	else
	printf("\nerror: only root can remove a group\n");	
	return (-2);
}

int add_group_leader()
{
int j = 0, i = 0;
if(mp->mp_realuid == 0)
{
	while(mailbox[i].group_id != m_in.m1_i1 && i<16)
	i++;
	if(i<16)
	{
	while(mailbox[i].members[j][0] != m_in.m1_i2 && j<16)
	j++;
	if(j<16)
	{
	mailbox[i].members[j][1] = 1;
	return(1);
	}
	else
	{
	j = 0;
	while(mailbox[i].members[j][0] != -1 && j<16)
	j++;
	if(j<16)
	{
	mailbox[i].members[j][0] = m_in.m1_i2; 
	mailbox[i].members[j][1] = 1;
	return(1);
	}
	else
	//list full
	return(-3);
	}
	}
	else
	{
	printf("error: No group id exists\n");
	return(-1);
	}
	}
	else
	printf("\nerror: only root can  add group leader\n");	
	return (-2);
}

int subscribe_to_group()
{
int j = 0, i = 0;
	while(mailbox[i].group_id != m_in.m1_i1 && i<16)
	i++;
	if(i<16)
	{
	while(mailbox[i].subscribe_ids[j] != m_in.m1_i2 && j<16)
	j++;
	if(j>=16)
	{
	j = 0;
	while(mailbox[i].subscribe_ids[j] != -1 && j<16)
	j++;
	if(j<16)
	{
	mailbox[i].subscribe_ids[j] = m_in.m1_i2;
	return(1);
	}
	else
	//list full
	return(-3);
	}
	else
	{
	printf("error: user id already exists in subscribe list\n");
	return(0);
	}
	}
	else
	{
	printf("error: No group id exists\n");
	return(-1);
	}
}

int unsubscribe_to_group()
{
int j = 0, i = 0;
	while(mailbox[i].group_id != m_in.m1_i1 && i<16)
	i++;
	if(i<16)
	{
	while(mailbox[i].subscribe_ids[j] != m_in.m1_i2 && j<16)
	j++;
	if(j<16)
	{
	mailbox[i].subscribe_ids[j] = -1;
	return(1);
	}
	else
	{
	printf("error: user id doesnot exists in subscribe list\n");
	return(0);
	}
	}
	else
	{
	printf("error: No group id exists\n");
	return(-1);
	}
}

int display_all_messages()
{
	int i=0, j=0, k=0;
	while(global_deny_rec[i] != m_in.m1_i1 && i<16)
	i++;
	if(i<16)
	{
	printf("you belong to deny list\n");
	return(-1);
	}
	i=0;
	for(i=0;i<16;i++)
	{
		if(mailbox[i].group_id != -1)
		{
			while(mailbox[i].subscribe_ids[j] != m_in.m1_i1 && j<16)
			j++;
			if(j<16)
			{
				k=0;
				printf("\ndisplaying messanges of mailbox with group id= %d \n",mailbox[i].group_id );
				printf("----------displaying secure messages--------\n");
				while(k<16)
				{
					if(mailbox[i].secure_ids[k]!=0)
					{
						printf("message %d is: \n %s \n",k,(mailbox[i].secure_mailbox[k]));
					}
				k++;
				}
				k=0;	
				printf("\n--------displaying public messages--------\n");
				while(k<16)
				{
					if(mailbox[i].secure_ids[k]!=0)
					{
						printf("message %d is: \n %s \n",k,(mailbox[i].public_mailbox[k]));
					}
				k++;
				}
			}
		}	
	}
	return(1);
}		
		

int remove_group_leader()
{
int j = 0, i = 0;
if(mp->mp_realuid == 0)
{
	while(mailbox[i].group_id != m_in.m1_i1 && i<16)
	i++;
	if(i<16)
	{
	while((mailbox[i].members[j][0] != m_in.m1_i2 || mailbox[i].members[j][1] != 1) && j<16)
	j++;
	if(j<16)
	{
	mailbox[i].members[j][1] = 2;
	return(1);
	}
	else
	{
	printf("\n no group leader with id = %d exists\n",m_in.m1_i2);
		return(-3);
	}
	}
	else
	{
	printf("error: no group id exists\n");
	return(-1);
	}
	}
	else
	printf("\nerror: only root can remove a group leader\n");	
	return(-2);
}

int deny_send()
{
int j = 0, i = 0;
if(mp->mp_realuid == 0)
{
	while(global_deny_send[i] != m_in.m1_i1 && i<30)
	i++;
	if (i>=30)
	{
	i =0;
	while(global_deny_send[i] != -1 && i<30)
	i++;
	if (i<30)
	{
	global_deny_send[i] = m_in.m1_i1;
	return(1);
	}
	else
	{
	printf("deny send list full\n");
	return(-3);
	}
	}
	else
	{
	printf("error: uid already denied send permission\n");
	return(0);
	}
	}
	else
	printf("\nerror: only root can perform this\n");	
	return(-2);
}

int allow_send()
{
int j = 0, i = 0;
if(mp->mp_realuid == 0)
{
	while(global_deny_send[i] != m_in.m1_i1 && i<30)
	i++;
	if (i<30)
	{
	global_deny_send[i] = -1;
	return(1);
	}
	else
	{
	printf("error: uid already has send permission\n");
	return(0);
	}
	}
	else
	printf("\nerror: only root can perform this\n");	
	return(-2);
}

int allow_receive()
{
int j = 0, i = 0;
if(mp->mp_realuid == 0)
{
	while(global_deny_rec[i] != m_in.m1_i1 && i<30)
	i++;
	if (i<30)
	{
	global_deny_rec[i] = -1;
	return(1);
	}
	else
	{
	printf("error: uid already has receive permission\n");
	return(0);
	}
	}
	else
	printf("\nerror: only root can perform this\n");	
	return(-2);
}

int deny_receive()
{
int j = 0, i = 0;
if(mp->mp_realuid == 0)
{
	while(global_deny_rec[i] != m_in.m1_i1 && i<30)
	i++;
	if (i>=30)
	{
	i =0;
	while(global_deny_rec[i] != -1 && i<30)
	i++;
	if(i<30)
	{
	global_deny_rec[i] = m_in.m1_i1;
	return(1);
	}
	else
	{
	printf("deny receive list full\n");
	return(-3);
	}
	}
	else
	{
	printf("\nerror: uid already denied receive permission\n");	
	return (0);
	}
	}
	else
	printf("\nerror: only root can perform this\n");	
	return (-2);	
}

int create_secure_group()
{
	int i = 0;
	while(mailbox[i].group_id != m_in.m1_i1 && i<16)
		i++;
	if (i<16)
	{
	if(mailbox[i].secure_flag == 1)
	{
	printf("secure group already exists for this group id\n");
	return(0);
	}
	else
	{
	mailbox[i].secure_flag = 1;
	return(1);
	}
	}
	else
	{
		printf("Group Id doesn't exist \n");
		return(-1);
	}
}
	
int remove_secure_group()
{
int i = 0;
	while(mailbox[i].group_id != m_in.m1_i1 && i<16)
	i++;
	if (i<16)
	{
	if(mailbox[i].secure_flag == 0)
	{
	printf("no secure group present for this group id\n");
	return(0);
	}
	else
	{
	mailbox[i].secure_flag = 0;
	return(1);
	}
	}
	else
	{
	printf("Group Id doesn't exist \n");
	return(-1);
	}
}

int create_public_group()
{
int i = 0;
	while(mailbox[i].group_id != m_in.m1_i1 && i<16)
	i++;
	if (i<16)
	{
	if(mailbox[i].public_flag == 1)
	{
	printf("public group already exist for this group id\n");
	return(0);
	}
	else
	{
	mailbox[i].public_flag = 1;
	return(1);
	}
	}
	else
	{
	printf("Group Id doesn't exist \n");
	return(-1);
	}
}

int remove_public_group()
{
int i = 0;
	while(mailbox[i].group_id != m_in.m1_i1 && i<16)
	i++;
	if (i<16)
	{
	if(mailbox[i].public_flag == 0)
	{
	printf("no secure group present for this group id\n");
	return(0);
	}
	else
	{
	mailbox[i].public_flag = 0;
	return(1);
	}
	}
	else
	{
	printf("Group Id doesn't exist \n");
	return(-1);
	}
}
	
int add_user_to_group()
{
int j=0, i = 0;
	while(mailbox[i].group_id != m_in.m1_i1 && i<16)
	i++;
	if (i<16)
	{
		while(mailbox[i].members[j][0] != m_in.m1_i2 && j<16)
		j++;
		if(j<16)
		{
		printf("user already exists\n");
		return(0);
		}
		else
		{
		j=0;
		while(mailbox[i].members[j][0] != -1 && j<16)
		j++;
		if(j<16)
		{
		mailbox[i].members[j][0] = m_in.m1_i2;
		mailbox[i].members[j][1] = 2;
		return(1);
		}
		else
		{
		printf("error: maximum limit reached, no more uids can be added\n remove some to add new one\n");
		return(-3);
		}
		}
	}
		else
		{
		printf("invalid group id\n");
		return(-1);
		}
}

int remove_user_from_group()
{
int j=0, i = 0;
	while(mailbox[i].group_id != m_in.m1_i1 && i<16)
	i++;
	if (i<16)
	{
 	while(mailbox[i].members[j][0] != m_in.m1_i2 && j<16)
	j++;
	if (j<16)
	{
	if(mailbox[i].members[j][1] == 1)
	{
	printf("error: user is group leader, Use function remove_group_leader\n");
	return(-2);
	}
	else
	{
	mailbox[i].members[j][0] = -1;
	return (1);
	}
	}
	else
	{
	printf("user doesn't exists\n");
	return(-3);
	}
	}
	else
	{
	printf("invalid group id\n");
	return(-1);
	}
}
int check_deny_send_list(int uid)
{
	int i=0,flag=1;
	for(i=0;i<30;i++)
	{
		if(global_deny_send[i]==uid)
			flag=0;
	}
	return flag;

}
int check_deny_rec_list(int uid)
{
	int i=0,flag=1;
	for(i=0;i<30;i++)
	{
		if(global_deny_rec[i]==uid)
			flag=0;
	}
	return flag;

}


int do_deposit()
{
	char *p = m_in.m1_p1;
	int i = 0;
	while(mailbox[i].group_id != m_in.m1_i1 && i<16)
		i++;
	if (i<16)
	{
			printf("found group at %d",i);
			if(mailbox[i].secure_flag==1)
			{

				int c=0;
				while(mailbox[i].secure_ids[c]!=0&&c<16)
							c++;
					if(c<16)
					{
							if(check_deny_send_list(m_in.m1_i2)!=0)
							{
								int i1;
								while(mailbox[i].secure_ids[i1] != 0 && i < 16)
										i1++;

								if(i1<16)
								{

									if (sys_datacopy(who_e, (vir_bytes)p, SELF, (vir_bytes) &mailbox[i].secure_mailbox[i1],(vir_bytes)30) != OK)
											return(-1);
									else
									{

										mailbox[i].secure_ids[i1] = m_in.m1_i2;
										return 1;
									}

								}
								else
								{
									return 0;
								}
							}
					}
					else
					{
						printf("\nError: Mailbox Full");
						return 0;
					}

			}
			else
			{
				//public mailbox
				int c=0;
				while(mailbox[i].public_ids[c]!=0&&c<16)
					c++;
				if(c<16)
				{
					if(check_deny_send_list(m_in.m1_i2)!=0)
					{
						int i1;
						while(mailbox[i].public_ids[i1] != 0 && i < 16)
								i1++;

						if(i1<16)
						{

							if (sys_datacopy(who_e, (vir_bytes)p, SELF, (vir_bytes) &mailbox[i].public_mailbox[i1],(vir_bytes)30) != OK)
									return(-1);
							else
							{

								mailbox[i].public_ids[i1]=m_in.m1_i2;
								return 1;
							}

						}
						else
						{
							return 0;
						}
					}
			}
			else
			{
				printf("\nError: Mailbox Full");
				return 0;
			}


		}
	}
	else
		printf("\n Then given  group doesn't exist");
	return -1;
}

	/*char *p = m_in.m1_p1;
	uid_t r = (unsigned int) m_in.m1_i1;
	int i = 0;
	printf("\nuid = %d \n",mp->mp_realuid);
	while(rec[i] != 0 && i < 16)
		i++;
	//printf(" i = %d with who as\n",i);
	if (i == 16)
	{
		printf("\n MailBox full, retry after sometime \n");
		return(0);
	}
    if (sys_datacopy(who_e, (vir_bytes)p, SELF, (vir_bytes) &m[i][0],(vir_bytes)30) != OK)
    	return(-1);
	rec[i] = r;
	printf("\n Msg is added to MailBox \n");
	return(1);*/
}
int do_clear()
{
	int i;

	for(i=0;i<15;i++)
	{
		strcpy(m[i],"");
	}
	return 1;
}

int do_setup()
{
	int i;
	for(i=0;i<15;i++)
	{
			rec[i]=0;
	}

	do_clear();
	return 1;
}

void printmailbox(char m[16][30])
{
	int i;
	for(i=0;i<=15;i++)
		printf("-%s",m[i]);
}
int tracker(int recv)
{

	if (counter==0)
		prev=recv;
	if(prev==recv)
		counter++;
	if (counter >= 100)
		return 0;
	return 1;
}

int do_retrieve()
{

	char *p = m_in.m1_p1;
	unsigned int r = (unsigned int) m_in.m1_i2;
	int gid = m_in.m1_i1;

	int gi = 0, j = 0,i=0,c=0;

	while(mailbox[i].group_id != m_in.m1_i1 && i<16)
			i++;
	if (i<16)
	{
		printf("found group at %d",i);
		if(mailbox[i].secure_flag==1)
		{
			//RETRIVE sECURE MAIL
			int mem=0;
			for(mem=0;mem<16;mem++)
			{
				if(mailbox[i].members[mem][0]== r)
					break;

			}
			if (mem ==0)
			{
				printf("\n The given user is not present in the given group");
				return 0;
			}
			else if(mem>0)
			{
				if(mailbox[i].members[mem][0]!=1)
				{
					while( gi < 16 )
					{
						j = mailbox[i].secure_ids[gi] & r; ///checking if the id matches the correct receiver
						if(mailbox[i].secure_ids[gi] & r)
						{
							if (sys_datacopy(SELF, (vir_bytes)mailbox[i].secure_mailbox[gi], who_e, (vir_bytes)p,
							   (vir_bytes)30) != OK) return(0);

							mailbox[i].secure_ids[gi] = mailbox[i].secure_ids[gi] & ~r; //Remove the receiver who already read
							int cindex=gi;

							if(mailbox[i].secure_ids[gi]==0)
							{
								do // for keeping the mails in continuous indexes
								{
									mailbox[i].secure_ids[cindex]= mailbox[i].secure_ids[cindex+1];
									mailbox[i].secure_ids[cindex+1] = 0;
									strcpy((char *)mailbox[i].secure_mailbox[cindex],(char *)mailbox[i].secure_mailbox[cindex+1]);
									cindex++;
								}while(cindex<16 && mailbox[i].secure_ids[cindex]!=0);

							}
							return(1);
						}
						else
						{
							if (tracker(r)==0) // for life lock
								return(-2);
						}


						gi++;
					}

				}
				else
				{
					printf("\n Permission Denied");
					return 0;

				}
			}


		}
		if(mailbox[i].public_flag==1)
		{
			//Retrieve public mail
			while( gi < 16 )
			{
				j = mailbox[i].public_ids[gi] & r; ///checking if the id matches the correct receiver
				if(mailbox[i].public_ids[gi] & r)
				{
					if (sys_datacopy(SELF, (vir_bytes)mailbox[i].public_mailbox[gi], who_e, (vir_bytes)p,
					   (vir_bytes)30) != OK) return(0);

					mailbox[i].public_ids[gi] = mailbox[i].public_ids[gi] & ~r; //Remove the receiver who already read
					int cindex=gi;

					if(mailbox[i].public_ids[gi]==0)
					{
						do // for keeping the mails in continuous indexes
						{
							mailbox[i].public_ids[cindex]= mailbox[i].public_ids[cindex+1];
							mailbox[i].public_ids[cindex+1] = 0;
							strcpy(mailbox[i].public_mailbox[cindex],mailbox[i].public_mailbox[cindex+1]);
							cindex++;
						}while(cindex<16 && mailbox[i].public_ids[cindex]!=0);

					}

					return(1);
				}
				else
				{
					if (tracker(r)==0) // for life lock
						return(-2);
				}


				gi++;
			}

		}
	}
	return(0);
}


