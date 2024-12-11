	.arch msp430g2553
	.p2align 1,0
	.text

    .global switch_interrupt_handler
    .extern switch_update_interrupt_sense
    .extern colVelocity
    .extern jumpVelocity
    .extern switches
    .extern buzzerEnabled
    .extern gameRunning
    .extern P1OUT
    .extern LED
    .extern fillRectangle
    .extern drawString5x7
    .extern BACKGROUND_COLOR
    .extern COLOR_RED

string_paused:
    .string "PAUSED"

switch_interrupt_handler:
    call #switch_update_interrupt_sense
    mov r12, r4
    xor #0xFF, r4
    and #15, r4
    mov r4, &switches

    mov r4, r5           
    and #1, r5            
    cmp #0, r5           
    jz button_3           
    mov &jumpVelocity, &colVelocity

button_3:
    mov r4, r6           
    and #4, r6            
    cmp #0, r6           
    jz button_4           
    xor #1, &buzzerEnabled

button_4:
    mov r4, r7           
    and #8, r7            
    cmp #0, r7           
    jz out           
    xor #1, &gameRunning
    mov &gameRunning, r7
    cmp #1, r7
    jnz paused
    bis &LED, &P1OUT
    mov #90, r5       
    mov #0, r6         
    mov #50, r7         
    mov #10, r8     
    mov &BACKGROUND_COLOR, r9
    push r5
    push r6
    push r7
    push r8
    push r9      
    call    #fillRectangle 
    pop r5
    pop r6
    pop r7
    pop r8
    pop r9   
    jmp out 

paused:
    bic &LED, &P1OUT
    mov #90, r5       
    mov #0, r6         
    mov #string_paused, r7        
    mov &COLOR_RED, r8     
    mov &BACKGROUND_COLOR, r9
    push r5
    push r6
    push r7
    push r8
    push r9        
    call #drawString5x7
    pop r5
    pop r6
    pop r7
    pop r8
    pop r9       
    jmp out 

out:
    reti

    