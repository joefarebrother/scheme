;;;; A standard library designed for the bootstrapper

(begin ;Just so I can colapse all this with my IDE
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
	(define (cddddr x) (cdr (cdr (cdr (cdr x))))))


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

(define (length lst)
	(define (iter lst count)
		(if (null? lst)
			count
			(iter (cdr lst) (+ 1 count))))
	(iter lst 0))

(define (reverse lst)
	(define (iter front back)
		(if (null? front)
			back
			(iter (cdr front) (cons (car front) back))))
	(iter lst '()))

(define (vector? v) #f) ;There are no vectors! 
                        ;But needed because the compiler wants to detect vectors.
                        ;No need for other vector operations, compiler only does
                        ;them to vectors, which it detects none of.

(define (range lo hi)
	(if (= lo hi)
		'()
		(cons lo (range (+ 1 lo) hi))))

(define (map f lst1 . more)
	(define (mapcar f lst) 
		(if (null? lst)
		'()
		(cons (f (car lst)) (map f (cdr lst)))))
	(if (null? lst1)
		'()
		(cons (apply f (mapcar car (cons lst1 more))) 
			(apply map (cons f (mapcar cdr (cons lst1 more)))))))

(define (for-each f l)
	(if (null? l)
		#t
		(begin 
			(f (car l))
			(for-each f (cdr l)))))

(define (filter p lst)
	(cond 
		((null? lst) '()
		((p (car lst)) (cons (car lst) (filter p (cdr lst))))
		(else (filter p (cdr lst))))))


(define (foldl f x lst)
	(if (null? lst)
		x
		(foldl f (f x (car lst)) (car lst))))

(define (foldr f x lst)
	(if (null? lst)
		x
		(f (foldr f x (cdr lst)) (car lst))))
(define accumulate foldr)

(define (mem equiv? x lst)
	(cond
		((null? lst) #f)
		((equiv? x (car lst)) lst)
		(else (mem equiv? x (cdr lst)))))
(define (memq x lst) (mem eq? x lst))
;eqv and equal don't exist yet, but might need to later so good to have generality.

(define (assoc-test equiv? key table)
	(cond 
		((null? table) #f)
		((equiv? key (caar table)) (car table))
		(else (assoc-test equiv? key (cdr table)))))
(define (assq key table) (assoc-test eq? key table))