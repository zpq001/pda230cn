 ;########################################################
 ;######        USART bootloader for ATMEGA8        ######
 ;######              (c) Avega 2013                ######
 ;######                                            ######
 ;########################################################

.include "m8def.inc" 


;===== system definitions ===========
.def Temp=r16
.def CNT0=r17		;buttons reg
.def CNT1=r18		;switches reg
.def ADDRL=r19
.def ADDRH=r20
.def NumBytes=r21
.def Temp2=r22
.def Flag=r23

.equ BUF_L = low(SRAM_START)
.equ BUF_H = high(SRAM_START)
.equ PAGESIZEBYTES = PAGESIZE*2

;====== IO definitions =============


;====== protocol definitions =======
.equ BOOT_VERSION 		= 0x10
.equ OPERATION_OK 		= 'o'
.equ PAGE_WRITTEN 		= '1'
.equ PAGE_EQUAL	  		= '2'
.equ SIGNATURE_REQUEST 	= 's'
.equ WRITE_DATA_ADRESS 	= 'a'
.equ FILL_DATA_BUFFER  	= 'b'
.equ PAGE_ERASE 		= 'c'
.equ PAGE_WRITE			= 'w'
.equ PAGE_READ			= 'r'
.equ EEPROM_WRITE		= 'e'
.equ EEPROM_READ		= 'p'
.equ READ_FUSES			= 'f'
.equ EXIT				= 'q'
.equ BAD_COMMAND		= '?'


;bootloader fits in 256-words space (in NRWW section)
;fuses must be set to BOOTSZ1=1, BOOTSZ0=1 (for ATMEGA32)
;bootloader starts if BOOT_CONDITION is true and remains until 
;reset is performed or specific command to restart is received
;Before call from user program make sure that interrupts are disabled
;(by "cli" instruction) and peripheral devices are disabled too

    .cseg				
	.org SECONDBOOTSTART	

;************ initialization *************;
;------------ stack init -----------;
	ldi Temp,low(RAMEND)		
	out SPL,Temp	
	ldi Temp,High(RAMEND)
	out SPH,Temp

;------------- IO setup ------------;
	; PORTC - all inputs
	ldi temp,0x00	
	out DDRC,temp
	out PortC,temp
	
	; PORTB - all inputs
	ldi temp,0x00
	out DDRB,temp
	out PortB,temp
	
	; PORTD - all inputs
	ldi temp,0x00
	out DDRD,temp
	out PortD,temp
	

	// Setup USART

	// Double speed
    ldi Temp,(1<<U2X)
    out UCSRA,Temp 
    ldi Temp,(1<<RXEN | 1<<TXEN | 0<<UCSZ2)
	out UCSRB,Temp
	// Even parity, 1 stop bit, 8 bit
	ldi Temp,(1<<URSEL | 1<<UPM1 | 0<<UPM0 | 0<<USBS | 1<<UCSZ1 | 1<<UCSZ0)
	out UCSRC,Temp
	// 57600 @16MHz, 2x
	ldi Temp,0x00
	out UBRRH,Temp
	ldi Temp,0x22	
	out UBRRL,Temp

;*******************************************;

;******* Check for BOOT_CONDITION **********;
	
	in Temp,PinD
	andi Temp,0x01

	brne Wait				;if BOOT_CONDITION is false,
	ldi ZL, 0x00
	ldi ZH, 0x00
	ijmp					;jump to start of main program

;*******************************************;

	
Wait:
	;sbi PortD,LED_RED
	rcall GetByte
	;cbi PortD,LED_RED
;---------- signature request -------------
;PC gets 4 bytes: version of program and AVR's original signature
NC0:
	cpi temp,SIGNATURE_REQUEST		
	brne NC1
	rcall SendSigVers
	rjmp Wait
;------------ write data adress -----------
NC1:
	cpi temp,WRITE_DATA_ADRESS		
	brne NC2
	rcall StoreAddrNum
	ldi temp,OPERATION_OK		;answer
	rcall SendByte
	rjmp Wait
;------------ fill data buffer ------------
NC2:
	cpi temp,FILL_DATA_BUFFER
	brne NC3
	rcall FillPageBuffer
	ldi temp,OPERATION_OK		;answer
	rcall SendByte
	rjmp Wait
;-------------- page write ----------------
NC3:
	cpi temp,PAGE_WRITE
	brne NC4
	rcall PageBufferWriteToFLASH
	rcall SendByte				;result = temp
	rjmp Wait

;-------------- page read -----------------
NC4:
	cpi temp,PAGE_READ
	brne NC5
	rcall ReadSendFlashPage
	rjmp Wait

