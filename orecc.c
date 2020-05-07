#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * @brief トークンの型
 */
typedef enum
{
    /**
     * @brief 記号
     */
    TK_RESERVED,

    /**
     * @brief 整数
     */
    TK_NUM,

    /**
     * @brief 終端
     */
    TK_EOF,
} TokenKind;

typedef struct Token Token;

/**
 * @brief トークン
 */
struct Token
{
    /**
     * @brief トークンの型
     */
    TokenKind kind;

    /**
     * @brief 次の入力トークン
     */
    Token *next;

    /**
     * @brief kindがTK_NUMの場合、その数値
     */
    int val;

    /**
     * @brief トークン文字列
     */
    char *str;

    /**
     * @brief トークンの長さ
     */
    int len;
};

/**
 * @brief 抽象構文木のノードの種類
 */
typedef enum
{
    /**
     * @brief +
     */
    ND_ADD,

    /**
     * @brief -
     */
    ND_SUB,

    /**
     * @brief *
     */
    ND_MUL,

    /**
     * @brief /
     */
    ND_DIV,

    /**
     * @brief 整数
     */
    ND_NUM,

    /**
     * @brief 等価(==)
     */
    ND_EQ,

    /**
     * @brief 不等価(!=)
     */
    ND_NE,

    /**
     * @brief より小さい(<)
     */
    ND_LT,

    /**
     * @brief より大きい(<=)
     */
    ND_LE
} NodeKind;

typedef struct Node Node;

/**
 * @brief 抽象構文木のノードの型
 */
struct Node
{
    /**
     * @brief ノードの型
     */
    NodeKind kind;

    /**
     * @brief 左辺
     */
    Node *lhs;

    /**
     * @brief 右辺
     */
    Node *rhs;

    /**
     * @brief kindがND_NUMのときの数の値
     */
    int val;
};

// 関数ヘッダー
Node *expr();
Node *equality();
Node *relational();
Node *add();
Node *mul();
Node *unary();
Node *primary();

// 現在着目しているトークン
Token *token;

// 入力プログラム文字列
char *user_input;

/**
 * @brief エラーを報告する。printfと同じ引数を取る。
 * @param loc エラーがある文字の位置を示すポインタ
 * @param fmt フォーマット
 */
void error_at(char *loc, char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);

    int pos = loc - user_input;
    fprintf(stderr, "%s\n", user_input);
    fprintf(stderr, "%*s", pos, ""); // pos個の空白を出力
    fprintf(stderr, "^ ");
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

/**
 * @brief 次のトークンが期待している記号であるかどうかを判定する。
 * @param op 期待するトークン文字列
 * @return 期待した文字列である場合true, それ以外の場合はfalse
 */
bool consume(char *op)
{
    if (token->kind != TK_RESERVED || strlen(op) != token->len || memcmp(token->str, op, token->len))
    {
        return false;
    }
    token = token->next;
    return true;
}

/**
 * @brief 次のトークンが記号の場合のときには、トークンを1つよみすすめる。
 * それ以外の場合はエラーを報告する。
 * @param op 期待するトークン文字列
 */
void expect(char *op)
{
    if (token->kind != TK_RESERVED || strlen(op) != token->len || memcmp(token->str, op, token->len))
    {
        error_at(token->str, "'%c'ではありません", op);
    }
    token = token->next;
}

/**
 * @brief 次のトークンが数値の場合、トークンを1つよみ進めてその数値を返す。
 * @return トークン(数値)
 */
int expect_number()
{
    if (token->kind != TK_NUM)
    {
        error_at(token->str, "数値ではありません。");
    }
    int val = token->val;
    token = token->next;
    return val;
}

/**
 * @brief 着目するトークンがEOFかどうか判定する
 * @return EOFの場合true,それ以外の場合false
 */
bool at_eof()
{
    return token->kind == TK_EOF;
}

Token *new_token(TokenKind kind, Token *cur, char *str, int len)
{
    Token *tok = calloc(1, sizeof(Token));
    tok->kind = kind;
    tok->str = str;
    tok->len = len;
    cur->next = tok;
    return tok;
}

bool startswith(char *p, char *q)
{
    return memcmp(p, q, strlen(q)) == 0;
}

/**
 * @brief 文字列をトークン構造体に変換する
 *
 * @param p トークン構造体に変換する文字列のポインタ
 * @return トークン構造体のポインタ
 */
