#Interpreter

熊郁文 3130000829

##需求说明

1. 程序流程控制，即“启动并初始化  ‘接收命令、处理命令、显示命令结果’循环  退出”流程。


CREATE TABLE 表名 (

```sql
DROP TABLE 表名；
CRATE INDEX 索引名 ON 表名 (列名);

DROP INDEX 索引名；

SELECT * FROM 表名;

```sql
INSERT INTO 表名 VALUES (值1，值2, ...,值n);

DELETE FORM 表名;


```sql
QUIT;
```

```sql
EXECFILE 脚本文件名；
```

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
    vector<AttrInfo> attrs;
    vector<Value> values;
    vector<Condition> conditions;
    void clear() {
        attrs.clear();
        values.clear();
        conditions.clear();
    }
};
```