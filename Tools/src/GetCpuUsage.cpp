
#ifdef WIN32

///////////////////////////////////////////////////////////////////
//
//		GetCpuUsage uses the performance counters to retrieve the
//		system cpu usage.
//		The cpu usage counter is of type PERF_100NSEC_TIMER_INV
//		which as the following calculation:
//
//		Element		Value 
//		=======		===========
//		X			CounterData 
//		Y			100NsTime 
//		Data Size	8 Bytes
//		Time base	100Ns
//		Calculation 100*(1-(X1-X0)/(Y1-Y0)) 
//
//      where the denominator (Y) represents the total elapsed time of the 
//      sample interval and the numerator (X) represents the time during 
//      the interval when the monitored components were inactive.
//
//
//		Note:
//		====
//		On windows NT, cpu usage counter is '% Total processor time'
//		under 'System' object. However, in Win2K/XP Microsoft moved
//		that counter to '% processor time' under '_Total' instance
//		of 'Processor' object.
//		Read 'INFO: Percent Total Performance Counter Changes on Windows 2000'
//		Q259390 in MSDN.
//
///////////////////////////////////////////////////////////////////

#include "GetCpuUsage.h"

#define SYSTEM_OBJECT_INDEX					2		// 'System' object
#define PROCESSOR_OBJECT_INDEX				238		// 'Processor' object
#define TOTAL_PROCESSOR_TIME_COUNTER_INDEX	240		// '% Total processor time' counter (valid in WinNT under 'System' object)
#define PROCESSOR_TIME_COUNTER_INDEX		6		// '% processor time' counter (for Win2K/XP)


GetCpuUsage:: PLATFORM GetCpuUsage:: getPlatform (void)

{
	OSVERSIONINFO osvi;
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	if (!GetVersionEx(&osvi))
		return UNKNOWN;
	switch (osvi.dwPlatformId)
	{
	case VER_PLATFORM_WIN32_WINDOWS:
		return WIN9X;
	case VER_PLATFORM_WIN32_NT:
		if (osvi.dwMajorVersion == 4)
			return WINNT;
		else
			return WIN2K_XP;
	}
	return UNKNOWN;
}

//
//	GetCpuUsage returns the cpu usage.
//	Since we calculate the cpu usage by two samplings, the first
//	call to GetCpuUsage() returns 0 and keeps the values for the next
//	sampling.
//  Read the comment at the beginning of this file for the formula.
//
int GetCpuUsage:: getCpuUsage (void)

{
	// Cpu usage counter is 8 byte length.
	CPerfCounters<LONGLONG> PerfCounters;
	char szInstance[256] = {0};

//		Note:
//		====
//		On windows NT, cpu usage counter is '% Total processor time'
//		under 'System' object. However, in Win2K/XP Microsoft moved
//		that counter to '% processor time' under '_Total' instance
//		of 'Processor' object.
//		Read 'INFO: Percent Total Performance Counter Changes on Windows 2000'
//		Q259390 in MSDN.

	DWORD dwObjectIndex;
	DWORD dwCpuUsageIndex;
	switch (Platform)
	{
	case WINNT:
		dwObjectIndex = SYSTEM_OBJECT_INDEX;
		dwCpuUsageIndex = TOTAL_PROCESSOR_TIME_COUNTER_INDEX;
		break;
	case WIN2K_XP:
		dwObjectIndex = PROCESSOR_OBJECT_INDEX;
		dwCpuUsageIndex = PROCESSOR_TIME_COUNTER_INDEX;
		strcpy(szInstance,"_Total");
		break;
	default:
		return -1;
	}

	int				CpuUsage = 0;
	LONGLONG		lnNewValue = 0;
	PPERF_DATA_BLOCK pPerfData = NULL;
	LARGE_INTEGER	NewPerfTime100nSec = {0};

	lnNewValue = PerfCounters.GetCounterValue(&pPerfData, dwObjectIndex, dwCpuUsageIndex, szInstance);
	NewPerfTime100nSec = pPerfData->PerfTime100nSec;

	if (bFirstTime)
	{
		bFirstTime = false;
		lnOldValue = lnNewValue;
		OldPerfTime100nSec = NewPerfTime100nSec;
		return 0;
	}

	LONGLONG lnValueDelta = lnNewValue - lnOldValue;
	double DeltaPerfTime100nSec = (double)NewPerfTime100nSec.QuadPart - (double)OldPerfTime100nSec.QuadPart;

	lnOldValue = lnNewValue;
	OldPerfTime100nSec = NewPerfTime100nSec;

	double a = (double)lnValueDelta / DeltaPerfTime100nSec;

	double f = (1.0 - a) * 100.0;
	CpuUsage = (int)(f + 0.5);	// rounding the result
	if (CpuUsage < 0)
		return 0;
	return CpuUsage;
}


