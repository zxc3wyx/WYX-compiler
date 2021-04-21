#include <type_traits>
//
// Created by hswyx666 on 2020/11/5.
//
#include <string>
#include "mips.h"
#include <vector>
#include <cstdio>
#include <unordered_map>
#include <unordered_set>
#include <map>
using namespace std;

vector<string> dataField;
vector<string> textField;
FILE* mips;

unordered_map<string, int> localVarName2offset;

unordered_set<string> localVarName;

vector<vector<string>> midExprList;               //中间表达式序列

vector<vector<string>> midExprOutput;               //待输出四元式

static int sno;

static int strno;

static int branchno;

int switchno = 1;

int whileno = 1;

int forno = 1;

int local_offset = 0;                //局部变量占了多少空间

extern vector<pair<string, sym_type>> globalSymStack;

extern std::map<string, vector<sym_type>> paramOfFunction;

void clear_local_var()
{
    localVarName.clear();
    localVarName2offset.clear();
}

void global_normal_var(const string& identifier, int initialValue)
{
    dataField.push_back(identifier + ":\t\t.word\t" + to_string(initialValue) + "\n");
}

void global_array_var_no_init(const string& identifier, int length)
{
    dataField.push_back(identifier + ":\t\t.word\t0:" + to_string(4 * length) + "\n");
}

void global_array_var_init(const string& identifier, int length, vector<int> initList)
{
    string s = identifier + ":\t\t.word\t";
    for (int i = 0; i < length - 1; i++) {
        s += to_string(initList[i]) + ", ";
    }
    s += to_string(initList[length - 1]);
    s += "\n";
    dataField.push_back(s);
}

void local_normal_var(const string& identifier, int initialValue)
{
    local_offset += -4;
    textField.push_back("li $t0, " + to_string(initialValue));
    textField.emplace_back("sw $t0, " + to_string(local_offset) + "($sp)");
    localVarName2offset[identifier] = local_offset;
    localVarName.insert(identifier);
}

void local_array_var(const string& identifier, int length, int initialValue[])
{
    localVarName2offset[identifier] = local_offset - 4;
    localVarName.insert(identifier);
    for (int i = 0; i < length; i++) {
        textField.push_back("li $t0, " + to_string(initialValue[i]));
        textField.emplace_back("sw $t0, " + to_string(local_offset - 4 * (i + 1)) + "($sp)");
    }
    local_offset += -4 * length;
}

void local_param(const string& identifier)
{
    local_offset += -4;
    localVarName2offset[identifier] = local_offset;
    localVarName.insert(identifier);
}

void mips_read(const string& identifier, sym_type type)
{
    if (type == var_int) {
        textField.emplace_back("ori $v0, $0, 5");
        textField.emplace_back("syscall");
    }
    else {
        textField.emplace_back("ori $v0, $0, 12");
        textField.emplace_back("syscall");
    }
    if (localVarName.find(identifier) != localVarName.end()) {
        textField.emplace_back("sw $v0, " + to_string(localVarName2offset[identifier]) + "($sp)");
    }
    else {
        textField.emplace_back("sw $v0,  " + identifier);
    }
}

void mips_putEnter()
{
    textField.emplace_back("li $v0, 11");
    textField.emplace_back("li $a0, 10");
    textField.emplace_back("syscall");
}

void mips_putString(const string& src)
{
    string src_clear = string();
    for (const char& c: src) {
        if (c == '\\') {
            src_clear.append("\\\\");
        } else {
            src_clear += c;
        }
    }
    dataField.emplace_back("$STR" + to_string(strno) + ":\t\t.asciiz\t" + "\"" + src_clear + "\"\n");
    textField.emplace_back("la $a0, $STR" + to_string(strno));
    strno++;
    textField.emplace_back("li $v0, 4");
    textField.emplace_back("syscall");
}

