	AREA Vect, CODE, READONLY

    IMPORT  Reset_Handler           ; In init.s
    IMPORT  ISR_IRQ					; In isr.c 
                         
	ENTRY 
	 LDR pc,=Reset_Handler
	 B   Undefined_Handler
	 B   SWI_Handler
	 B   Prefetch_Handler
	 B   DataAbort_Handler
	 NOP
	 B   IRQ_Handler
	 B   FIQ_Handler
	 
Undefined_Handler
	 B	Undefined_Handler

SWI_Handler
	 B	SWI_Handler

Prefetch_Handler
	 B	Prefetch_Handler

DataAbort_Handler
	 B	DataAbort_Handler 
IRQ_Handler
 	 STMDB	R13!, {R0-R12, LR}	; push R0~R12, LR to STACK_IRQ
 	 MRS	R0, SPSR		 	; store SPSR in r0
 	 STMDB	R13!, {R0}			; push SPSR to STACK_IRQ
 	 LDR    LR,=IRQ_Return
 	 LDR    PC,=ISR_IRQ					; branck to ISR_IRQ
IRQ_Return
 	 LDMIA	R13!, {R0}			; pop SPSR from STACK_IRQ to R0
 	 MSR	SPSR_cxsf, R0		; restore flags from R0 to CPSR
 	 LDMIA	R13!, {R0-R12, LR}	; pop R0~R12, LR to STACK_IRQ
 	 SUBS	PC, LR, #4			; return to the address before interrupted

FIQ_Handler
 	 B	FIQ_Handler



	END


