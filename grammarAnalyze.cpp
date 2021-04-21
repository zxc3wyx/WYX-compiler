#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif

#include<vector>
#include<set>
#include <utility>
#include <map>
#include <cstdlib>
#include <string>
#include "lexlib.h"
#include "gralib.h"
#include "macrodef.h"
#include "errorHandler.h"
#include "mips.h"

#define E_WRONG_FORMAT 1
#define END 2


using std::vector;
using std::set;
using std::map;
using std::exception;
using std::string;

extern vector<Myword> word_get_list;

set<Term> term_set;

set<string> func_have_ret;

map<Term, int> termOrder;

static int term_cnt = 0;

vector<pair<string, sym_type>> localSymStack;                //符号栈，<IDENFR, type>

vector<pair<string, sym_type>> globalSymStack;

bool isInLocal = false;                                       //是否在局部

map<string, vector<sym_type>> paramOfFunction;                 //函数包含的参数类型

int argumentNum = 0;

string currentFunction;                                         //当前函数

vector<pair<int, int>> returnSentenceList;                   //返回语句列表<位置，长度>

map<string, int> array2width;                                   //二维数组宽度

string switchType;

vector<string> array_identifier_list;

string branchOp;

extern int switchno;
extern int whileno;
extern int forno;
extern int local_offset;
extern vector<string> textField;

static int temp_var_no;
extern vector<vector<string>> midExprList;               //中间表达式序列
string last_poly_name;
string last_expr_name;
string last_factor_name;
string b_left_expr;
string b_right_expr;

bool isCharRetExpression(int startIndex, int len)                     //判断表达式的值是不是字符型
{
    int getFuncRetCallSentence(int);
    int ret;
    if (len == 1 && word_get_list[startIndex].type == "CHARCON")
        return true;
    else if ((ret = getFuncRetCallSentence(startIndex))) {
        if (ret == len) {
            for (auto &i : globalSymStack) {
                if (i.first == word_get_list[startIndex].word && i.second == func_char)
                    return true;
            }
        }
    }
    else {
        int getExpression(int);
        int status = 0, i = 0;
        while (i < len && status != -E_WRONG_FORMAT && status != 7) {
            switch (status) {
                case 0:
                    status = (word_get_list[i + startIndex].type == "IDENFR") ? 1 : -E_WRONG_FORMAT;
                    i++;
                    break;
                case 1:
                    status = (word_get_list[i + startIndex].type == "LBRACK") ? 2 : -E_WRONG_FORMAT;
                    i += (word_get_list[i + startIndex].type == "LBRACK") ? 1 : 0;
                    break;
                case 2:
                    if ((ret = getExpression(i + startIndex)) != 0)
                    {
                        i += ret;
                        status = 3;							//a[<>
                    }
                    else {
                        status = -E_WRONG_FORMAT;
                    }
                    break;
                case 3:
                    status = (word_get_list[i + startIndex].type == "RBRACK") ? 4 : -E_WRONG_FORMAT;
                    i++;
                    break;
                case 4:
                    status = (word_get_list[i + startIndex].type == "LBRACK") ? 5 : 7;
                    i += (word_get_list[i + startIndex].type == "LBRACK") ? 1 : 0;
                    break;
                case 5:
                    if ((ret = getExpression(i + startIndex)) != 0)
                    {
                        i += ret;
                        status = 6;							//a[<>][<>
                    }
                    else {
                        status = -E_WRONG_FORMAT;
                    }
                    break;
                case 6:
                    status = (word_get_list[i + startIndex].type == "RBRACK") ? 7 : -E_WRONG_FORMAT;
                    i++;
                    break;
                default:
                    break;
            }
        }
        if (status == -END || status == 1) {
            for (const pair <string, sym_type>& id: localSymStack) {
                if (id.first == word_get_list[startIndex].word && (id.second == var_char || id.second == const_char))
                    return true;
                if (id.first == word_get_list[startIndex].word && (id.second == var_int || id.second == const_int))
                    return false;
            }
            for (const pair <string, sym_type>& id: globalSymStack) {
                if (id.first == word_get_list[startIndex].word && (id.second == var_char || id.second == const_char))
                    return true;
            }
            return false;
        }
        if (status == 4 || status == 7) {
            for (const pair <string, sym_type>& id: localSymStack) {
                if (id.first == word_get_list[startIndex].word && id.second == var_char)
                    return true;
                if (id.first == word_get_list[startIndex].word && id.second == var_int)
                    return false;
            }
            for (const pair <string, sym_type>& id: globalSymStack) {
                if (id.first == word_get_list[startIndex].word && id.second == var_char)
                    return true;
            }
            return false;
        }
    }
    return false;
}

bool searchInLocal(const string& identifier) {
    for (const pair<string, sym_type>& id: localSymStack) {
        if (id.first == identifier) {
            return true;
        }
    }
    return false;
}

bool searchInGlobal(const string& identifier) {
    for (const pair<string, sym_type>& id: globalSymStack) {
        if (id.first == identifier) {
            return true;
        }
    }
    return false;
}

sym_type getSymType(const string& identifier) {
    if (isInLocal) {
        for (const pair<string, sym_type>& id: localSymStack) {
            if (id.first == identifier) {
                return id.second;
            }
        }
        for (const pair<string, sym_type>& id: globalSymStack) {
            if (id.first == identifier) {
                return id.second;
            }
        }
    }
    for (const pair<string, sym_type>& id: globalSymStack) {
        if (id.first == identifier) {
            return id.second;
        }
    }
    return unknown;
}

Term newTerm(const string& type, int lastIndex, int length) {		//构造语法成分
	pair<int, int>index = std::make_pair(lastIndex, length);
	Term term = std::make_pair(type, index);
	if (term_set.find(term) == term_set.end()) {
        termOrder[term] = term_cnt;
        term_cnt++;
	}
	return term;
}

string termGetType(const pair<string, pair<int, int>>& term) {

    return term.first;
}

int termGetLastIndex(const pair<string, pair<int, int>>& term)
{
	return term.second.first;
}

int termGetLength(const pair<string, pair<int, int>>& term)
{
	return term.second.second;
}

bool getUnsignedInt(int startIndex)			//无符号整数
{
	if (word_get_list[startIndex].type == "INTCON") {
		term_set.insert(newTerm(WFHZS, startIndex, 1));
		return true;
	}
	return false;
}

bool isRelationOperator(int index)			//关系运算符
{
	string type = word_get_list[index].type;
	return (type == "EQL" || type == "NEQ" || type == "LSS"
		|| type == "LEQ" || type == "GRE" || type == "GEQ");
}

bool getString(int index)
{
	if (word_get_list[index].type == "STRCON") {
		term_set.insert(newTerm(ZFC, index, 1));
		return true;
	}
	return false;
}

int getInt(int startIndex)					//整数，返回包含单词个数
{
	if ((word_get_list[startIndex].type == "PLUS" || word_get_list[startIndex].type == "MINU")
		&& getUnsignedInt(startIndex + 1))
	{
		term_set.insert(newTerm(ZS, startIndex + 1, 2));
		return 2;
	}
	else if (getUnsignedInt(startIndex))
	{
		term_set.insert(newTerm(ZS, startIndex, 1));
		return 1;
	}
	return 0;
}

int getConst(int startIndex)				//常量
{
	int tmp;
	if (word_get_list[startIndex].type == "CHARCON")
	{
		term_set.insert(newTerm(CL, startIndex, 1));
		return 1;
	}
	else if ((tmp = getInt(startIndex)) != 0) {
		term_set.insert(newTerm(CL, startIndex + tmp - 1, tmp));
		return tmp;
	}
	return 0;
}

int judgeArgumentNoInit(int startIndex)
{
	int ret;
	int status = 0, i = startIndex;
	while (status != -E_WRONG_FORMAT && status != -END && i < word_get_list.size()) {
		switch (status)
		{
		case 0:
			status = (word_get_list[i].type == "CHARTK" || word_get_list[startIndex].type == "INTTK") ? 1 : -E_WRONG_FORMAT;
			i++;
			break;
		case 1:
			status = (word_get_list[i].type == "IDENFR") ? 2 : -E_WRONG_FORMAT;
			i++;
			break;
		case 2:
			status = (word_get_list[i].type == "COMMA") ? 1 :
				(word_get_list[i].type == "LBRACK") ? 3 :
				(word_get_list[i].type == "ASSIGN") ? -E_WRONG_FORMAT : -END;
			i += (word_get_list[i].type == "COMMA" || word_get_list[i].type == "LBRACK") ? 1 : 0;
			break;
		case 3:
			if ((ret = getUnsignedInt(i)) != 0)
			{
				i += ret;
				status = 4;
			}
			else
				status = -E_WRONG_FORMAT;
			break;
		case 4:
            if (word_get_list[i].type == "RBRACK") {
                status = 5;
                i++;
            } else {
                status = 5;
                missingRbrack(getWordLine(i));
            }
                break;
		case 5:
			status = (word_get_list[i].type == "LBRACK") ? 6 :
				(word_get_list[i].type == "COMMA") ? 1 :
				(word_get_list[i].type == "ASSIGN") ? -E_WRONG_FORMAT : -END;
			i += (word_get_list[i].type == "LBRACK" || word_get_list[i].type == "COMMA") ? 1 : 0;
			break;
		case 6:
			if ((ret = getUnsignedInt(i)) != 0)
			{
				i += ret;
				status = 7;
			}
			else
				status = -E_WRONG_FORMAT;
			break;
		case 7:
			if (word_get_list[i].type == "RBRACK") {
			    status = 8;
			    i++;
			} else {
			    status = 8;
                missingRbrack(getWordLine(i));
			}
			break;
		case 8:
			status = (word_get_list[i].type == "COMMA") ? 1 : 
				(word_get_list[i].type == "ASSIGN") ? -E_WRONG_FORMAT : -END;
			i += (word_get_list[i].type == "COMMA") ? 1 : 0;
			break;
		default:
			break;
		}
	}
	if (status != -E_WRONG_FORMAT) {
		return i - startIndex;
	}
	return 0;
}

