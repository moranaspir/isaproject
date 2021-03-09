		beq $imm, $zero, $zero, START			#brench to START
LED:	
		add $s2, $s2, $imm, 1 					#i++
		blt $imm, $s1, $s2, EXIT				# if n<i EXIT, else light the next leds
		sll $t1, $s0, $s2,0						# set $t1 as the next led. $t1=1<<i
		or $t2, $t2, $t1,0	
		out $t2, $zero, $imm, 9					# tern on the next led and keep the old on in
		out $zero, $zero, $imm, 3				# set irq0 status to 0
		reti $zero, $zero, $zero,0				# return from interrupt
		
EXIT:
		out $zero, $zero, $imm, 3				# set irq0 status to 0
		halt $zero, $zero, $zero, 0				# end 	
	
START:
		add $t0, $zero, $imm, 6 				#set $t0=6
		out $imm, $t0, $zero, LED 				#set irqhandler as LED
		add $s0, $zero, $imm, 1					#set $s0=1
		out $s0, $zero, $zero, 0 				#enable irq0
		add $t0, $zero, $imm, 1019 
		out $t0, $zero, $imm, 13				#set timer max value as 1023. that way the interrupt is enable every second
		add $s1, $zero, $imm, 31 				#set n as number of leds, 31.
		add $s2, $zero, $imm, -1				#set i=-1
		add $t2, $zero, $imm, 1
		out $s0, $zero, $imm, 11 				#enable timer. now wait until the interrupt 