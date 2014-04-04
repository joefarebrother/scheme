;;;; A standard library designed for the bootstrapper


(define number? integer?)

(define (null? x) (eq? x '()))

(define (list . x) x)

(define (not x)
	(eq? x #f))

(define (append x . ys)
	(cond
		((null? ys) x)
		((null? x) (apply append ys))
		(else (cons (car x) (apply append (cons (cdr x) ys))))))

(define (length things)
	(define (iter things count)
		(if (null? things)
			count
			(iter (car things) (+ 1 count))))
	(iter things 0))

(define (reverse things)
	(define (iter front back)
		(if (null? front)
			back
			(iter (cdr front) (cons (car front) back))))
	(iter things '()))

(define (vector? v) #f) ;There are no vectors! 
                        ;But needed because the compiler wants to detect vectors.
                        ;No need for other vector operations, compiler only does
                        ;them to vectors, which it detects none of.

(define (range lo hi)
	(if (= lo hi)
		'()
		(cons lo (range (+ 1 lo) hi))))

(define (map f things)
	(if (null? things)
		'()
		(cons (f (car things)) (map f (cdr things)))))

(define (for-each f l)
	(if (null? l)
		#t
		(begin 
			(f (car l))
			(for-each f (cdr l)))))

(define (filter p things)
	(cond 
		((null? things) '()
		((p (car things)) (cons (car things) (filter p (cdr things))))
		(else (filter p (cdr things))))))


(define (foldl f x things)
	(if (null? things)
		x
		(foldl f (f x (car things)) (car things))))

(define (foldr f x things)
	(if (null? things)
		x
		(f (foldr f x (cdr things)) (car things))))
(define accumulate foldr)