int getArgumentNoInit(int startIndex)		//变量定义无初始化
{
	int ret;
	if ((ret = judgeArgumentNoInit(startIndex)) != 0) {
        bool b = isInLocal ? searchInLocal(word_get_list[startIndex + 1].word) :
                searchInGlobal(word_get_list[startIndex + 1].word);
        if (b) {
            duplicatedIdfName(getWordLine(startIndex + 1));
        }
        sym_type type = word_get_list[startIndex].type == "INTTK" ? var_int : var_char;
        if (isInLocal) {
            if (word_get_list[startIndex + 2].type == "COMMA" || ret == 2) {
                local_normal_var(word_get_list[startIndex + 1].word, 0);
            } else {
                if (word_get_list[startIndex + 5].type == "LBRACK") {
                    int size1 = atoi(word_get_list[startIndex + 3].word.c_str());
                    int size2 = atoi(word_get_list[startIndex + 6].word.c_str());
                    int* init = new int [size1 * size2]();
                    local_array_var(word_get_list[startIndex + 1].word, size1 * size2, init);
                    array2width[word_get_list[startIndex + 1].word] = size2;
                }
                else {
                    int size = atoi(word_get_list[startIndex + 3].word.c_str());
                    int* init = new int [size]();
                    local_array_var(word_get_list[startIndex + 1].word, size, init);
                }
            }
            localSymStack.emplace_back(word_get_list[startIndex + 1].word, type);
        } else {
            if (word_get_list[startIndex + 2].type == "COMMA" || ret == 2) {
                global_normal_var(word_get_list[startIndex + 1].word, 0);
            } else {
                if (word_get_list[startIndex + 5].type == "LBRACK") {
                    int size1 = atoi(word_get_list[startIndex + 3].word.c_str());
                    int size2 = atoi(word_get_list[startIndex + 6].word.c_str());
                    global_array_var_no_init(word_get_list[startIndex + 1].word, size1 * size2);
                    array2width[word_get_list[startIndex + 1].word] = size2;
                }
                else {
                    int size = atoi(word_get_list[startIndex + 3].word.c_str());
                    global_array_var_no_init(word_get_list[startIndex + 1].word, size);
                }
            }
            globalSymStack.emplace_back(word_get_list[startIndex + 1].word, type);
        }
        for(int j = startIndex + 1; j < startIndex + ret; j++) {
            if (word_get_list[j].type == "COMMA") {
                b = isInLocal ? searchInLocal(word_get_list[j + 1].word) :
                         searchInGlobal(word_get_list[j + 1].word);
                if (b) {
                    duplicatedIdfName(getWordLine(j + 1));
                }
                if (isInLocal) {
                    if (word_get_list[j + 2].type == "COMMA" || word_get_list[j + 2].type == "SEMICN") {
                        local_normal_var(word_get_list[j + 1].word, 0);
                    } else {
                        if (word_get_list[j + 5].type == "LBRACK") {
                            int size1 = atoi(word_get_list[j + 3].word.c_str());
                            int size2 = atoi(word_get_list[j + 6].word.c_str());
                            int* init = new int [size1 * size2]();
                            local_array_var(word_get_list[j + 1].word, size1 * size2, init);
                            array2width[word_get_list[j + 1].word] = size2;
                        }
                        else {
                            int size = atoi(word_get_list[j + 3].word.c_str());
                            int* init = new int [size]();
                            local_array_var(word_get_list[j + 1].word, size, init);
                        }
                    }
                    localSymStack.emplace_back(word_get_list[j + 1].word, type);
                } else {
                    if (word_get_list[j + 2].type == "COMMA" || word_get_list[j + 2].type == "SEMICN") {
                        global_normal_var(word_get_list[j + 1].word, 0);
                    } else {
                        if (word_get_list[j + 5].type == "LBRACK") {
                            int size1 = atoi(word_get_list[j + 3].word.c_str());
                            int size2 = atoi(word_get_list[j + 6].word.c_str());
                            global_array_var_no_init(word_get_list[j + 1].word, size1 * size2);
                            array2width[word_get_list[j + 1].word] = size2;
                        }
                        else {
                            int size = atoi(word_get_list[j + 3].word.c_str());
                            global_array_var_no_init(word_get_list[j + 1].word, size);
                        }
                    }
                    globalSymStack.emplace_back(word_get_list[j + 1].word, type);
                }
            }
        }
		term_set.insert(newTerm(BLDYWCSH, ret + startIndex - 1, ret));
		return ret;
	}
	return 0;
}

int judgeArgumentInit(int startIndex)
{
    int ret;
    int status = 0, i = startIndex;
    while (status != -E_WRONG_FORMAT && status != -END && i < word_get_list.size()) {
        switch (status)
        {
            case 0:
                status = (word_get_list[i].type == "CHARTK" || word_get_list[i].type == "INTTK") ? 1 : -E_WRONG_FORMAT;
                i++;
                break;
            case 1:
                status = (word_get_list[i].type == "IDENFR") ? 2 : -E_WRONG_FORMAT;
                i++;
                break;
            case 2:
                status = (word_get_list[i].type == "ASSIGN") ? 3 :
                         (word_get_list[i].type == "LBRACK") ? 4 : -E_WRONG_FORMAT;
                i++;
                break;
            case 3:
                if ((ret = getConst(i)) != 0)
                {
                    i += ret;
                    status = -END;
                }
                else
                    status = -E_WRONG_FORMAT;
                break;
            case 4:
                if ((ret = getUnsignedInt(i)) != 0)
                {
                    i += ret;
                    status = 5;
                }
                else
                    status = -E_WRONG_FORMAT;
                break;
            case 5:
                if (word_get_list[i].type == "RBRACK") {
                    status = 6;
                    i++;
                } else {
                    status = 6;
                    missingRbrack(getWordLine(i));
                }
                break;
            case 6:
                status = (word_get_list[i].type == "ASSIGN") ? 7 :
                         (word_get_list[i].type == "LBRACK") ? 10 : -E_WRONG_FORMAT;
                i++;
                break;
            case 7:
                status = (word_get_list[i].type == "LBRACE") ? 8 : -E_WRONG_FORMAT;
                i++;
                break;
            case 8:
                if ((ret = getConst(i)) != 0)
                {
                    status = 9;
                    i += ret;
                }
                else
                    status = -E_WRONG_FORMAT;
                break;
            case 9:
                status = (word_get_list[i].type == "COMMA") ? 8 :
                         (word_get_list[i].type == "RBRACE") ? -END : -E_WRONG_FORMAT;
                i++;
                break;
            case 10:
                status = (getUnsignedInt(i)) ? 11 : -E_WRONG_FORMAT;
                i++;
                break;
            case 11:
                if (word_get_list[i].type == "RBRACK") {
                    status = 12;
                    i++;
                } else {
                    status = 12;
                    missingRbrack(getWordLine(i));
                }
                break;
            case 12:
                status = (word_get_list[i].type == "ASSIGN") ? 13 : -E_WRONG_FORMAT;
                i++;
                break;
            case 13:
                status = (word_get_list[i].type == "LBRACE") ? 14 : -E_WRONG_FORMAT;
                i++;
                break;
            case 14:
                status = (word_get_list[i].type == "LBRACE") ? 15 : -E_WRONG_FORMAT;
                i++;
                break;
            case 15:
                if ((ret = getConst(i)) != 0)
                {
                    status = 16;
                    i += ret;
                }
                else
                    status = -E_WRONG_FORMAT;
                break;
            case 16:
                status = (word_get_list[i].type == "COMMA") ? 15 :
                         (word_get_list[i].type == "RBRACE") ? 17 : -E_WRONG_FORMAT;
                i++;
                break;
            case 17:
                status = (word_get_list[i].type == "COMMA") ? 14 :
                         (word_get_list[i].type == "RBRACE") ? -END : -E_WRONG_FORMAT;
                i++;
                break;
            default:
                break;
        }
    }
    if (status != -E_WRONG_FORMAT) {
        return i - startIndex;
    }
    return 0;
}

