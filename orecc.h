#define _POSIX_C_SOURCE 200809L
#include <assert.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//
// tokenize.c
//

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
     * @brief 識別子
     */
    TK_IDENT,

    /**
     * @brief 整数
     */
    TK_NUM,

    /**
     * @brief 終端
     */
    TK_EOF,
} TokenKind;

/**
 * @brief トークン
 */
typedef struct Token Token;
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
    long val;

    /**
     * @brief トークンの位置
     */
    char *loc;

    /**
     * @brief トークンの長さ
     */
    int len;
};

/**
 * @brief エラーを報告する。printfと同じ引数を取る。
 * @param fmt フォーマット
 */
void error(char *fmt, ...);

/**
 * @brief エラーの報告とプログラムの終了
 *
 * @param tok トークン列のポインタ
 * @param fmt 可変長フォーマット列
 */
void error_tok(Token *tok, char *fmt, ...);

/**
 * @brief 現在のトークンの文字列がopと一致しているか判定する
 *
 * @param tok トークン
 * @param op 比較対象文字列のポインタ
 * @return 一致している場合true, それ以外の場合はfalse
 */
bool equal(Token *tok, char *op);

/**
 * @brief 現在のトークンの文字列がopであること判定し、次のトークンのポインタを取得する。
 * トークン文字列がopでない場合、エラーを表示してプログラムを終了する。
 *
 * @param tok 現在のトークン
 * @param op 比較対象文字列のポインタ
 * @return Token* 次のトークンのポインタ。
 */
Token *skip(Token *tok, char *op);

/**
 * @brief 文字列をトークン構造体に変換する
 *
 * @param input トークン構造体に変換する文字列のポインタ
 * @return トークン構造体のポインタ
 */
Token *tokenize(char *input);

//
// parser.c
//

typedef struct Var Var;
struct Var
{
    /**
     * @brief 次の変数を指すポインタ
     */
    Var *next;

    /**
     * @brief 変数名
     */
    char *name;

    /**
     * @brief RBPからアクセス対象のローカル変数までのバイト数
     */
    int offset;
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
    ND_LE,

    /**
     * @brief 代入(=)
     */
    ND_ASSIGN,

    /**
     * @brief return
     */
    ND_RETURN,

    /**
     * @brief if
     */
    ND_IF,

    /**
     * @brief else
     */
    ND_ELSE,

    /**
     * @brief Expresseion Statement
     */
    ND_EXPR_STMT,

    /**
     * @brief 変数
     */
    ND_VAR,

    /**
     * @brief 整数
     */
    ND_NUM,
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
     * @brief 次のノードを指すポインタ
     */
    Node *next;

    /**
     * @brief 左辺
     */
    Node *lhs;

    /**
     * @brief 右辺
     */
    Node *rhs;

    /**
     * @brief [if] 条件式部分のノード
     */
    Node *cond;

    /**
     * @brief [if] 条件式が真のときに実行するコードのノード
     */
    Node *then;

    /**
     * @brief [if] 条件式が偽のときに実行するコードのノード
     */
    Node *els;

    /**
     * @brief 変数のポインタ。kindがND_VARの場合に使用。
     */
    Var *var;

    /**
     * @brief 整数の値。kindがND_NUMの場合に使用。
     */
    long val;
};

typedef struct LVar LVar;

struct LVar
{
    /**
     * @brief 次の変数の構造体のポインタ
     */
    LVar *next;

    /**
     * @brief 変数名
     */
    char *name;

    /**
     * @brief 名前の長さ
     */
    int len;

    /**
     * @brief RBPからのオフセット
     */
    int offset;
};

typedef struct Function Function;
struct Function
{
    /**
     * @brief 現在のノード
     */
    Node *node;

    /**
     * @brief ローカル変数群
     */
    Var *locals;

    /**
     * @brief スタックサイズ(byte)
     */
    int stack_size;
};

Function *parse(Token *tok);

//
// codegen.c
//

/**
 * @brief 抽象構文木を元にアセンブリを出力する
 */
void codegen(Function *prog);
