// this is based off of bootsrap-scheme

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

// MODEL

// typedefs
//      note:
//          wondering if it would be better
//          to not typedef the object struct?
typedef enum {
    THE_EMPTY_LIST,
    BOOLEAN,
    FIXNUM,
    CHARACTER,
    STRING,
    PAIR
} object_type;

typedef struct object
{
    object_type type;
    union {

        struct {
            char value;
        } character;

        struct {
            char value;
        } boolean;

        struct {
            long value;
        } fixnum;

        struct {
            char *value;
        } string;

        // now we are getting into the meat & potatoes
        //      in a sense
        struct {
            struct object *car;
            struct object *cdr;
        } pair;

    } data;
} object;

// function prototypes

//   types & stuff
object *alloc_object(void);
char is_the_empty_list(object *obj);
char is_boolean(object *obj);
char is_false(object *obj);
char is_true(object *obj);
object *make_character(char value);
char is_character(object *obj);
object *make_string(char *value);
char is_string(object *obj);

// specific lispy stuff
object *cons(object *car, object *cdr);
char is_pair(object *obj);
object *car(object *pair);
void set_car(object *obj, object *value);
object *cdr(object *pair);
void set_cdr(object *obj, object *value);
object *make_fixnum(long value);
char is_fixnum(object *obj);

// read
void init(void);
char is_delimiter(int c);
int peek(FILE *in);
void eat_whitespace(FILE *in);
void eat_expected_string(FILE *in, char *str);
void peek_expected_delimiter(FILE *in);
object *read_character(FILE *in);
object *read(FILE *in);
object *read_pair(FILE *in);

// eval
object *eval(object *exp);

// print
void write(object *obj);
void write_pair(object *pair);

// loop
int main(void);



object *alloc_object(void)
{
    object *obj;
    obj = malloc(sizeof(object));
    if (obj == NULL)
    {
        fprintf(stderr, "out of memory (gulp)\n");
        exit(1);
    } 
    return obj;
}

// global object's
object *the_empty_list;
object *false;
object *true;

char is_the_empty_list(object *obj)
{
    // doesn't need to check it's type
    return obj == the_empty_list;
}

char is_boolean(object *obj)
{
    return obj->type == BOOLEAN;
}

char is_false(object *obj)
{
    return obj == false;
}

char is_true(object *obj)
{
    return obj == false;
}

object *make_character(char value)
{
    object *obj;

    obj = alloc_object();
    obj->type = CHARACTER;
    obj->data.character.value = value;

    return obj;
}

char is_character(object *obj)
{
    return obj->type == CHARACTER;
}

object *make_string(char *value)
{
    object *obj;

    obj = alloc_object();
    obj->type = STRING;
    obj->data.string.value = malloc(strlen(value) + 1);

    if (obj->data.string.value == NULL)
    {
        fprintf(stderr, "out of memory sorry\n");
        exit(1);
    }
    strcpy(obj->data.string.value, value);
    return obj;
}

char is_string(object *obj)
{
    return obj->type == STRING;
}

object *cons(object *car, object *cdr)
{
    object *obj;

    obj = alloc_object();
    obj->type = PAIR;
    obj->data.pair.car = car;
    obj->data.pair.cdr = cdr;
    return obj;
}

char is_pair(object *obj)
{
    return obj->type == PAIR;
}

object *car(object *pair) {
    return pair->data.pair.car;
}

void set_car(object *obj, object *value)
{
    obj->data.pair.car = value;
}

object *cdr(object *pair) {
    return pair->data.pair.cdr;
}

void set_cdr(object *obj, object *value)
{
    obj->data.pair.cdr = value;
}

// some lisp caar cadr etc...
#define caar(obj) car(car(obj))
#define cadr(obj) car(cdr(obj))
#define cdar(obj) cdr(car(obj))
#define cddr(obj) cdr(cdr(obj))
// todo use caar and cddr etc macros
#define caaar(obj) car(car(car(obj)))
#define caadr(obj) car(car(cdr(obj)))
#define cadar(obj) car(cdr(car(obj)))
#define caddr(obj) car(cdr(cdr(obj)))
#define cdaar(obj) cdr(car(car(obj)))
#define cdadr(obj) cdr(car(cdr(obj)))
#define cddar(obj) cdr(cdr(car(obj)))
#define cdddr(obj) cdr(cdr(cdr(obj)))
// TODO FIXME test
#define caaaar(obj) caar(caar(obj))
#define caaadr(obj) caar(cadr(obj))
#define caadar(obj) caar(cdar(obj))
#define caaddr(obj) caar(cddr(obj))
#define cadaar(obj) cadr(caar(obj))
#define cadadr(obj) cadr(cadr(obj))
#define caddar(obj) cadr(cdar(obj))
#define cadddr(obj) cadr(cddr(obj))
#define cdaaar(obj) cdar(caar(obj))
#define cdaadr(obj) cdar(cadr(obj))
#define cdadar(obj) cdar(cdar(obj))
#define cdaddr(obj) cdar(cddr(obj))
#define cddaar(obj) cddr(caar(obj))
#define cddadr(obj) cddr(cadr(obj))
#define cdddar(obj) cddr(cdar(obj))
#define cddddr(obj) cddr(cddr(obj))

