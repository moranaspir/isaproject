	add $t3, $zero, $imm, 0x11F			# init $t3 = 0x11F
	add $s0, $zero, $zero, 0			# set i = 0
LOOP1:
	add $s1, $zero, $zero, 0			# set r = 0	
LOOP2:
	add $s2, $zero, $zero, 0			# set c = 0
	add $t3, $t3, $imm, 1				# $t3 ++
	lw $t2, $t3, $zero, 0				# $t2 = C[i,r]
LOOP3:
	sll $t0, $s0, $imm, 2				# $t0 = 4*i
	add $t0, $t0, $s2, 0				# $t0 = 4*i + c
	lw $t0, $t0, $imm, 0x100			# $t0 = A[i,c]
	sll $t1, $s2, $imm, 2				# $t1 = 4*c
	add $t1, $t1, $s1, 0				# $t1 = 4*c + r
	lw $t1, $t1, $imm, 0x110			# $t1 = B[c,r]
	mul $t0, $t0, $t1, 0				# $t0 = A[i,c]*B[c,r]
	add $t2, $t2, $t0, 0				# C[i,r] += A[i,c]*B[c,r]
	add $s2, $s2, $imm, 1				# c++
	add $t0, $imm, $zero, 4				# set $t0 = 4
	bne $imm, $s2, $t0, LOOP3			# if (c < 4) jump to LOOP3
	sw $t2, $t3, $zero, 0				# C[i,r] = $t2
	add $s1, $s1, $imm, 1				# r++
	bne $imm, $s1, $t0, LOOP2			# if (r < 4) jump to LOOP2
	add $s0, $s0, $imm, 1				# i++ 
	bne $imm, $s0, $t0, LOOP1			# if (i < 4) jump to LOOP3
	halt $zero, $zero, $zero, 0 		# Exit


.word 0x100 1						# A[0,0] = 1
.word 0x101 3						# A[0,1] = 3
.word 0x102 -1						# A[0,2] = -1
.word 0x103 4						# A[0,3] = 4
.word 0x104 0						# A[1,1] = 0
.word 0x105 5						# A[1,0] = 5
.word 0x106 12						# A[1,1] = 12
.word 0x107 3						# A[1,2] = 3
.word 0x108 0xF						# A[2,0] = 0xF
.word 0x109 -3						# A[2,1] = -3
.word 0x10A 12						# A[2,2] = 12
.word 0x10B 7						# A[2,3] = 7
.word 0x10C 0						# A[3,0] = 0
.word 0x10D 2						# A[3,1] = 2
.word 0x10E 2						# A[3,2] = 2
.word 0x10F 1						# A[3,3] = 1

.word 0x110 0x023					# B[0,0] = 0x023
.word 0x111 1						# B[0,1] = 1
.word 0x112 2						# B[0,2] = 2
.word 0x113 3						# B[0,3] = 3
.word 0x114 4						# B[1,1] = 4
.word 0x115 0						# B[1,0] = 0
.word 0x116 0						# B[1,1] = 0
.word 0x117 -23						# B[1,2] = -23
.word 0x118 2						# B[2,0] = 2
.word 0x119 7						# B[2,1] = 7
.word 0x11A 13						# B[2,2] = 13
.word 0x11B 3						# B[2,3] = 3
.word 0x11C 4						# B[3,0] = 4
.word 0x11D 9						# B[3,1] = 9
.word 0x11E 8						# B[3,2] = 8
.word 0x11F -3						# B[3,3] = -3

.word 0x120 0						# C[0,0] = 0
.word 0x121 0						# C[0,1] = 0
.word 0x122 0						# C[0,2] = 0
.word 0x123 0						# C[0,3] = 0
.word 0x124 0						# C[1,1] = 0
.word 0x125 0						# C[1,0] = 0
.word 0x126 0						# C[1,1] = 0
.word 0x127 0						# C[1,2] = 0
.word 0x128 0						# C[2,0] = 0
.word 0x129 0						# C[2,1] = 0
.word 0x12A 0						# C[2,2] = 0
.word 0x12B 0						# C[2,3] = 0
.word 0x12C 0						# C[3,0] = 0
.word 0x12D 0						# C[3,1] = 0
.word 0x12E 0						# C[3,2] = 0
.word 0x12F 0						# C[3,3] = 0

