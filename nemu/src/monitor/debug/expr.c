#include "nemu.h"

int b = 0;
/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>
#include <string.h>
enum {
  TK_NOTYPE = 256, TK_EQ,TK_HEX,TK_DEC,TK_REGISTER,DEREF,MINUS,TK_AND,TK_OR,TK_NOTEQ

  /* TODO: Add more token types */

};
int  op[][9]={{'+','-','*','/',MINUS,TK_OR,TK_AND,TK_NOTEQ,TK_EQ},{4,4,5,5,6,1,2,3,3}};
static struct rule {
  char *regex;
  int token_type;
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */

  {" +", TK_NOTYPE},    // spaces
  {"\\+", '+'},         // plus
  {"==", TK_EQ}, 	 // equal
  {"-",'-'},		//sub
  {"\\*",'*'},		//multiply
  {"/",'/'},		//divide
  {"\\(",'('},		//left bracket
  {"\\)",')'},		//right bracket
  {"0x[0-9a-fA-F]+",TK_HEX},	//hex
  {"[0-9]+",TK_DEC},	//dec
  {"\\$e(ax|dx|cx|bx|bp|si|di|sp|ip)",TK_REGISTER},	//REG
  {"&&",TK_AND},
  {"\\|\\|",TK_OR},
  {"!=",TK_NOTEQ},
  {"\\!",'!'},
};

#define NR_REGEX (sizeof(rules) / sizeof(rules[0]) )

static regex_t re[NR_REGEX];

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
  int i;
  char error_msg[128];
  int ret;

  for (i = 0; i < NR_REGEX; i ++) {
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
    if (ret != 0) {
      regerror(ret, &re[i], error_msg, 128);
      panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
    }
  }
}

typedef struct token {
  int type;
  char str[32];
} Token;

Token tokens[32];
int nr_token;

static bool make_token(char *e){
  int position = 0;
  int i;
  regmatch_t pmatch;

  nr_token = 0;
  while (e[position] != '\0') {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i++) {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;
/*	 Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
            i, rules[i].regex, position, substr_len, substr_len, substr_start);*/
        position += substr_len;
//	Log("len is %d\n",substr_len);
        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */
	if(rules[i].token_type==TK_NOTYPE)
	break;
	tokens[nr_token].type = rules[i].token_type;
        switch (rules[i].token_type)
       	{
        	case '+':
		case '-':
		case '*':
		case '/':
		case '(':
		case ')':
		case '!':
		case TK_AND:
		case TK_OR:
		case TK_EQ:
		case TK_NOTEQ:
			break;
		case TK_DEC:
	   	case TK_HEX:
		case TK_REGISTER:
			if(substr_len>31)
	     		{
					printf("Wrong: the number is too long");
					assert(0);
			}
				for(int j=0;j<substr_len;j++)
				{
					tokens[nr_token].str[j]=substr_start[j];
				}
				tokens[nr_token].str[substr_len]='\0';
				break;
		default:printf("We can't recognize the token type");
			return false;			
        }
	nr_token++;
       break;
      }    
    }
   
    if (i == NR_REGEX) {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
   }	
  return true;
}

bool check_parentheses(int p,int q)
{
	int count=0;
	bool watch=false;
	if(tokens[p].type!='('||tokens[q].type!=')')
	{
		return false;
	}
	else
	{
		for(int i=p+1;i<q;i++)
		{
			if(tokens[i].type=='(')
				count++;
			else if(tokens[i].type==')')
				count--;
			if(count==-1)
			{
				watch=true;
			}
			if(watch==true&&count==-2)
			{
				assert(0);
			}
			if(watch==true&&count==0)
			{
				return false;
			}

		}
		if(count!=0)
		{
			assert(0);
		}
		else
		{
			return true;
		}
	}
}


