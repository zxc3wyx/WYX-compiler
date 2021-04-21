#include <string>
#include <cctype>
#include <vector>
#include<unordered_map>
#include<unordered_set>
#include "lexlib.h"
#include "errorHandler.h"

using namespace std;

extern unordered_map<string, string> wordName2typeCode;
extern unordered_set<string> wordNameSet;
extern vector<Myword> word_get_list;
FILE* in;
vector<char> word;
vector<string> word_list;
vector<int> wordIndex2Line;

namespace strHandle {
    string vector2string(const vector<char>& src)
    {
        string s = string();
        for (char i : src) {
            s += i;
        }
        return s;
    }

    string lower(const string& src) {
        string s = string();
        for (char i : src)
        {
            s += tolower(i);
        }
        return s;
    }

    bool isIntCon(const string& src) {
        for (char c : src)
        {
            if (!isdigit(c))
            {
                return false;
            }
        }
        return true;
    }

    bool isIdentifier(string src) {
        if (src.empty())
        {
            return false;
        }
        if (!isalpha(src[0]) && src[0] != '_')
        {
            return false;
        }
        for (size_t i = 1; i < src.size(); i++)
        {
            if (!isalnum(src[i]) && src[i] != '_')
            {
                return false;
            }
        }
        return true;
    }
}

void readIn() {
    using namespace strHandle;
	char input = 0;
	int line = 1;
	while ((input = (char)fgetc(in)) != EOF) {
		if (input == '>' || input == '<' || input == '=' || input == '!') {
			if (!word.empty()) {
				word_list.push_back(vector2string(word));
				wordIndex2Line.push_back(line);
				word.clear();
			}
			char next = (char)fgetc(in);
			if (next == '=') {
				word_list.push_back(string() + input + "=");
                wordIndex2Line.push_back(line);
				next = (char)fgetc(in);
			}
			else {
				if (input != '!') {
					word_list.push_back(string() += input);
                    wordIndex2Line.push_back(line);
				}
			}
			input = next;
		}
		if (input == '\"') {
			string s = string();
			while (true)
			{
				input = (char)fgetc(in);
				if (input == '\"') {
					word_list.push_back(string("\"").append(s) + '\"');
                    wordIndex2Line.push_back(line);
					if (s.empty()) {
					    illegalSymOrLex(line);
					}
					break;
				} else if (input != 32 && input != 33 && (input < 35 || input > 126)) {
                    illegalSymOrLex(line);
                }
				s += input;
			}
		}
		if (input == '\'')
		{
			input = (char)fgetc(in);
			word_list.push_back(string("\'") + input + '\'');
            wordIndex2Line.push_back(line);
            if (input != '+' && input != '-' && input != '*' && input != '/' && !isalnum(input) && input != '_') {
                illegalSymOrLex(line);
            }
			input = (char)fgetc(in);
		}
		if (isalnum(input) || input == '_') {
			word.push_back(input);
		}
		else {
			if (!word.empty()) {
				word_list.push_back(vector2string(word));
                wordIndex2Line.push_back(line);
				word.clear();
			}
			if (wordNameSet.find(string() += input) != wordNameSet.end()) {
				word_list.push_back(string() += input);
                wordIndex2Line.push_back(line);
			}
			if (input == '\n') {
			    line++;
			}
		}
	}
}

int getWordLine(int index)                          //根据下标获取行号
{
    return wordIndex2Line[index];
}

void lexical_analyze()
{
	initMap();
	initSet();
	using namespace strHandle;
	in = fopen("testfile.txt", "r");
	if (in == nullptr) {
	    exit(1);
	}
    readIn();
	for (const string& it : word_list) {
		if (wordNameSet.find(lower(it)) != wordNameSet.end()) {
			word_get_list.emplace_back(it, wordName2typeCode[lower(it)]);
		}
		else if (it[0] == '\'') {
			word_get_list.emplace_back(string() += it[1], "CHARCON");
		}
		else if (it[0] == '\"') {
			word_get_list.emplace_back(it.substr(1, it.size() - 2), "STRCON");
		}
		else if (isIntCon(it)) {
			word_get_list.emplace_back(it, "INTCON");
		}
		else if (isIdentifier(it)) {
			word_get_list.emplace_back(lower(it), "IDENFR");
		}
	}
}