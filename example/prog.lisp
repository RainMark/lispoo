(prog
 (set! i 3)
 (while i (prog (message i) (set! i (+ i -1))))
 (prog (set! x 0) (message (if x 1 2)))
 (prog (set! x 1) (message x))
 )