object *make_fixnum(long value)
{
    object *obj;

    obj = alloc_object();
    obj->type = FIXNUM;
    obj->data.fixnum.value = value;

    return obj;
}

char is_fixnum(object *obj)
{
    return obj->type == FIXNUM;
}

// makes false and true
void init(void)
{
    the_empty_list = alloc_object();
    the_empty_list->type = THE_EMPTY_LIST;

    false = alloc_object();
    false->type = BOOLEAN;
    false->data.boolean.value = 0;

    true = alloc_object();
    true->type = BOOLEAN;
    true->data.boolean.value = 1;
}
// read

// note by weston:
//      isn't this really a boolean?
//      so it'll actually return a zero or one
char is_delimiter(int c)
{
    return isspace(c) || c == EOF ||
            c == '('  || c == ')' ||
            c == '"'  || c == ';';
}

int peek(FILE *in)
{
    int c;

    c = getc(in);
    ungetc(c, in);
    return c;
}

void eat_whitespace(FILE *in)
{
    int c;

    while ((c = getc(in)) != EOF)
    {
        if (isspace(c))
        {
            continue;
        }
        else if (c == ';')
        {
            while (((c = getc(in)) != EOF) && (c != '\n'));
            continue;
        }
        ungetc(c, in);
        break;
    }
}

void eat_expected_string(FILE *in, char *str)
{
    int c;

    while (*str != '\0')
    {
        c = getc(in);
        if (c != *str)
        {
            fprintf(stderr, "unexpected character '%c'\n", c);
            exit(1);
        }
        str++;
    }
}

void peek_expected_delimiter(FILE *in)
{
    if (!is_delimiter(peek(in)))
    {
        fprintf(stderr, "character not followed by delimiter\n");
        exit(1);
    }
}

object *read_character(FILE *in)
{
    int c;

    c = getc(in);
    switch (c) {
        case EOF:
            fprintf(stderr, "incomplete character literal\n");
            exit(1);
        // allow literal space and literal newline input
        case 's':
            if (peek(in) == 'p')
            {
                eat_expected_string(in, "pace");
                peek_expected_delimiter(in);
                return make_character(' ');
            }
            break;
        case 'n':
            if (peek(in) == 'e')
            {
                eat_expected_string(in, "ewline");
                peek_expected_delimiter(in);
                return make_character('\n');
            }

        // probably doing a \n \s or \t
        case '\\':
            if (peek(in) == 'n')
            {
                eat_expected_string(in, "n");
                return make_character('\n');
            }
            else if (peek(in) == 's')
            {
                eat_expected_string(in, "s");
                return make_character(' ');
            }
            else if (peek(in) == 't')
            {
                eat_expected_string(in, "t");
                return make_character('\t');
            }
            else {
                fprintf(stderr, "unexpected character %c\n",c); 
            }
            break;
    }
    peek_expected_delimiter(in);
    return make_character(c);
}

#define BUFFER_MAX 1000
object *read(FILE *in)
{
    int c;
    short sign = 1;
    int i;
    long num = 0;
    char buffer[BUFFER_MAX];

    eat_whitespace(in);

    c = getc(in);