#else	// WIN32

#include <stdlib.h>
#include <stdio.h>

#include "GetCpuUsage.h"

#ifdef DARWIN
/* Mac OS X & Darwin */
#include <mach/mach_init.h>
#include <mach/mach_host.h>
#endif

#ifdef FREEBSD
#include <kvm.h>
#include <sys/dkstat.h>
#include <sys/vmmeter.h>
#include <sys/resource.h>
#include <sys/sysctl.h>
#include <vm/vm_param.h>

#define pagetob(size) ((size) << pageshift)

static kvm_t *kd = NULL;
static struct nlist nlst[] = {
    {"_cp_time", 0},
    {"_cnt", 0},
    {"_bufspace", 0},
    {0, 0}
};
static int pageshift;
int previous_total=0, previous_load=0;

#endif


#ifdef DARWIN
/* Mac OS X & Darwin code written by Ben Hines <bhines@alumni.ucsd.edu> */

int GetCpuUsage:: getCpuUsage (void)

{
	processor_cpu_load_info_data_t *pinfo;
	mach_msg_type_number_t info_count;
	unsigned long composite_user, composite_nice, composite_sys, composite_idle;
   unsigned int cpuload, n_cpus;
   u_int64_t load, total;
	int i;
    if (firsttimes == 0) {
		for (i = 0; i < CPUSMOOTHNESS; i++)
			cpu_average_list[i] = 0;
    }
    /* Wait until we have CPUSMOOTHNESS messures */
    if (firsttimes != CPUSMOOTHNESS)
		firsttimes++;
	
	if (host_processor_info (mach_host_self (),
						  PROCESSOR_CPU_LOAD_INFO,
						  &n_cpus,
						  (processor_info_array_t*)&pinfo,
						  &info_count)) {
		return 0;
	}

	composite_user = composite_nice = composite_sys = composite_idle = 0;

	for (i = 0; i < n_cpus; i++) {
		composite_user  += pinfo[i].cpu_ticks [CPU_STATE_USER];
		composite_sys   += pinfo[i].cpu_ticks [CPU_STATE_SYSTEM];
		composite_idle  += pinfo[i].cpu_ticks [CPU_STATE_IDLE];
		composite_nice  += pinfo[i].cpu_ticks [CPU_STATE_NICE];
	}
	vm_deallocate (mach_task_self (), (vm_address_t) pinfo, info_count);

	/* user + sys = load
	* total = total */
	load = composite_user + composite_sys;	/* cpu.user + cpu.sys; */
	total = load + composite_idle + composite_nice;	/* cpu.total; */

	/* Calculates an average from the last CPUSMOOTHNESS messures */

    if(total!=ototal)
	cpu_average_list[current] = (100 * (load - oload)) / (total - ototal);
    else
	cpu_average_list[current] = (load - oload);

    current++;
    if (current == CPUSMOOTHNESS)
		current = 0;

    oload = load;
    ototal = total;

    if (firsttimes != CPUSMOOTHNESS)
		return 0;

    cpuload = 0;

    for (i = 0; i < CPUSMOOTHNESS; i++)
		cpuload += cpu_average_list[i];
    return (cpuload / CPUSMOOTHNESS);
	
}