void mips_putExpression(sym_type type, const string& expr)
{
    if (type == var_int) {
        textField.emplace_back("ori $v0, $0, 1");
    }
    else {
        textField.emplace_back("ori $v0, $0, 11");
    }
    if (localVarName.find(expr) != localVarName.end()) {
        textField.emplace_back("lw $a0, " + to_string(localVarName2offset[expr]) + "($sp)");
    } else {
        textField.emplace_back("lw $a0, " + expr + "($zero)");
    }
    textField.emplace_back("syscall");
}


void midExpr2mips()                     //四元式转mips
{
    for (vector<string> midExpr: midExprList) {
        string c = "#";
        for (const string& s: midExpr) {
            printf("%s ", s.c_str());
            c += s;
            c += " ";
        }
        textField.emplace_back(c);
        putchar('\n');
        string sym = midExpr[0];
        if (sym.at(0) == '@') {
            string func_name = sym.substr(1);
            for (size_t j = 1; j < midExpr.size(); j++) {
                string para_name = midExpr[j];
                if (localVarName.find(para_name) != localVarName.end()) {
                    textField.emplace_back("lw $a1, " + to_string(localVarName2offset[para_name]) + "($sp)");
                } else {
                    textField.emplace_back("lw $a1, " + para_name + "($zero)");
                }
                textField.emplace_back("sw $a1, " + to_string(local_offset - 8 - 4 * (int)j) + "($sp)");
            }
            mips_call_func();
            textField.emplace_back("jal $$" + func_name);
            mips_call_func_restore();
        } else if (sym == "GETRET") {
            string dest = midExpr[1];
            local_offset += -4;
            textField.emplace_back("sw $k1, " + to_string(local_offset) + "($sp)");
            localVarName.insert(dest);
            localVarName2offset[dest] = local_offset;
        } else if (sym == "SINGLE") {
            string op = midExpr[1];
            string dest = midExpr[2];
            if (op.at(0) == '-' || op.at(0) == '+' || (op.at(0) >= '0' && op.at(0) <= '9')) {
                textField.emplace_back("li $s0, " + op);
            } else {
                if (localVarName.find(op) != localVarName.end()) {
                    textField.emplace_back("lw $s0, " + to_string(localVarName2offset[op]) + "($sp)");
                } else {
                    textField.emplace_back("lw $s0, " + op + "($zero)");
                }
            }
            local_offset += -4;
            textField.emplace_back("sw $s0, " + to_string(local_offset) + "($sp)");
            localVarName.insert(dest);
            localVarName2offset[dest] = local_offset;
        } else if (sym == "ASSIGN") {
            string op = midExpr[1];
            string src = midExpr[2];
            if (localVarName.find(src) != localVarName.end()) {
                textField.emplace_back("lw $s0, " + to_string(localVarName2offset[src]) + "($sp)");
            } else {
                textField.emplace_back("lw $s0, " + src + "($zero)");
            }
            if (localVarName.find(op) != localVarName.end()) {
                textField.emplace_back("sw $s0, " + to_string(localVarName2offset[op]) + "($sp)");
            } else {
                textField.emplace_back("sw $s0, " + op + "($zero)");
            }
        } else if (sym == "GET") {
            string src = midExpr[1];
            string offset = midExpr[2];
            string dst = midExpr[3];
            if (localVarName.find(src) != localVarName.end()) {
                textField.emplace_back("addiu $s1, $sp, " + to_string(localVarName2offset[src]));
                textField.emplace_back("lw $s2, " + to_string(localVarName2offset[offset]) + "($sp)");
                textField.emplace_back("sll $s2, $s2, 2");
                textField.emplace_back("subu $s1, $s1, $s2");
                textField.emplace_back("lw $s0, 0($s1)");
                local_offset += -4;
                textField.emplace_back("sw $s0, " + to_string(local_offset) + "($sp)");
                localVarName.insert(dst);
                localVarName2offset[dst] = local_offset;
            }
            else {
                textField.emplace_back("lw $s1, " + to_string(localVarName2offset[offset]) + "($sp)");
                textField.emplace_back("sll $s1, $s1, 2");
                textField.emplace_back("lw $s0, " + src + "($s1)");
                local_offset += -4;
                textField.emplace_back("sw $s0, " + to_string(local_offset) + "($sp)");
                localVarName.insert(dst);
                localVarName2offset[dst] = local_offset;
            }
        }
        else if (sym == "LOAD") {
            string src = midExpr[1];
            string offset = midExpr[2];
            string dst = midExpr[3];
            if (localVarName.find(src) != localVarName.end()) {
                textField.emplace_back("addiu $s1, $sp, " + to_string(localVarName2offset[src]));
                textField.emplace_back("lw $s2, " + to_string(localVarName2offset[offset]) + "($sp)");
                textField.emplace_back("sll $s2, $s2, 2");
                textField.emplace_back("subu $s1, $s1, $s2");
                textField.emplace_back("lw $s0, " + to_string(localVarName2offset[dst]) + "($sp)");
                textField.emplace_back("sw $s0, 0($s1)");
            }
            else {
                textField.emplace_back("lw $s1, " + to_string(localVarName2offset[offset]) + "($sp)");
                textField.emplace_back("sll $s1, $s1, 2");
                textField.emplace_back("lw $s0, " + to_string(localVarName2offset[dst]) + "($sp)");
                textField.emplace_back("sw $s0, " + src + "($s1)");
            }
        }
        else {
            string op1 = midExpr[1];
            string op2 = midExpr[2];
            string dest = midExpr[3];
            if (op1.at(0) == '-' || op1.at(0) == '+' || (op1.at(0) >= '0' && op1.at(0) <= '9')) {
                //TODO
                textField.emplace_back("li $s1, " + op1);
            } else {
                if (localVarName.find(op1) != localVarName.end()) {
                    textField.emplace_back("lw $s1, " + to_string(localVarName2offset[op1]) + "($sp)");
                } else {
                    textField.emplace_back("lw $s1, " + op1 + "($zero)");
                }
            }
            if (op2.at(0) == '-' || op2.at(0) == '+' || (op2.at(0) >= '0' && op2.at(0) <= '9')) {
                //TODO
                textField.emplace_back("li $s2, " + op2);
            } else {
                if (localVarName.find(op2) != localVarName.end()) {
                    textField.emplace_back("lw $s2, " + to_string(localVarName2offset[op2]) + "($sp)");
                } else {
                    textField.emplace_back("lw $s2, " + op2 + "($zero)");
                }
            }
            switch (sym.at(0)) {
                case '+':
                    textField.emplace_back("addu $s0, $s1, $s2");
                    break;
                case '-':
                    textField.emplace_back("subu $s0, $s1, $s2");
                    break;
                case '*':
                    textField.emplace_back("mul $s0, $s1, $s2");
                    break;
                case '/':
                    textField.emplace_back("div $s0, $s1, $s2");
                    break;
            }
            local_offset += -4;
            textField.emplace_back("sw $s0, " + to_string(local_offset) + "($sp)");
            localVarName.insert(dest);
            localVarName2offset[dest] = local_offset;
        }
    }
}