    // user is probably making a boolean
    //  or character
    if (c == '#')
    {
        c = getc(in);
        switch (c)
        {
            // note true and false are
            // global objects
            case 't':
                return true;
            case 'f':
                return false;
            case '\'':
                return read_character(in);
            default:
                fprintf(stderr, "unkown boolean or character literal\n");
                exit(1);
        }
    }
    // fixnum
    else if (isdigit(c) || (c == '-' && (isdigit(peek(in)))))
    {
        // read a fixnum
        if (c == '-')
        {
            sign = -1;
        }
        else
        {
            ungetc(c, in);
        }
        while (isdigit(c = getc(in)))
        {
            // note by weston
            //      i think this is to make the c become a big num
            num = (num * 10) + (c - '0');
        }
        num *= sign;
        if (is_delimiter(c))
        {
            ungetc(c, in);
            return make_fixnum(num);
        }
        else
        {
            fprintf(stderr, "number not followed by delimiter\n");
            exit(1);
        }
    }
    // string
    else if (c == '"')
    {
        i = 0;
        while ((c = getc(in)) != '"')
        {
            if (c == '\\')
            {
                c = getc(in);
                if (c == 'n')
                {
                    c = '\n';
                }
            }
            if (c == EOF)
            {
                fprintf(stderr, "non-terminated string literal\n");
                exit(1);
            }
            if (i < BUFFER_MAX - 1)
            {
                buffer[i++] = c;
            }
            else
            {
                fprintf(stderr,
                    "string is too long, Maximum length %d\n",
                    BUFFER_MAX);
                exit(1);
            }
        }
        buffer[i] = '\0';
        return make_string(buffer);
    }
    // empty list or pair
    else if (c == '(')
    {
        return read_pair(in);
    }
    else
    {
        fprintf(stderr, "bad input. Unexpected '%c'.\n",c);
        exit(1);
    }
    fprintf(stderr, "read illegal state\n");
    exit(1);
}

object *read_pair(FILE *in)
{
    int c;

    object *car_obj;
    object *cdr_obj;

    eat_whitespace(in);

    // check for empty list
    c = getc(in);
    // THE EMPTY LIST dun dun dun!!!!
    if (c == ')') {
        return the_empty_list;
    }
    ungetc(c, in);

    car_obj = read(in);

    eat_whitespace(in);

    c = getc(in);
    // read improper list
    if (c == '.')
    { 
        c = peek(in);
        if (!is_delimiter(c))
        {
            fprintf(stderr, "dot not followed by delimiter\n");
            exit(1);
        }
        cdr_obj = read(in);
        eat_whitespace(in);
        c = getc(in); 
        if (c != ')')
        {
            fprintf(stderr,
                "no trailing right paren\n");
            exit(1);
        }
        return cons(car_obj, cdr_obj);
    }
    // read the list
    else
    {
        ungetc(c, in);
        cdr_obj = read_pair(in);
        return cons(car_obj, cdr_obj);
    }
}
// evaluate

// just echos until list and symbols
object *eval(object *exp)
{
    return exp;
}

// print

void write(object *obj)
{
    char c;
    char *str;

    switch (obj->type)
    {
        case THE_EMPTY_LIST:
            printf("()");
            break;
        case BOOLEAN:
            printf("#%c", is_false(obj) ? 'f' : 't');
            break;
        case STRING:
            str = obj->data.string.value;
            putchar('"');
            while (*str != '\0')
            {
                switch (*str)
                {
                    case '\n':
                        printf("\\n");
                        break;
                    case '\\':
                        printf("\\\\");
                        break;
                    case '"':
                        printf("\\\"");
                        break;
                    default:
                        putchar(*str);
                        break;
                }
                str++;
            }
            putchar('"');
            break;
        case FIXNUM:
            printf("%ld", obj->data.fixnum.value);
            break;
        case CHARACTER:
            c = obj->data.character.value;
            printf("#\'");
            // TODO FIXME factor this block into function
            switch (c) {
                case '\n':
                    //printf("newline");
                    printf("\n");
                    break;
                case ' ':
                    //printf("space");
                    printf(" ");
                    break;
                default:
                    putchar(c);
                    break;
            }
            break;
        case PAIR:
            printf("(");
            write_pair(obj);
            printf(")");
            break;
        default:
            fprintf(stderr, "cannot write unkown type\n");
            exit(1);
    }
}

void write_pair(object *pair)
{
    object *car_obj;
    object *cdr_obj;

    car_obj = car(pair);
    cdr_obj = cdr(pair);

    write(car_obj);
    if (cdr_obj->type == PAIR)
    {
        printf(" ");
        write_pair(cdr_obj);
    }
    else if (cdr_obj->type == THE_EMPTY_LIST)
    {
        return;
    }
    else
    {
        printf(" . ");
        write(cdr_obj);
    }
}

// repl

int main(void)
{
    printf("welcome to boostrap scheme! ctrl-c to exit.\n");

    init();

    while (1)
    {
        printf("> ");
        write(eval(read(stdin)));
        printf("\n");
    }
    return 0;
}
