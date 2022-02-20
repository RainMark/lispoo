(prog
 (def abc (lambda (a b c) (+ a (+ b c))))
 (set! x (abc 1 2 3))
 (message x)
 (prog (+ 1.2 3.9) (def plus (lambda (a b) (+ a b))) (message (plus 9 9)))
 )