void clear_midExpr()
{
    for (const vector<string>& midExpr: midExprList) {
        midExprOutput.emplace_back(midExpr);
    }
    /*for (const vector<string>& expr: midExprList) {
        for (const string& s: expr) {
            printf("%s ", s.c_str());
        }
        putchar('\n');
    }*/
    midExprList.clear();
}

void mips_generate_label_func(const string& func_name)
{
    textField.emplace_back("$$" + func_name + ":");
}


void mips_call_func()
{
    textField.emplace_back("move $k0, $sp");
    textField.emplace_back("addiu $sp, $sp, " + to_string(local_offset));
    textField.emplace_back("sw $k0, -4($sp)");
    textField.emplace_back("sw $ra, -8($sp)");

}

void mips_call_func_restore()
{
    textField.emplace_back("lw $ra, -8($sp)");
    textField.emplace_back("lw $sp, -4($sp)");
}

int mips_branch(const string& op)
{
    branchno++;
    string command = (op == ">") ? "ble" :
                     (op == "<") ? "bge" :
                     (op == ">=") ? "blt" :
                     (op == "<=") ? "bgt" :
                     (op == "==") ? "bne" :
                     (op == "!=") ? "beq" : "b";
    textField.emplace_back(command + " $s3, $s4, $iflabel" + to_string(branchno));
    return branchno;
}