#elif FREEBSD

int init_cpu_load()
{
    /* calculate page shift to convert pages into kilobytes */
    int pagesize = getpagesize();
    pageshift = 0;

    while (pagesize > 1) {
	pageshift++;
	pagesize >>= 1;
    }

    /* open kernel memory */
    kd = kvm_open(NULL, NULL, NULL, O_RDONLY, "kvm_open");

    if (kd == NULL) {
	puts("Could not open kernel virtual memory");
	return 1;
    }

    kvm_nlist(kd, nlst);

    if (nlst[0].n_type == 0 || nlst[1].n_type == 0 || nlst[2].n_type == 0) {
	puts("Error extracting symbols");
	return 2;
    }

    /* drop setgid & setuid (the latter should not be there really) */
    seteuid(getuid());
    setegid(getgid());

    if (geteuid() != getuid() || getegid() != getgid()) {
	puts("Unable to drop privileges");
	return 3;
    }

    return 0;
}

/* Returns the current CPU load in percent */
int GetCpuUsage:: getCpuUsage (void)

{
    int loadPercentage;
    int total, load;
    unsigned long int cpu_time[CPUSTATES];
    int i, cpuload;


    if (firsttimes == 0) {
	for (i = 0; i < CPUSMOOTHNESS; i++)
	    cpu_average_list[i] = 0;
    }
    /* Wait until we have CPUSMOOTHNESS messures */
    if (firsttimes != CPUSMOOTHNESS)
	firsttimes++;


    if (kvm_read(kd, nlst[0].n_value, &cpu_time, sizeof(cpu_time))
	!= sizeof(cpu_time))
	return 0;

    load = cpu_time[CP_USER] + cpu_time[CP_SYS] + cpu_time[CP_NICE];
    total = load + cpu_time[CP_IDLE];


    if(total!=previous_total)
	cpu_average_list[current] = (100 * (load - previous_load)) / (total - previous_total);
    else
	cpu_average_list[current] = (load - previous_load);
	
    current++;
    if (current == CPUSMOOTHNESS)
	current = 0;

    previous_load = load;
    previous_total = total;

    if (firsttimes != CPUSMOOTHNESS)
	return 0;

    cpuload = 0;

    for (i = 0; i < CPUSMOOTHNESS; i++)
	cpuload += cpu_average_list[i];
    return (cpuload / CPUSMOOTHNESS);

}


#else	// LINUX

/* returns current CPU load in percent, 0 to 100 */

int GetCpuUsage:: getCpuUsage (void)

{
    unsigned int cpuload;
    unsigned long long load, total;
    unsigned long long ab, ac, ad, ae;
    FILE *stat;
    int i;

    stat = fopen("/proc/stat", "r");
    fscanf(stat, "%*s %lld %lld %lld %lld", &ab, &ac, &ad, &ae);
    fclose(stat);

    if (firsttimes == 0) {
	for (i = 0; i < CPUSMOOTHNESS; i++)
	    cpu_average_list[i] = 0;
    }
    /* Wait until we have CPUSMOOTHNESS messures */
    if (firsttimes != CPUSMOOTHNESS)
	firsttimes++;

    /* Find out the CPU load */
    /* user + sys = load
     * total = total */
    load = ab + ac + ad;	/* cpu.user + cpu.sys; */
    total = ab + ac + ad + ae;	/* cpu.total; */

    /* Calculates and average from the last CPUSMOOTHNESS messures */
    if(total!=ototal)
	cpu_average_list[current] = (100 * (load - oload)) / (total - ototal);
    else
	cpu_average_list[current] = (load - oload);

    current++;
    if (current == CPUSMOOTHNESS)
	current = 0;

    oload = load;
    ototal = total;


    if (firsttimes != CPUSMOOTHNESS)
	return 0;

    cpuload = 0;

    for (i = 0; i < CPUSMOOTHNESS; i++)
	cpuload += cpu_average_list[i];
    return (cpuload / CPUSMOOTHNESS);
}
#endif
#endif

