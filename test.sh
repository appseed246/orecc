#!/bin/bash
assert() {
    expected="$1"
    input="$2"

    ./orecc "$input" > tmp.s
    cc -o tmp tmp.s
    ./tmp
    actual="$?"

    if [ "$actual" = "$expected" ]; then
        echo "$input => $actual"
    else
        echo "$input => $expected expected, but got $actual"
        exit 1
    fi
}

assert 0 'return 0;'
assert 42 'return 42;'
assert 21 'return 5+20-4;'
assert 41 'return 12 + 34 - 5 ;'
assert 47 'return 5+6*7;'
assert 15 'return 5*(9-6);'
assert 4 'return (3+5)/2;'
assert 2 'return -3+5;'
assert 13 'return +3+5*2;'

assert 1 'return 3 == 3;'
assert 0 'return 1 == 2;'
assert 1 'return 3+1 == 2+2;'

assert 1 'return 1 != 2;'
assert 0 'return 2!=2;'

assert 1 'return 10 < 11;'
assert 0 'return 10 < 10;'
assert 1 'return 1*(2-2)+3 < (3+1)*1+2;'

assert 1 'return 2 > 1;'
assert 0 'return 1 > 1;'

assert 1 'return 2 <= 5;'
assert 1 'return 2 <= 2;'
assert 0 'return (3*2) <= 1+1*3;'

assert 1 'return 55 >= 1;'
assert 1 'return 55 >= 55;'
assert 0 'return 1*(3+1) >= 5;'

assert 3 'a = 1; b = 2; return a + b;'
assert 1 'x=3;y=2;z=5; return x+y == z;'

assert 6 'fo0 = 3; b4r = 3; return fo0 + b4r;'
assert 1 'fo0 = 3; b4r = 3; return fo0 == b4r;'

assert 42 'return 42;'
assert 5 'return 5; return 8;'
assert 7 'aa = 3; bb = 2; return aa + bb * 2;'

assert 2 'if (1) return 2; else return 3;'
assert 3 'if (0) return 2; else return 3;'
assert 3 'if (1-1) return 2; else return 3;'
assert 2 'if (2-1) return 2; else return 3;'

assert 55 'i=0; j=0; for (i=0; i<=10; i=i+1) j=i+j; return j;'
assert 3 'for (;;) return 3; return 5;'

assert 10 'i=0; while(i<10) i=i+1; return i;'

echo OK