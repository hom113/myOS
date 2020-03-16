;haribote-ipl
;TAB=4
 
 CYLS   EQU     10              ;定义变量CYLS（柱面）=10，用来读盘时确定读到哪个柱面
 
        ORG     0x7c00          ;意义在于汇编器根据这个计算程序块名的地址，这个地址也是启动区程序读取到内存的默认地址
      
      
      
		JMP		entry
		DB		0x90
		DB		"HARIBOTE"		; 启动区的名称可以是任意的字符串（8bytes）
		DW		512				; 每个扇区（sector）的大小（512bytes）
		DB		1				; 簇（cluster）的大小（1个扇区）
		DW		1				; FAT的起始位置（一般从第一个sector开始）
		DB		2				; FAT的个数（个数为2）
		DW		224				; 根目录的大小（224项）
		DW		2880			; 该盘的大小（一般为2880sectors）
		DB		0xf0			; 磁盘的种类（固定）
		DW		9				; FAT的长度（9个sectors）
		DW		18				; 1个磁道（track）有几个扇区
		DW		2				; 磁头数
		DD		0				; 不使用分区，必须是0
		DD		2880			; 重写一次磁盘大小
		DB		0,0,0x29		; 意义不明，固定
		DD		0xffffffff		; （可能是）卷标号码
		DB		"HARIBOTEOS "	; 磁盘的名称（11字节）
		DB		"FAT12   "		; 磁盘格式名称（8字节）
		RESB	18	            ; 空出18字节
        
;程序主体
    
entry:                          ;entry表达接下来的程序块首地址，在ORG确定后汇编器会算出具体地址值
        MOV     AX,0            ;初始化AX累加寄存器为0
        MOV     SS,AX           ;初始化SS栈段寄存器为0
        MOV     SP,0x7c00       ;初始化SP栈指针寄存器为0x7c00,即程序开始地址（bios会把启动区的512字节读到这里）
        MOV     DS,AX           ;初始化DS数据段寄存器为0

;从软盘读取数据到内存
     
        MOV     AX,0x0820
        MOV     ES,AX           ;bios会从内存ES:BX处放置读取数据
        MOV     CH,0            ;柱面0
        MOV     DH,0            ;磁头0
        MOV     CL,2            ;扇区数2，扇区数1用来放启动区程序，先前已被读取值0x7c00（系统）
readloop:
        MOV     SI,0            ;记录失败次数的寄存器
retry:         
        MOV     AH,0x02         ;表示读盘
        MOV     AL,1            ;处理对象的扇区数
        MOV     BX,0            ;结合前面的ES，ES:BX表示将数据读取到内存的内存地址
        MOV     DL,0x00         ;驱动器号，为0
        INT     0x13            ;终断，调用13号函数
        JNC     next            ;没出错转到readloop
        ADD     SI,1
        CMP     SI,5            ;与5比较
        JAE     error           ;SI>=5时，转到error
        MOV     AH,0x00
        MOV     DL,0x00
        INT     0x13            ;重置驱动器     
        JMP     retry           ;跳转，重新读盘
next:
        MOV     AX,ES
        ADD     AX,0x0020
        MOV     ES,AX
        ADD     CL,1
        CMP     CL,18
        JBE     readloop        
        MOV     CL,1
        ADD     DH,1
        CMP     DH,2
        JB      readloop
        MOV     DH,0
        ADD     CH,1
        CMP     CH,CYLS
        JB      readloop

		MOV		[0x0ff0],CH		; 把CYLS的值存到内存0xff0中
		JMP		0xc200          ;跳转到内存地址0xc200，开始执行操作系统命令
        
error:
        MOV     SI,msg
        
putloop:
        MOV     AL,[SI]         ;取内存中地址SI的值给AL
        ADD     SI,1            ;跳到下一个地址
        CMP     AL,0
        JE      fin
        MOV     AH,0x0e      
        MOV     BX,15
        INT     0x10
        JMP     putloop
msg:
        DB      0x0a,0x0a
        DB      "load error"
        DB      0x0a
        DB      0                
 
fin:
        HLT                     ;让cpu停止;等待指令
        JMP     fin             ;无限循环
        
        RESB    0x7dfe-$
        
        DB      0x55,0xaa
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        