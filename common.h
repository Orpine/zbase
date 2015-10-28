//
// Created by Orpine on 10/27/15.
//

#ifndef ZBASE_COMMON_H
#define ZBASE_COMMON_H

#include <string>
#include <vector>
#include "zbase.h"
using namespace std;

typedef enum {
    CREATETABLE,
    DROPTABLE,
    CREATEINDEX,
    DROPINDEX,
    SELECT,
    INSERT,
    DELETE,
    EXIT,
    EMPTY,
    ATTR,
    VALUE
} nodeType;

struct State {
    nodeType type;

    string relationName;
    string indexName;
    string attrName;
    string pAttrName;
    vector<AttrInfo> attrs;
    vector<Value> values;
    vector<Condition> conditions;
};

struct Type {
    int ival;
    float fval;
    string sval;
    CmpOp cval;
    Property pval;
    State tval;
};

#define YYSTYPE Type

extern "C" {
    extern int yylex(void);
}

void yyerror(const char *);

#endif //ZBASE_COMMON_H