int getArgumentInit(int startIndex)			//变量定义及初始化
{
    int ret;
    int status = 0, i = startIndex;
    int dimension1 = -1, dimension2 = -1;                   //期望维数
    int dimension1_get = 0, dimension2_get = 0;             //实际初始化位数
    string type;
    int initValue = 0;
    vector<int> initList;
    while (status != -E_WRONG_FORMAT && status != -END && i < word_get_list.size()) {
        switch (status)
        {
            case 0:
                status = (word_get_list[i].type == "CHARTK" || word_get_list[i].type == "INTTK") ? 1 : -E_WRONG_FORMAT;
                if (word_get_list[i].type == "CHARTK" || word_get_list[i].type == "INTTK") {
                    type = word_get_list[i].type;
                }
                i++;
                break;
            case 1:
                status = (word_get_list[i].type == "IDENFR") ? 2 : -E_WRONG_FORMAT;
                if (word_get_list[i].type == "IDENFR") {
                    bool b = isInLocal ? searchInLocal(word_get_list[i].word) : searchInGlobal(word_get_list[i].word);
                    if (b) {
                        duplicatedIdfName(getWordLine(i));
                    }
                }
                i++;
                break;
            case 2:
                status = (word_get_list[i].type == "ASSIGN") ? 3 :
                         (word_get_list[i].type == "LBRACK") ? 4 : -E_WRONG_FORMAT;
                i++;
                break;
            case 3:
                if ((ret = getConst(i)) != 0)
                {
                    if (word_get_list[i].type == "CHARCON" && type == "INTTK") {
                        constTypeNoMatch(getWordLine(i));
                    }
                    if (word_get_list[i].type != "CHARCON" && type == "CHARTK") {
                        constTypeNoMatch(getWordLine(i));
                    }
                    if (type == "INTTK") {
                        string basic = string();
                        for (int j = i; j < i + ret; j++) {
                            basic += word_get_list[j].word;
                        }
                        initValue = atoi(basic.c_str());
                    } else {
                        initValue = (int)(word_get_list[i].word.at(0));
                    }
                    i += ret;
                    status = -END;
                }
                else
                    status = -E_WRONG_FORMAT;
                break;
            case 4:
                if ((ret = getUnsignedInt(i)) != 0)
                {
                    dimension1 = atoi(word_get_list[i].word.c_str());
                    i += ret;
                    status = 5;
                }
                else
                    status = -E_WRONG_FORMAT;
                break;
            case 5:
                if (word_get_list[i].type == "RBRACK") {
                    status = 6;
                    i++;
                } else {
                    status = 6;
                    missingRbrack(getWordLine(i));
                }
                break;
            case 6:
                status = (word_get_list[i].type == "ASSIGN") ? 7 :
                         (word_get_list[i].type == "LBRACK") ? 10 : -E_WRONG_FORMAT;
                i++;
                break;
            case 7:
                status = (word_get_list[i].type == "LBRACE") ? 8 : -E_WRONG_FORMAT;
                i++;
                break;
            case 8:
                if ((ret = getConst(i)) != 0)
                {
                    if (ret == 1) {
                        if (word_get_list[i].type == "CHARCON") {
                            initList.emplace_back((int)(word_get_list[i].word.at(0)));
                        }
                        else {
                            initList.emplace_back(atoi(word_get_list[i].word.c_str()));
                        }
                    } else {
                        string s = string();
                        s += word_get_list[i].word;
                        s += word_get_list[i+1].word;
                        initList.emplace_back(atoi(s.c_str()));
                    }
                    status = 9;
                    dimension1_get++;
                    i += ret;
                }
                else
                    status = -E_WRONG_FORMAT;
                break;
            case 9:
                status = (word_get_list[i].type == "COMMA") ? 8 :
                         (word_get_list[i].type == "RBRACE") ? -END : -E_WRONG_FORMAT;
                if (word_get_list[i].type == "RBRACE") {
                    if (dimension1 != dimension1_get) {
                        arrayInitNoMatch(getWordLine(i));
                    }
                    dimension1_get = 0;
                }
                i++;
                break;
            case 10:
                if ((ret = getUnsignedInt(i)) != 0)
                {
                    dimension2 = atoi(word_get_list[i].word.c_str());
                    array2width[word_get_list[startIndex + 1].word] = dimension2;
                    i += ret;
                    status = 11;
                }
                else
                    status = -E_WRONG_FORMAT;
                break;
            case 11:
                if (word_get_list[i].type == "RBRACK") {
                    status = 12;
                    i++;
                } else {
                    status = 12;
                    missingRbrack(getWordLine(i));
                }
                break;
            case 12:
                status = (word_get_list[i].type == "ASSIGN") ? 13 : -E_WRONG_FORMAT;
                i++;
                break;
            case 13:
                status = (word_get_list[i].type == "LBRACE") ? 14 : -E_WRONG_FORMAT;
                i++;
                break;
            case 14:
                status = (word_get_list[i].type == "LBRACE") ? 15 : -E_WRONG_FORMAT;
                i++;
                break;
            case 15:
                if ((ret = getConst(i)) != 0)
                {
                    if (ret == 1) {
                        if (word_get_list[i].type == "CHARCON") {
                            initList.emplace_back((int)(word_get_list[i].word.at(0)));
                        }
                        else {
                            initList.emplace_back(atoi(word_get_list[i].word.c_str()));
                        }
                    } else {
                        string s = string();
                        s += word_get_list[i].word;
                        s += word_get_list[i+1].word;
                        initList.emplace_back(atoi(s.c_str()));
                    }
                    status = 16;
                    i += ret;
                    dimension2_get++;
                }
                else
                    status = -E_WRONG_FORMAT;
                break;
            case 16:
                status = (word_get_list[i].type == "COMMA") ? 15 :
                         (word_get_list[i].type == "RBRACE") ? 17 : -E_WRONG_FORMAT;
                if (word_get_list[i].type == "RBRACE") {
                    if (dimension2_get != dimension2) {
                        arrayInitNoMatch(getWordLine(i));
                    }
                    dimension1_get++;
                    dimension2_get = 0;
                }
                i++;
                break;
            case 17:
                status = (word_get_list[i].type == "COMMA") ? 14 :
                         (word_get_list[i].type == "RBRACE") ? -END : -E_WRONG_FORMAT;
                if (word_get_list[i].type == "RBRACE") {
                    if (dimension1_get != dimension1) {
                        arrayInitNoMatch(getWordLine(i));
                    }
                }
                i++;
                break;
            default:
                break;
        }
    }
	if (status != -E_WRONG_FORMAT) {
        term_set.insert(newTerm(BLDYJCSH, i - 1, i - startIndex));
        sym_type stype = word_get_list[startIndex].type == "INTTK" ? var_int : var_char;
        if (isInLocal) {
            localSymStack.emplace_back(word_get_list[startIndex + 1].word, stype);
            if (dimension1 != -1) {
                if (dimension2 == -1) {
                    int* init = new int [dimension1]();
                    for (int j = 0; j < dimension1; j++) {
                        init[j] = initList[j];
                    }
                    local_array_var(word_get_list[startIndex + 1].word, dimension1, init);
                } else {
                    int* init = new int [dimension1 * dimension2]();
                    for (int j = 0; j < dimension1 * dimension2; j++) {
                        init[j] = initList[j];
                    }
                    local_array_var(word_get_list[startIndex + 1].word, dimension1 * dimension2, init);
                }
            }
            else {
                local_normal_var(word_get_list[startIndex + 1].word, initValue);
            }
        } else {
            globalSymStack.emplace_back(word_get_list[startIndex + 1].word, stype);
            if (dimension1 != -1) {
                if (dimension2 == -1) {
                    global_array_var_init(word_get_list[startIndex + 1].word, dimension1, initList);
                } else {
                    global_array_var_init(word_get_list[startIndex + 1].word, dimension1 * dimension2, initList);
                }
            } else {
                global_normal_var(word_get_list[startIndex + 1].word, initValue);
            }
        }
	}
    return i - startIndex;
}

int judgeArgumentDefine(int startIndex)
{
	int ret;
	if ((ret = judgeArgumentNoInit(startIndex)) != 0) {
		return ret;
	}
	else if ((ret = judgeArgumentInit(startIndex)) != 0) {
		return ret;
	}
	return 0;
}

int getArgumentDefine(int startIndex)					//变量定义
{
	int ret;
	if ((ret = judgeArgumentNoInit(startIndex)) != 0) {
		getArgumentNoInit(startIndex);
		term_set.insert(newTerm(BLDY, ret + startIndex - 1, ret));
		return ret;
	} else if ((ret = judgeArgumentInit(startIndex)) != 0) {
        getArgumentInit(startIndex);
        term_set.insert(newTerm(BLDY, ret + startIndex - 1, ret));
        return ret;
	}
	return 0;
}

int getArgumentTK(int startIndex)			//变量说明
{
	int status = 0, i = startIndex;
	int ret = 0;
	while (status != -E_WRONG_FORMAT && status != -END && i < word_get_list.size()) {
		switch (status)
		{
		case 0:
			if ((ret = judgeArgumentDefine(i)) != 0) {
				i += ret;
				status = 1;
			}
			else {
				status = -E_WRONG_FORMAT;
			}
			break;
		case 1:
			if (word_get_list[i].type == "SEMICN") {
				getArgumentDefine(i - ret);
				status = 2;
				i++;
			}
			else if (word_get_list[i].type != "LPARENT") {
                getArgumentDefine(i - ret);
                status = 2;
                missingSemicn(getWordLine(i - 1));
			}
			else {
				status = -END;
				i -= ret;					//回退
			}
			break;
		case 2:
			if ((ret = judgeArgumentDefine(i)) != 0) {
				i += ret;
				status = 1;
			}
			else {
				status = -END;
			}
			break;
		default:
			break;
		}
	}
	if (status != -E_WRONG_FORMAT && i > startIndex) {
		term_set.insert(newTerm(BLSM, i - 1, i - startIndex));
		return i - startIndex;
	}
	return 0;
}

int getTKHeader(int startIndex)				//声明头部
{
	int status = 0, i = startIndex;
	while (status != -E_WRONG_FORMAT && status != 2 && i < word_get_list.size()) {
		switch (status) {
		case 0:
			status = (word_get_list[i].type == "INTTK" ||
				word_get_list[i].type == "CHARTK") ? 1 : -E_WRONG_FORMAT;
			break;
		case 1:
			status = (word_get_list[i].type == "IDENFR") ? 2 : -E_WRONG_FORMAT;
            if (word_get_list[i].type == "IDENFR") {
                if (searchInGlobal(word_get_list[i].word)) {
                    duplicatedIdfName(getWordLine(i));
                }
                sym_type type = (word_get_list[i - 1].type == "INTTK") ? func_int : func_char;
                globalSymStack.emplace_back(word_get_list[i].word, type);
                mips_generate_label_func(word_get_list[i].word);
                local_offset = -8;
            }
            default:
                break;
		}
		i++;
	}
	if (status == 2) {
		term_set.insert(newTerm(SMTB, i - 1, i - startIndex));
		return i - startIndex;
	}
	return 0;
}

int getConstDefine(int startIndex)					//常量定义
{
	int status = 0, i = startIndex;
	vector<int> idIndexList;
	vector<int> initValueList;
	while (status != -E_WRONG_FORMAT && status != -END && i < word_get_list.size()) {
		switch (status)
		{
		case 0:
			status = (word_get_list[i].type == "INTTK" || word_get_list[i].type == "CHARTK") ? 1 : -E_WRONG_FORMAT;
			i++;
			break;
		case 1:
			status = (word_get_list[i].type == "IDENFR") ? 2 : -E_WRONG_FORMAT;
			if (status == 2) {
			    idIndexList.push_back(i);
			}
			i++;
			break;
		case 2:
			status = (word_get_list[i].type == "ASSIGN") ? 3 : -E_WRONG_FORMAT;
			i++;
			break;
		case 3:
		{
			int tmp;
			if ((tmp = getInt(i)) != 0)
			{
                if (tmp == 1) {
                    initValueList.push_back(atoi(word_get_list[i].word.c_str()));
                }
                else {
                    initValueList.push_back(atoi((word_get_list[i].word + word_get_list[i + 1].word).c_str()));
                }
				i += tmp;
				status = 4;
			}
			else if (word_get_list[i].type == "CHARCON") {
			    initValueList.push_back((int)(word_get_list[i].word.at(0)));
				i++;
				status = 4;
			}
			else
				status = -E_WRONG_FORMAT;
			break;
		}
		case 4:
			status = (word_get_list[i].type == "COMMA") ? 1 : -END;
			i += (word_get_list[i].type == "COMMA") ? 1 : 0;
			break;
		default:
			break;
		}
	}
	if (status != -E_WRONG_FORMAT) {
	    int j = 0;
        for (int idIndex: idIndexList) {
            bool b = isInLocal ? searchInLocal(word_get_list[idIndex].word) : searchInGlobal(word_get_list[idIndex].word);
            if (b) {
                duplicatedIdfName(getWordLine(idIndex));
            }
            sym_type type = word_get_list[startIndex].type == "INTTK" ? const_int : const_char;
            if (isInLocal) {
                localSymStack.emplace_back(word_get_list[idIndex].word, type);
                local_normal_var(word_get_list[idIndex].word, initValueList[j]);
            } else {
                globalSymStack.emplace_back(word_get_list[idIndex].word, type);
                global_normal_var(word_get_list[idIndex].word, initValueList[j]);
            }
            j++;
        }
		term_set.insert(newTerm(CLDY, i - 1, i - startIndex));
		return i - startIndex;
	}
	return 0;
}