;------------- EEPROM write ---------------
NC5:
	cpi temp,EEPROM_WRITE
	brne NC6
	rcall PageBufferWriteToEEPROM
	rcall SendByte				;result = temp
	rjmp Wait
;------------- EEPROM read ----------------
NC6:
	cpi temp,EEPROM_READ
	brne NC7
	rcall ReadSendEEPROMData
	rjmp Wait
;------- read FUSES and LOCK bits ---------
NC7:
	cpi temp,READ_FUSES
	brne NC8
	rcall ReadFusesLockBits
	rjmp Wait
;-------------- EXIT ----------------------
NC8:
	cpi temp,EXIT
	brne NC9
	ldi temp,OPERATION_OK		;answer
	rcall SendByte
	ldi ZL, 0x00
	ldi ZH, 0x00
	ijmp					;jump to start of main program
;------------- Page erase -----------------	
NC9:
	cpi temp,PAGE_ERASE
	brne NC10
	rcall PageErase
	ldi temp,OPERATION_OK		;answer
	rcall SendByte
	rjmp Wait

NC10:
	ldi temp,BAD_COMMAND
	rcall SendByte
	rjmp Wait



;********** sending signature and version *************
;Affects: temp
SendSigVers:
	ldi temp,BOOT_VERSION		;bootloader's version
	rcall SendByte
	ldi temp,SIGNATURE_000		;AVR'MCU signature byte 0
	rcall SendByte
	ldi temp,SIGNATURE_001		;AVR'MCU signature byte 1
	rcall SendByte	
	ldi temp,SIGNATURE_002		;AVR'MCU signature byte 2
	rcall SendByte
	ret
;******************************************************

;**** getting and storing adress and num of bytes *****
;Affects: temp
StoreAddrNum:
	rcall GetByte		;first get number of bytes to write
	mov NumBytes,temp
	rcall GetByte		;get low adress
	mov ADDRL,temp	
	rcall GetByte		;get high adress
	mov ADDRH,temp		
	ret
;******************************************************

;******* filling temporary buffer from USART **********
;Purpose:	get and store in RAM program data,amount = NumBytes
;Result:	filled buffer in SRAM
;Affects: temp,Y,CNT0
FillPageBuffer:
	ldi YH,BUF_H		
	ldi YL,BUF_L
	mov CNT0,NumBytes	;copy number of bytes to write
Fill_0:
	rcall GetByte
	st Y+,temp
	dec CNT0
	brne Fill_0
	ret
;****************************************************

;********** compare and write FLASH page ************;
;Purpose:	write previously got (into buffer) program data
;			specified by adress ADDRL:ADDRH in bytes
;			amount=NumBytes.
;Returns:	temp=PAGE_EQUAL if erase and write operations were not performed
;			temp=PAGE_WRITTEN if page has been written
;Affects:	temp,Y,Z,CNT0,CNT1
PageBufferWriteToFLASH:
	eor Flag,Flag
	eor CNT0,CNT0
	ldi YH,BUF_H
	ldi YL,BUF_L
	mov ZH,ADDRH
	mov ZL,ADDRL			
Check_0:
	lpm temp,Z+			;read present byte
	ld CNT1,Y			;read data to be written
	cp temp,CNT1		;compare
	breq Check_1		;data is equal, get away
	cp CNT0,NumBytes	;CNT0-NumBytes
	brsh Check_2		;if CNT0>=NumBytes
	ldi Flag,0x01		;data is not equal and CNT0<NumBytes, set flag
	rjmp Check_1
Check_2:
	st Y,temp			;rewrite buffer data with present FLASH byte				
Check_1:
	adiw YH:YL,1		;inc Y
	inc CNT0
	cpi CNT0,PAGESIZEBYTES
	brne Check_0
	cpi Flag,0x01
	breq Check_Failed	 
	ldi temp,PAGE_EQUAL	;new and present pages are equal
	ret					;return
	
Check_Failed:			;now we've got to write page data
	eor ZH,ZH			;clear Z-buffer
	eor ZL,ZL			
	ldi YH,BUF_H		
	ldi YL,BUF_L
	ldi CNT0,PAGESIZEBYTES
Fill_FLASH_Buffer:
	ld R0,Y+			;read low byte from buffer
	ld R1,Y+			;read high byte from buffer
	ldi temp,0x01 		;write R1:R0 to temp buffer
	rcall Do_spm
	adiw ZH:ZL, 2		    ;inc Z by 2
	subi CNT0,2				;decrease counter by 2 
	brne Fill_FLASH_Buffer	;repeat until specified number of words is received
	mov ZL,ADDRL			;adress for page to store
	mov ZH,ADDRH
	rcall	PageErase					;page erase
	ldi Temp, (1<<PGWRT) | (1<<SPMEN)	;page write
	rcall	Do_spm
	rcall EnableRWW
	ldi temp,PAGE_WRITTEN	;data has been written
	ret	
