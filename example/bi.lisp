(progn
  (message (+ 1 1))
  (message (- 1 1))
  (message (* 2 2))
  (message (/ 8 2))

  (message (> 8 2))
  (message (< 8 2))
  (message (<= 2 2))
  (message (>= 2 2))
  (message (== 2 2))

  (message (&& 1 0))
  (message (|| 1 0))
  (message (&& 1 1))
  (message (|| 1 1))
  (message (&& 0 0))
  (message (|| 0 0))
  )
