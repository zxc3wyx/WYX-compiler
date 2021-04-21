//
// Created by hswyx666 on 2020/10/21.
//

#ifndef COHW4_ERRORHANDLER_H
#define COHW4_ERRORHANDLER_H

class CompileError {
private:
    char errorCode;
    int line;
public:
    CompileError(char errorCode, int line) : errorCode(errorCode), line(line) {}

    char getErrorCode() const {
        return errorCode;
    }

    int getLine() const {
        return line;
    }

    bool operator< (const CompileError &other) const;
};

void illegalSymOrLex(int line);
void duplicatedIdfName(int);
void neverStatedIdfName(int);
void paramNumNoMatch(int);
void paramTypeNoMatch(int);
void arrayInitNoMatch(int);
void branchTypeIllegal(int);
void funcVoidHasRet(int);
void returnSentenceIllegal(int);
void charArraySubIndex(int);
void assignConstVar(int);
void missingSemicn(int);
void missingRparent(int);
void missingRbrack(int);
void constTypeNoMatch(int);
void missingDefault(int);
void errorHandle();

#endif //COHW4_ERRORHANDLER_H