void mips_branch_while(const string& op, int wno)
{
    string command = (op == ">") ? "ble" :
                     (op == "<") ? "bge" :
                     (op == ">=") ? "blt" :
                     (op == "<=") ? "bgt" :
                     (op == "==") ? "bne" :
                     (op == "!=") ? "beq" : "b";
    textField.emplace_back(command + " $s3, $s4, $while" + to_string(wno) + "loop");
}

void mips_branch_for(const string& op, int fno, const string& identidier, const string& expr)
{
    string command = (op == ">") ? "ble" :
                     (op == "<") ? "bge" :
                     (op == ">=") ? "blt" :
                     (op == "<=") ? "bgt" :
                     (op == "==") ? "bne" :
                     (op == "!=") ? "beq" : "b";
    if (localVarName.find(identidier) != localVarName.end()) {
        textField.emplace_back("lw $t4, " + to_string(localVarName2offset[identidier]) + "($sp)");
    } else {
        textField.emplace_back("lw $t4, " + identidier + "($zero)");
    }
    if (localVarName.find(expr) != localVarName.end()) {
        textField.emplace_back("lw $t5, " + to_string(localVarName2offset[expr]) + "($sp)");
    } else {
        textField.emplace_back("lw $t5, " + expr + "($zero)");
    }
    textField.emplace_back(command + " $t4, $t5, $for" + to_string(fno) + "loop");
}

void mips_if_label(int no)
{
    textField.emplace_back("$iflabel" + to_string(no) + ":");
}

void mips_else_label(int no)
{
    textField.emplace_back("$elselabel" + to_string(no) + ":");
}

void mips_switch(const string& switchConst, int swno, int cano)
{
    textField.emplace_back("bne $s6, " + switchConst + ", $switch" + to_string(swno)
    + "case" + to_string(cano) + "end");
}

void mips_switch_label(int swno, int cano)
{
    textField.emplace_back("$switch" + to_string(swno) + "case" + to_string(cano) + "end:");
}

void mips_move_to_register(const string& reg, const string& var_name)
{
    if (localVarName.find(var_name) != localVarName.end()) {
        textField.emplace_back("lw $s0, " + to_string(localVarName2offset[var_name]) + "($sp)");
    } else {
        textField.emplace_back("lw $s0, " + var_name + "($sp)");
    }
    textField.emplace_back("move " + reg + ", $s0");
}

void mips_step_move(const string& id1, const string& id2, const string& op, int step)
{
    if (localVarName.find(id2) != localVarName.end()) {
        textField.emplace_back("lw $t5, " + to_string(localVarName2offset[id2]) + "($sp)");
    } else {
        textField.emplace_back("lw $t5, " + id2 + "($zero)");
    }
    string command = (op == "+") ? "addiu" : "subiu";
    textField.emplace_back(command + " $t5, $t5, " + to_string(step));
    if (localVarName.find(id1) != localVarName.end()) {
        textField.emplace_back("sw $t5, " + to_string(localVarName2offset[id1]) + "($sp)");
    } else {
        textField.emplace_back("sw $t5, " + id1 + "($zero)");
    }
}

void code_generate()
{
    mips = fopen("mips.txt", "w");
    fprintf(mips, ".data\n");
    for (const string& s: dataField) {
        fputs(s.c_str(), mips);
    }
    fputc('\n', mips);
    fprintf(mips, ".text\n");
    fputs("jal $$main\n", mips);
    fputs("li $v0, 10\n", mips);
    fputs("syscall\n\n", mips);
    for (const string& s: textField) {
        fprintf(mips, "%s\n", s.c_str());
    }
}