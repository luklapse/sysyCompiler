#ifndef GRANDPARENT_H
#define GRANDPARENT_H

#define DEBUG
#define MAXCHARS 10000

#include "koopa.h"

#include <cassert>
#include <iostream>
#include <cstdio>
#include <memory>
#include <cstring>
#include <map>

using namespace std;

struct Symbol
{
  int kind;    // 0为const常量 1为符号变量
  int val;     // 没有初始化时，默认为0
  string type; //变量类型
};

extern map<string, Symbol> Symbolmap;
extern string btype_str; // 声明时变量类型，以便所有声明的变量种类初始化

class Baseast
{
public:
  virtual ~Baseast() = default;
  static int count_all;
  int count; //代表该节点结果的代数
  int kind;  //字节点序号

  virtual string Dump() = 0;
  virtual int Calc() = 0;
};

string Dumpop(unique_ptr<Baseast> &op1, unique_ptr<Baseast> &op2, const char *temp2, const char *temp1, const char *op);
// 所有 ast 的基类
string DumpUnaryOp(unique_ptr<Baseast> &op1, const char *temp1, const char *op);
// CompUnit 是 Baseast
class CompUnitast : public Baseast // CompUnit    ::= FuncDef;
{
public:
  // 用智能指针管理对象
  unique_ptr<Baseast> func_def;

  string Dump() override // override确保虚函数覆盖基类的虚函数
  {
    string temp = func_def->Dump();
    count = func_def->count;
    return temp;
  }

  int Calc() override
  {
    return func_def->Calc();
  }
};

// Decl          ::= ConstDecl|VarDecl;
class Declast : public Baseast
{
public:
  unique_ptr<Baseast> constdecl;
  unique_ptr<Baseast> vardecl;
  string Dump() override
  {
    string temp;
    if (kind == 1){
      temp="";
    }
    else if (kind == 2){
      temp=vardecl->Dump();
    }
    return temp;
  }
  int Calc() override
  { 
    int temp=0;
    if(kind==1)
    temp=constdecl->Calc();
    else 
      temp=0;
    return temp;
  }
};
// ConstDecl     ::= "const" BType ConstDef ";";
class ConstDeclast : public Baseast
{
public:
  unique_ptr<Baseast> btype; // btype 目前都为int 先不管
  unique_ptr<Baseast> constdef;
  string Dump() override
  {
    btype_str = btype->Dump();
    return "";
  }
  int Calc() override
  {
    return constdef->Calc();
  }
};
// BType         ::= "int";
class BTypeast : public Baseast
{
public:
  string str;
  string Dump() override
  {
    return str;
  }
  int Calc() override
  {
    return 0;
  }
};
// ConstDef      ::= IDENT "=" ConstInitVal|IDENT "=" ConstInitVal ',' ConstDef;
class ConstDefast : public Baseast
{
public:
  unique_ptr<Baseast> constinitval;
  string ident;
  unique_ptr<Baseast> constdef;
  string Dump() override
  {
    return "";
  }
  int Calc() override
  {
    Symbolmap.insert({ident, {0, constinitval->Calc(), btype_str}});
    #ifdef DEBUG
    cout<<ident<<" "<<Symbolmap[ident].val<<endl;
    #endif
    if (kind == 2)
    {
      constdef->Calc();
    }
    return 0;
  }
};

// ConstInitVal  ::= ConstExp;
class ConstInitValast : public Baseast
{
public:
  unique_ptr<Baseast> constexp;
  string Dump() override
  {
    return "";
  }
  int Calc() override
  {
    return constexp->Calc();
  }
};

// ConstExp      ::= Exp;
class ConstExpast : public Baseast
{
public:
  unique_ptr<Baseast> exp;
  string Dump() override
  {
    return "";
  }
  int Calc() override
  {
    return exp->Calc();
  }
};

// VarDecl       ::= BType VarDef  ";";
class VarDeclast : public Baseast
{
public:
  unique_ptr<Baseast> btype;
  unique_ptr<Baseast> vardef;
  string Dump() override
  { // alloc
    btype_str = btype->Dump();
    return vardef->Dump();
  }
  int Calc() override
  {
    return 0;
  }
};

