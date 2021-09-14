;cmtCryp assembly support
;amd64 version for masm
.code

;cx dx r8 r9
;void cmtRealRand(cmtUint64* buf, cmtUint64 len)
cmtRealRand proc
	mov rbx, rcx
	xor rcx, rcx

	;rbx = buf
	;rdx = len
	;rcx = r
	cRR_GenLoopStart:
	cmp rcx, rdx
	jae cRR_End
	;生成随机数
	cRR_GenRand:
	rdrand eax
	;检查结果，失败重来
	jnc cRR_GenRand
	;写入数组
	mov qword ptr [rbx + rcx*8], rax
	;i++
	inc rcx
	jmp cRR_GenLoopStart
	
	cRR_End:
	ret
cmtRealRand endp

end