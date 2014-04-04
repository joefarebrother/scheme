;;;; A standard library designed for the bootstrapper

(define (caar x) (car (car x)))
(define (cadr x) (car (cdr x)))
(define (cdar x) (cdr (car x)))
(define (cddr x) (cdr (cdr x)))
(define (caaar x) (car (car (car x))))
(define (caadr x) (car (car (cdr x))))
(define (cadar x) (car (cdr (car x))))
(define (caddr x) (car (cdr (cdr x))))
(define (cdaar x) (cdr (car (car x))))
(define (cdadr x) (cdr (car (cdr x))))
(define (cddar x) (cdr (cdr (car x))))
(define (cdddr x) (cdr (cdr (cdr x))))
(define (caaaar x) (car (car (car (car x)))))
(define (caaadr x) (car (car (car (cdr x)))))
(define (caadar x) (car (car (cdr (car x)))))
(define (caaddr x) (car (car (cdr (cdr x)))))
(define (cadaar x) (car (cdr (car (car x)))))
(define (cadadr x) (car (cdr (car (cdr x)))))
(define (caddar x) (car (cdr (cdr (car x)))))
(define (cadddr x) (car (cdr (cdr (cdr x)))))
(define (cdaaar x) (cdr (car (car (car x)))))
(define (cdaadr x) (cdr (car (car (cdr x)))))
(define (cdadar x) (cdr (car (cdr (car x)))))
(define (cdaddr x) (cdr (car (cdr (cdr x)))))
(define (cddaar x) (cdr (cdr (car (car x)))))
(define (cddadr x) (cdr (cdr (car (cdr x)))))
(define (cdddar x) (cdr (cdr (cdr (car x)))))
(define (cddddr x) (cdr (cdr (cdr (cdr x)))))


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
			(iter (cdr things) (+ 1 count))))
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

(define (memq x lst)
	(cond
		((null? lst) #f)
		((eq? x (car lst)) lst)
		(else (memq x (cdr lst)))))

(define (assq key table)
	(cond 
		((null? table) #f)
		((eq? x (caar table)) (car table))
		(else (assq x (cdr table)))))