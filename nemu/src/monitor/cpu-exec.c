#include "nemu.h"
#include "monitor/monitor.h"
#include "monitor/watchpoint.h"
#include "monitor/expr.h"
/* The assembly code of instructions executed is only output to the screen
 * when the number of instructions executed is less than this value.
 * This is useful when you use the `si' command.
 * You can modify this value as you want.
 */
#define MAX_INSTR_TO_PRINT 10000
uint32_t threshold = 0xfffffff0;
uint32_t Count=0;
int nemu_state = NEMU_STOP;

void exec_wrapper(bool);
//uint32_t addr;
/* Simulate how the CPU works. */
void cpu_exec(uint64_t n) {
  if (nemu_state == NEMU_END) {
    printf("Program execution has ended. To restart the program, exit NEMU and run again.\n");
    return;
  }
  bool success=true;
  nemu_state = NEMU_RUNNING;
  addr=expr("$eip",&success);
  bool print_flag = n < MAX_INSTR_TO_PRINT;
 
  for (; n > 0; n --) {
    /* Execute one instruction, including instruction fetch,
     * instruction decode, and the actual execution. */
    	
    exec_wrapper(print_flag);

#ifdef DEBUG
    /* TODO: check watchpoints here. */
   // printf("%x\n",addr);
    if(scan_watchpoint())
    {
	nemu_state=NEMU_STOP;
    }

#endif

#ifdef HAS_IOE
    extern void device_update();
    device_update();
#endif

    if (nemu_state != NEMU_RUNNING) { return; }
    Count++;
    if(Count>threshold)
    {
	    Log("endless loop");
	    nemu_state=NEMU_STOP;
	    Count=0;
    }
  }

  if (nemu_state == NEMU_RUNNING) { nemu_state = NEMU_STOP; }
}