// VarDef        ::= (IDENT , | IDENT "=" InitVal) {',' VarDef};
class VarDefast : public Baseast
{
public:
  string ident;
  unique_ptr<Baseast> initval;
  unique_ptr<Baseast> vardef;
  string Dump() override
  { //@ident = alloc btype_str
    char temp[MAXCHARS] = {0};
    char temp2[MAXCHARS] = {0};
    char temp3[MAXCHARS] = {0};
    sprintf(temp3, "  @%s = alloc i32\n", ident.c_str());
    if (kind == 2 || kind == 4)
    {
      string initval_temp = initval->Dump();
      if(initval_temp[0]==' '){
        sprintf(temp2, "%s  store %%%d, @%s\n", initval_temp.c_str(),count_all ,ident.c_str());
      }
      else {
        sprintf(temp2,"  store %s, @%s\n",initval_temp.c_str(),ident.c_str());
      }
    }
      Symbolmap.insert({ident, {1, 0, btype_str}});//变量只需要保持符号
    if (kind == 3 || kind == 4)
      sprintf(temp, "%s%s%s", temp3, temp2, vardef->Dump().c_str());
    else
      sprintf(temp, "%s%s", temp3, temp2);

    return temp;
  }
  int Calc() override
  {
    return 0;
  }
};

// InitVal       ::= Exp;
class InitValast : public Baseast
{
public:
  unique_ptr<Baseast> exp;
  string Dump() override
  {
    return exp->Dump();
  }
  int Calc() override
  {
    return exp->Calc();
  }
};

// Block         ::= "{" BlockItem "}";
class Blockast : public Baseast
{
public:
  unique_ptr<Baseast> blockitem;

  string Dump() override
  {

    char temp[MAXCHARS] = {0};
    if (blockitem->kind != 1) // blockitem 不为空 为空返回空
    {
      string temp_stmt = blockitem->Dump();
      count = blockitem->count;
      sprintf(temp, "%%entry:\n%s", temp_stmt.c_str());
    }
    // cout<<"\%entry:"<<endl;
    // stmt->Dump();
    return temp;
  }

  int Calc() override
  {
    int temp = 0;
    blockitem->Calc();
    return temp;
  }
};
// BlockItem     ::=  |Decl BlockItem| Stmt BlockItem;
class BlockItemast : public Baseast
{
public:
  unique_ptr<Baseast> decl;
  unique_ptr<Baseast> stmt;
  unique_ptr<Baseast> blockitem;
  string Dump() override
  {
    string temp;
    if (kind == 1) //为空返回值无用
    {
      return "";
    }
    else if (kind == 2)
    {

      temp = decl->Dump() + blockitem->Dump();
    }
    else if (kind == 3)
    {
      temp = stmt->Dump() + blockitem->Dump();
    }
    return temp;
  }

  int Calc() override
  {
    int temp = 0;
    if (kind == 1)
    {
      //
    }
    else if (kind == 2)
    {
      decl->Calc();
      blockitem->Calc();
    }
    else if (kind == 3)
    {
      blockitem->Calc();
    }
    return temp;
  }
};

// LVal          ::= IDENT;
class LValast : public Baseast
{ //用符号表记录LVal和其对应的值
public:
  string ident;
  string Dump() override //返回name
  {
    return ident;
  }
  int Calc() override //返回value
  {

    return Symbolmap[ident].val;
  }
};

// PrimaryExp    ::= "(" Exp ")"  | Number| LVal;
class PrimaryExpast : public Baseast // lval 分为常量和变量 常量直接换成number 变量需要load
{
public:
  unique_ptr<Baseast> exp;
  int number;
  unique_ptr<Baseast> lval;

  string Dump() override
  {
    string temp;
    if (kind == 1)
    {
      temp = exp->Dump();
      count = exp->count;
    }
    else if (kind == 2)
    {
      temp = to_string(number);
      count = count_all;
    }
    else if (kind == 3)
    {
      string name = lval->Dump();
      Symbol lval_sym = Symbolmap[name];
      if (lval_sym.kind == 0)
      { // const常量
        temp = to_string(lval_sym.val);
      }
      else if (lval_sym.kind == 1)
      { // 变量
        char temp1[MAXCHARS] = {0};
        sprintf(temp1, "  %%%d = load @%s\n", count_all + 1, name.c_str());
        temp = temp1;
        count_all++;
      }
      count = count_all;
    }
    return temp;
  }

