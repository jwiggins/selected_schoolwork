	.file	"my_gemm.c"
	.text
	.align 16
.globl my_gemm
	.type	my_gemm,@function
my_gemm:
	pushl	%ebp
	pushl	%edi
	pushl	%esi
	pushl	%ebx
	subl	$28, %esp
	movl	52(%esp), %eax
	movl	$0, 24(%esp)
	cmpl	%eax, 24(%esp)
	jge	.L46
	.p2align 4
.L5:
	movl	52(%esp), %eax
	movl	%eax, 16(%esp)
	movl	24(%esp), %eax
	subl	%eax, 16(%esp)
	cmpl	$40, 16(%esp)
	jle	.L6
	movl	$40, 16(%esp)
.L6:
	movl	$0, 20(%esp)
	movl	56(%esp), %edx
	cmpl	%edx, 20(%esp)
	jge	.L47
	.p2align 4
.L10:
	movl	56(%esp), %ebp
	movl	20(%esp), %ebx
	subl	%ebx, %ebp
	cmpl	$40, %ebp
	jle	.L11
	movl	$40, %ebp
.L11:
	xorl	%edi, %edi
	cmpl	48(%esp), %edi
	jge	.L48
	movl	24(%esp), %eax
	movl	72(%esp), %ebx
	movl	20(%esp), %edx
	movl	68(%esp), %esi
	imull	%ebx, %eax
	addl	%edx, %eax
	leal	(%esi,%eax,8), %ecx
	movl	%ecx, 12(%esp)
	.p2align 4
.L15:
	movl	48(%esp), %edx
	subl	%edi, %edx
	cmpl	$40, %edx
	jle	.L51
	movl	$40, %edx
.L51:
	subl	$12, %esp
	movl	92(%esp), %esi
	pushl	%esi
	movl	96(%esp), %eax
	movl	40(%esp), %esi
	movl	92(%esp), %ecx
	imull	%eax, %esi
	leal	(%edi,%esi), %eax
	leal	(%ecx,%eax,8), %ebx
	pushl	%ebx
	movl	92(%esp), %eax
	pushl	%eax
	movl	36(%esp), %ecx
	pushl	%ecx
	movl	92(%esp), %ebx
	pushl	%ebx
	movl	96(%esp), %eax
	movl	52(%esp), %ebx
	movl	92(%esp), %ecx
	imull	%eax, %ebx
	leal	(%edi,%ebx), %eax
	leal	(%ecx,%eax,8), %eax
	pushl	%eax
	pushl	%ebp
	movl	56(%esp), %eax
	pushl	%eax
	pushl	%edx
	call	my_gemm_util
	addl	$48, %esp
	leal	40(%edi), %ecx
	cmpl	48(%esp), %ecx
	jge	.L48
	movl	48(%esp), %edx
	subl	%ecx, %edx
	cmpl	$40, %edx
	jle	.L54
	movl	$40, %edx
.L54:
	subl	$12, %esp
	movl	92(%esp), %eax
	addl	$80, %edi
	pushl	%eax
	leal	(%ecx,%esi), %eax
	movl	92(%esp), %esi
	leal	(%esi,%eax,8), %eax
	leal	(%ecx,%ebx), %esi
	pushl	%eax
	movl	92(%esp), %eax
	pushl	%eax
	movl	36(%esp), %eax
	pushl	%eax
	movl	92(%esp), %eax
	pushl	%eax
	movl	92(%esp), %eax
	leal	(%eax,%esi,8), %ebx
	pushl	%ebx
	pushl	%ebp
	movl	56(%esp), %eax
	pushl	%eax
	pushl	%edx
	call	my_gemm_util
	addl	$48, %esp
	cmpl	48(%esp), %edi
	jl	.L15
.L48:
	addl	$40, 20(%esp)
	movl	56(%esp), %edi
	cmpl	%edi, 20(%esp)
	jl	.L10
.L47:
	addl	$40, 24(%esp)
	movl	52(%esp), %eax
	cmpl	%eax, 24(%esp)
	jl	.L5
.L46:
	addl	$28, %esp
	popl	%ebx
	popl	%esi
	popl	%edi
	popl	%ebp
	ret
.Lfe1:
	.size	my_gemm,.Lfe1-my_gemm
	.align 16
	.type	my_gemm_util,@function
my_gemm_util:
	pushl	%ebp
	pushl	%edi
	pushl	%esi
	pushl	%ebx
	subl	$24, %esp
	movl	48(%esp), %eax
	movl	$0, 16(%esp)
	movl	56(%esp), %edi
	cmpl	%eax, 16(%esp)
	je	.L56
	movl	$0, 4(%esp)
	movl	44(%esp), %eax
	movl	$0, (%esp)
	subl	$5, %eax
	movl	%eax, 12(%esp)
	.p2align 4
