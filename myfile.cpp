// Names:                IDs:
// ----------------------------
// Basmala Khalifa       20226122
// Amira Emad            20226017
// Raghad Ahmed          20226041

// Extended BNF (EBNF) Grammar:
// ----------------------------
// expr   -> factor { '.' factor }      left associative (product)
// factor -> base [ '^' '-' '1' ]       inverse (right binding, optional)
// base   -> '(' expr ')' | id
// id     -> any single lowercase letter (x, y, z, e, a, b, ...)

#include <cstdio>
#include <cstdlib>
#include <cstring>
using namespace std;

////////////////////////////////////////////////////////////////////////////////////

struct InFile
{
    FILE* file;

    InFile(const char* str) {file=fopen(str, "r");}
    ~InFile() {if(file) fclose(file);}

    char GetNextChar()
    {
        int ch=fgetc(file);
        if(ch==EOF) return 0;
        return ch;
    }

    char PeekNextChar()
    {
        int ch=fgetc(file);
        if(ch==EOF) return 0;
        ungetc(ch, file);
        return ch;
    }
};

struct OutFile
{
    FILE* file;

    OutFile(const char* str) {file=0; if(str) file=fopen(str, "w");}
    ~OutFile() {if(file) fclose(file);}

    void Out(const char* s)
    {
        fprintf(file, "%s\n", s); fflush(file);
    }
};

struct CompilerInfo
{
    InFile in_file;
    OutFile out_file;

    CompilerInfo(const char* in_str, const char* out_str)
                : in_file(in_str), out_file(out_str)
    {
    }
};

////////////////////////////////////////////////////////////////////////////////////

enum TokenType
{
    ID, PRODUCT, INVERSE, LEFT_PAREN, RIGHT_PAREN, ENDFILE, ERROR
};

const char* TokenTypeStr[]=
{
    "ID", "PRODUCT", "INVERSE", "LEFT_PAREN", "RIGHT_PAREN", "ENDFILE", "ERROR"
};

struct Token
{
    TokenType type;
    char ch;

    Token() {ch=0; type=ERROR;}
    Token(TokenType _type, char _ch) {type=_type; ch=_ch;}
};

const Token symbolic_tokens[]=
{
    Token(PRODUCT, '.'),
    Token(LEFT_PAREN, '('),
    Token(RIGHT_PAREN, ')')
};
const int num_symbolic_tokens=sizeof(symbolic_tokens)/sizeof(symbolic_tokens[0]);

void GetNextToken(CompilerInfo* pci, Token* ptoken)
{
    ptoken->type=ERROR;
    ptoken->ch=0;

    char ch=pci->in_file.GetNextChar();

    if(ch==0) {ptoken->type=ENDFILE; return;}

    int i;
    for(i=0;i<num_symbolic_tokens;i++)
    {
        if(ch==symbolic_tokens[i].ch)
        {
            ptoken->type=symbolic_tokens[i].type;
            ptoken->ch=ch;
            return;
        }
    }

    if(ch=='^')
    {
        char c2=pci->in_file.GetNextChar();
        char c3=pci->in_file.GetNextChar();
        if(c2=='-' && c3=='1') {ptoken->type=INVERSE; ptoken->ch=0; return;}
        ptoken->type=ERROR; ptoken->ch=ch; return;
    }

    if(ch>='a' && ch<='z') {ptoken->type=ID; ptoken->ch=ch; return;}

    ptoken->type=ERROR; ptoken->ch=ch;
}

////////////////////////////////////////////////////////////////////////////////////
//Parser


////////////////////////////////////////////////////////////////////////////////////
//Print


////////////////////////////////////////////////////////////////////////////////////
//Main
int main()
{

    return 0;
}

////////////////////////////////////////////////////////////////////////////////////