  int Calc() override
  {
    int temp = 0;
    if (kind == 1)
    {
      temp = exp->Calc();
    }
    else if (kind == 2)
    {
      temp = number;
    }
    else if (kind == 3)
    {
      temp = lval->Calc();
    }
    return temp;
  }
};

// FuncDef 也是 Baseast
class FuncDefast : public Baseast // FuncDef     ::= FuncType IDENT "(" ")" Block;
{
public:
  unique_ptr<Baseast> func_type;
  string ident;
  unique_ptr<Baseast> block;

  string Dump() override
  {
    char temp[MAXCHARS] = {0};
    string temp_func_type = func_type->Dump();
    string temp_block = block->Dump();
    count = block->count;
    sprintf(temp, "fun @%s(): %s{\n%s}\n", ident.c_str(), temp_func_type.c_str(), temp_block.c_str());

    return string(temp);
  }

  int Calc() override
  {
    int temp = 0;
    block->Calc();
    return temp;
  }
};

class FuncTypeast : public Baseast // FuncType    ::= "int";
{
public:
  string str;

  string Dump() override
  {
    char temp[MAXCHARS] = {0};
    sprintf(temp, "i32 ");
    return temp;
  }
  int Calc() override
  {
    return 0;
  }
};

class Stmtast : public Baseast // Stmt        ::= "return" Exp ";"|LVal "=" Exp ";";
{
public:
  unique_ptr<Baseast> exp;
  unique_ptr<Baseast> lval;
  string Dump() override
  {
    char temp[MAXCHARS] = {0};
    if (kind == 1)
    {
      string temp_exp = exp->Dump();
      count = exp->count;
      if (temp_exp[0] != ' ') //非表达式
      {
        sprintf(temp, "  ret %s\n", temp_exp.c_str());
      }
      else
      {
        sprintf(temp, "%s  ret %%%d\n", temp_exp.c_str(), count_all);
      }
    }
    else if (kind == 2) // load 在exp里面实现
    {                   // lval需要 store
      string temp_exp = exp->Dump();
      count = exp->count;
      if (temp_exp[0] != ' ') //非表达式
      {
        sprintf(temp, "  store %s, @%s\n", temp_exp.c_str(), lval->Dump().c_str());
      }
      else
      {
        sprintf(temp, "%s  store %%%d, @%s\n", temp_exp.c_str(), count_all, lval->Dump().c_str());
      }
    }
    return temp;
  }

  int Calc() override
  {
    return 0;
  }
};

class Expast : public Baseast // Exp         ::= LOrExp;
{
public:
  unique_ptr<Baseast> lorExp;

  string Dump() override
  {
    string temp = lorExp->Dump();
    count = lorExp->count;
    return temp;
  }
  int Calc() override
  {
    return lorExp->Calc();
  }
};

class UnaryExpast : public Baseast // UnaryExp    ::= PrimaryExp | UnaryOp(+ - !) UnaryExp;
{
public:
  unique_ptr<Baseast> primary;
  unique_ptr<Baseast> unaryop;
  unique_ptr<Baseast> unaryexp;

  string Dump() override
  {
    string temp;
    if (kind == 1)
    {
      temp = primary->Dump();
      count = primary->count;
    }
    else if (kind == 2)
    {
      if (unaryop->kind == 1)
      { //正,不需要处理
        temp = unaryexp->Dump();
        count = unaryexp->count;
      }
      else
      {
        temp = DumpUnaryOp(unaryexp, unaryexp->Dump().c_str(), unaryop->Dump().c_str());
        count_all++;
        count = count_all;
      }
    }
    return temp;
  }

