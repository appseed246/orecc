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
     * @brief 代入(=)
     */
    ND_ASSIGN,

    /**
     * @brief ローカル変数
     */
    ND_LVAR,

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
     * @brief 整数の値。kindがND_NUMの場合に使用。
     */
    int val;

    /**
     * @brief RBPからアクセス対象のローカル変数までのオフセット値。kindがND_LVARの場合に使用。
     */
    int offset;
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

// ローカル変数群
extern LVar *locals;

/**
 * @brief エラーを報告する。printfと同じ引数を取る。
 * @param fmt フォーマット
 */
void error(char *fmt, ...);

/**
 * @brief エラーが発生した文字を示し、エラー内容を報告する。printfと同じ引数を取る。
 * @param loc エラーがある文字の位置を示すポインタ
 * @param fmt フォーマット
 */
void error_at(char *loc, char *fmt, ...);

/**
 * @brief 次のトークンが期待している記号であるかどうかを判定する。
 * @param op 期待するトークン文字列
 * @return 期待した文字列である場合true, それ以外の場合はfalse
 */
bool consume(char *op);

/**
 * @brief 次のトークンが変数であるかを判定する。
 * @return 次のトークンが変数である場合現在のTokenのポインタ, それ以外の場合はNULLポインタ
 */
Token *consume_ident();

/**
 * @brief 次のトークンが記号の場合のときには、トークンを1つよみすすめる。
 * それ以外の場合はエラーを報告する。
 * @param op 期待するトークン文字列
 */
void expect(char *op);

/**
 * @brief 次のトークンが数値の場合、トークンを1つよみ進めてその数値を返す。
 * @return トークン(数値)
 */
int expect_number();

/**
 * @brief 着目するトークンがEOFかどうか判定する
 * @return EOFの場合true,それ以外の場合false
 */
bool at_eof();

/**
 * @brief Token構造体を生成する
 *
 * @param kind トークン種別
 * @param cur 現在のトークン構造体
 * @param str トークン文字列
 * @param len トークン文字列の長さ
 * @return Token構造体のポインタ
 */
Token *new_token(TokenKind kind, Token *cur, char *str, int len);

/**
 * @brief 文字列をトークン構造体に変換する
 *
 * @param p トークン構造体に変換する文字列のポインタ
 * @return トークン構造体のポインタ
 */
Token *tokenize(char *p);

// 入力プログラム文字列
extern char *user_input;

// 現在着目しているトークン
extern Token *token;

/**
 * @brief 文字列pが文字列qから始まることを判定する
 * @param p 検索対象の文字列
 * @param q 検索する文字列 ""で直接文字列を渡す
 * @return pがqから始まる文字列である場合true, それ以外false
 */
bool startswith(char *p, char *q);

/**
 * @brief ASTの子要素を生成する
 * @param kind ノードの種別
 * @param lhs 左の子ノード
 * @param rhs 右の子ノード
 * @return Nodeのポインタ
 */
Node *new_node(NodeKind kind, Node *lhs, Node *rhs);

/**
 * @brief 整数のノードを生成する
 * @param 整数の値
 * @return Nodeのポインタ
 */
Node *new_node_num(int val);

void *program();

// 複数ステートメントを保持する領域
extern Node *code[100];

/**
 * @brief 左辺値を評価する。
 */
void gen_lval(Node *node);

/**
 * @brief 抽象構文木を元にアセンブリを出力する
 */
void gen(Node *node);
