#include "monitor/watchpoint.h"
#include "monitor/expr.h"
#include "cpu/reg.h"
#include "memory/memory.h"
#define NR_WP 32

static WP wp_pool[NR_WP];
static WP *head, *free_;

void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i ++) {
    wp_pool[i].NO = i;
    wp_pool[i].next = &wp_pool[i + 1];
  }
  wp_pool[NR_WP - 1].next = NULL;

  head = NULL;
  free_ = wp_pool;
}

/* TODO: Implement the functionality of watchpoint */

WP* new_wp()
{
	if (free_==NULL) assert(0);//没有空闲监视点
	WP* p=free_;
	free_=free_->next;
//	p->next=NULL;
	if(head==NULL)
	{	
	head=p;
	p->next=NULL;
	}
	else
	{
		p->next=head;
		head=p;
//		head->next=p;
		
	}
	return p;
}

void free_wp(WP *wp)
{
	WP *p;
	if(wp==head)//归还头节点
	{
		head=head->next;
		wp->next=free_;
	}
	else		//删除中间的
	{
		p=head;
		while(p)
		{
			if(p->next==wp)
			{
				p->next=wp->next;
				wp->next=free_;
				break;
			}
			p=p->next;
		}
	}
	free_=wp;
}

int set_watchpoint(char *e)
{
	bool success=true;
	WP *p=new_wp();
	uint32_t value=expr(e,&success);
	int len=strlen(e);
	if(p&&success)
	{
		for(int i=0;i<len;i++)
		{
			p->expr[i]=e[i];
		}
		p->expr[len]='\0';
		p->old_val=value;
		p->temp=vaddr_read(value,1);
		printf("Set watchpoint #%d\n",p->NO);
		printf("expr=\t%s\n",p->expr);
		printf("old value = 0x%x\n",p->old_val);
		return p->NO;
	}
	else return -1;
}

bool delete_watchpoint(int NO)
{
	WP *p=head;
	while(p)
	{
		if(p->NO==NO)
		{
			printf("Watchpoint %d delete\n",p->NO);
			p->expr[0]='\0';
			p->old_val=-1;
			p->new_val=-1;
			free_wp(p);
			return true;
		}
		p=p->next;
	}
	return false;

}

void list_watchpoint(void)//列出正在使用中的监视点列表
{
	WP *p=head;
	while(p)
	{
		printf("%d\t%s\t   0x%x\n",p->NO,p->expr,p->old_val);
		p=p->next;
	}	
}

WP* scan_watchpoint(void)
{
	WP *p=head;
	bool success=true;
	while(p)
	{
		p->new_val=expr(p->expr,&success);
		if(success&&p->new_val!=p->old_val)
		{
			printf("Hit watchpoint %d at address 0x%x\n",p->NO,addr/*cpu.eip*/);
			printf("old value = 0x%08x\n",p->old_val); 
			printf("new value = 0x%08x\n",p->new_val);
			printf("promgram paused\n");
			p->old_val=p->new_val;
			return p;
		}
		p=p->next;
	}
	return NULL;
}

uint32_t getbyte(uint32_t addr)
{
	WP *p=head;
        while(p)
       {
          if(addr==p->old_val)
	 {																			                        return p->temp;
	 }																                				p=p->next;								        }	
	return 0;
}
