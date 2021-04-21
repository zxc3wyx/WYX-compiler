//
// Created by hswyx666 on 2020/10/15.
//

#ifndef LEXLIB_H
#define LEXLIB_H

#include <string>
#include <utility>
#include <vector>

using std::string;
using std::vector;

class Myword{
public:
    string word;
    string type;
    Myword(string word, string type) {
        this->word = std::move(word);
        this->type = std::move(type);
    }
};


/*init*/
void initMap();
void initSet();

/*lexicalAnalyze*/
namespace strHandle {
    string vector2string(const vector<char>& src);
    string lower(const string& src);
    bool isIntCon(const string& src);
    bool isIdentifier(string src);
}


void lexical_analyze();
int getWordLine(int index);


#endif // !1

