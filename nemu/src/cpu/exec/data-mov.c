#include "cpu/exec.h"

make_EHelper(mov) {
  operand_write(id_dest, &id_src->val);
  print_asm_template2(mov);
}

make_EHelper(push) {
/*  rtl_li(&t0,id_dest->val);//立即数读取
  rtl_push(&t0);*/
  if (id_dest->width == 1) 
  {
	  id_dest->val = (int32_t)(int8_t)id_dest->val;	
  }
  rtl_push(&id_dest->val);
  print_asm_template1(push);
}

make_EHelper(pop) {
  rtl_pop(&t0);
  operand_write(id_dest,&t0);//写回
  print_asm_template1(pop);
}

make_EHelper(pusha) {
  if(decoding.is_operand_size_16)
  {
	  t0=cpu.esp;
	  rtl_lr_w(&t1,R_AX);
	  rtl_push(&t1);
	  rtl_lr_w(&t2,R_CX);
	  rtl_push(&t2);
	  rtl_lr_w(&t3,R_DX);
	  rtl_push(&t3);
	  rtl_lr_w(&t1,R_BX);
	  rtl_push(&t1);
	  rtl_push(&t0);
	  rtl_lr_w(&t2,R_BP);
	  rtl_push(&t2);
	  rtl_lr_w(&t3,R_SI);
	  rtl_push(&t3);
	  rtl_lr_w(&t1,R_DI);
	  rtl_push(&t1);
  }
  else{
	  t0=cpu.esp;
	  rtl_push(&cpu.eax);
	  rtl_push(&cpu.ecx);
	  rtl_push(&cpu.edx);
	  rtl_push(&cpu.ebx);
	  rtl_push(&t0);
	  rtl_push(&cpu.ebp);
	  rtl_push(&cpu.esi);
	  rtl_push(&cpu.edi);
  }
  print_asm("pusha");
}

make_EHelper(popa) {
  if(decoding.is_operand_size_16)
  {
	  rtl_pop(&t1);
	  rtl_sr_w(R_DI,&t1);
	  rtl_pop(&t1);
          rtl_sr_w(R_SI,&t1);
	  rtl_pop(&t1);
	  rtl_sr_w(R_BP,&t1);
	  rtl_pop(&t0);
	  rtl_pop(&t1);
	  rtl_sr_w(R_BX,&t1);
	  rtl_pop(&t1);
	  rtl_sr_w(R_DX,&t1);
	  rtl_pop(&t1);
	  rtl_sr_w(R_CX,&t1);
	  rtl_pop(&t1);
	  rtl_sr_w(R_AX,&t1);
  }
  else
  {
	  rtl_pop(&cpu.edi);
	  rtl_pop(&cpu.esi);
	  rtl_pop(&cpu.ebp);
	  rtl_pop(&t0);
	  rtl_pop(&cpu.ebx);
	  rtl_pop(&cpu.edx);
	  rtl_pop(&cpu.ecx);
	  rtl_pop(&cpu.eax);
  }  
  print_asm("popa");
}

make_EHelper(leave) {
//  rtl_li(&t0,cpu.ebp);
//  rtl_mv(&cpu.esp,&t0);
  rtl_mv(&cpu.esp, &cpu.ebp);
  rtl_pop(&cpu.ebp);
  print_asm("leave");
}

make_EHelper(cltd) {
  if (decoding.is_operand_size_16) {
	  rtl_lr(&t0,R_AX,2);
	  if ((int32_t)(int16_t)(uint16_t)t0 < 0)
	  { 	
  		  rtl_addi(&t1, &tzero, 0xffff);							 rtl_sr(R_DX, 2, &t1);			
	  }	
	  else
	  {
  		  rtl_sr(R_DX, 2, &tzero); 
  	  }
  }
  else {
    rtl_lr(&t0,R_EAX,4);
    if ((int32_t)t0 < 0) {	
	    rtl_addi(&t1, &tzero, 0xffffffff);		
	    rtl_sr(R_EDX, 4, &t1);					
    }
    else 
    {	
	    rtl_sr(R_EDX, 4, &tzero);		
    }
  }
  print_asm(decoding.is_operand_size_16 ? "cwtl" : "cltd");
}

make_EHelper(cwtl) {
  if (decoding.is_operand_size_16) {
	  rtl_lr(&t0, R_AL, 1);
//	  rtl_sext(&t0,&t0,2);
     	  t0 = (int16_t)(int8_t)(uint8_t)t0;
	  rtl_sr(R_AX, 2, &t0);
  }
  else {
    rtl_lr(&t0, R_AX, 2);
    t0 = (int32_t)(int16_t)(uint16_t)t0;
//    rtl_sext(&t0,&t0,4);
    rtl_sr(R_EAX, 4, &t0);
  }

  print_asm(decoding.is_operand_size_16 ? "cbtw" : "cwtl");
}

make_EHelper(movsx) {
  id_dest->width = decoding.is_operand_size_16 ? 2 : 4;
  rtl_sext(&t2, &id_src->val, id_src->width);
  operand_write(id_dest, &t2);
  print_asm_template2(movsx);
}

make_EHelper(movzx) {
  id_dest->width = decoding.is_operand_size_16 ? 2 : 4;
  operand_write(id_dest, &id_src->val);
  print_asm_template2(movzx);
}

make_EHelper(lea) {
  rtl_li(&t2, id_src->addr);
  operand_write(id_dest, &t2);
  print_asm_template2(lea);
}