  int Calc() override
  {
    int temp = 0;
    if (kind == 1)
    {
      temp = primary->Calc();
    }
    else if (kind == 2)
    {
      temp = unaryexp->Calc();
      if (unaryop->kind == 1)
      {
        // + 忽略
      }
      else if (unaryop->kind == 2)
      {
        temp = -temp;
      }
      else if (unaryop->kind == 3)
      {
        temp = (temp == 0 ? 0 : 1);
      }
    }
    return temp;
  }
};

class MulExpast : public Baseast // UnaryExp | MulExp ("*" | "/" | "%") UnaryExp;
{
public:
  unique_ptr<Baseast> unaryExp;
  unique_ptr<Baseast> mulop;
  unique_ptr<Baseast> mulExp;

  string Dump() override
  {
    string temp;
    if (kind == 1)
    {
      temp = unaryExp->Dump();
      count = unaryExp->count;
    }
    else if (kind == 2)
    {
      // unaryexp 优先参与运算
      temp = Dumpop(mulExp, unaryExp, unaryExp->Dump().c_str(), mulExp->Dump().c_str(), mulop->Dump().c_str());
      count_all++;
      count = count_all;

#ifdef DEBUG
      cout << temp;
#endif
    }
    return temp;
  }

  int Calc() override
  {
    int temp = 0;
    if (kind == 1)
    {
      temp = unaryExp->Calc();
    }
    else if (kind == 2)
    {
      temp = unaryExp->Calc();
      int temp2 = mulExp->Calc();
      if (mulop->kind == 1)
      {
        temp = temp2 * temp;
      }
      else if (mulop->kind == 2)
      {
        temp = temp2 / temp;
      }
      else if (mulop->kind == 3)
      {
        temp = temp2 % temp;
      }
    }
    return temp;
  }
};

class AddExpast : public Baseast // MulExp | AddExp ("+" | "-") MulExp
{
public:
  unique_ptr<Baseast> mulExp;
  unique_ptr<Baseast> addOp;
  unique_ptr<Baseast> addExp;
  string Dump() override
  {
    string temp;
    if (kind == 1)
    {
      temp = mulExp->Dump();
      count = mulExp->count;
    }
    else if (kind == 2)
    {
      temp = Dumpop(addExp, mulExp, mulExp->Dump().c_str(), addExp->Dump().c_str(), addOp->Dump().c_str());
      count_all++;
      count = count_all;

#ifdef DEBUG
      cout << temp << endl;
#endif
    }
    return temp;
  }

  int Calc() override
  {
    int temp = 0;
    if (kind == 1)
    {
      temp = mulExp->Calc();
    }
    else if (kind == 2)
    {
      temp = mulExp->Calc();
      int temp2 = addExp->Calc();
      if (addOp->kind == 1)
      {
        temp = temp2 + temp;
      }
      else if (addOp->kind == 2)
      {
        temp = temp2 - temp;
      }
    }
    return temp;
  }
};

// RelExp      ::= AddExp | RelExp ("<" | ">" | "<=" | ">=") AddExp;
class RelExpast : public Baseast
{
public:
  unique_ptr<Baseast> addexp;
  unique_ptr<Baseast> relexp;
  unique_ptr<Baseast> relop;
  string Dump() override
  {
    string temp;
    if (kind == 1)
    {
      temp = addexp->Dump();
      count = addexp->count;
    }
    else if (kind == 2)
    {
      temp = Dumpop(relexp, addexp, addexp->Dump().c_str(), relexp->Dump().c_str(), relop->Dump().c_str());
      count_all++;
      count = count_all;
#ifdef DEBUG
      cout << temp << endl;
#endif
    }
    return temp;
  }

  int Calc() override
  {
    int temp = 0;
    if (kind == 1)
    {
      temp = addexp->Calc();
    }
    else if (kind == 2)
    {
      temp = addexp->Calc();
      int temp_rel = relexp->Calc();
      if (relop->kind == 1)
      {
        temp = (temp_rel < temp);
      }
      else if (relop->kind == 2)
      {
        temp = (temp_rel > temp);
      }
      else if (relop->kind == 3)
      {
        temp = (temp_rel <= temp);
      }
      else if (relop->kind == 4)
      {
        temp = (temp_rel >= temp);
      }
    }
    return temp;
  }
};

