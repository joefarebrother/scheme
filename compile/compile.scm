;;;;A compiler for scheme, written in scheme. It compiles to a bytecode language readable by vm.c.

;;Manipulating instructions
(define (make-instruction opcode . operands)
	(cons opcode operands))
(define opcode car)
(define operand cdr)

(define (single-instruction ins)
	(cons ins '()))
(define combine-instructions append)


(define label-count 0)
(define (new-label name)
	(set! label-count (+ 1 label-count))
	(string->symbol (string-append name (number->string label-count))))

;;The instructions
(define (instr opcode . operands)
	(single-instruction (apply make-instruction (cons opcode operands))))
(define (label l)
	(instr 'label l))
(define (goto l)
	(instr 'goto l))
(define (goto-if l)
	(instr 'goto-if l))
;;more later

;;Hooks
(define hooks '())
(define (register-compiler-hook! name fun)
	(let ((hook (assq name hooks))) 
		(let ((fun (if (not hook) 
						fun 
						(lambda (arg) ((cdr hook) (fun arg))))))
			(set! hooks (cons (cons name fun) hooks)))))
(define (call-hook name arg)
	(let ((hook (assq name hooks)))
		(if hook
			((cdr hook) arg)
			arg)))
(define (hook-registered? name)
	(assq name hooks))

;;Compiler core
;Not implemented yet