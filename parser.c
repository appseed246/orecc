#include "orecc.h"

// パース中に生成されたすべてのローカル変数インスタンス
Var *locals;

static Node *expr(Token **rest, Token *tok);
static Node *assign(Token **rest, Token *tok);
static Node *equality(Token **rest, Token *tok);
static Node *relational(Token **rest, Token *tok);
static Node *add(Token **rest, Token *tok);
static Node *mul(Token **rest, Token *tok);
static Node *unary(Token **rest, Token *tok);
static Node *primary(Token **rest, Token *tok);

/**
 * @brief トークンの文字列からローカル変数のポインタを得る
 *
 * @param tok トークンのポインタ
 * @return 変数構造体のポインタ。トークン文字列に一致する名前の変数がない場合NULL。
 */
static Var *find_var(Token *tok)
{
    for (Var *var = locals; var; var = var->next)
    {
        if (strlen(var->name) == tok->len && !strncmp(tok->loc, var->name, tok->len))
        {
            return var;
        }
    }
    return NULL;
}

static Node *new_node(NodeKind kind)
{
    Node *node = calloc(1, sizeof(Node));
    node->kind = kind;
    return node;
}

static Node *new_binary(NodeKind kind, Node *lhs, Node *rhs)
{
    Node *node = new_node(kind);
    node->lhs = lhs;
    node->rhs = rhs;
    return node;
}

static Node *new_unary(NodeKind kind, Node *expr)
{
    Node *node = new_node(kind);
    node->lhs = expr;
    return node;
}

static Node *new_num(long val)
{
    Node *node = new_node(ND_NUM);
    node->val = val;
    return node;
}

static Node *new_var_node(Var *var)
{
    Node *node = new_node(ND_VAR);
    node->var = var;
    return node;
}

static Var *new_lvar(char *name)
{
    Var *var = calloc(1, sizeof(Var));
    var->name = name;
    var->next = locals;
    locals = var;
    return var;
}

static long get_number(Token *tok)
{
    if (tok->kind != TK_NUM)
    {
        error_tok(tok, "expected a number");
    }
    return tok->val;
}

/**
 * stmt = expr ";"
 *      | "return" expr ";"
 *      | "if" "(" expr ")" stmt ("else" stmt)?
 *      | "for" "(" expr? ";" expr? ";" expr? ")" stmt
 *      | "while" "(" expr ")" stmt
 */
static Node *stmt(Token **rest, Token *tok)
{
    if (equal(tok, "return"))
    {
        Node *node = new_unary(ND_RETURN, expr(&tok, tok->next));
        *rest = skip(tok, ";");
        return node;
    }

    if (equal(tok, "if"))
    {
        Node *node = new_node(ND_IF);
        tok = skip(tok->next, "(");
        node->cond = expr(&tok, tok);
        tok = skip(tok, ")");
        node->then = stmt(&tok, tok);
        if (equal(tok, "else"))
        {
            node->els = stmt(&tok, tok->next);
        }
        *rest = tok;
        return node;
    }

    if (equal(tok, "for"))
    {
        Node *node = new_node(ND_FOR);
        tok = skip(tok->next, "(");

        // 初期化式の有無
        if (!equal(tok, ";"))
        {
            node->init = new_unary(ND_EXPR_STMT, expr(&tok, tok));
        }
        tok = skip(tok, ";");

        // 条件式の有無
        if (!equal(tok, ";"))
        {
            node->cond = expr(&tok, tok);
        }
        tok = skip(tok, ";");

        // カウンタ変数の更新式の有無
        if (!equal(tok, ")"))
        {
            node->inc = new_unary(ND_EXPR_STMT, expr(&tok, tok));
        }
        tok = skip(tok, ")");

        // 実行部
        node->then = stmt(rest, tok);
        return node;
    }

    if (equal(tok, "while"))
    {
        Node *node = new_node(ND_FOR);
        tok = skip(tok->next, "(");
        node->cond = expr(&tok, tok);
        tok = skip(tok, ")");
        node->then = stmt(rest, tok);
        return node;
    }

    Node *node = new_unary(ND_EXPR_STMT, expr(&tok, tok));
    *rest = skip(tok, ";");
    return node;
}

// expr = assign
static Node *expr(Token **rest, Token *tok)
{
    return assign(rest, tok);
}

