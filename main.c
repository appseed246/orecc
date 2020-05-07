#include "orecc.h"

/**
 * @brief プログラムのエントリポイント
 */
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