int getConstTK(int startIndex)					//常量说明
{
	int status = 0, i = startIndex;
	int ret;
	while (status != -E_WRONG_FORMAT && status != -END && i < word_get_list.size()) {
		switch (status)
		{
		case 0:
			status = (word_get_list[i].type == "CONSTTK") ? 1 : -E_WRONG_FORMAT;
			i++;
			break;
		case 1:
			if ((ret = getConstDefine(i)) != 0) {
				i += ret;
				status = 2;
			}
			else {
				status = -E_WRONG_FORMAT;
			}
			break;
		case 2:
			if (word_get_list[i].type == "SEMICN") {
			    status = 3;
			    i++;
			} else {
			    status = 3;
			    missingSemicn(getWordLine(i - 1));
			}
			break;
		case 3:
			status = (word_get_list[i].type == "CONSTTK") ? 1 : -END;
			i += (word_get_list[i].type == "CONSTTK") ? 1 : 0;
			break;
		default:
			break;
		}
	}
	if (status != -E_WRONG_FORMAT) {
		term_set.insert(newTerm(CLSM, i - 1, i - startIndex));
		return i - startIndex;
	}
	return 0;
}

int getArgumentList(int startIndex)						//参数表
{
	int status = 0, i = startIndex;
	while (status != -E_WRONG_FORMAT && status != -END && i < word_get_list.size()) {
		switch (status) {
		case 0:
			status = (word_get_list[i].type == "INTTK" || word_get_list[i].type == "CHARTK") ? 1 : -END;
			i += (word_get_list[i].type == "INTTK" || word_get_list[i].type == "CHARTK") ? 1 : 0;
			break;
		case 1:
			status = (word_get_list[i].type == "IDENFR") ? 2 : -E_WRONG_FORMAT;
            if (word_get_list[i].type == "IDENFR") {
                if (searchInLocal(word_get_list[i].word)) {
                    duplicatedIdfName(getWordLine(i));
                }
                sym_type type = (word_get_list[i - 1].type == "INTTK") ? var_int : var_char;
                localSymStack.emplace_back(word_get_list[i].word, type);
                local_param(word_get_list[i].word);
            }
			i++;
			break;
		case 2:
			status = (word_get_list[i].type == "COMMA") ? 3 : -END;
			i += (word_get_list[i].type == "COMMA") ? 1 : 0;
			break;
		case 3:
			status = (word_get_list[i].type == "INTTK" || word_get_list[i].type == "CHARTK") ? 1 : -E_WRONG_FORMAT;
			i++;
			break;
		default:
			break;
		}
	}
	if (status != -E_WRONG_FORMAT)
	{
		term_set.insert(newTerm(CSB, i - 1, i - startIndex));
		return i - startIndex;
	}
	return -E_WRONG_FORMAT;
}

int getFactor(int startIndex)							//因子
{
	int getFuncRetCallSentence(int);
	int getExpression(int);
	int status = 0, i = startIndex;
	int ret;
	string expr1, expr2, idenfr;
	while (status != -E_WRONG_FORMAT && status != -END && i < word_get_list.size())
	{
		switch (status)
		{
		case 0:
			if ((ret = getInt(i)) != 0) {
				status = -END;
				i += ret;
			}
			else if (word_get_list[i].type == "CHARCON") {
				status = -END;
				i++;
			}
			else if ((ret = getFuncRetCallSentence(i)) != 0) {
			    temp_var_no++;
			    midExprList.push_back({"GETRET", "#t" + std::to_string(temp_var_no)});
			    last_factor_name = "#t" + std::to_string(temp_var_no);
				status = -END;
				i += ret;
			}
			else if (word_get_list[i].type == "LPARENT") {
				status = 7;
				i++;
			}
			else {
				status = (word_get_list[i].type == "IDENFR") ? 1 : -E_WRONG_FORMAT;
                if (word_get_list[i].type == "IDENFR") {
                    idenfr = word_get_list[i].word;
                    if (!searchInLocal(word_get_list[i].word) && !searchInGlobal(word_get_list[i].word)) {
                        neverStatedIdfName(getWordLine(i));
                    }
                }
				i++;
			}
			break;
		case 1:
			status = (word_get_list[i].type == "LBRACK") ? 2 : -END;
			i += (word_get_list[i].type == "LBRACK") ? 1 : 0;
			break;
		case 2:
			if ((ret = getExpression(i)) != 0)
			{
                vector<vector<string>> prev = midExprList;
                int p = temp_var_no;
                string l = last_expr_name;
			    if (isCharRetExpression(i, ret)) {
			        charArraySubIndex(getWordLine(i));
			    }
			    prev = midExprList;
			    temp_var_no = p;
			    last_expr_name = l;
			    expr1 = last_expr_name;
				i += ret;
				status = 3;							//a[<>
			}
			else {
				status = -E_WRONG_FORMAT;
			}
			break;
		case 3:
            if (word_get_list[i].type == "RBRACK") {
                status = 4;
                i++;
            } else {
                status = 4;
                missingRbrack(getWordLine(i));
            }
                break;
		case 4:
			status = (word_get_list[i].type == "LBRACK") ? 5 : -END;
			i += (word_get_list[i].type == "LBRACK") ? 1 : 0;
			break;
		case 5:
			if ((ret = getExpression(i)) != 0)
			{
                vector<vector<string>> prev = midExprList;
                int p = temp_var_no;
                string l = last_expr_name;
                if (isCharRetExpression(i, ret)) {
                    charArraySubIndex(getWordLine(i));
                }
                midExprList = prev;
                temp_var_no = p;
                last_expr_name = l;
                expr2 = last_expr_name;
                midExprList.push_back({"*", expr1, std::to_string(array2width[idenfr]), expr1});
                midExprList.push_back({"+", expr1, expr2, expr1});
                last_expr_name = expr1;
				i += ret;
				status = 6;							//a[<>][<>
			}
			else {
				status = -E_WRONG_FORMAT;
			}
			break;
		case 6:
            if (word_get_list[i].type == "RBRACK") {
                status = -END;
                i++;
            } else {
                status = -END;
                missingRbrack(getWordLine(i));
            }
                break;
		case 7:
			if ((ret = getExpression(i)) != 0)
			{
				i += ret;
				status = 8;							//(<>
			}
			else {
				status = -E_WRONG_FORMAT;
			}
			break;
		case 8:
            if (word_get_list[i].type == "RPARENT") {
                status = -END;
                i++;
            } else {
                status = -END;
                missingRparent(getWordLine(i));
            }
			break;
		default:
			break;
		}
		
	}
	if (status != -E_WRONG_FORMAT)
	{
		term_set.insert(newTerm(YZ, i - 1, i - startIndex));
		return i - startIndex;
	}
	return 0;
}

int getPoly(int startIndex)								//项
{
	int status = 0, i = startIndex;
	int ret;
	string sym;
	string op1;
	string op2;
	while (status != -E_WRONG_FORMAT && status != -END && i < word_get_list.size())
	{
		switch (status)
		{
		case 0:
			if ((ret = getFactor(i)) != 0) {
			    if (ret < 3) {
			        op1 = string();
			        for (int j = i; j < i + ret; j++) {
			            op1 += (word_get_list[j].type == "CHARCON") ?
			                    std::to_string((int)(word_get_list[i].word.at(0))) : word_get_list[j].word;
                        last_poly_name = op1;
			        }
			    }
			    else if (word_get_list[i].type == "LPARENT") {
			        op1 = last_expr_name;
                    last_poly_name = op1;
			    }
			    else if (word_get_list[i + 1].type == "LBRACK") {
			        temp_var_no++;
			        midExprList.push_back({"GET", word_get_list[i].word, last_expr_name, "#t" + std::to_string(temp_var_no)});
			        op1 = "#t" + std::to_string(temp_var_no);
			        last_poly_name = op1;
			    }
			    else {
                    op1 = last_factor_name;
                    last_poly_name = op1;
			    }
				i += ret;
				status = 1;
			}
			else {
				status = -E_WRONG_FORMAT;
			}
			break;
		case 1:
			status = (word_get_list[i].type == "MULT" || word_get_list[i].type == "DIV") ? 2 : -END;
			sym = (word_get_list[i].type == "MULT") ? "*" : "/";
			i += (word_get_list[i].type == "MULT" || word_get_list[i].type == "DIV") ? 1 : 0;
			break;
        case 2:
            if ((ret = getFactor(i)) != 0) {
                op2 = string();
                if (ret < 3) {
                    for (int j = i; j < i + ret; j++) {
                        op2 += (word_get_list[j].type == "CHARCON") ?
                               std::to_string((int)(word_get_list[i].word.at(0))) : word_get_list[j].word;
                    }
                } else if (word_get_list[i].type == "LPARENT") {
                    op2 = last_expr_name;
                }
                else if (word_get_list[i + 1].type == "LBRACK") {
                    temp_var_no++;
                    midExprList.push_back({"GET", word_get_list[i].word, last_expr_name, "#t" + std::to_string(temp_var_no)});
                    op2 = "#t" + std::to_string(temp_var_no);
                }
                else {
                    op2 = last_factor_name;
                }
                temp_var_no++;
                midExprList.push_back({sym, op1, op2, "#t" + std::to_string(temp_var_no)});
                op1 = "#t" + std::to_string(temp_var_no);
                last_poly_name = op1;
                i += ret;
                status = 1;
            }
            else {
                status = -E_WRONG_FORMAT;
            }
            break;
		default:
			break;
		}
	}
	if (status != -E_WRONG_FORMAT)
	{
		term_set.insert(newTerm(X, i - 1, i - startIndex));
		return i - startIndex;
	}
	return 0;
}

int getExpression(int startIndex)						//表达式
{
	int status = 0, i = startIndex;
	int ret;
	string sym;
	bool firstFactorMinus = false;
	string op1;
	string op2;
	while (status != -E_WRONG_FORMAT && status != -END && i < word_get_list.size())
	{
		switch (status)
		{
		case 0:
			if (word_get_list[i].type == "PLUS")
			{
				i++;
				status = 1;
			} else if (word_get_list[i].type == "MINU") {
			    firstFactorMinus = true;
			    i++;
			    status = 1;
			}
			else if ((ret = getPoly(i)) != 0) {
			    op1 = last_poly_name;
				i += ret;
				status = 2;
			}
			else
				status = -E_WRONG_FORMAT;
			break;
		case 1:
			if ((ret = getPoly(i)) != 0) {
                op1 = last_poly_name;
				i += ret;
				status = 2;
				if (firstFactorMinus) {
				    temp_var_no++;
                    midExprList.push_back({"-", "0", op1, "#t" + std::to_string(temp_var_no)});
                    op1 = "#t" + std::to_string(temp_var_no);
				}
			}
			else
				status = -E_WRONG_FORMAT;
			break;
		case 2:
			if (word_get_list[i].type == "PLUS" || word_get_list[i].type == "MINU")
			{
			    sym = (word_get_list[i].type == "PLUS") ? "+" : "-";
				i++;
				status = 3;
			}
			else {
			    temp_var_no++;
			    midExprList.push_back({"SINGLE", op1, "#t" + std::to_string(temp_var_no)});
			    op1 = "#t" + std::to_string(temp_var_no);
                status = -END;
			}
			break;
        case 3:
            if ((ret = getPoly(i)) != 0) {
                op2 = last_poly_name;
                temp_var_no++;
                midExprList.push_back({sym, op1, op2, "#t" + std::to_string(temp_var_no)});
                op1 = "#t" + std::to_string(temp_var_no);
                i += ret;
                status = 2;
            }
            else
                status = -E_WRONG_FORMAT;
            break;
		default:
			break;
		}
	}
	if (status != -E_WRONG_FORMAT)
	{
		term_set.insert(newTerm(BDS, i - 1, i - startIndex));
     //   midExpr_generate();
     //   midExpr2mips();
      //  clear_midExpr();
        last_expr_name = op1;
		return i - startIndex;
	}
	return 0;
}



