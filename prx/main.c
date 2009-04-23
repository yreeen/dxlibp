#include <pspsdk.h> 
#include <pspkernel.h> 
#include <pspdebug.h> 


#define VERS 1 
#define REVS 0 


PSP_MODULE_INFO("Profiler", 0x1006, 1, 0); 
PSP_MAIN_THREAD_ATTR(0); 


#define PROFILER_REG_BASE 0xBC400000 
#define PROFILER_REG_COUNT 20 


void InitProfiler(void) 
{ 
     u32 k1; 
     volatile PspDebugProfilerRegs *regs = (volatile PspDebugProfilerRegs *)PROFILER_REG_BASE; 

     k1 = pspSdkSetK1(0); 

    regs->enable = 1; 

     pspSdkSetK1(k1); 
} 


void ExitProfiler(void) 
{ 
     u32 k1; 
     volatile PspDebugProfilerRegs *regs = (volatile PspDebugProfilerRegs *)PROFILER_REG_BASE; 

     k1 = pspSdkSetK1(0); 

    regs->enable = 0; 
     asm("sync\r\n"); 

     pspSdkSetK1(k1); 
} 


void ClrProfileRegs(void) 
{ 
     u32 k1, i; 
     volatile u32 *regs = (volatile u32 *)PROFILER_REG_BASE; 

     k1 = pspSdkSetK1(0); 

     /* Don't clear the enable register */ 
     for(i = 1; i < PROFILER_REG_COUNT; i++) 
          regs[i] = 0; 

     pspSdkSetK1(k1); 
} 


void GetProfileRegs(PspDebugProfilerRegs *dest) 
{ 
     u32 k1, i; 
     volatile u32 *regs = (volatile u32 *)PROFILER_REG_BASE; 
    u32 *u_regs = (u32 *)dest; 

     k1 = pspSdkSetK1(0); 

     for(i = 0; i < PROFILER_REG_COUNT; i++) 
          u_regs[i] = regs[i]; 

     pspSdkSetK1(k1); 
} 


int module_start(SceSize args, void *argp) 
{ 
   return 0; 
} 

int module_stop() 
{ 
   return 0; 
}