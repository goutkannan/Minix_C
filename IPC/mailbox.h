#define clear(m1)		_syscall(PM_PROC_NR,PM_CLEAR, &m1)
#define retrieve(m1)	_syscall(PM_PROC_NR,PM_RETRIEVE, &m1)
#define deposit(m1)		_syscall(PM_PROC_NR,PM_DEPOSIT, &m1)
#define setup(m1)		_syscall(PM_PROC_NR,PM_SETUP, &m1)
