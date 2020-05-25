#include "orecc.h"

/**
 * @brief TODO
 *
 * @param n
 * @param align
 * @return int
 */
static int align_to(int n, int align)
{
    return (n + align - 1) & ~(align - 1);
}

/**
 * @brief プログラムのエントリポイント
 */
int main(int argc, char **argv)
{
    if (argc != 2)
    {
        error("%s: invalid number of arguments", argv[0]);
    }

    Token *tok = tokenize(argv[1]);
    Function *prog = parse(tok);

    // ローカル変数の領域確保
    int offset = 32; // 32 for callee-saved registers
    for (Var *var = prog->locals; var; var = var->next)
    {
        offset += 8;
        var->offset = offset;
    }
    prog->stack_size = align_to(offset, 16); // TODO: what is this process

    // ASTをさかのぼってアセンブリを出力する
    codegen(prog);

    return 0;
}