int getAssignSentence(int startIndex)					//赋值语句
{
	int status = 0, i = startIndex;
	int ret;
	string op;
	string expr1, expr2;
	while (status != -E_WRONG_FORMAT && status != -END && i < word_get_list.size())
	{
		switch (status)
		{
		case 0:
			status = (word_get_list[i].type == "IDENFR") ? 1 : -E_WRONG_FORMAT;
            if (word_get_list[i].type == "IDENFR") {
                if (!searchInLocal(word_get_list[i].word) && !searchInGlobal(word_get_list[i].word)) {
                    neverStatedIdfName(getWordLine(i));
                }
                op = word_get_list[i].word;
                sym_type type = getSymType(word_get_list[i].word);
                if (type == const_char || type == const_int) {
                    assignConstVar(getWordLine(i));
                }
            }
			i++;
			break;
		case 1:
			status = (word_get_list[i].type == "ASSIGN") ? 2 :
				(word_get_list[i].type == "LBRACK") ? 3 : -E_WRONG_FORMAT;
			i++;
			break;
		case 2:
			if ((ret = getExpression(i)) != 0)
			{
                if (!expr1.empty()) {
                    midExprList.push_back({"LOAD", op, expr1, last_expr_name});
                } else {
                    midExprList.push_back({"ASSIGN", op, last_expr_name});
                }
				i += ret;
				status = -END;
			}
			else
				status = -E_WRONG_FORMAT;
			break;
		case 3:
			if ((ret = getExpression(i)) != 0)
			{
                expr1 = last_expr_name;
				i += ret;
				status = 4;
			}
			else
				status = -E_WRONG_FORMAT;
			break;
		case 4:
            if (word_get_list[i].type == "RBRACK") {
                status = 5;
                i++;
            } else {
                status = 5;
                missingRbrack(getWordLine(i));
            }
                break;
		case 5:
			status = (word_get_list[i].type == "ASSIGN") ? 2 :
				(word_get_list[i].type == "LBRACK") ? 6 : -E_WRONG_FORMAT;
			i++;
			break;
		case 6:
			if ((ret = getExpression(i)) != 0)
			{
                expr2 = last_expr_name;
                midExprList.push_back({"*", expr1, std::to_string(array2width[op]), expr1});
                midExprList.push_back({"+", expr1, expr2, expr1});
				i += ret;
				status = 7;
			}
			else
				status = -E_WRONG_FORMAT;
			break;
		case 7:
            if (word_get_list[i].type == "RBRACK") {
                status = 8;
                i++;
            } else {
                status = 8;
                missingRbrack(getWordLine(i));
            }
                break;
		case 8:
			status = (word_get_list[i].type == "ASSIGN") ? 2 : -E_WRONG_FORMAT;
			i++;
			break;
		default:
			break;
		}
	}
	if (status != -E_WRONG_FORMAT)
	{
		term_set.insert(newTerm(FZYJ, i - 1, i - startIndex));
        midExpr2mips();
        clear_midExpr();
		return i - startIndex;
	}
	return 0;
}

int getBranch(int startIndex)							//条件
{
	int status = 0, i = startIndex;
	int ret;
	int left = 0, right = 0, leftLen = 0, rightLen = 0;
	while (status != -E_WRONG_FORMAT && status != -END && i < word_get_list.size())
	{
		switch (status)
		{
		case 0:
			if ((ret = getExpression(i)) != 0)
			{
			    left = i;
			    leftLen = ret;
				i += ret;
				status = 1;
                midExpr2mips();
                clear_midExpr();
                b_left_expr = last_expr_name;
				//mips_move_to_register("$s3", last_expr_name);
			}
			else
				status = -E_WRONG_FORMAT;
			break;
		case 1:
			if (isRelationOperator(i))
			{
			    branchOp = word_get_list[i].word;
				i++;
				status = 2;
			}
			else
				status = -E_WRONG_FORMAT;
			break;
		case 2:
			if ((ret = getExpression(i)) != 0)
			{
			    right = i;
			    rightLen = ret;
				i += ret;
				status = -END;
                midExpr2mips();
                clear_midExpr();
                b_right_expr = last_expr_name;
               // mips_move_to_register("$s4", last_expr_name);
			}
			else
				status = -E_WRONG_FORMAT;
			break;
		default:
			break;
		}
	}
	if (status != -E_WRONG_FORMAT)
	{
        vector<vector<string>> prev = midExprList;
        int p = temp_var_no;
        string l = last_expr_name;
	    if (isCharRetExpression(left, leftLen) || isCharRetExpression(right, rightLen)) {
	        branchTypeIllegal(getWordLine(startIndex));
	    }
	    last_expr_name = l;
	    midExprList = prev;
	    temp_var_no = p;
		term_set.insert(newTerm(TJ, i - 1, i - startIndex));
		return i - startIndex;
	}
	return 0;
}

int getBranchSentence(int startIndex)					//条件语句
{
	int getSentence(int);
	int status = 0, i = startIndex;
	int ret;
	int no = 0;
	while (status != -E_WRONG_FORMAT && status != -END && i < word_get_list.size())
	{
		switch (status)
		{
		case 0:
			status = (word_get_list[i].type == "IFTK") ? 1 : -E_WRONG_FORMAT;
			i++;
			break;
		case 1:
			status = (word_get_list[i].type == "LPARENT") ? 2 : -E_WRONG_FORMAT;
			i++;
			break;
		case 2:
			if ((ret = getBranch(i)) != 0)
			{
                mips_move_to_register("$s3", b_left_expr);
                mips_move_to_register("$s4", b_right_expr);
                no = mips_branch(branchOp);
				i += ret;
				status = 3;
			}
			else {
				status = -E_WRONG_FORMAT;
			}
			break;
		case 3:
            if (word_get_list[i].type == "RPARENT") {
                status = 4;
                i++;
            } else {
                status = 4;
                missingRparent(getWordLine(i));
            }
                break;
		case 4:
			if ((ret = getSentence(i)) != 0)
			{
				i += ret;
				status = 5;
			}
			else {
				status = -E_WRONG_FORMAT;
			}
			break;
		case 5:
			status = (word_get_list[i].type == "ELSETK") ? 6 : -END;
			i += (word_get_list[i].type == "ELSETK") ? 1 : 0;
			if (status == -END) {
			    mips_if_label(no);
			} else if (status == 6) {
			    textField.emplace_back("j $elselabel" + std::to_string(no));
			    mips_if_label(no);
			}
			break;
		case 6:
			if ((ret = getSentence(i)) != 0)
			{
                mips_else_label(no);
				i += ret;
				status = -END;
			}
			else {
				status = -E_WRONG_FORMAT;
			}
			break;
		default:
			break;
		}
	}
	if (status != -E_WRONG_FORMAT)
	{
		term_set.insert(newTerm(TJYJ, i - 1, i - startIndex));
		return i - startIndex;
	}
	return 0;
}

int getStepLength(int startIndex)
{
	if (getUnsignedInt(startIndex)) {
		term_set.insert(newTerm(BC, startIndex, 1));
		return 1;
	}
	return 0;
}

int getLoopSentence(int startIndex)						//循环语句
{

	int status = 0, i = startIndex;
	int ret;
	int wno = whileno;
	int fno = forno;
	string id1;
	string id2;
	int step = 0;
	string stepop;
	while (status != -E_WRONG_FORMAT && status != -END && i < word_get_list.size())
	{
        int getSentence(int);
		switch (status)
		{
		case 0:
			status = (word_get_list[i].type == "WHILETK") ? 1 :
				(word_get_list[i].type == "FORTK") ? 5 : -E_WRONG_FORMAT;
			i++;
			break;
		case 1:
		    whileno++;
			status = (word_get_list[i].type == "LPARENT") ? 2 : -1;
			i++;
			break;
		case 2:
            textField.emplace_back("$while" + std::to_string(wno) + "start:");
			if ((ret = getBranch(i)) != 0)
			{
                mips_move_to_register("$s3", b_left_expr);
                mips_move_to_register("$s4", b_right_expr);
			    mips_branch_while(branchOp, wno);
				i += ret;
				status = 3;
			}
			else
				status = -E_WRONG_FORMAT;
			break;
		case 3:
            if (word_get_list[i].type == "RPARENT") {
                status = 4;
                i++;
            } else {
                status = 4;
                missingRparent(getWordLine(i));
            }
			break;
		case 4:
			if ((ret = getSentence(i)) != 0)
			{
				i += ret;
				textField.emplace_back("j $while" + std::to_string(wno) + "start");
				textField.emplace_back("$while" + std::to_string(wno) + "loop:");
				status = -END;
			}
			else
				status = -E_WRONG_FORMAT;
			break;
		case 5:
		    forno++;
			status = (word_get_list[i].type == "LPARENT") ? 6 : -1;
			i++;
			break;
		case 6:
			status = (word_get_list[i].type == "IDENFR") ? 7 : -1;
            if (word_get_list[i].type == "IDENFR") {
                if (!searchInLocal(word_get_list[i].word) && !searchInGlobal(word_get_list[i].word)) {
                    neverStatedIdfName(getWordLine(i));
                }
                id1 = word_get_list[i].word;
            }
			i++;
			break;
		case 7:
			status = (word_get_list[i].type == "ASSIGN") ? 8 : -1;
			i++;
			break;
		case 8:
			if ((ret = getExpression(i)) != 0)
			{
			    midExprList.push_back({"ASSIGN", id1, last_expr_name});
			    midExpr2mips();
			    clear_midExpr();
				i += ret;
				status = 9;
			}
			else
				status = -E_WRONG_FORMAT;
			break;
		case 9:
			if (word_get_list[i].type == "SEMICN") {
			    status = 10;
			    i++;
			} else {
			    status = 10;
			    missingSemicn(getWordLine(i - 1));
			}
			break;
		case 10:
            textField.emplace_back("$for" + std::to_string(fno) + "start:");
			if ((ret = getBranch(i)) != 0)
			{
                mips_move_to_register("$t4", b_left_expr);
                mips_move_to_register("$t5", b_right_expr);
                mips_branch_for(branchOp, fno, id1, last_expr_name);
				i += ret;
				status = 11;
			}
			else
				status = -E_WRONG_FORMAT;
			break;
		case 11:
            if (word_get_list[i].type == "SEMICN") {
                status = 12;
                i++;
            } else {
                status = 12;
                missingSemicn(getWordLine(i - 1));
            }
			break;
		case 12:
			status = (word_get_list[i].type == "IDENFR") ? 13 : -1;
            if (word_get_list[i].type == "IDENFR") {
                if (!searchInLocal(word_get_list[i].word) && !searchInGlobal(word_get_list[i].word)) {
                    neverStatedIdfName(getWordLine(i));
                }
                id1 = word_get_list[i].word;
            }
			i++;
			break;
		case 13:
			status = (word_get_list[i].type == "ASSIGN") ? 14 : -1;
			i++;
			break;
		case 14:
			status = (word_get_list[i].type == "IDENFR") ? 15 : -1;
            if (word_get_list[i].type == "IDENFR") {
                if (!searchInLocal(word_get_list[i].word) && !searchInGlobal(word_get_list[i].word)) {
                    neverStatedIdfName(getWordLine(i));
                }
                id2 = word_get_list[i].word;
            }
			i++;
			break;
		case 15:
			status = (word_get_list[i].type == "PLUS" || word_get_list[i].type == "MINU") ? 16 : -1;
			stepop = word_get_list[i].word;
			i++;
			break;
		case 16:
			if ((ret = getStepLength(i)) != 0)
			{
			    step = atoi(word_get_list[i].word.c_str());
				i += ret;
				status = 17;
			}
			else
				status = -E_WRONG_FORMAT;
			break;
		case 17:
            if (word_get_list[i].type == "RPARENT") {
                status = 18;
                i++;
            } else {
                status = 18;
                missingRparent(getWordLine(i));
            }
			break;
		case 18:
			if ((ret = getSentence(i)) != 0)
			{
				i += ret;
				mips_step_move(id1, id2, stepop, step);
                textField.emplace_back("j $for" + std::to_string(fno) + "start");
                textField.emplace_back("$for" + std::to_string(fno) + "loop:");
				status = -END;
			}
			else
				status = -E_WRONG_FORMAT;
			break;
		default:
			break;
		}
	}
	if (status != -E_WRONG_FORMAT)
	{
		term_set.insert(newTerm(XHYJ, i - 1, i - startIndex));
		return i - startIndex;
	}
	return 0;
}

