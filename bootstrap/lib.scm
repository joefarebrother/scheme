;; A standard library designed for the bootstrapper

(define (map f x)
	(if (null? x)
		'()
		(cons (f (car x)) (map f (cdr x)))))

