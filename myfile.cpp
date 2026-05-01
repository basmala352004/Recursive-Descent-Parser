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
        if(c2=='-' && c3=='1')
        {
            ptoken->type=INVERSE;
            return;
        }
        ptoken->type=ERROR;
        return;
    }

    if(ch>='a' && ch<='z')
    {
        ptoken->type=ID;
        ptoken->ch=ch;
        return;
    }

    ptoken->type=ERROR;
}

////////////////////////////////////////////////////////////////////////////////////
// Parser

enum NodeKind
{
    PRODUCT_NODE, INVERSE_NODE, ID_NODE
};

#define MAX_CHILDREN 2

struct TreeNode
{
    TreeNode* child[MAX_CHILDREN];
    NodeKind node_kind;
    char id;

    TreeNode()
    {
        int i;
        for(i=0;i<MAX_CHILDREN;i++) child[i]=0;
        id=0;
    }
};

struct ParseInfo
{
    Token next_token;
};

void Match(CompilerInfo* pci, ParseInfo* ppi, TokenType expected_type)
{
    if(ppi->next_token.type!=expected_type) throw 0;
    GetNextToken(pci, &ppi->next_token);
}

TreeNode* Expr(CompilerInfo*, ParseInfo*);

TreeNode* Base(CompilerInfo* pci, ParseInfo* ppi)
{
    TreeNode* tree=0;

    if(ppi->next_token.type==ID)
    {
        tree=new TreeNode();
        tree->node_kind=ID_NODE;
        tree->id=ppi->next_token.ch;
        Match(pci, ppi, ID);
        return tree;
    }

    if(ppi->next_token.type==LEFT_PAREN)
    {
        Match(pci, ppi, LEFT_PAREN);
        tree=Expr(pci, ppi);
        Match(pci, ppi, RIGHT_PAREN);
        return tree;
    }

    throw 0;
}

TreeNode* Factor(CompilerInfo* pci, ParseInfo* ppi)
{
    TreeNode* tree=Base(pci, ppi);

    if(ppi->next_token.type==INVERSE)
    {
        TreeNode* inv_tree=new TreeNode();
        inv_tree->node_kind=INVERSE_NODE;
        inv_tree->child[0]=tree;
        Match(pci, ppi, INVERSE);
        return inv_tree;
    }

    return tree;
}

TreeNode* Expr(CompilerInfo* pci, ParseInfo* ppi)
{
    TreeNode* tree=Factor(pci, ppi);

    while(ppi->next_token.type==PRODUCT)
    {
        TreeNode* prod_tree=new TreeNode();
        prod_tree->node_kind=PRODUCT_NODE;
        prod_tree->child[0]=tree;

        Match(pci, ppi, PRODUCT);

        prod_tree->child[1]=Factor(pci, ppi);
        tree=prod_tree;
    }

    return tree;
}

TreeNode* Parse(CompilerInfo* pci)
{
    ParseInfo parse_info;

    GetNextToken(pci, &parse_info.next_token);

    TreeNode* tree=Expr(pci, &parse_info);

    if(parse_info.next_token.type!=ENDFILE)
        printf("Error\n");

    return tree;
}

////////////////////////////////////////////////////////////////////////////////////
// Print

void PrintTree(TreeNode* tree, int level)
{
    if(!tree) return;

    int i;
    for(i=0;i<level;i++) printf("   ");

    if(level>0) printf("|--");

    if(tree->node_kind==PRODUCT_NODE)
        printf("product\n");
    else if(tree->node_kind==INVERSE_NODE)
        printf("inverse\n");
    else
        printf("%c\n", tree->id);

    for(i=0;i<MAX_CHILDREN;i++)
        PrintTree(tree->child[i], level+1);
}

////////////////////////////////////////////////////////////////////////////////////
// Destroy

void DestroyTree(TreeNode* tree)
{
    if(!tree) return;

    int i;
    for(i=0;i<MAX_CHILDREN;i++)
        DestroyTree(tree->child[i]);

    delete tree;
}

////////////////////////////////////////////////////////////////////////////////////
// Main

int main()
{
    CompilerInfo compiler("input.txt", "output.txt");

    TreeNode* tree=0;

    try
    {
        tree=Parse(&compiler);
        PrintTree(tree, 0);
    }
    catch(...)
    {
        printf("Parse Error\n");
    }

    DestroyTree(tree);

    return 0;
}

////////////////////////////////////////////////////////////////////////////////////