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
};

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
bool consume(char op)
{
    if (token->kind != TK_RESERVED || token->str[0] != op)
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
void expect(char op)
{
    if (token->kind != TK_RESERVED || token->str[0] != op)
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

Token *new_token(TokenKind kind, Token *cur, char *str)
{
    Token *tok = calloc(1, sizeof(Token));
    tok->kind = kind;
    tok->str = str;
    cur->next = tok;
    return tok;
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

        if (*p == '+' || *p == '-')
        {
            cur = new_token(TK_RESERVED, cur, p++);
            continue;
        }

        if (isdigit(*p))
        {
            cur = new_token(TK_NUM, cur, p);
            cur->val = strtol(p, &p, 10);
            continue;
        }

        error_at(p, "トークナイズできません");
    }

    new_token(TK_EOF, cur, p);
    return head.next;
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
    token = tokenize(argv[1]);

    // アセンブリの前半部分を出力する
    printf(".intel_syntax noprefix\n");
    printf(".global main\n");
    printf("main:\n");

    // 式の最初は数でなければならないのでチェックし、
    // 最初のmov命令を出力
    printf("    mov rax, %d\n", expect_number());

    // `+ <数>` あるいは`- <数>`というトークンの並びを消費しつつ
    // アセンブリを出力
    while (!at_eof())
    {
        if (consume('+'))
        {
            printf("    add rax, %d\n", expect_number());
            continue;
        }

        expect('-');
        printf("    sub rax, %d\n", expect_number());
    }

    printf("    ret\n");
    return 0;
}