int getSituationSonSentence(int startIndex, int swno, int cano)				//情况子语句
{
	int getSentence(int);
	int status = 0, i = startIndex;
	int ret;
	while (status != -E_WRONG_FORMAT && status != -END && i < word_get_list.size()) {
		switch (status)
		{
		case 0:
			status = (word_get_list[i].type == "CASETK") ? 1 : -E_WRONG_FORMAT;
			i++;
			break;
		case 1:
			if ((ret = getConst(i)) != 0)
			{
			    if (word_get_list[i].type == "CHARCON" && switchType != "CHARTK") {
			        constTypeNoMatch(getWordLine(i));
			    }
			    if (word_get_list[i].type != "CHARCON" && switchType == "CHARTK") {
			        constTypeNoMatch(getWordLine(i));
			    }
                string s = string();
                if (word_get_list[i].type == "CHARCON") {
                    s = std::to_string((int)(word_get_list[i].word.at(0)));
                }
                else {
                    for (int j = i; j < i + ret; j++) {
                        s += word_get_list[j].word;
                    }
                }
                mips_switch(s, swno, cano);
				i += ret;
				status = 2;
			}
			else
				status = -E_WRONG_FORMAT;
			break;
		case 2:
			status = (word_get_list[i].type == "COLON") ? 3 : -E_WRONG_FORMAT;
			i++;
			break;
		case 3:
			if ((ret = getSentence(i)) != 0)
			{
				i += ret;
				textField.emplace_back("j $switch" + std::to_string(swno) + "end");
				mips_switch_label(swno, cano);
				status = -END;
			}
			else
				status = -E_WRONG_FORMAT;
			break;
		default:
			break;
		}
	}
	if (status != -E_WRONG_FORMAT) {
		term_set.insert(newTerm(QKZYJ, i - 1, i - startIndex));
		return i - startIndex;
	}
	return 0;
}

int getSituationList(int startIndex, int swno)					//情况表
{
	int status = 0, i = startIndex;
	int ret;
    int cano = 1;
	while (status != -E_WRONG_FORMAT && status != -END && i < word_get_list.size()) {
		switch (status)
		{
		case 0:
			if ((ret = getSituationSonSentence(i, swno, cano)) != 0)
			{
				i += ret;
				status = 1;
                cano++;
			}
			else
				status = -E_WRONG_FORMAT;
			break;
		case 1:
			if ((ret = getSituationSonSentence(i, swno, cano)) != 0)
			{
				i += ret;
				cano++;
			}
			else
				status = -END;
			break;
		default:
			break;
		}
	}
	if (status != -E_WRONG_FORMAT) {
		term_set.insert(newTerm(QKB, i - 1, i - startIndex));
		return i - startIndex;
	}
	return 0;
}

int getDefault(int startIndex)							//缺省
{
	int getSentence(int);
	int status = 0, i = startIndex;
	int ret;
	while (status != -E_WRONG_FORMAT && status != -END && i < word_get_list.size()) {
		switch (status)
		{
		case 0:
			status = (word_get_list[i].type == "DEFAULTTK") ? 1 : -E_WRONG_FORMAT;
			i++;
			break;
		case 1:
			status = (word_get_list[i].type == "COLON") ? 2 : -E_WRONG_FORMAT;
			i++;
			break;
		case 2:
			if ((ret = getSentence(i)) != 0)
			{
				i += ret;
				status = -END;
			}
			else
				status = -E_WRONG_FORMAT;
			break;
		default:
			break;
		}
	}
	if (status != -E_WRONG_FORMAT) {
		term_set.insert(newTerm(QS, i - 1, i - startIndex));
		return i - startIndex;
	}
	return 0;
}

int getSituationSentence(int startIndex)				//情况语句
{
	int status = 0, i = startIndex;
	int ret;
	bool missDefault = false;
    string prevSwitchType = switchType;
    int swno = switchno;
	while (status != -E_WRONG_FORMAT && status != -END && i < word_get_list.size()) {
		switch (status)
		{
		case 0:
			status = (word_get_list[i].type == "SWITCHTK") ? 1 : -E_WRONG_FORMAT;
			i++;
			break;
		case 1:
            switchno++;
			status = (word_get_list[i].type == "LPARENT") ? 2 : -E_WRONG_FORMAT;
			i++;
			break;
		case 2:
			if ((ret = getExpression(i)) != 0)
			{
                vector<vector<string>> prev = midExprList;
                int p = temp_var_no;
                string l = last_expr_name;
			    switchType = isCharRetExpression(i, ret) ? "CHARTK" : "INTTK";
			    temp_var_no = p;
			    midExprList = prev;
			    last_expr_name = l;
                midExpr2mips();
                clear_midExpr();
			    textField.emplace_back("move $s6, $s0");
				i += ret;
				status = 3;
			}
			else
				status = -E_WRONG_FORMAT;
			break;
		case 3:
            if (word_get_list[i].type == "RPARENT") {
                status = 4;
                i++;
            } else {
                status = 4;
                missingRparent(getWordLine(i));
            }
			break;
		case 4:
			status = (word_get_list[i].type == "LBRACE") ? 5 : -E_WRONG_FORMAT;
			i++;
			break;
		case 5:
			if ((ret = getSituationList(i, swno)) != 0)
			{
				i += ret;
				status = 6;
			}
			else
				status = -E_WRONG_FORMAT;
			break;
		case 6:
			if ((ret = getDefault(i)) != 0)
			{
				i += ret;
				status = 7;
			}
			else
            {
			    missDefault = true;
                status = 7;
            }
			break;
		case 7:
			status = (word_get_list[i].type == "RBRACE") ? -END : -E_WRONG_FORMAT;
			i++;
			break;
		default:
			break;
		}
	}
	if (status != -E_WRONG_FORMAT) {
	    if (missDefault) {
	        missingDefault(getWordLine(i - 1));
	    }
        textField.emplace_back("$switch" + std::to_string(swno) + "end:");
		term_set.insert(newTerm(QKYJ, i - 1, i - startIndex));
	    switchType = prevSwitchType;
		return i - startIndex;
	}
	switchType = prevSwitchType;
	return 0;
}

int getValueArgumentList(int startIndex)							//值参数表，可以为空
{
	int status = 0, i = startIndex;
	int ret;
	argumentNum = 0;
	vector<sym_type> paramTypeList;
	vector<string> paramNameList;
	paramNameList.push_back("@" + currentFunction);
	while (status != -E_WRONG_FORMAT && status != -END && i < word_get_list.size())
	{
		switch (status)
		{
		case 0:
			if ((ret = getExpression(i)) != 0)
			{
			    vector<vector<string>> prev = midExprList;
                int p = temp_var_no;
                string l = last_expr_name;
			    paramTypeList.push_back(isCharRetExpression(i, ret) ? var_char : var_int);
			    temp_var_no = p;
			    midExprList = prev;
			    last_expr_name = l;
				i += ret;
				status = 1;
				argumentNum++;
                paramNameList.push_back(last_expr_name);
			}
			else
				status = -END;
			break;
		case 1:
			status = (word_get_list[i].type == "COMMA") ? 2 : -END;
			i += (word_get_list[i].type == "COMMA") ? 1 : 0;
			break;
		case 2:
			if ((ret = getExpression(i)) != 0)
			{
                vector<vector<string>> prev = midExprList;
                int p = temp_var_no;
                string l = last_expr_name;
                paramTypeList.push_back(isCharRetExpression(i, ret) ? var_char : var_int);
                temp_var_no = p;
                last_expr_name = l;
                midExprList = prev;
				i += ret;
				status = 1;
				argumentNum++;
                paramNameList.push_back(last_expr_name);
			}
			else
				status = -E_WRONG_FORMAT;
			break;
		default:
			break;
		}
	}
	if (paramTypeList.size() != paramOfFunction[currentFunction].size()) {
	    paramNumNoMatch(getWordLine(startIndex));
	}
	else {
        for(size_t j = 0; j < paramTypeList.size(); j++) {
            if (paramTypeList[j] != paramOfFunction[currentFunction][j])
                paramTypeNoMatch(getWordLine(startIndex));
        }
	}
	if (status != -E_WRONG_FORMAT)
	{
	    midExprList.push_back(paramNameList);
		term_set.insert(newTerm(ZCSB, i - 1, i - startIndex));
		return i - startIndex;
	}
	return -1;
}

