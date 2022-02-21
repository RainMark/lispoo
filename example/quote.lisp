(prog
 (def test (lambda (x) x))
 (if (test (quote (+ 1 -1)))
     (message (quote true))
   (message (quote false))
   )
 (message (quote (+ 1 2)))
 )
