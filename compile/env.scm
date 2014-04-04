(declare non-standard-feature expose-names)
(declare not-in-bootstraper (load "lib/lib.scm"))

(expose-names (global-env 
			   env-is-global? 
			   env-new-frame 
			   env-parent 
			   env-find-binding)

(define global-env '((() () ())))
(define (env-is-global? env)
	(null? (cdr env)))
(define (env-new-frame parent vars exposed-names)
	(cons (list vars '() exposed-names) env))

(define env-parent cdr)
(define env-exposed caddar)
(define env-vars caar)
(define env-decls cadar)

(define (env-find-binding env var)
	(cond
		((env-is-global? env) 'global)
		((memq? var (env-exposed env)) 
			(binding-up-one-level (env-find-binding (env-parent env))))
		((memq? var (env-vars env)) => (lambda (rest)
			(make-binding 0 (- (length (env-vars env)) (length rest)-1)))) ;get the right position
		(else (binding-up-one-level (env-find-binding (env-parent (env)))))))

(define make-binding list)
(define binding-depth car) ; 0 is current frame, higher #s go higher into surrounding scopes
(define binding-pos cadr)

;;TODO: Write this
(define (env-add-decl env type data)
	(error "env-add-decl not yet implemented") )) 

