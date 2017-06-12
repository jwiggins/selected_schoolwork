	.data
	.align 16
	.bss
	.align 16
	.text
/ -- Begin dclock
        .align 16
	.globl   dclock
dclock:
	.byte	0xf		 
	.byte	0x31

        movl	%eax, .low	 
        movl	%edx, .high

        fildll  .low		 
		fmull   .clock

        ret

        .align 16
	.type	dclock,@function
	.size	dclock,.-dclock
	.data

	/ 133 MHz
/ .clock: .double 0.0000000075
	/ 150 MHz
/ .clock: .double 0.00000000666667
	/ 166 MHz
/ .clock: .double 0.0000000060
	/ 200 MHz
/ .clock: .double 0.0000000050
	/ 233 Mhz
/ .clock: .double 0.00000000429184
	/ 300 MHz
/ .clock: .double 0.00000000333333
	/ 366 MHz
/ .clock: .double 0.00000000272727 
	/ uncomment the next line if 600 MHz processor
/ .clock: .double 0.000000001666667 
	/ 650 MHz
/ .clock: .double 0.0000000015384615  
	/ 600 MHz
/ .clock: .double 0.0000000016666667
	/ 866 MHz
  .clock: .double 0.0000000011547344

.low:   .4byte 0x00000000
.high:  .4byte 0x00000000
	.text
