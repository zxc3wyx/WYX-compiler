//
// Created by hswyx666 on 2020/11/5.
//

#ifndef COHW5_MIPS_H
#define COHW5_MIPS_H

#include <string>
#include <vector>

using std::string;


typedef enum {
    const_char, const_int, var_char, var_int, func_char, func_int, func_void, unknown
} sym_type;

void clear_local_var();
void global_normal_var(const string& identifier, int initialValue);
void global_array_var_no_init(const string& identifier, int length);
void global_array_var_init(const string& identifier, int length, std::vector<int> initList);
void local_normal_var(const string& identifier, int initialValue);
void local_array_var(const string& identifier, int length, int initialValue[]);
void local_param(const string& identifier);
void clear_midExpr();
void mips_read(const string& identifier, sym_type type);
void mips_putString(const string& src);
void mips_putExpression(sym_type type, const string& expr);
void mips_putEnter();
void midExpr2mips();                   //四元式转mips
void mips_generate_label_func(const string& func_name);
void mips_call_func();
void mips_call_func_restore();
int mips_branch(const string& op);
void mips_if_label(int no);
void mips_else_label(int no);
void code_generate();
void mips_switch(const string& switchConst, int swno, int cano);
void mips_switch_label(int swno, int cano);
void mips_branch_while(const string& op, int wno);
void mips_branch_for(const string& op, int fno, const string& idenfr, const string& expr);
void mips_move_to_register(const string& reg, const string& var_name);
void mips_step_move(const string& id1, const string& id2, const string& op, int step);

#endif //COHW5_MIPS_H
