	.word 0x100 100					#set radius of circle = 100
	.word 0x101 15					#set radius of eye = 15
	
	
	lw $s0, $zero, $imm, 0x100		# $s0 = radius
	sub $fp, $s0, $imm, 20			# smile radius
	lw $gp, $zero, $imm, 0x101		# gp = eye radius
	mul $s0, $s0, $s0, 0       	 	# $s0 = radius^2
	mul $gp, $gp, $gp, 0       	 	# $gp = eye radius^2
	mul $fp, $fp, $fp, 0       	 	# $fp = smile radius^2
	add $a1, $zero, $imm, 160		# smile y limit
	add $t0, $zero, $imm, -1		# X = -1
	add $t2, $zero, $imm, 352		# $t2 = number_of rows
	add $t3, $zero, $imm, 288		# $t3 = number_of columes
LOOP1:
	add $t1, $zero, $imm, 0			# Y = 0
	add $t0, $t0, $imm, 1			# X++	
	beq $imm, $t0, $t2, END			# if X == 352 go to END
	
LOOP2:
	beq $imm, $t1, $t3, LOOP1
	sub $s1, $t0, $imm, 175			# $s1 = x-175
	mul $s1, $s1, $s1, 0			# $s1 = (x-175)^2
	sub $s2, $t1, $imm, 143			# $s2 = y-143
	mul $s2, $s2, $s2, 0			# $s2 = (y-143)^2		
	add $s1, $s1, $s2, 0 			# $s1 = (x-175)^2 + (y-143)^2
	bgt $imm, $s1, $s0, ENDLOOP	 	# not in circle
	
	sub $s1, $t0, $imm, 150			# $s1 = x-150
	mul $s1, $s1, $s1, 0			# $s1 = (x-150)^2
	sub $s2, $t1, $imm, 125			# $s2 = y-125
	mul $s2, $s2, $s2, 0			# $s2 = (x-125)^2		
	add $s1, $s1, $s2, 0 			# $s1 = (x-150)^2 + (y-125)^2
	blt $imm, $s1, $gp, ENDLOOP	 	# in eye so jump

	sub $s1, $t0, $imm, 200			# $s1 = x-200
	mul $s1, $s1, $s1, 0			# $s1 = (x-200)^2
	sub $s2, $t1, $imm, 125			# $s2 = y-125
	mul $s2, $s2, $s2, 0			# $s2 = (y-125)^2		
	add $s1, $s1, $s2, 0 			# $s1 = (x-200)^2 + (y-125)^2
	blt $imm, $s1, $gp, ENDLOOP	 	# in eye so jump

	sub $s1, $t0, $imm, 175			# $s1 = x-175
	mul $s1, $s1, $s1, 0			# $s1 = (x-175)^2
	sub $s2, $t1, $imm, 143			# $s2 = y-143
	mul $s2, $s2, $s2, 0			# $s2 = (y-143)^2		
	add $s1, $s1, $s2, 0 			# $s1 = (x-175)^2 + (y-143)^2
	
	blt $imm, $s1, $fp, SMILE	 	# in smile so jump to check
	beq $imm, $zero, $zero, MONITOR	# jump to MONITOR


SMILE:
	bgt $imm, $t1, $a1, ENDLOOP		# y is lower than smile barrier so NO
	beq $imm, $zero, $zero, MONITOR	# jump to MONITOR


ENDLOOP:
	add $t1, $t1, $imm, 1			# Y++	
	beq $imm, $zero, $zero, LOOP2	# jump to LOOP2	
	
	
MONITOR:
	out $t0, $zero, $imm, 19 		# set monitorX
	out $t1, $zero, $imm, 20 		# set monitorY
	add $s2, $zero, $imm, 255		# set $s2 = 255
	out $s2, $zero, $imm, 21 		# set monitorData to 255
	add $s2, $zero, $imm, 1			# set $s2 = 1
	out $s2, $zero, $imm, 18		# monitorcmd = 1 
	out $zero, $zero, $imm, 18		# monitorcmd = 0
	add $t1, $t1, $imm, 1			# Y++
	beq $imm, $zero, $zero, LOOP2	# jump to LOOP2
END:
	halt $zero, $zero, $zero, 0 	# end program
	
	
	


