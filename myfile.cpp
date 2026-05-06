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

// Reduction Rules (from page 460):
// ----------------------------
// Rule 1: x^-1^-1 -> x             (double inverse cancels out)
// Rule 2: (x.y)^-1 -> y^-1 . x^-1 (inverse of product flips order)

#include <cstdio>
#include <cstdlib>

////////////////////////////////////////////////////////////////////////////////////
// Input

const char* g_input;
int g_pos;

char GetNextChar()
{
    if(g_input[g_pos]==0) return 0;
    return g_input[g_pos++];
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
// Tree To Expression

void BuildExpr(TreeNode* node, char* buf, int* pos)
{
    if(!node) return;
    if(node->node_kind==ID_NODE)
    {
        buf[(*pos)++]=node->id;
        buf[*pos]=0;
        return;
    }
    if(node->node_kind==INVERSE_NODE)
    {
        int needs_parens=0;
        if(node->child[0] && node->child[0]->node_kind==PRODUCT_NODE)
            needs_parens=1;
        if(needs_parens) buf[(*pos)++]='(';
        BuildExpr(node->child[0], buf, pos);
        if(needs_parens) buf[(*pos)++]=')';
        buf[(*pos)++]='^';
        buf[(*pos)++]='-';
        buf[(*pos)++]='1';
        buf[*pos]=0;
        return;
    }
    if(node->node_kind==PRODUCT_NODE)
    {
        int right_needs_parens=0;
        if(node->child[1] && node->child[1]->node_kind==PRODUCT_NODE)
            right_needs_parens=1;
        BuildExpr(node->child[0], buf, pos);
        buf[(*pos)++]='.';
        if(right_needs_parens) buf[(*pos)++]='(';
        BuildExpr(node->child[1], buf, pos);
        if(right_needs_parens) buf[(*pos)++]=')';
        buf[*pos]=0;
        return;
    }
}

void PrintExpr(TreeNode* node)
{
    char buf[512];
    int pos=0;
    buf[0]=0;
    BuildExpr(node, buf, &pos);
    printf("%s\n", buf);
}

////////////////////////////////////////////////////////////////////////////////////
// Reduction Rules TAKE CARE ANA 3MALT EL KETAB REFERENCE

// Helper: creates a new inverse node wrapping the given child
TreeNode* MakeInverse(TreeNode* child)
{
    TreeNode* node=new TreeNode();
    node->node_kind=INVERSE_NODE;
    node->child[0]=child;
    return node;
}

// Helper: creates a new product node with left and right children
TreeNode* MakeProduct(TreeNode* left, TreeNode* right)
{
    TreeNode* node=new TreeNode();
    node->node_kind=PRODUCT_NODE;
    node->child[0]=left;
    node->child[1]=right;
    return node;
}

// Rule 1: x^-1^-1 -> x
// If we see inverse( inverse(x) ), we just return x and free the two wrapper nodes
TreeNode* ReduceDoubleInverse(TreeNode* node)
{
    // Check: is this node an inverse whose child is also an inverse?
    if(node->node_kind==INVERSE_NODE)
    {
        if(node->child[0] && node->child[0]->node_kind==INVERSE_NODE)
        {
            // Yes! x^-1^-1 found. Pull out the inner child (x) and free wrappers.
            TreeNode* inner=node->child[0]->child[0];
            node->child[0]->child[0]=0; // disconnect so DestroyTree doesn't touch inner
            DestroyTree(node->child[0]);
            node->child[0]=0;
            DestroyTree(node);
            return inner;  // return x directly
        }
    }
    return node; // no match, return unchanged
}

// Rule 2: (x.y)^-1 -> y^-1 . x^-1
// If we see inverse( product(x, y) ), replace with product( inverse(y), inverse(x) )
TreeNode* ReduceProductInverse(TreeNode* node)
{
    // Check: is this node an inverse whose child is a product?
    if(node->node_kind==INVERSE_NODE)
    {
        if(node->child[0] && node->child[0]->node_kind==PRODUCT_NODE)
        {
            // Yes! (x.y)^-1 found. Extract x and y from the product.
            TreeNode* x=node->child[0]->child[0];
            TreeNode* y=node->child[0]->child[1];

            // Disconnect children so DestroyTree does not free them
            node->child[0]->child[0]=0;
            node->child[0]->child[1]=0;
            node->child[0]=0;
            DestroyTree(node);

            // Build: y^-1 . x^-1
            TreeNode* inv_y=MakeInverse(y);
            TreeNode* inv_x=MakeInverse(x);
            return MakeProduct(inv_y, inv_x);
        }
    }
    return node; // no match, return unchanged
}

// ApplyOnce: walks the whole tree once.
// At each node, tries to apply a reduction rule.
// If a rule fires, sets *changed=1 so the caller knows to keep looping.
// Returns the (possibly new) root of the subtree.
TreeNode* ApplyOnce(TreeNode* node, int* changed)
{
    if(!node) return 0;

    // First recurse into children so we reduce bottom-up
    int i;
    for(i=0;i<MAX_CHILDREN;i++)
        node->child[i]=ApplyOnce(node->child[i], changed);

    // Try Rule 1: x^-1^-1 -> x
    if(node->node_kind==INVERSE_NODE)
    {
        if(node->child[0] && node->child[0]->node_kind==INVERSE_NODE)
        {
            *changed=1;
            return ReduceDoubleInverse(node);
        }
    }

    // Try Rule 2: (x.y)^-1 -> y^-1 . x^-1
    if(node->node_kind==INVERSE_NODE)
    {
        if(node->child[0] && node->child[0]->node_kind==PRODUCT_NODE)
        {
            *changed=1;
            return ReduceProductInverse(node);
        }
    }

    return node; // nothing matched, return unchanged
}

////////////////////////////////////////////////////////////////////////////////////
// ReduceAll

void ReduceAll(TreeNode** root)
{
    int changed=1;
    while(changed)
    {
        changed=0;
        *root=ApplyOnce(*root, &changed);
        if(changed)
        {
            PrintTree(*root, 0);
            PrintExpr(*root);
            printf("\n");
            fflush(NULL);
        }
    }
}
////////////////////////////////////////////////////////////////////////////////////
// Main

int main()
{
    g_input="((x.y^-1).z)^-1";
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
        PrintExpr(tree);
        printf("\n");
        fflush(NULL);

        ReduceAll(&tree);

        printf("Final normal form:\n");
        PrintTree(tree, 0);
        PrintExpr(tree);
        printf("---------------------------------\n");
        fflush(NULL);
    }
    catch(...)
    {
        printf("Parse Error\n");
    }
    DestroyTree(tree);
    return 0;
}