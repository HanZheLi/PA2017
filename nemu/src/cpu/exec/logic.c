#include "cpu/exec.h"

make_EHelper(test) {//不需要写回
  rtl_and(&t0,&id_dest->val,&id_src->val);
  rtl_update_ZFSF(&t0,id_dest->width);
  rtl_set_OF(&tzero);
  rtl_set_CF(&tzero);
  print_asm_template2(test);
}

make_EHelper(and) {
  rtl_and(&t0,&id_dest->val,&id_src->val);
  operand_write(id_dest,&t0);
  rtl_update_ZFSF(&t0,id_dest->width);
  rtl_set_CF(&tzero);
  rtl_set_OF(&tzero);
  print_asm_template2(and);
}

make_EHelper(xor) {
  rtl_xor(&t0,&id_dest->val,&id_src->val);
  operand_write(id_dest,&t0);//写回
  rtl_update_ZFSF(&t0,id_dest->width);//初始化ZF和SF位
  rtl_set_CF(&tzero);//CF位置0
  rtl_set_OF(&tzero);//OF位置0
  print_asm_template2(xor);
}

make_EHelper(or) {
  rtl_or(&t0,&id_dest->val,&id_src->val);
  operand_write(id_dest,&t0);
  rtl_update_ZFSF(&t0,id_dest->width);
  rtl_set_CF(&tzero);
  rtl_set_OF(&tzero);
  print_asm_template2(or);
}

make_EHelper(sar) {
  // unnecessary to update CF and OF in NEMU
//  rtl_sext(&t0,&id_dest->val,id_dest->width);
  rtl_sar(&t1,&id_dest->val,&id_src->val);
  operand_write(id_dest,&t1);
  rtl_update_ZFSF(&t1,id_dest->width);
  print_asm_template2(sar);
}

make_EHelper(shl) {
  // unnecessary to update CF and OF in NEMU
  rtl_shl(&t0,&id_dest->val,&id_src->val);
  rtl_update_ZFSF(&t0,id_dest->width);
  operand_write(id_dest,&t0);
  print_asm_template2(shl);
}

make_EHelper(shr) {
  // unnecessary to update CF and OF in NEMU
  rtl_shr(&t0,&id_dest->val,&id_src->val);
  rtl_update_ZFSF(&t0,id_dest->width);
  operand_write(id_dest,&t0);
  print_asm_template2(shr);
}

make_EHelper(setcc) {
  uint8_t subcode = decoding.opcode & 0xf;
  rtl_setcc(&t2, subcode);
  operand_write(id_dest, &t2);

  print_asm("set%s %s", get_cc_name(subcode), id_dest->str);
}

make_EHelper(not) {
  rtl_not(&id_dest->val);
  operand_write(id_dest,&id_dest->val);
  print_asm_template1(not);
}

make_EHelper(rol)
{
  uint32_t W=8*id_dest->width-id_src->val;
  rtl_li(&t0,id_dest->val);
  rtl_shri(&t1,&t0,W);//先将符号位右移到指定位置
  rtl_shli(&t0,&t0,id_src->val);
  rtl_or(&t2,&t1,&t0);
  operand_write(id_dest,&t2);  
  print_asm_template1(rol);
}

