# compilerbookノート

## 疑問点

### ステップ3
+ expectとconsumeの違いは？
    + error報告のためのラッパー関数

### ステップ8
+ externでグローバル変数を宣言する。
    + いずれかのCファイルで定義する。
    + 重複するとエラーになると書いてあるが、ならないのはなぜ？
        + gccでは許されているっぽい
        + `-fno-common`をつけるとコンパイル(リンク)エラーになった
    + ヘッダーファイルに「宣言」ではなく「定義」を書いてはいけない？
        + 上と同じくgccでは許されている？
+ 参考: http://solid.kmckk.com/doc/skit/current/solid_toolchain/overview.html