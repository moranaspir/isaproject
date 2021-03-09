		add $s0, $zero, $imm, 1024				# set $so as the start of the array
		add $a0, $zero, $imm, 16				# $a0=n=16 length of the array
START:
		add $t0, $zero, $zero, 0				# i=0
		add $s1, $zero, $zero, 0				# set $s1 as swapped flag = 0
LOOP:
		add $t0, $t0, $imm, 1					# i++
		bge $imm, $t0, $a0, TEST				# if i>=n TEST
		lw $t1, $s0, $t0, 0						# $t1 = array[i]
		add $t2, $t0, $imm, -1					# $t2 = i-1
		lw $t3, $s0, $t2, 0						# $t3 = array[i-1]
		bge $imm, $t3, $t1, LOOP				# if array[i-1] >= array[i] LOOP, else swapp
		sw $t1, $s0, $t2, 0						# array[i-1] = array[i]
		sw $t3, $s0, $t0, 0						# array[i] = array [i-1]
		add $s1, $zero, $imm, 1					# swapped flag = 1
		beq $imm, $zero, $zero, LOOP			# make another iteration
TEST:
		beq $imm, $s1, $zero, END				# if swapped flag = 0, END
		beq $imm, $zero, $zero, START			# else, go to the start
END:
		halt $zero, $zero, $zero, 0			# exit
		
.word 1024 1
.word 1025 -3
.word 1026 6
.word 1027 2
.word 1028 10
.word 1029 7
.word 1030 19
.word 1031 3
.word 1032 4
.word 1033 7
.word 1034 10
.word 1035 11
.word 1036 16
.word 1037 6
.word 1038 8
.word 1039 8
