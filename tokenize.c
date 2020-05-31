#include "orecc.h"

static char *current_input;

void error(char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);

    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

static void verror_at(char *loc, char *fmt, va_list ap)
{
    int pos = loc - current_input;
    fprintf(stderr, "%s\n", current_input);
    fprintf(stderr, "%*s", pos, ""); // pos個のスペースを出力
    fprintf(stderr, "^ ");
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

/**
 * @brief エラー箇所の報告とプログラムの終了
 *
 * @param loc
 * @param fmt
 * @param ap
 */
static void error_at(char *loc, char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    verror_at(loc, fmt, ap);
}

void error_tok(Token *tok, char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    verror_at(tok->loc, fmt, ap);
}

bool equal(Token *tok, char *op)
{
    return strlen(op) == tok->len && !strncmp(tok->loc, op, tok->len);
}

Token *skip(Token *tok, char *op)
{
    if (!equal(tok, op))
    {
        error_tok(tok, "expected '%s'", op);
    }
    return tok->next;
}

/**
 * @brief 新しくトークン構造体を生成し、生成したトークンをcurの次のトークンに追加する
 *
 * @param kind トークンの種別
 * @param cur 現在のトークン
 * @param str トークンの文字列
 * @param len トークン文字列の長さ
 * @return Token* 新しいトークン構造体のポインタ
 */
static Token *new_token(TokenKind kind, Token *cur, char *str, int len)
{
    Token *tok = calloc(1, sizeof(Token));
    tok->kind = kind;
    tok->loc = str;
    tok->len = len;
    cur->next = tok;
    return tok;
}

/**
 * @brief 文字列pが文字列qから始まることを判定する
 * @param p 検索対象の文字列
 * @param q 検索する文字列 ""で直接文字列を渡す
 * @return pがqから始まる文字列である場合true, それ以外false
 */
static bool startswith(char *p, char *q)
{
    return memcmp(p, q, strlen(q)) == 0;
}

static bool is_alpha(char c)
{
    return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || c == '_';
}

static bool is_alnum(char c)
{
    return is_alpha(c) || ('0' <= c && c <= '9');
}

static bool is_keyword(Token *tok)
{
    // Keywords
    static char *kw[] = {"return", "if", "else", "for", "while"};

    for (int i = 0; i < sizeof(kw) / sizeof(*kw); i++)
    {
        if (equal(tok, kw[i]))
        {
            return true;
        }
    }
    return false;
}

/**
 * @brief TK_IDENTとして判定した予約語のトークンのkindをTK_RESERVEDに変換する
 *
 * @param tok 先頭のトークンのポインタ
 */
static void convert_keywords(Token *tok)
{
    for (Token *t = tok; t->kind != TK_EOF; t = t->next)
    {
        if (t->kind == TK_IDENT && is_keyword(t))
        {
            t->kind = TK_RESERVED;
        }
    }
}

Token *tokenize(char *p)
{
    current_input = p;
    Token head = {};
    Token *cur = &head;

    while (*p)
    {
        // 空白文字をスキップ
        if (isspace(*p))
        {
            p++;
            continue;
        }

        // 数値の判定
        if (isdigit(*p))
        {
            cur = new_token(TK_NUM, cur, p, 0);
            char *tmp = p;
            cur->val = strtoul(p, &p, 10);
            cur->len = p - tmp;
            continue;
        }

        // 変数の判定
        if (is_alpha(*p))
        {
            char *q = p++;
            while (is_alnum(*p))
            {
                p++;
            }
            cur = new_token(TK_IDENT, cur, q, p - q);
            cur->len = p - q;
            continue;
        }

        // 等号・不等号
        if (startswith(p, "==") || startswith(p, "!=") || startswith(p, "<=") || startswith(p, ">="))
        {
            cur = new_token(TK_RESERVED, cur, p, 2);
            p += 2;
            continue;
        }

        // 区切り文字(+-*/, <>, (), =, etc.)
        if (ispunct(*p))
        {
            cur = new_token(TK_RESERVED, cur, p++, 1);
            continue;
        }

        error_at(p, "invalid token");
    }

    new_token(TK_EOF, cur, p, 0);
    convert_keywords(head.next);
    return head.next;
}
