// this is based off of bootsrap-scheme
//

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

// MODEL

typedef enum {BOOLEAN, FIXNUM, CHARACTER} object_type;

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

    } data;
} object;

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

// note from weston:
//      what does this mean / do?
object *false;
object *true;

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
            break;
    }
    peek_expected_delimiter(in);
    return make_character(c);
}

object *read(FILE *in)
{
    int c;
    short sign = 1;
    long num = 0;

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
            case '\\':
                return read_character(in);
            default:
                fprintf(stderr, "unkown boolean or character literal\n");
                exit(1);
        }
    }
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
    else {
        fprintf(stderr, "bad input. unexpected '%c'\n", c);
    }
    fprintf(stderr, "read illegal state\n");
    exit(1);
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

    switch (obj->type)
    {
        case BOOLEAN:
            printf("#%c", is_false(obj) ? 'f' : 't');
            break;
        case FIXNUM:
            printf("%ld", obj->data.fixnum.value);
            break;
        case CHARACTER:
            c = obj->data.character.value;
            printf("#\\");
            // TODO FIXME factor this block into function
            switch (c) {
                case '\n':
                    printf("newline");
                    break;
                case ' ':
                    printf("space");
                    break;
                default:
                    putchar(c);
                    break;
            }
            break;
        default:
            fprintf(stderr, "cannot write unkown type\n");
            exit(1);
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
