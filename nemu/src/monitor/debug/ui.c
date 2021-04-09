#include "monitor/monitor.h"
#include "monitor/expr.h"
#include "monitor/watchpoint.h"
#include "nemu.h"
#include "cpu/reg.h"
#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>

void cpu_exec(uint64_t);
static int cmd_c(char *args); 
/* We use the `readline' library to provide more flexibility to read from stdin. */
char* rl_gets() {
  static char *line_read = NULL;

  if (line_read) {
    free(line_read);
    line_read = NULL;
  }

  line_read = readline("(nemu) ");

  if (line_read && *line_read) {
    add_history(line_read);
  }

  return line_read;
}

static int cmd_si(char *args){
  int n=0;
  char *str=strtok(NULL," ");
  if(str==NULL)
  n=1;
  else
  sscanf(str,"%d",&n);
  if(n>0)
  cpu_exec(n);
  else cpu_exec(-1);
  return 0;
}

static int cmd_p(char *args)
{
	bool success=true;
	int num;
	if(args!=NULL)
	{
		num=expr(args,&success);
		if(success)
		{
			printf("0x%x\n",num);
		}
		else printf("Couldn't calculate!");
	}
	else printf("Invalid command!\n");
	return 0;
}

static int cmd_x(char *args)
{
  char *str=strtok(NULL," ");
  int n=0;
  vaddr_t vaddr;
  char *str1;
  bool success=true;
  sscanf(str,"%d",&n);
  str1=strtok(NULL," ");
  vaddr=expr(str1,&success);
//  str=strtok(NULL," ");
//  sscanf(str,"%x",&vaddr);
  printf("Address\tDword block...Byte sequence\n");
  if(n!=0)
  {
     for(int i=0;i<n;i++)
     {
	     printf("0x%08x\t0x%08x...",vaddr,vaddr_read(vaddr,4));
	     for(int j=0;j<4;j++)
	     {
	    	 printf("%02x ",vaddr_read(vaddr+j,1));
	     }
	     printf("\n");
	     vaddr=vaddr+4;
     }
  }
  return 0;
}

static int cmd_b(char *args){
  char *str=strtok(NULL," ");   
  uint32_t Addr;
  sscanf(str,"%x",&Addr);
  set_watchpoint(str);
  vaddr_write(Addr,1,0xcc);//替换
  return 0;
}
static int cmd_w(char *args){
  char *str=strtok(NULL," ");
/*  if(set_watchpoint(str)!=-1)
  {
	  cmd_c("NULL");
  } 
  else
	  printf("Set failed");*/
  set_watchpoint(str);
  return 0; 
}

static int cmd_d(char *args){
  char *str=strtok(NULL," ");
  int no;
  sscanf(str,"%d",&no);
  delete_watchpoint(no);  
  return 0;
}

static int cmd_info(char *args){
  char *str=strtok(NULL," ");
  if(strcmp(str,"r")==0)
  {
	for(int i=0;i<8;i++)
	printf("%s:\t0x%08x\t%u\n",regsl[i],cpu.gpr[i]._32,cpu.gpr[i]._32);
	printf("eip:\t0x%08x\t%u\n",cpu.eip,cpu.eip);
	for(int i=0;i<8;i++)
	printf("%s:\t0x%04x\t%u\n",regsw[i],cpu.gpr[i]._16,cpu.gpr[i]._16);
	for(int i=0;i<4;i++)
	{
		printf("%s:\t0x%02x\t%d\n",regsb[i],cpu.gpr[i]._8[0],cpu.gpr[i]._8[0]);
		printf("%s:\t0x%02x\t%d\n",regsb[i+4],cpu.gpr[i]._8[1],cpu.gpr[i]._8[1]);
	}
  }
  else if(strcmp(str,"w")==0)
  {
	printf("NO\tEXPR\t   Old Value\n");
	list_watchpoint();	
  }
  return 0;  
}

static int cmd_c(char *args) {
  cpu_exec(-1);
  return 0;
}

static int cmd_q(char *args) {
  return -1;
}

static int cmd_help(char *args);

static struct {
  char *name;
  char *description;
  int (*handler) (char *);
} cmd_table [] = {
  { "help", "Display informations about all supported commands", cmd_help },
  { "c", "Continue the execution of the program", cmd_c },
  { "q", "Exit NEMU", cmd_q },
  {"si","Execute some steps of command",cmd_si},
  {"info","printf all data of register",cmd_info},
  {"x","Scan memory",cmd_x},
  {"p","Calculate expression",cmd_p},
  {"w","Set a new watchpoint",cmd_w},
  {"d","Delete a watchpoint",cmd_d},
  {"b","Software breakpoint",cmd_b},


  /* TODO: Add more commands */

};

#define NR_CMD (sizeof(cmd_table) / sizeof(cmd_table[0]))

static int cmd_help(char *args) {
  /* extract the first argument */
  char *arg = strtok(NULL, " ");
  int i;

  if (arg == NULL) {
    /* no argument given */
    for (i = 0; i < NR_CMD; i ++) {
      printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
    }
  }
  else {
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(arg, cmd_table[i].name) == 0) {
        printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
        return 0;
      }
    }
    printf("Unknown command '%s'\n", arg);
  }
  return 0;
}

void ui_mainloop(int is_batch_mode) {
  if (is_batch_mode) {
    cmd_c(NULL);
    return;
  }

  while (1) {
    char *str = rl_gets();
    char *str_end = str + strlen(str);

    /* extract the first token as the command */
    char *cmd = strtok(str, " ");
    if (cmd == NULL) { continue; }

    /* treat the remaining string as the arguments,
     * which may need further parsing
     */
    char *args = cmd + strlen(cmd) + 1;
    if (args >= str_end) {
      args = NULL;
    }

#ifdef HAS_IOE
    extern void sdl_clear_event_queue(void);
    sdl_clear_event_queue();
#endif

    int i;
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(cmd, cmd_table[i].name) == 0) {
        if (cmd_table[i].handler(args) < 0) { return; }
        break;
      }
    }

    if (i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
  }
}