.L24:
	movl	$0, 20(%esp)
	movl	12(%esp), %eax
	cmpl	%eax, 20(%esp)
	jg	.L57
	movl	4(%esp), %ebx
	.p2align 4
.L28:
	xorl	%ebp, %ebp
	movl	72(%esp), %eax
	cmpl	52(%esp), %ebp
	fldl	(%eax,%ebx,8)
	fldl	8(%eax,%ebx,8)
	fldl	16(%eax,%ebx,8)
	fldl	24(%eax,%ebx,8)
	fldl	32(%eax,%ebx,8)
	je	.L58
	movl	52(%esp), %edx
	movl	(%esp), %eax
	movl	64(%esp), %esi
	andl	$3, %edx
	leal	(%esi,%eax,8), %ecx
	movl	20(%esp), %eax
	je	.L32
	cmpl	$1, %edx
	jle	.L82
	cmpl	$2, %edx
	jle	.L83
	fldl	(%ecx)
	movl	$1, %ebp
	addl	$8, %ecx
	fldl	(%edi,%eax,8)
	movl	60(%esp), %edx
	fmul	%st(1), %st
	faddp	%st, %st(6)
	fldl	8(%edi,%eax,8)
	fmul	%st(1), %st
	faddp	%st, %st(5)
	fldl	16(%edi,%eax,8)
	fmul	%st(1), %st
	faddp	%st, %st(4)
	fldl	24(%edi,%eax,8)
	fmul	%st(1), %st
	faddp	%st, %st(3)
	fldl	32(%edi,%eax,8)
	addl	%edx, %eax
	fmulp	%st, %st(1)
	faddp	%st, %st(1)
.L83:
	fldl	(%ecx)
	incl	%ebp
	addl	$8, %ecx
	fldl	(%edi,%eax,8)
	movl	60(%esp), %edx
	fmul	%st(1), %st
	faddp	%st, %st(6)
	fldl	8(%edi,%eax,8)
	fmul	%st(1), %st
	faddp	%st, %st(5)
	fldl	16(%edi,%eax,8)
	fmul	%st(1), %st
	faddp	%st, %st(4)
	fldl	24(%edi,%eax,8)
	fmul	%st(1), %st
	faddp	%st, %st(3)
	fldl	32(%edi,%eax,8)
	addl	%edx, %eax
	fmulp	%st, %st(1)
	faddp	%st, %st(1)
.L82:
	fldl	(%ecx)
	incl	%ebp
	addl	$8, %ecx
	fldl	(%edi,%eax,8)
	fxch	%st(1)
	fxch	%st(2)
	fxch	%st(1)
	movl	60(%esp), %esi
	jmp	.L99
	.p2align 4,,7
.L32:
	fldl	(%ecx)
	addl	$4, %ebp
	movl	60(%esp), %edx
	fldl	(%edi,%eax,8)
	movl	60(%esp), %esi
	fmul	%st(1), %st
	faddp	%st, %st(6)
	fldl	8(%edi,%eax,8)
	fmul	%st(1), %st
	faddp	%st, %st(5)
	fldl	16(%edi,%eax,8)
	fmul	%st(1), %st
	faddp	%st, %st(4)
	fldl	24(%edi,%eax,8)
	fmul	%st(1), %st
	faddp	%st, %st(3)
	fldl	32(%edi,%eax,8)
	addl	%edx, %eax
	movl	60(%esp), %edx
	fmulp	%st, %st(1)
	fldl	8(%ecx)
	fxch	%st(2)
	faddp	%st, %st(1)
	fldl	(%edi,%eax,8)
	fmul	%st(2), %st
	faddp	%st, %st(6)
	fldl	8(%edi,%eax,8)
	fmul	%st(2), %st
	faddp	%st, %st(5)
	fldl	16(%edi,%eax,8)
	fmul	%st(2), %st
	faddp	%st, %st(4)
	fldl	24(%edi,%eax,8)
	fmul	%st(2), %st
	faddp	%st, %st(3)
	fldl	32(%edi,%eax,8)
	addl	%esi, %eax
	movl	60(%esp), %esi
	fmulp	%st, %st(2)
	fldl	16(%ecx)
	fxch	%st(1)
	faddp	%st, %st(2)
	fldl	(%edi,%eax,8)
	fmul	%st(1), %st
	faddp	%st, %st(6)
	fldl	8(%edi,%eax,8)
	fmul	%st(1), %st
	faddp	%st, %st(5)
	fldl	16(%edi,%eax,8)
	fmul	%st(1), %st
	faddp	%st, %st(4)
	fldl	24(%edi,%eax,8)
	fmul	%st(1), %st
	faddp	%st, %st(3)
	fldl	32(%edi,%eax,8)
	addl	%edx, %eax
	fmulp	%st, %st(1)
	fldl	24(%ecx)
	fxch	%st(2)
	addl	$32, %ecx
	faddp	%st, %st(1)
	fldl	(%edi,%eax,8)
