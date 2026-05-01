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
using namespace std;

////////////////////////////////////////////////////////////////////////////////////
// Input

const char* g_input;
int g_pos;

char GetNextChar()
{
    if(g_input[g_pos]==0) return 0;
    return g_input[g_pos++];
}

char PeekNextChar()
{
    if(g_input[g_pos]==0) return 0;
    return g_input[g_pos];
}

struct CompilerInfo
{
    const char* input;
};

////////////////////////////////////////////////////////////////////////////////////
// Scanner

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

    char ch=GetNextChar();

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
        char c2=GetNextChar();
        char c3=GetNextChar();
        if(c2=='-' && c3=='1') {ptoken->type=INVERSE; ptoken->ch=0; return;}
        ptoken->type=ERROR; ptoken->ch=ch; return;
    }

    if(ch>='a' && ch<='z') {ptoken->type=ID; ptoken->ch=ch; return;}

    ptoken->type=ERROR; ptoken->ch=ch;
}

////////////////////////////////////////////////////////////////////////////////////
// Parser

enum NodeKind
{
    PRODUCT_NODE, INVERSE_NODE, ID_NODE
};

const char* NodeKindStr[]=
{
    "product", "inverse", "ID"
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

// base -> '(' expr ')' | id
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

// factor -> base [ '^-1' ]     inverse is right binding and optional
TreeNode* Factor(CompilerInfo* pci, ParseInfo* ppi)
{
    TreeNode* tree=Base(pci, ppi);
    if(ppi->next_token.type==INVERSE)
    {
        TreeNode* inv_tree=new TreeNode;
        inv_tree->node_kind=INVERSE_NODE;
        inv_tree->child[0]=tree;
        Match(pci, ppi, INVERSE);
        return inv_tree;
    }
    return tree;
}

// expr -> factor { '.' factor }     left associative
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
    TreeNode* syntax_tree=Expr(pci, &parse_info);
    if(parse_info.next_token.type!=ENDFILE)
        printf("Error: expression ends before file ends\n");
    return syntax_tree;
}

////////////////////////////////////////////////////////////////////////////////////
// Print

void PrintTree(TreeNode* tree, int level)
{
    if(!tree) return;

    int i;
    for(i=0;i<level;i++) printf("   ");

    if(level>0) printf("|--");

    if(tree->node_kind==PRODUCT_NODE) printf("product\n");
    else if(tree->node_kind==INVERSE_NODE) printf("inverse\n");
    else printf("%c\n", tree->id);

    for(i=0;i<MAX_CHILDREN;i++) PrintTree(tree->child[i], level+1);
}

////////////////////////////////////////////////////////////////////////////////////
// Destroy

void DestroyTree(TreeNode* tree)
{
    if(!tree) return;

    int i;
    for(i=0;i<MAX_CHILDREN;i++) DestroyTree(tree->child[i]);

    delete tree;
}

////////////////////////////////////////////////////////////////////////////////////
// Main

int main()
{
    //g_input="((x.y^-1).z)^-1";
    //g_input="((a.b)^-1.(b.c)^-1.(c.a)^-1)^-1";
    //g_input="(x^-1.(x.y))^-1.z";
    //g_input="((a.b).(c.d))^-1.((d.c).(b.a))";
    //g_input="(x.(y.(z.x^-1)^-1)^-1)^-1";
    //g_input="((a^-1.b)^-1.(b^-1.c)^-1.(c^-1.a)^-1)^-1";
    //g_input="(x.y^-1.z)^-1.(z^-1.y.x^-1)";
    //g_input="((a.b^-1).(b.c^-1).(c.a^-1))^-1";
    //g_input="(x^-1.(y^-1.(z^-1.x)^-1)^-1)^-1";
    //g_input="((a.b)^-1.c^-1).((c.b^-1).a^-1)^-1";
    //g_input="(x.(y.z^-1)^-1)^-1.(z.(y^-1.x^-1)^-1)";
    //g_input="((a^-1.b^-1)^-1.(b^-1.c^-1)^-1)^-1.c";
    //g_input="(x^-1.y)^-1.(y^-1.z)^-1.(z^-1.x)^-1";
    //g_input="((a.b.c)^-1.(c.b.a)^-1)^-1";
    //g_input="(x.(y.(z.(x.y)^-1)^-1)^-1)^-1";
    //g_input="((a^-1.(b.c)^-1)^-1.((a.b)^-1.c^-1)^-1)^-1";
    //g_input="(x^-1.y^-1.z^-1)^-1.(z.y.x)^-1";
    //g_input="((a.b^-1.c)^-1.(c^-1.b.a^-1)^-1)^-1";
    //g_input="(x.(y^-1.(z.x^-1)^-1.(x.z^-1)^-1).y)^-1";
    //g_input="((a^-1.b).(b^-1.c).(c^-1.a))^-1.((a^-1.c).(c^-1.b).(b^-1.a))^-1";
    g_pos=0;
    CompilerInfo compiler;
    TreeNode* tree=0;
    try
    {
        tree=Parse(&compiler);
        printf("Parse Tree:\n");
        PrintTree(tree, 0);
    }
    catch(...)
    {
        printf("Parse Error\n");
    }
    DestroyTree(tree);
    return 0;
}