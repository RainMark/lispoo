(progn
  (set! i 3)
  (while i (progn (message i) (set! i (+ i -1))))
  (progn (set! x 0) (message (if x 1 2)))
  (message (progn (+ 1 1) (+ 2 2)))
  )