.L99:
	fmul	%st(2), %st
	faddp	%st, %st(6)
	fldl	8(%edi,%eax,8)
	fmul	%st(2), %st
	faddp	%st, %st(5)
	fldl	16(%edi,%eax,8)
	fmul	%st(2), %st
	faddp	%st, %st(4)
	fldl	24(%edi,%eax,8)
	fmul	%st(2), %st
	faddp	%st, %st(3)
	fldl	32(%edi,%eax,8)
	addl	%esi, %eax
	cmpl	52(%esp), %ebp
	fmulp	%st, %st(2)
	faddp	%st, %st(1)
	jne	.L32
.L58:
	fxch	%st(4)
	movl	72(%esp), %eax
	fstpl	(%eax,%ebx,8)
	fxch	%st(2)
	fstpl	8(%eax,%ebx,8)
	fstpl	16(%eax,%ebx,8)
	fstpl	24(%eax,%ebx,8)
	fstpl	32(%eax,%ebx,8)
	addl	$5, %ebx
	movl	12(%esp), %ecx
	addl	$5, 20(%esp)
	cmpl	%ecx, 20(%esp)
	jle	.L28
.L57:
	movl	44(%esp), %eax
	cmpl	%eax, 20(%esp)
	jge	.L59
	movl	(%esp), %ebx
	sall	$3, %ebx
	movl	%ebx, 8(%esp)
	.p2align 4
.L38:
	xorl	%ebp, %ebp
	cmpl	52(%esp), %ebp
	je	.L60
	movl	20(%esp), %eax
	movl	4(%esp), %ebx
	movl	20(%esp), %esi
	movl	8(%esp), %ecx
	addl	%eax, %ebx
	movl	64(%esp), %eax
	movl	52(%esp), %edx
	addl	%eax, %ecx
	leal	(%edi,%esi,8), %eax
	movl	60(%esp), %esi
	sall	$3, %esi
	andl	$3, %edx
	je	.L42
	cmpl	$1, %edx
	jle	.L63
	cmpl	$2, %edx
	jle	.L64
	fldl	(%eax)
	movl	72(%esp), %ebp
	addl	%esi, %eax
	fmull	(%ecx)
	addl	$8, %ecx
	faddl	(%ebp,%ebx,8)
	fstpl	(%ebp,%ebx,8)
	movl	$1, %ebp
.L64:
	fldl	(%eax)
	incl	%ebp
	movl	72(%esp), %edx
	addl	%esi, %eax
	fmull	(%ecx)
	addl	$8, %ecx
	faddl	(%edx,%ebx,8)
	fstpl	(%edx,%ebx,8)
.L63:
	fldl	(%eax)
	incl	%ebp
	movl	72(%esp), %edx
	addl	%esi, %eax
	fmull	(%ecx)
	addl	$8, %ecx
	faddl	(%edx,%ebx,8)
	fstpl	(%edx,%ebx,8)
	cmpl	52(%esp), %ebp
	je	.L60
	.p2align 4
.L42:
	fldl	(%eax)
	addl	$4, %ebp
	movl	72(%esp), %edx
	addl	%esi, %eax
	fmull	(%ecx)
	faddl	(%edx,%ebx,8)
	fstl	(%edx,%ebx,8)
	fldl	(%eax)
	addl	%esi, %eax
	fmull	8(%ecx)
	faddp	%st, %st(1)
	fstl	(%edx,%ebx,8)
	fldl	(%eax)
	addl	%esi, %eax
	fmull	16(%ecx)
	faddp	%st, %st(1)
	fstl	(%edx,%ebx,8)
	fldl	(%eax)
	addl	%esi, %eax
	fmull	24(%ecx)
	addl	$32, %ecx
	faddp	%st, %st(1)
	fstpl	(%edx,%ebx,8)
	cmpl	52(%esp), %ebp
	jne	.L42
.L60:
	incl	20(%esp)
	movl	44(%esp), %eax
	cmpl	%eax, 20(%esp)
	jl	.L38
.L59:
	incl	16(%esp)
	movl	76(%esp), %ebx
	movl	68(%esp), %eax
	movl	48(%esp), %ecx
	addl	%ebx, 4(%esp)
	addl	%eax, (%esp)
	cmpl	%ecx, 16(%esp)
	jne	.L24
.L56:
	addl	$24, %esp
	popl	%ebx
	popl	%esi
	popl	%edi
	popl	%ebp
	ret
.Lfe2:
	.size	my_gemm_util,.Lfe2-my_gemm_util
	.ident	"GCC: (GNU) 3.0.3"
