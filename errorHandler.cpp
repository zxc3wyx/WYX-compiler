//
// Created by hswyx666 on 2020/10/21.
//

#include "errorHandler.h"
#include <set>
#include <cstdio>

using std::set;

set<CompileError> errorList;

void illegalSymOrLex(int line) {
    errorList.insert(CompileError('a', line));
}

void duplicatedIdfName(int line) {
    errorList.insert(CompileError('b', line));
}

void neverStatedIdfName(int line) {
    errorList.insert(CompileError('c', line));
}

void paramNumNoMatch(int line) {
    errorList.insert(CompileError('d', line));
}

void paramTypeNoMatch(int line) {
    errorList.insert(CompileError('e', line));
}

void branchTypeIllegal(int line) {
    errorList.insert(CompileError('f', line));
}

void funcVoidHasRet(int line) {
    errorList.insert(CompileError('g', line));
}

void returnSentenceIllegal(int line) {
    errorList.insert(CompileError('h', line));
}

void charArraySubIndex(int line) {
    errorList.insert(CompileError('i', line));
}

void assignConstVar(int line) {
    errorList.insert(CompileError('j', line));
}

void missingSemicn(int line) {
    errorList.insert(CompileError('k', line));
}

void missingRparent(int line) {
    errorList.insert(CompileError('l', line));
}

void missingRbrack(int line) {
    errorList.insert(CompileError('m', line));
}

void arrayInitNoMatch(int line) {
    errorList.insert(CompileError('n', line));
}

void constTypeNoMatch(int line) {
    errorList.insert(CompileError('o', line));
}

void missingDefault(int line) {
    errorList.insert(CompileError('p', line));
}

void errorHandle() {
    FILE *out = fopen("error.txt", "w");
    for (CompileError error: errorList) {
        fprintf(out, "%d %c\n", error.getLine(), error.getErrorCode());
    }
}

bool CompileError::operator<(const CompileError &other) const {
    if (this->errorCode == 'a' && other.errorCode != 'a') {
        return true;
    }
    else if (this->errorCode != 'a' && other.errorCode == 'a') {
        return false;
    }
    return this->line < other.line;
}