int getFuncNoRetCallSentence(int startIndex)					//无返回值函数调用语句
{
	int status = 0, i = startIndex;
	int ret;
	string prevFunction = currentFunction;
	while (status != -E_WRONG_FORMAT && status != -END && i < word_get_list.size())
	{
		switch (status)
		{
		case 0:
            if (word_get_list[i].type == "IDENFR")
            {
                currentFunction = word_get_list[i].word;
                if (!searchInLocal(word_get_list[i].word) && !searchInGlobal(word_get_list[i].word)) {
                    neverStatedIdfName(getWordLine(i));
                }
                if (func_have_ret.find(word_get_list[i].word) == func_have_ret.end()) {
                    status = 1;
                    i++;
                } else {
                    status = -E_WRONG_FORMAT;
                }
            }
            else {
                status = -E_WRONG_FORMAT;
            }
                break;
		case 1:
			status = (word_get_list[i].type == "LPARENT") ? 2 : -E_WRONG_FORMAT;
			i++;
			break;
		case 2:
			if ((ret = getValueArgumentList(i)) != -1)
			{
				i += ret;
				status = 3;
			}
			else
				status = -E_WRONG_FORMAT;
			break;
		case 3:
            if (word_get_list[i].type == "RPARENT") {
                status = -END;
                i++;
            } else {
                status = -END;
                missingRparent(getWordLine(i));
            }
			break;
		default:
			break;
		}
	}
	if (status != -E_WRONG_FORMAT)
	{
		term_set.insert(newTerm(WFHZHSDYYJ, i - 1, i - startIndex));
		argumentNum = 0;
		currentFunction = prevFunction;
		return i - startIndex;
	}
    argumentNum = 0;
    currentFunction = prevFunction;
	return 0;
}

int getFuncRetCallSentence(int startIndex)					//有返回值函数调用语句
{
	int status = 0, i = startIndex;
	int ret;
	string prevFunction = currentFunction;
	while (status != -E_WRONG_FORMAT && status != -END && i < word_get_list.size())
	{
		switch (status)
		{
		case 0:
            if (word_get_list[i].type == "IDENFR")
            {
                currentFunction = word_get_list[i].word;
                if (!searchInLocal(word_get_list[i].word) && !searchInGlobal(word_get_list[i].word)) {
                    neverStatedIdfName(getWordLine(i));
                }
                if (func_have_ret.find(word_get_list[i].word) != func_have_ret.end()) {
                    status = 1;
                    i++;
                } else {
                    status = -E_WRONG_FORMAT;
                }
            }
            else {
                status = -E_WRONG_FORMAT;
            }
                break;
		case 1:
			status = (word_get_list[i].type == "LPARENT") ? 2 : -E_WRONG_FORMAT;
			i++;
			break;
		case 2:
			if ((ret = getValueArgumentList(i)) != -1)
			{
				i += ret;
				status = 3;
			}
			else
				status = -E_WRONG_FORMAT;
			break;
		case 3:
            if (word_get_list[i].type == "RPARENT") {
                status = -END;
                i++;
            } else {
                status = -END;
                missingRparent(getWordLine(i));
            }
			break;
		default:
			break;
		}
	}
	if (status != -E_WRONG_FORMAT)
	{
		term_set.insert(newTerm(YFHZHSDYYJ, i - 1, i - startIndex));
		argumentNum = 0;
		currentFunction = prevFunction;
		return i - startIndex;
	}
    argumentNum = 0;
    currentFunction = prevFunction;
	return 0;
}

int getScanfSentence(int startIndex)							//读语句
{
	int status = 0, i = startIndex;
	while (status != -E_WRONG_FORMAT && status != -END && i < word_get_list.size())
	{
		switch (status) {
		case 0:
			status = (word_get_list[i].type == "SCANFTK") ? 1 : -E_WRONG_FORMAT;
			i++;
			break;
		case 1:
			status = (word_get_list[i].type == "LPARENT") ? 2 : -E_WRONG_FORMAT;
			i++;
			break;
		case 2:
			status = (word_get_list[i].type == "IDENFR") ? 3 : -E_WRONG_FORMAT;
			if (word_get_list[i].type == "IDENFR") {
			    if (!searchInLocal(word_get_list[i].word) && !searchInGlobal(word_get_list[i].word)) {
			        neverStatedIdfName(getWordLine(i));
			    }
			    sym_type type = getSymType(word_get_list[i].word);
			    if (type == const_int || type == const_char) {
			        assignConstVar(getWordLine(i));
			    }
			    mips_read(word_get_list[i].word, type);
			}
			i++;
			break;
		case 3:
            if (word_get_list[i].type == "RPARENT") {
                status = -END;
                i++;
            } else {
                status = -END;
                missingRparent(getWordLine(i));
            }
			break;
		default:
			break;
		}
	}
	if (status != -E_WRONG_FORMAT)
	{
		term_set.insert(newTerm(DYJ, i - 1, i - startIndex));
		return i - startIndex;
	}
	return 0;
}

int getPrintfSentence(int startIndex)					//写语句
{
	int status = 0, i = startIndex;
	int ret;
	while (status != -E_WRONG_FORMAT && status != -END && i < word_get_list.size())
	{
		switch (status)
		{
		case 0:
			status = (word_get_list[i].type == "PRINTFTK") ? 1 : -E_WRONG_FORMAT;
			i++;
			break;
		case 1:
			status = (word_get_list[i].type == "LPARENT") ? 2 : -E_WRONG_FORMAT;
			i++;
			break;
		case 2:
			if ((ret = getString(i)) != 0)
			{
			    mips_putString(word_get_list[i].word);
				i += ret;
				status = 3;
			}
			else if ((ret = getExpression(i)) != 0)
			{
                vector<vector<string>> prev = midExprList;
                int p = temp_var_no;
                string l = last_expr_name;
                sym_type type = isCharRetExpression(i, ret) ? var_char : var_int;
                temp_var_no = p;
                last_expr_name = l;
                midExprList = prev;
                midExpr2mips();
                clear_midExpr();
                mips_putExpression(type, last_expr_name);
				i += ret;
				status = 4;
			}
			else
				status = -E_WRONG_FORMAT;
			break;
		case 3:
			if (word_get_list[i].type == "RPARENT") {
			    status = -END;
			    i++;
			} else if (word_get_list[i].type == "COMMA") {
			    status = 5;
			    i++;
			} else {
			    status = -END;
			    missingRparent(getWordLine(i));
			}
			break;
		case 4:
            if (word_get_list[i].type == "RPARENT") {
                status = -END;
                i++;
            } else {
                status = -END;
                missingRparent(getWordLine(i));
            }
			break;
		case 5:
			if ((ret = getExpression(i)) != 0)
			{
                vector<vector<string>> prev = midExprList;
                int p = temp_var_no;
                string l = last_expr_name;
			    sym_type type = isCharRetExpression(i, ret) ? var_char : var_int;
			    temp_var_no = p;
			    midExprList = prev;
			    last_expr_name = l;
                midExpr2mips();
                clear_midExpr();
			    mips_putExpression(type, last_expr_name);
				i += ret;
				status = 4;
			}
			else
				status = -E_WRONG_FORMAT;
			break;
		default:
			break;
		}
	}
	if (status != -E_WRONG_FORMAT)
	{
		term_set.insert(newTerm(XYJ, i - 1, i - startIndex));
		mips_putEnter();
		return i - startIndex;
	}
	return 0;
}

int getReturnSentence(int startIndex)					//返回语句
{
	int status = 0, i = startIndex;
	int ret;
	while (status != -E_WRONG_FORMAT && status != -END && i < word_get_list.size())
	{
		switch (status)
		{
		case 0:
			status = (word_get_list[i].type == "RETURNTK") ? 1 : -E_WRONG_FORMAT;
			i++;
			break;
		case 1:
			status = (word_get_list[i].type == "LPARENT") ? 2 : -END;
			i += (word_get_list[i].type == "LPARENT") ? 1 : 0;
			if (status == -END) {
			    returnSentenceList.emplace_back(startIndex, 1);
			}
			break;
		case 2:
			if ((ret = getExpression(i)) != 0)
			{
                midExpr2mips();
                clear_midExpr();
			    textField.emplace_back("move $k1, $s0");
				i += ret;
				status = 3;
			}
			else
				status = -E_WRONG_FORMAT;
			break;
		case 3:
            if (word_get_list[i].type == "RPARENT") {
                status = -END;
                i++;
                returnSentenceList.emplace_back(startIndex, i - startIndex);
            } else {
                status = -END;
                missingRparent(getWordLine(i));
            }
			break;
		default:
			break;
		}
	}
	if (status != -E_WRONG_FORMAT)
	{
        textField.emplace_back("jr $ra");
		term_set.insert(newTerm(FANHUIYJ, i - 1, i - startIndex));
		return i - startIndex;
	}
	return 0;
}

int getSentence(int startIndex)							//语句
{
	int getSentenceList(int);
	int status = 0, i = startIndex;
	int ret;
	while (status != -E_WRONG_FORMAT && status != -END && i < word_get_list.size())
	{
		switch (status)
		{
		case 0:
			if (word_get_list[i].type == "SEMICN") {
				i++;
				status = -END;
			}
			else if ((ret = getLoopSentence(i)) != 0) {
				i += ret;
				status = -END;
			}
			else if ((ret = getBranchSentence(i)) != 0) {
				i += ret;
				status = -END;
			}
			else if ((ret = getSituationSentence(i)) != 0) {
				i += ret;
				status = -END;
			}
			else if ((ret = getFuncRetCallSentence(i)) != 0) {
                midExpr2mips();
                clear_midExpr();
				i += ret;
				status = 1;
			}
			else if ((ret = getFuncNoRetCallSentence(i)) != 0) {
                midExpr2mips();
                clear_midExpr();
				i += ret;
				status = 1;
			}
			else if ((ret = getAssignSentence(i)) != 0) {
				i += ret;
				status = 1;
			}
			else if ((ret = getScanfSentence(i)) != 0) {
				i += ret;
				status = 1;
			}
			else if ((ret = getPrintfSentence(i)) != 0) {
				i += ret;
				status = 1;
			}
			else if ((ret = getReturnSentence(i)) != 0) {
				i += ret;
				status = 1;
			}
			else if (word_get_list[i].type == "LBRACE") {
				i++;
				status = 2;
			}
			else {
                if (word_get_list[i].type == "SEMICN") {
                    status = -END;
                    i++;
                } else {
                    status = -END;
                }
			}
			break;
		case 1:
            if (word_get_list[i].type == "SEMICN") {
                status = -END;
                i++;
            } else {
                status = -END;
                missingSemicn(getWordLine(i - 1));
            }
			break;
		case 2:
			i += getSentenceList(i);
			status = 3;
			break;
		case 3:
			status = (word_get_list[i].type == "RBRACE") ? -END : -E_WRONG_FORMAT;
			i++;
			break;
		default:
			break;
		}
	}
	if (status != -E_WRONG_FORMAT) {
		term_set.insert(newTerm(YJ, i - 1, i - startIndex));
		clear_midExpr();
		return i - startIndex;
	}
	return 0;
}

