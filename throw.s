	.file	"throw.cpp"
	.text
	.globl	_Z5raisev
	.type	_Z5raisev, @function
_Z5raisev:
.LFB0:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	movl	$1, %edi
	call	__cxa_allocate_exception@PLT
	movl	$0, %edx
	leaq	_ZTI9Exception(%rip), %rsi
	movq	%rax, %rdi
	call	__cxa_throw@PLT
	.cfi_endproc
.LFE0:
	.size	_Z5raisev, .-_Z5raisev
	.section	.rodata
	.align 8
.LC0:
	.string	"try_but_dont_catch handled the exception"
.LC1:
	.string	"Caught a Fake_Exception!"
	.text
	.globl	_Z18try_but_dont_catchv
	.type	_Z18try_but_dont_catchv, @function
_Z18try_but_dont_catchv:
.LFB1:
	.cfi_startproc
	.cfi_personality 0x9b,DW.ref.__gxx_personality_v0
	.cfi_lsda 0x1b,.LLSDA1
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	pushq	%rbx
	subq	$24, %rsp
	.cfi_offset 3, -24
.LEHB0:
	call	_Z5raisev
.LEHE0:
.L6:
	leaq	.LC0(%rip), %rdi
.LEHB1:
	call	puts@PLT
	jmp	.L10
.L8:
	cmpq	$1, %rdx
	je	.L5
	movq	%rax, %rdi
	call	_Unwind_Resume@PLT
.LEHE1:
.L5:
	movq	%rax, %rdi
	call	__cxa_begin_catch@PLT
	movq	%rax, -24(%rbp)
	leaq	.LC1(%rip), %rdi
.LEHB2:
	call	puts@PLT
.LEHE2:
	call	__cxa_end_catch@PLT
	jmp	.L6
.L9:
	movq	%rax, %rbx
	call	__cxa_end_catch@PLT
	movq	%rbx, %rax
	movq	%rax, %rdi
.LEHB3:
	call	_Unwind_Resume@PLT
.LEHE3:
.L10:
	addq	$24, %rsp
	popq	%rbx
	popq	%rbp
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE1:
	.globl	__gxx_personality_v0
	.section	.gcc_except_table,"a",@progbits
	.align 4
.LLSDA1:
	.byte	0xff
	.byte	0x9b
	.uleb128 .LLSDATT1-.LLSDATTD1
.LLSDATTD1:
	.byte	0x1
	.uleb128 .LLSDACSE1-.LLSDACSB1
.LLSDACSB1:
	.uleb128 .LEHB0-.LFB1
	.uleb128 .LEHE0-.LEHB0
	.uleb128 .L8-.LFB1
	.uleb128 0x1
	.uleb128 .LEHB1-.LFB1
	.uleb128 .LEHE1-.LEHB1
	.uleb128 0
	.uleb128 0
	.uleb128 .LEHB2-.LFB1
	.uleb128 .LEHE2-.LEHB2
	.uleb128 .L9-.LFB1
	.uleb128 0
	.uleb128 .LEHB3-.LFB1
	.uleb128 .LEHE3-.LEHB3
	.uleb128 0
	.uleb128 0
.LLSDACSE1:
	.byte	0x1
	.byte	0
	.align 4
	.long	DW.ref._ZTI14Fake_Exception-.
.LLSDATT1:
	.text
	.size	_Z18try_but_dont_catchv, .-_Z18try_but_dont_catchv
	.section	.rodata
.LC2:
	.string	"catchit handled the exception"
.LC3:
	.string	"Caught an Exception!"
	.text
	.globl	_Z7catchitv
	.type	_Z7catchitv, @function
_Z7catchitv:
.LFB2:
	.cfi_startproc
	.cfi_personality 0x9b,DW.ref.__gxx_personality_v0
	.cfi_lsda 0x1b,.LLSDA2
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	pushq	%rbx
	subq	$24, %rsp
	.cfi_offset 3, -24
.LEHB4:
	call	_Z18try_but_dont_catchv
.LEHE4:
.L15:
	leaq	.LC2(%rip), %rdi
.LEHB5:
	call	puts@PLT
	jmp	.L19
.L17:
	cmpq	$1, %rdx
	je	.L14
	movq	%rax, %rdi
	call	_Unwind_Resume@PLT
.LEHE5:
.L14:
	movq	%rax, %rdi
	call	__cxa_begin_catch@PLT
	movq	%rax, -24(%rbp)
	leaq	.LC3(%rip), %rdi
.LEHB6:
	call	puts@PLT
.LEHE6:
	call	__cxa_end_catch@PLT
	jmp	.L15
.L18:
	movq	%rax, %rbx
	call	__cxa_end_catch@PLT
	movq	%rbx, %rax
	movq	%rax, %rdi
