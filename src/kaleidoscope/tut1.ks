def foo(x y) x+foo(y, 4.0);
def fod(a b) a*a + 2*a*b + b*b;
def bar(a) fod(a, 4.0) + bar(31337); 
