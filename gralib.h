#ifndef GRALIB_H
#define GRALIB_H

#include <string>


using std::string;
using std::pair;

typedef pair<string, pair<int, int>> Term;


pair<string, pair<int, int>> newTerm(const string& type, int lastIndex, int length);

string termGetType(const pair<string, pair<int, int>>& term);

int termGetLastIndex(const pair<string, pair<int, int>>& term);

int termGetLength(const pair<string, pair<int, int>>& term);


void grammar_analyze();

#endif // DEBUG

