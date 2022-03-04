(progn
  (define fib (lambda (n) (if (<= n 2) 1 (+ (fib (- n 1)) (fib (- n 2))))))
  (message (fib 4))
  (message (fib 8))
  )
