(declare non-standard-feature expose-names)

(expose-names (*global-env*
			   env-is-global? 
			   env-new-frame 
			   env-parent 
			   env-find-binding

			   make-binding
			   binding-var
			   binding-depth
			   binding-type)

(define (env-new-frame parent vars decls exposed-names)
	(cons (list vars decls exposed-names) parent))
(define *global-env* (env-new-frame '() '() '() '()))
(define (env-is-global? env)
	(eq? env *global-env*))

(define env-parent cdr)
(define env-exposed caddar)
(define env-vars caar)
(define env-decls cadar)

(define (env-find-binding env var)
	(cond
		((env-is-global? env) 'global)
		((or (memq? var (env-exposed env)) (eq? (env-exposed env) 'all)) 
			(binding-up-one-level (env-find-binding (env-parent env))))
		((assq var (env-vars env)) => (lambda (binding) 
			(make-binding var (binding-type binding) 0)))
		(else (binding-up-one-level (env-find-binding (env-parent (env)))))))

(define make-binding list)
(define binding-var car)
(define binding-depth caddr) ; 0 is current frame, higher #s go higher into surrounding scopes
(define binding-type cadr)

(define (binding-up-one-level bind)
	(error "not written")))