;****************************************************

;********** FLASH page erase ************************
PageErase:
	mov ZL,ADDRL			;adress for page to erase
	mov ZH,ADDRH
	ldi Temp, (1<<PGERS) | (1<<SPMEN)  ; page erase
	rcall	Do_spm
	ret
;****************************************************

;********* reading and sending page *****************
;Purpose:	read page, specified by adress ADDRL:ADDRH in bytes into buffer
;			amount = PAGESIZE, send it over the USART
;Affects: temp,Z,Y,CNT0
ReadSendFlashPage:
	mov ZH,ADDRH
	mov ZL,ADDRL
;	ldi ZH,0x00
;	ldi ZL,0x00
	mov CNT0,NumBytes
Read_0:
	lpm temp,Z+
	rcall SendByte
	dec CNT0
	brne Read_0
	ret
;****************************************************

;********* writing received data to EEPROM ************
PageBufferWriteToEEPROM:
	out EEARH,ADDRH
	out EEARL,ADDRL
	ldi YH,BUF_H		
	ldi YL,BUF_L
	mov CNT0,NumBytes
	eor CNT1,CNT1
	eor r2,r2
	inc r2
	eor r3,r3
EE_write:
	ld temp,Y+					; data to be written
	sbi EECR,EERE
	in temp2,EEDR 				; Read data from data register
	cp temp,temp2
	breq EE_equal
	out EEDR,temp   			; Write data to data register
	sbi EECR,EEMWE  			; Write logical one to EEMWE
 	sbi EECR,EEWE   			; Start eeprom write by setting EEWE
	sbic EECR,EEWE  			; waiting
	rjmp PC-1
	inc CNT1
EE_equal:
	dec CNT0
	brne EE_next
	mov temp,CNT1
	ret
EE_next:
	rcall Inc_EEPROM_addr
	rjmp EE_write
;******************************************************

;********* reading and sending EEPROM data ************
ReadSendEEPROMData:
	out EEARH,ADDRH
	out EEARL,ADDRL
	mov CNT0,NumBytes
EE_read:
	sbi EECR,EERE
	in temp,EEDR
	rcall SendByte
	rcall Inc_EEPROM_addr
	dec CNT0
	brne EE_read
	ret	
;******************************************************

;******** reading FUSES and LOCK bits *****************
ReadFusesLockBits:
	eor ZH,ZH
	;reading LOCK bits
	ldi ZL,0x01
	rcall GetFuseLock
	;reading FUSE bits low
	ldi ZL,0x00
	rcall GetFuseLock
	;reading FUSE bits high
	ldi ZL,0x03
	rcall GetFuseLock
	ret

GetFuseLock:
	ldi Temp, (1<<BLBSET) | (1<<SPMEN)  ; read lock bits
	out SPMCR,temp
	lpm temp,Z
	rcall SendByte
	ret	
;******************************************************

;******* Increment EEARH:EEARL registers **************
Inc_EEPROM_addr:
	in XL,EEARL
	in XH,EEARH
	adiw XH:XL,0x01
	out EEARL,XL
	out EEARH,XH
	ret
;******************************************************


;******* executing spm instruction ********************
; input: temp determines SPM action
Do_spm:
	out SPMCR, temp
	spm
	; wait for SPM complete
Wait_spm:
	in temp, SPMCR
	sbrc temp, SPMEN
	rjmp Wait_spm
	ret	
;*******************************************************


;****************** enable RWW section ******************
; verify that RWW section is safe to read
EnableRWW:
	in temp, SPMCR
	sbrs temp, RWWSB ; If RWWSB is set, the RWW section is not ready yet
	ret
	; re-enable the RWW section
	ldi temp, (1<<RWWSRE) | (1<<SPMEN)
	rcall Do_spm
	rjmp EnableRWW
;********************************************************


//************ getting a byte from USART ****************//
GetByte:
	sbis UCSRA, RXC ; Wait for data to be received
	rjmp PC-1
	in Temp, UDR ; Get and return received data from buffer
	ret
//*******************************************************//


//************** sending a byte by USART ****************//
SendByte:
	sbis UCSRA,UDRE
	rjmp PC-1
	out UDR,temp
	ret
//*******************************************************//
