# lispoo

Code Oops Lisp Interpreter

# build

```sh
$ ./build.sh
```

# example

> lambda.lisp

```lisp
(progn
  (define abc (lambda (a b c) (+ a (+ b c))))
  (set! x (abc 1 2 3))
  (message x)
  (progn (+ 1.2 3.9) (define plus (lambda (a b) (+ a b))) (message (plus 9 9)))
  )
```

```sh
$ ./lispoo example/lambda.lisp
8
18
```

> quote.lisp

```lisp
(progn
  (define test (lambda (x) x))
  (if (test (quote (+ 1 -1)))
      (message (quote true))
    (message (quote false))
    )
  (message (quote (+ 1 2)))
  )
```

```sh
$ ./lispoo example/quote.lisp
false
(+ 1 2)
```

[More Examples](./example)