// assign = equality ("=" assign)?
static Node *assign(Token **rest, Token *tok)
{
    Node *node = equality(&tok, tok);
    if (equal(tok, "="))
    {
        node = new_binary(ND_ASSIGN, node, assign(&tok, tok->next));
    }
    *rest = tok;
    return node;
}

// equality = relational ("==" relational | "!=" relational)*
static Node *equality(Token **rest, Token *tok)
{
    Node *node = relational(&tok, tok);

    for (;;)
    {
        if (equal(tok, "=="))
        {
            Node *rhs = relational(&tok, tok->next);
            node = new_binary(ND_EQ, node, rhs);
            continue;
        }

        if (equal(tok, "!="))
        {
            Node *rhs = relational(&tok, tok->next);
            node = new_binary(ND_NE, node, rhs);
            continue;
        }
        *rest = tok;
        return node;
    }
}

// relational = add ("<" add | "<=" add | ">" add | ">=" add)*
static Node *relational(Token **rest, Token *tok)
{
    Node *node = add(&tok, tok);

    for (;;)
    {
        if (equal(tok, "<"))
        {
            Node *rhs = add(&tok, tok->next);
            node = new_binary(ND_LT, node, rhs);
            continue;
        }

        if (equal(tok, "<="))
        {
            Node *rhs = add(&tok, tok->next);
            node = new_binary(ND_LE, node, rhs);
            continue;
        }

        if (equal(tok, ">"))
        {
            Node *rhs = add(&tok, tok->next);
            node = new_binary(ND_LT, rhs, node);
            continue;
        }

        if (equal(tok, ">="))
        {
            Node *rhs = add(&tok, tok->next);
            node = new_binary(ND_LE, rhs, node);
            continue;
        }

        *rest = tok;
        return node;
    }
}

// add = mul ("+" mul | "-" mul)*
static Node *add(Token **rest, Token *tok)
{
    Node *node = mul(&tok, tok);

    for (;;)
    {
        if (equal(tok, "+"))
        {
            Node *rhs = mul(&tok, tok->next);
            node = new_binary(ND_ADD, node, rhs);
            continue;
        }
        if (equal(tok, "-"))
        {
            Node *rhs = mul(&tok, tok->next);
            node = new_binary(ND_SUB, node, rhs);
            continue;
        }

        *rest = tok;
        return node;
    }
}

// mul = unary ("*" unary | "/" unary)*
static Node *mul(Token **rest, Token *tok)
{
    Node *node = unary(&tok, tok);

    for (;;)
    {
        if (equal(tok, "*"))
        {
            Node *rhs = unary(&tok, tok->next);
            node = new_binary(ND_MUL, node, rhs);
            continue;
        }

        if (equal(tok, "/"))
        {
            Node *rhs = unary(&tok, tok->next);
            node = new_binary(ND_DIV, node, rhs);
            continue;
        }

        *rest = tok;
        return node;
    }
}

// unary = ("+" | "-")? unary
//       | prinary
static Node *unary(Token **rest, Token *tok)
{
    if (equal(tok, "+"))
    {
        return unary(rest, tok->next);
    }

    if (equal(tok, "-"))
    {
        return new_binary(ND_SUB, new_num(0), unary(rest, tok->next));
    }

    return primary(rest, tok);
}

// primary = "(" expr ")" | ident | num
static Node *primary(Token **rest, Token *tok)
{
    if (equal(tok, "("))
    {
        Node *node = expr(&tok, tok->next);
        *rest = skip(tok, ")");
        return node;
    }

    if (tok->kind == TK_IDENT)
    {
        Var *var = find_var(tok);
        if (!var)
        {
            var = new_lvar(strndup(tok->loc, tok->len));
        }
        *rest = tok->next;
        return new_var_node(var);
    }

    Node *node = new_num(get_number(tok));
    *rest = tok->next;
    return node;
}

// program = stmt*
Function *parse(Token *tok)
{
    Node head = {};
    Node *cur = &head;

    while (tok->kind != TK_EOF)
    {
        cur = cur->next = stmt(&tok, tok);
    }

    Function *prog = calloc(1, sizeof(Function));
    prog->node = head.next;
    prog->locals = locals;
    return prog;
}