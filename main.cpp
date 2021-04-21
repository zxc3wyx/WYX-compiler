#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif

#include<cstdio>
#include <cstring>
#include<cctype>
#include<vector>
#include <map>
#include<set>
#include "lexlib.h"
#include "gralib.h"
#include "errorHandler.h"
#include "mips.h"


using namespace std;
extern vector<Myword> word_get_list;
extern set<Term> term_set;
extern map<Term, int> termOrder;

int termCompare(const Term &a, const Term &b) {
    return termOrder.at(a) - termOrder.at(b);
}

int main() {
    lexical_analyze();
    grammar_analyze();
    errorHandle();
    code_generate();
    FILE *out = fopen("output.txt", "w");
    size_t cnt = 0, size_array_set = term_set.size();
    Term *term_array = new Term[size_array_set];
    for (const Term &term : term_set) {
        term_array[cnt] = term;
        char dest[50] = {0};
        strcpy(dest, termGetType(term_array[cnt]).c_str());
        memset(dest, 0, 50);
        cnt++;
    }
    //sort
    for (size_t i = 0; i < cnt - 1; ++i) {
        for (size_t j = 0; j < cnt - i - 1; ++j) {
            if (termCompare(term_array[j], term_array[j + 1]) > 0) {
                Term tmp = term_array[j];
                term_array[j] = term_array[j + 1];
                term_array[j + 1] = tmp;
            }
        }
    }
    //
    int term_array_ptr = 0;
    for (size_t index = 0; index < word_get_list.size(); index++) {
        Myword myword = word_get_list[index];
        fprintf(out, "%s %s\n", myword.type.c_str(), myword.word.c_str());
        while (term_array_ptr < cnt && termGetLastIndex(term_array[term_array_ptr]) == index) {
            fprintf(out, "%s\n", termGetType(term_array[term_array_ptr]).c_str());
            term_array_ptr++;
        }
    }
    while (term_array_ptr < cnt && termGetLastIndex(term_array[term_array_ptr]) == word_get_list.size()) {
        fprintf(out, "%s\n", termGetType(term_array[term_array_ptr]).c_str());
        term_array_ptr++;
    }
    return 0;
}