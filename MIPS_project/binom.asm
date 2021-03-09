	add $sp, $zero, $imm, 4096			#
	lw $a0, $zero, $imm, 0x100			# set $a0 as n
	lw $a1, $zero, $imm, 0x101			# set $a0 as k
	jal $imm, $zero, $zero, BINOM		# calles BINOM(n,k)
	sw $v0, $zero, $imm, 0x102			# save value in 0x102 address
	halt $zero, $zero, $zero, 0
BINOM:
	add $sp, $sp, $imm, -4
	sw $ra, $sp, $imm, 0 				#
	sw $a0, $sp, $imm, 1				# $a0 = n
	sw $a1, $sp, $imm, 2				# $a1 = k
	sw $s0, $sp, $imm, 3
	beq $imm, $a1, $zero, RET 
	beq $imm, $a0, $a1, RET
	add $a0, $a0, $imm, -1 				# $a0 = n-1
	jal $imm, $zero, $zero, BINOM
	add $s0, $v0, $zero, 0				# save BINOM(n-1,k)
	lw $a0, $sp, $imm, 1				# $a0 = n
	add $a0, $a0, $imm, -1 				# $a0 = n-1
	add $a1, $a1, $imm, -1			 	# $a1 = k-1
	jal $imm, $zero, $zero, BINOM	
	add $v0, $s0, $v0, 0				# $v0 = BINOM(n-1,k) + BINOM(n-1,k-1)
	lw $s0, $sp, $imm, 3
	lw $a1, $sp, $imm, 2
	lw $a0, $sp, $imm, 1	
	lw $ra, $sp, $imm, 0 
	add $sp, $sp, $imm, 4
	beq $ra, $zero, $zero, 0
RET:
	add $sp, $sp, $imm, 4	
	add $v0, $zero, $imm, 1 			# return 1
	beq $ra, $zero, $zero, 0
				
.word 0x100 15 							# n = 15
.word 0x101 3							# k = 3