int getSentenceList(int startIndex)						//语句列，可以为空
{
	int status = 0, i = startIndex;
	int ret;
	while (status != -E_WRONG_FORMAT && status != -END && i < word_get_list.size())
	{
		switch (status)
		{
		case 0:

			if ((ret = getSentence(i)) != 0)
			{
				i += ret;
			}
			else {
				status = -END;
			}
		default:
			break;
		}
	}
	term_set.insert(newTerm(YJL, i - 1, i - startIndex));
	return i - startIndex;
}

int getComplexSentence(int startIndex)					//复合语句
{
 	int status = 0, i = startIndex;
	int ret;
	while (status != -E_WRONG_FORMAT && status != -END && i < word_get_list.size())
	{
		switch (status)
		{
		case 0:
			if ((ret = getConstTK(i)) != 0)
			{
				i += ret;
				status = 1;
			}
			else if ((ret = getArgumentTK(i)) != 0)
			{
				i += ret;
				status = 2;
			}
			else {
				i += getSentenceList(i);
				status = -END;
			}
			break;
		case 1:
			if ((ret = getArgumentTK(i)) != 0)
			{
				i += ret;
				status = 2;
			}
			else {
				i += getSentenceList(i);
				status = -END;
			}
			break;
		case 2:
			i += getSentenceList(i);
			status = -END;
			break;
		default:
			break;
		}
	}
	term_set.insert(newTerm(FUHEYJ, i - 1, i - startIndex));
	return i - startIndex;
}

int getFuncDefNoReturn(int startIndex)					//无返回值函数定义
{
    clear_local_var();
	int status = 0, i = startIndex;
	int ret;
	isInLocal = true;
	while (status != -E_WRONG_FORMAT && status != -END && i < word_get_list.size())
	{
		switch (status)
		{
		case 0:
			status = (word_get_list[i].type == "VOIDTK") ? 7 : -E_WRONG_FORMAT;
			i++;
			break;
		case 7:
			status = (word_get_list[i].type == "IDENFR") ? 1 : -E_WRONG_FORMAT;
			if (word_get_list[i].type == "IDENFR") {
                mips_generate_label_func(word_get_list[i].word);
			}
			local_offset = -8;
			i++;
			break;
		case 1:
			status = (word_get_list[i].type == "LPARENT") ? 2 : -E_WRONG_FORMAT;
			i++;
			break;
		case 2:
            isInLocal = true;
			if ((ret = getArgumentList(i)) != -E_WRONG_FORMAT) {
			    paramOfFunction[word_get_list[startIndex + 1].word] = vector<sym_type>();
			    if (ret != 0) {
			        for(int j = 0; 3 * j < ret + 1; j++) {
			            sym_type type = (word_get_list[i + 3 * j].type == "INTTK") ? var_int : var_char;
                        paramOfFunction[word_get_list[startIndex + 1].word].push_back(type);
			        }
			    }
				i += ret;
				status = 3;
			}
			else {
				status = -E_WRONG_FORMAT;
			}
			break;
		case 3:
            if (word_get_list[i].type == "RPARENT") {
                status = 4;
                i++;
            } else {
                status = 4;
                missingRparent(getWordLine(i));
            }
			break;
		case 4:
			status = (word_get_list[i].type == "LBRACE") ? 5 : -E_WRONG_FORMAT;
			i++;
			break;
		case 5:
			ret = getComplexSentence(i);
			i += ret;
			status = 6;
			break;
		case 6:
			status = (word_get_list[i].type == "RBRACE") ? -END : -E_WRONG_FORMAT;
			i++;
			break;
		default:
			break;
		}
	}
	if (status != -E_WRONG_FORMAT) {
        if (searchInGlobal(word_get_list[startIndex + 1].word)) {
            duplicatedIdfName(getWordLine(startIndex));
        }
        for (pair<int, int> returnSentence: returnSentenceList) {
            if (returnSentence.second > 1) {
                funcVoidHasRet(getWordLine(returnSentence.first));
            }
        }
        globalSymStack.emplace_back(word_get_list[startIndex + 1].word, func_void);
		term_set.insert(newTerm(WFHZHSDY, i - 1, i - startIndex));
        isInLocal = false;
        localSymStack.clear();
        if (returnSentenceList.empty()) {
            textField.emplace_back("jr $ra");
        }
        returnSentenceList.clear();
		return i - startIndex;
	}
    isInLocal = false;
    localSymStack.clear();
	returnSentenceList.clear();
	return 0;
}

int getFuncDefReturn(int startIndex)									//有返回值函数定义
{
    clear_local_var();
	int status = 0, i = startIndex;
	int ret;
	isInLocal = true;
	while (status != -E_WRONG_FORMAT && status != -END && i < word_get_list.size())
	{
		switch (status)
		{
		case 0:
			if ((ret = getTKHeader(i)) != 0) {
				i += ret;
				status = 1;
			}
			else {
				status = -E_WRONG_FORMAT;
			}
			break;
		case 1:
			status = (word_get_list[i].type == "LPARENT") ? 2 : -E_WRONG_FORMAT;
			i++;
			break;
		case 2:
            isInLocal = true;
			if ((ret = getArgumentList(i)) != -E_WRONG_FORMAT) {
                if (ret != 0) {
                    paramOfFunction[word_get_list[startIndex + 1].word] = vector<sym_type>();
                    for(int j = 0; 3 * j < ret + 1; j++) {
                        sym_type type = (word_get_list[i + 3 * j].type == "INTTK") ? var_int : var_char;
                        paramOfFunction[word_get_list[startIndex + 1].word].push_back(type);
                    }
                } else {
                    paramOfFunction[word_get_list[startIndex + 1].word] = vector<sym_type>();
                }
				i += ret;
				status = 3;
			}
			else {
				status = -E_WRONG_FORMAT;
			}
			break;
		case 3:
            if (word_get_list[i].type == "RPARENT") {
                status = 4;
                i++;
            } else {
                status = 4;
                missingRparent(getWordLine(i));
            }
            func_have_ret.insert(word_get_list[startIndex + 1].word);
			break;
		case 4:
			status = (word_get_list[i].type == "LBRACE") ? 5 : -E_WRONG_FORMAT;
			i++;
			break;
		case 5:
			i += getComplexSentence(i);
			status = 6;
			break;
		case 6:
			status = (word_get_list[i].type == "RBRACE") ? -END : -E_WRONG_FORMAT;
			i++;
			break;
		default:
			break;
		}
	}
	if (status != -E_WRONG_FORMAT) {
	    if (returnSentenceList.empty()) {
	        returnSentenceIllegal(getWordLine(i - 1));
	    }
        for (pair<int, int> returnSentence: returnSentenceList) {
            if (returnSentence.second <= 3) {
                returnSentenceIllegal(getWordLine(returnSentence.first));
            }
            vector<vector<string>> prev = midExprList;
            int p = temp_var_no;
            string l = last_expr_name;
            string type = isCharRetExpression(returnSentence.first + 2, returnSentence.second - 3) ? "CHARTK" : "INTTK";
            temp_var_no = p;
            midExprList = prev;
            last_expr_name = l;
            if (word_get_list[startIndex].type != type) {
                returnSentenceIllegal(getWordLine(returnSentence.first));
            }
        }
		term_set.insert(newTerm(YFHZHSDY, i - 1, i - startIndex));
        isInLocal = false;
        localSymStack.clear();
        returnSentenceList.clear();
		return i - startIndex;
	}
	returnSentenceList.clear();
    isInLocal = false;
    localSymStack.clear();
	return 0;
}

int getMainFunc(int startIndex)
{
    clear_local_var();
	int status = 0, i = startIndex;
	int ret;
	isInLocal = true;
	while (status != -E_WRONG_FORMAT && status != -END && i < word_get_list.size())
	{
		switch (status)
		{
		case 0:
			status = (word_get_list[i].type == "VOIDTK") ? 7 : -E_WRONG_FORMAT;
			i++;
			break;
		case 7:
			status = (word_get_list[i].type == "MAINTK") ? 1 : -E_WRONG_FORMAT;
			mips_generate_label_func("main");
			local_offset = -8;
			i++;
			break;
		case 1:
			status = (word_get_list[i].type == "LPARENT") ? 2 : -E_WRONG_FORMAT;
			i++;
			break;
		case 2:
            isInLocal = true;
            if (word_get_list[i].type == "RPARENT") {
                status = 3;
                i++;
            } else {
                status = 3;
                missingRparent(getWordLine(i));
            }
			break;
		case 3:
			status = (word_get_list[i].type == "LBRACE") ? 4 : -E_WRONG_FORMAT;
			i++;
			break;
		case 4:
			i += getComplexSentence(i);
			status = 5;
			break;
		case 5:
			status = (word_get_list[i].type == "RBRACE") ? -END : -E_WRONG_FORMAT;
			i++;
			break;
		default:
			break;
		}
	}
	isInLocal = false;
	localSymStack.clear();
	if (status != -E_WRONG_FORMAT) {
        for (pair<int, int> returnSentence: returnSentenceList) {
            if (returnSentence.second > 1) {
                funcVoidHasRet(getWordLine(returnSentence.first));
            }
        }
		term_set.insert(newTerm(ZHS, i - 1, i - startIndex));
        if (returnSentenceList.empty()) {
            textField.emplace_back("jr $ra");
        }
        returnSentenceList.clear();
		return i - startIndex;
	}
	returnSentenceList.clear();
	return 0;
}

void grammar_analyze()
{
	int index = 0;
	int ret;
	if (index < word_get_list.size()) {
		ret = getConstTK(index);
		index += ret;
	}
	if (index < word_get_list.size()) {
		ret = getArgumentTK(index);
		index += ret;
	}
	while (true)
	{
		ret = getFuncDefNoReturn(index);
		if (ret != 0)
		{
			index += ret;
			continue;
		}
		else {
			ret = getFuncDefReturn(index);
			if (ret != 0)
			{
				index += ret;
				continue;
			}
			else
				break;
		}
	}
	if (index < word_get_list.size()) {
		ret = getMainFunc(index);
		index += ret;
	}
	if (ret != 0 && index == word_get_list.size())
	{
		term_set.insert(newTerm(CX, index - 1, index));
	}
}