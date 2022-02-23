(progn
  (define abc (lambda (a b c) (+ a (+ b c))))
  (set! x (abc 1 2 3))
  (message x)
  (progn (+ 1.2 3.9) (define plus (lambda (a b) (+ a b))) (message (plus 9 9)))
  )