.LEHB7:
	call	_Unwind_Resume@PLT
.LEHE7:
.L19:
	addq	$24, %rsp
	popq	%rbx
	popq	%rbp
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE2:
	.section	.gcc_except_table
	.align 4
.LLSDA2:
	.byte	0xff
	.byte	0x9b
	.uleb128 .LLSDATT2-.LLSDATTD2
.LLSDATTD2:
	.byte	0x1
	.uleb128 .LLSDACSE2-.LLSDACSB2
.LLSDACSB2:
	.uleb128 .LEHB4-.LFB2
	.uleb128 .LEHE4-.LEHB4
	.uleb128 .L17-.LFB2
	.uleb128 0x1
	.uleb128 .LEHB5-.LFB2
	.uleb128 .LEHE5-.LEHB5
	.uleb128 0
	.uleb128 0
	.uleb128 .LEHB6-.LFB2
	.uleb128 .LEHE6-.LEHB6
	.uleb128 .L18-.LFB2
	.uleb128 0
	.uleb128 .LEHB7-.LFB2
	.uleb128 .LEHE7-.LEHB7
	.uleb128 0
	.uleb128 0
.LLSDACSE2:
	.byte	0x1
	.byte	0
	.align 4
	.long	DW.ref._ZTI9Exception-.
.LLSDATT2:
	.text
	.size	_Z7catchitv, .-_Z7catchitv
	.globl	seppuku
	.type	seppuku, @function
seppuku:
.LFB3:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	call	_Z7catchitv
	nop
	popq	%rbp
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE3:
	.size	seppuku, .-seppuku
	.weak	_ZTI14Fake_Exception
	.section	.data.rel.ro._ZTI14Fake_Exception,"awG",@progbits,_ZTI14Fake_Exception,comdat
	.align 8
	.type	_ZTI14Fake_Exception, @object
	.size	_ZTI14Fake_Exception, 16
_ZTI14Fake_Exception:
	.quad	_ZTVN10__cxxabiv117__class_type_infoE+16
	.quad	_ZTS14Fake_Exception
	.weak	_ZTS14Fake_Exception
	.section	.rodata._ZTS14Fake_Exception,"aG",@progbits,_ZTS14Fake_Exception,comdat
	.align 16
	.type	_ZTS14Fake_Exception, @object
	.size	_ZTS14Fake_Exception, 17
_ZTS14Fake_Exception:
	.string	"14Fake_Exception"
	.weak	_ZTI9Exception
	.section	.data.rel.ro._ZTI9Exception,"awG",@progbits,_ZTI9Exception,comdat
	.align 8
	.type	_ZTI9Exception, @object
	.size	_ZTI9Exception, 16
_ZTI9Exception:
	.quad	_ZTVN10__cxxabiv117__class_type_infoE+16
	.quad	_ZTS9Exception
	.weak	_ZTS9Exception
	.section	.rodata._ZTS9Exception,"aG",@progbits,_ZTS9Exception,comdat
	.align 8
	.type	_ZTS9Exception, @object
	.size	_ZTS9Exception, 11
_ZTS9Exception:
	.string	"9Exception"
	.hidden	DW.ref._ZTI14Fake_Exception
	.weak	DW.ref._ZTI14Fake_Exception
	.section	.data.rel.local.DW.ref._ZTI14Fake_Exception,"awG",@progbits,DW.ref._ZTI14Fake_Exception,comdat
	.align 8
	.type	DW.ref._ZTI14Fake_Exception, @object
	.size	DW.ref._ZTI14Fake_Exception, 8
DW.ref._ZTI14Fake_Exception:
	.quad	_ZTI14Fake_Exception
	.hidden	DW.ref._ZTI9Exception
	.weak	DW.ref._ZTI9Exception
	.section	.data.rel.local.DW.ref._ZTI9Exception,"awG",@progbits,DW.ref._ZTI9Exception,comdat
	.align 8
	.type	DW.ref._ZTI9Exception, @object
	.size	DW.ref._ZTI9Exception, 8
DW.ref._ZTI9Exception:
	.quad	_ZTI9Exception
	.hidden	DW.ref.__gxx_personality_v0
	.weak	DW.ref.__gxx_personality_v0
	.section	.data.rel.local.DW.ref.__gxx_personality_v0,"awG",@progbits,DW.ref.__gxx_personality_v0,comdat
	.align 8
	.type	DW.ref.__gxx_personality_v0, @object
	.size	DW.ref.__gxx_personality_v0, 8
DW.ref.__gxx_personality_v0:
	.quad	__gxx_personality_v0
	.ident	"GCC: (Ubuntu 7.4.0-1ubuntu1~18.04.1) 7.4.0"
	.section	.note.GNU-stack,"",@progbits
