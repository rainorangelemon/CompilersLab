typedef struct Type_* Type;
typedef struct FieldList_* FieldList;

struct Type_
{
  enum { BASIC, ARRAY, STRUCTURE } kind;
  union
  {
    // basic type
    int basic;
    // array type
    struct { Type elem; int size; } array;
    FieldList structure;
  } u;
};

struct FieldList_
{
  char* name; // name of field
  Type type; // type of field
  FieldList tail; // next field
};
