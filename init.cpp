#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif

#include<unordered_map>
#include<unordered_set>
#include<string>
#include<vector>
#include "lexlib.h"


using namespace std;


unordered_map<string, string> wordName2typeCode;
unordered_set<string> wordNameSet;
vector<Myword> word_get_list;


void initMap()
{
    wordName2typeCode["const"] = "CONSTTK";
    wordName2typeCode["int"] = "INTTK";
    wordName2typeCode["char"] = "CHARTK";
    wordName2typeCode["void"] = "VOIDTK";
    wordName2typeCode["main"] = "MAINTK";
    wordName2typeCode["if"] = "IFTK";
    wordName2typeCode["else"] = "ELSETK";
    wordName2typeCode["switch"] = "SWITCHTK";
    wordName2typeCode["case"] = "CASETK";
    wordName2typeCode["default"] = "DEFAULTTK";
    wordName2typeCode["while"] = "WHILETK";
    wordName2typeCode["for"] = "FORTK";
    wordName2typeCode["scanf"] = "SCANFTK";
    wordName2typeCode["printf"] = "PRINTFTK";
    wordName2typeCode["return"] = "RETURNTK";
    wordName2typeCode["+"] = "PLUS";
    wordName2typeCode["-"] = "MINU";
    wordName2typeCode["*"] = "MULT";
    wordName2typeCode["/"] = "DIV";
    wordName2typeCode["<"] = "LSS";
    wordName2typeCode["<="] = "LEQ";
    wordName2typeCode[">"] = "GRE";
    wordName2typeCode[">="] = "GEQ";
    wordName2typeCode["="] = "ASSIGN";
    wordName2typeCode["=="] = "EQL";
    wordName2typeCode["!="] = "NEQ";
    wordName2typeCode[":"] = "COLON";
    wordName2typeCode[";"] = "SEMICN";
    wordName2typeCode[","] = "COMMA";
    wordName2typeCode["("] = "LPARENT";
    wordName2typeCode[")"] = "RPARENT";
    wordName2typeCode["{"] = "LBRACE";
    wordName2typeCode["}"] = "RBRACE";
    wordName2typeCode["["] = "LBRACK";
    wordName2typeCode["]"] = "RBRACK";
}

void initSet()
{
	wordNameSet.insert("const");
	wordNameSet.insert("int");
	wordNameSet.insert("char");
	wordNameSet.insert("void");
	wordNameSet.insert("main");
	wordNameSet.insert("if");
	wordNameSet.insert("else");
	wordNameSet.insert("switch");
	wordNameSet.insert("case");
	wordNameSet.insert("default");
	wordNameSet.insert("while");
	wordNameSet.insert("for");
	wordNameSet.insert("scanf");
	wordNameSet.insert("printf");
	wordNameSet.insert("return");
	wordNameSet.insert("+");
	wordNameSet.insert("-");
	wordNameSet.insert("*");
	wordNameSet.insert("/");
	wordNameSet.insert("<");
	wordNameSet.insert("<=");
	wordNameSet.insert(">");
	wordNameSet.insert(">=");
	wordNameSet.insert("=");
	wordNameSet.insert("==");
	wordNameSet.insert("!=");
	wordNameSet.insert(":");
	wordNameSet.insert(",");
	wordNameSet.insert("=");
	wordNameSet.insert(";");
	wordNameSet.insert("(");
	wordNameSet.insert(")");
	wordNameSet.insert("[");
	wordNameSet.insert("]");
	wordNameSet.insert("{");
	wordNameSet.insert("}");
}