// EqExp       ::= RelExp | EqExp ("==" | "!=") RelExp;
class EqExpast : public Baseast
{
public:
  unique_ptr<Baseast> relexp;
  unique_ptr<Baseast> eqexp;
  unique_ptr<Baseast> eqop;

  string Dump() override
  {
    string temp;
    if (kind == 1)
    {
      temp = relexp->Dump();
      count = relexp->count;
    }
    else if (kind == 2)
    {
      temp = Dumpop(eqexp, relexp, relexp->Dump().c_str(), eqexp->Dump().c_str(), eqop->Dump().c_str());
      count_all++;
      count = count_all;

#ifdef DEBUG
      cout << temp << endl;
#endif
    }
    return temp;
  }

  int Calc() override
  {
    int temp = 0;
    if (kind == 1)
    {
      temp = relexp->Calc();
    }
    else if (kind == 2)
    {
      temp = relexp->Calc();
      if (eqop->kind == 1)
        temp = (eqexp->Calc() == temp);
      else if (eqop->kind == 2)
        temp = (eqexp->Calc() != temp);
    }
    return temp;
  }
};

// LAndExp     ::= EqExp | LAndExp "&&" EqExp;
class LAndExpast : public Baseast
{
public:
  unique_ptr<Baseast> eqexp;
  unique_ptr<Baseast> op;
  unique_ptr<Baseast> landexp;
  string Dump() override
  {
    string temp;
    if (kind == 1)
    {
      temp = eqexp->Dump();
      count = eqexp->count;
    }
    else if (kind == 2)
    {
      // 先用ne 将数值转换为逻辑
      string temp_eqexp = DumpUnaryOp(eqexp, eqexp->Dump().c_str(), "ne");
      count_all++;
      eqexp->count = count_all;

      string temp_landexp = DumpUnaryOp(landexp, landexp->Dump().c_str(), "ne");
      count_all++;
      landexp->count = count_all;

      temp = Dumpop(landexp, eqexp, temp_eqexp.c_str(), temp_landexp.c_str(), "and");
      count_all++;
      count = count_all;

#ifdef DEBUG
      cout << temp;
#endif
    }
    return temp;
  }

  int Calc() override
  {
    int temp = 0;
    if (kind == 1)
    {
      temp = eqexp->Calc();
    }
    else if (kind == 2)
    {
      temp = eqexp->Calc();
      temp = landexp->Calc() && temp;
    }
    return temp;
  }
};

// LOrExp      ::= LAndExp | LOrExp "||" LAndExp;
class LOrExpast : public Baseast
{
public:
  unique_ptr<Baseast> landexp;
  unique_ptr<Baseast> op;
  unique_ptr<Baseast> lorexp;
  string Dump() override
  {
    string temp;
    if (kind == 1)
    {
      temp = landexp->Dump();
      count = landexp->count;
    }
    else if (kind == 2)
    {
      temp = Dumpop(lorexp, landexp, landexp->Dump().c_str(), lorexp->Dump().c_str(), "or");
      count_all++;
      count = count_all;

      //按位或和逻辑或相同 将结果转换为逻辑
      unique_ptr<Baseast> temp_ptr = unique_ptr<Baseast>(this);
      temp = DumpUnaryOp(temp_ptr, temp.c_str(), "ne");
      temp_ptr.release();
      count_all++;
      count = count_all;

#ifdef DEBUG
      cout << temp;
#endif
    }
    return temp;
  }

  int Calc() override
  {
    int temp = 0;
    if (kind == 1)
    {
      temp = landexp->Calc();
    }
    else if (kind == 2)
    {
      temp = landexp->Calc();
      temp = lorexp->Calc() || temp;
    }
    return temp;
  }
};

class Opast : public Baseast
{
public:
  string str;
  string Dump() override
  {
    return str;
  }
  int Calc() override
  {
    return 0;
  }
};

// ...

// 函数声明略
void Visit(const koopa_raw_slice_t &slice);
void Visit(const koopa_raw_function_t &func);
void Visit(const koopa_raw_basic_block_t &bb);
void Visit(const koopa_raw_value_t &value);
void Visit(const koopa_raw_program_t &program);
void AnalyzeIR(const char *str);

#endif