int find_dominated_op(int p, int q)
{
	int i=0;
	int pre=10,pos=0;
	for(i=p;i<=q;i++)
	{
		if(tokens[i].type==TK_DEC||tokens[i].type==TK_REGISTER||tokens[i].type==TK_HEX)
			      continue;
		else if(tokens[i].type=='(')
		{
			int count=0;
			for(int j=i+1;j<=q;j++)
			{
				if(tokens[j].type==')')
				{
					count++;
					i=i+count;
					break;
				}
				else count++; 
			}
		}
		else 
		{
			for(int k=0;k<9;k++)
			{
					if(op[0][k]==tokens[i].type&&pre>=(op[1][k]))
				{
					pre=op[1][k];
					pos=i;
					break;
				}

			}

		}	
	}
	return pos;
}

uint32_t transfer(char ch)
{
	if (ch>='0'&&ch<='9') 
		return ch-'0';
	else if(ch>='a'&&ch<='f') 
	        return ch-'a'+10;
	else if(ch>='A'&&ch<='F') 
	        return ch-'A'+10;
        return 0;
}

uint32_t eval(int p,int q){
	if (p > q)
       	{
		assert(0);
	}
	else if(p==q)
	{
		if(tokens[p].type==TK_DEC)
		{
			uint32_t num=0;
			int i=0;
			while(i<strlen(tokens[p].str))
			{
				num=num*10+tokens[p].str[i]-'0';
				i++;
			}
			return num;
		}
		else if(tokens[p].type==TK_HEX)
		{
			int len=strlen(tokens[p].str);
			int weight=1;
			uint32_t num=0;
			for(int i=len-1;i>=0;i--)
			{
				num=num+weight*transfer(tokens[p].str[i]);
				weight=weight*16;
			}
			return num;
		}
		else if(tokens[p].type==TK_REGISTER)
		{
			char *str=tokens[p].str;
			str++;
			if(strcmp(str,"eip")==0)
				return cpu.eip;
			for(int i=0;i<8;i++)
			{
				if(strcmp(str,regsl[i])==0)
				{
					return cpu.gpr[i]._32;
				}
			}
		}
//		else assert(0);
	}
	else if (check_parentheses(p, q) == true)
	{
		return eval(p+1,q-1);
	}
	else
	{
		int op=find_dominated_op(p,q);
//		Log("op= %d",op);
		uint32_t val1=0,val2=0;
		if(tokens[op].type==MINUS||tokens[op].type==DEREF||tokens[op].type=='!')
		{
			val2=eval(op+1,q);
		}
		else
		{
			val1=eval(p,op-1);
			val2=eval(op+1,q);
		}
		switch(tokens[op].type)
		{
			case '+':return val1+val2;
			case '-':return val1-val2;
			case '*':return val1*val2;
			case '/':return val1/val2;
			case '!':return !val2;
			case MINUS:return -val2;
			case DEREF:return vaddr_read(val2,4);
			case TK_AND:return val1&&val2;
			case TK_OR:return val1||val2;
			case TK_EQ:return val1==val2;
			case TK_NOTEQ:return val1!=val2;
			default:assert(0);
		}
	}
	return 0;
}

uint32_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    *success = false;
    return 0;
  }
  /* TODO: Insert codes to evaluate the expression. */
  for(int i=0;i<nr_token;i++)
  {
	if(tokens[i].type=='-'&&((i==0)||(tokens[i-1].type!=TK_DEC&&tokens[i-1].type!=TK_HEX&&tokens[i-1].type!=TK_REGISTER&&tokens[i-1].type!=')')))
	{
//		Log("MINUS");
		tokens[i].type=MINUS;
	}
	if(tokens[i].type=='*'&&(i==0||(tokens[i-1].type!=TK_DEC&&tokens[i-1].type!=TK_REGISTER&&tokens[i-1].type!=TK_HEX&&tokens[i-1].type!=')')))
	{
//		Log("DEREF");
		tokens[i].type=DEREF;
	}

  }
 // Log("%d",nr_token);
  return eval(0,nr_token-1);
}