Token *tokenize(char *p)
{
    Token head;
    head.next = NULL;
    Token *cur = &head;

    while (*p)
    {
        // 空白文字をスキップ
        if (isspace(*p))
        {
            p++;
            continue;
        }

        // 2文字記号の判定
        if (startswith(p, "==") || startswith(p, "!=") || startswith(p, ">=") || startswith(p, "<="))
        {
            cur = new_token(TK_RESERVED, cur, p, 2);
            p += 2;
            continue;
        }

        if (strchr("+-*/()<>", *p))
        {
            cur = new_token(TK_RESERVED, cur, p++, 1);
            continue;
        }

        if (isdigit(*p))
        {
            cur = new_token(TK_NUM, cur, p, 1);
            char *tmp = p;
            cur->val = strtol(p, &p, 10);
            cur->len = p - tmp;
            continue;
        }

        error_at(p, "トークナイズできません");
    }

    new_token(TK_EOF, cur, p, 1);
    return head.next;
}

Node *new_node(NodeKind kind, Node *lhs, Node *rhs)
{
    Node *node = calloc(1, sizeof(Node));
    node->kind = kind;
    node->lhs = lhs;
    node->rhs = rhs;
    return node;
}

Node *new_node_num(int val)
{
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_NUM;
    node->val = val;
    return node;
}

// expr = equality
Node *expr()
{
    return equality();
}

// equality = relational ("==" relational | "!=" relational)*
Node *equality()
{
    Node *node = relational();

    for (;;)
    {
        if (consume("=="))
        {
            node = new_node(ND_EQ, node, relational());
        }
        else if (consume("!="))
        {
            node = new_node(ND_NE, node, relational());
        }
        else
        {
            return node;
        }
    }
}

// relational = add ("<" add | "<=" add | ">" add | ">=" add)*
Node *relational()
{
    Node *node = add();

    for (;;)
    {
        if (consume("<"))
        {
            node = new_node(ND_LT, node, add());
        }
        else if (consume(">"))
        {
            node = new_node(ND_LT, add(), node);
        }
        else if (consume("<="))
        {
            node = new_node(ND_LE, node, add());
        }
        else if (consume(">="))
        {
            node = new_node(ND_LE, add(), node);
        }
        else
        {
            return node;
        }
    }
}

// add = mul ("+" mul | "-" mul)*
Node *add()
{
    Node *node = mul();

    for (;;)
    {
        if (consume("+"))
        {
            node = new_node(ND_ADD, node, mul());
        }
        else if (consume("-"))
        {
            node = new_node(ND_SUB, node, mul());
        }
        else
        {
            return node;
        }
    }
}

// mul = unary ("*" unary | "/" unary)*
Node *mul()
{
    Node *node = unary();

    for (;;)
    {
        if (consume("*"))
        {
            node = new_node(ND_MUL, node, unary());
        }
        else if (consume("/"))
        {
            node = new_node(ND_DIV, node, unary());
        }
        else
        {
            return node;
        }
    }
}

// unary = ("+" | "-")? primary
Node *unary()
{
    if (consume("+"))
    {
        return primary();
    }
    if (consume("-"))
    {
        return new_node(ND_SUB, new_node_num(0), primary());
    }
    return primary();
}

// primary = num | "(" expr ")"
Node *primary()
{
    if (consume("("))
    {
        Node *node = expr();
        expect(")");
        return node;
    }

    return new_node_num(expect_number());
}

void gen(Node *node)
{
    if (node->kind == ND_NUM)
    {
        printf("    push %d\n", node->val);
        return;
    }

    gen(node->lhs);
    gen(node->rhs);

    printf("    pop rdi\n");
    printf("    pop rax\n");

    switch (node->kind)
    {
    case ND_ADD:
        printf("    add rax, rdi\n");
        break;
    case ND_SUB:
        printf("    sub rax, rdi\n");
        break;
    case ND_MUL:
        printf("    imul rax, rdi\n");
        break;
    case ND_DIV:
        printf("    cqo\n");
        printf("    idiv rdi\n");
        break;
    case ND_EQ:
        printf("    cmp rax, rdi\n");
        printf("    sete al\n");
        printf("    movzb rax, al\n");
        break;
    case ND_NE:
        printf("    cmp rax, rdi\n");
        printf("    setne al\n");
        printf("    movzb rax, al\n");
        break;
    case ND_LT:
        printf("    cmp rax, rdi\n");
        printf("    setl al\n");
        printf("    movzb rax, al\n");
        break;
    case ND_LE:
        printf("    cmp rax, rdi\n");
        printf("    setle al\n");
        printf("    movzb rax, al\n");
        break;
    }

    printf("    push rax\n");
}

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        error_at("", "引数の個数が正しくありません。");
        return 1;
    }

    // 入力プログラムを保持する。
    user_input = argv[1];

    // トークナイズする
    token = tokenize(user_input);
    Node *node = expr();

    // アセンブリの前半部分を出力する
    printf(".intel_syntax noprefix\n");
    printf(".global main\n");
    printf("main:\n");

    // 抽象構文木を下りながらコードを生成
    gen(node);

    // スタックトップに式全体の値が残っているはずなので
    // それをRAXにロードしてから関数からの返り値とする
    printf("    pop rax\n");
    printf("    ret\n");
    return 0;
}