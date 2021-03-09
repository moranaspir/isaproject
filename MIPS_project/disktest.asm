		add $s0, $zero, $imm, -1			# i = -1 sector number counter
		add $a0, $zero, $imm, 1024			# 1024 ad for temp store in mem
		add $sp, $a0, $zero, 0				# $sp = 1024
		add $a1, $zero, $zero, 0			# j = 0 word in sector
		add $s1, $imm, $zero, 1				# const 1
		add $s2, $imm, $zero, 3				# const 3
		add $t0, $zero, $imm, 6				# set $t0 = 6
		add $t2, $zero, $imm, 128			# set $t2 = 128 (number of words in one sector)
		add $t3, $zero, $zero, 0			# init $t3 = 0
		out $imm, $t0, $zero, DISK			# set irqhandler as DISK
DISK:
		bge $imm, $s0, $s2, WRITE			# if i>3 go to WRITE
		in $t1, $zero, $imm, 17				# get diskstatus value
		bne $imm, $t1, $zero, DISK			# if diskstatus isn't free, go back to DISK
		add $s0, $s0, $imm, 1				# i++
		mul $t3, $s0, $t2, 0 				# $t3 = i*128
		add $a0, $a0, $t3, 0				# $a0 += $t3 new adress
		out $a0, $imm, $zero, 16			# set sector buffer ad
		out $s0, $imm, $zero, 15			# set sector number
		out $s1, $imm, $zero, 14			# set read
		out $s1, $imm, $zero, 1				# enable irq1
		beq $imm, $s0, $zero, DISK			# if first sector go back to DISK
		
LOOP:
		lw $v0, $a0, $a1, 0					# $v0 = disk[i+j] 
		lw $t0, $sp, $a1, 0					# $to = disk[sector 0 +j] 			
		xor $v0, $v0, $t0, 0				# 
		sw $v0, $sp, $a1, 0					# mem[1024+j] = xor
		add $a1, $a1, $imm, 1				# j++
		blt $imm, $a1, $t2, LOOP			# if j<128 go to LOOP
		add $a1, $zero, $zero, 0			# after the last word, j=0
		blt $imm, $s0, $s2, DISK			# if i<3 go to DISK
		
WRITE:
		in $t1, $zero, $imm, 17				# get diskstatus value
		bne $imm, $t1, $zero, WRITE			# if diskstatus isn't free, go back to WRITE
		add $s0, $s0, $imm, 1				# i++
		add $a0, $zero, $imm, 1024			# $a0 = 1024
		out $s1, $imm, $zero, 1				# enable irq1
		out $a0, $imm, $zero, 16			# set sector buffer ad
		out $s0, $imm, $zero, 15			# set sector number
		add $s1, $s1, $imm, 1				# $s1 = 2
		out $s1, $imm, $zero, 14			# set write
		out $s1, $imm, $zero, 0				# disable irq1
		
END:
		out $zero, $zero, $imm, 4			# set irq1 status to 0
		halt $zero, $zero, $zero, 0			# exit
		