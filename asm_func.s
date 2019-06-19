.syntax unified

.global	read_ctrl
read_ctrl:
	mrs	r0,	control
	bx	lr

.global	start_user
start_user:
	movs	lr,	r0
	msr	psp,	r1

	movs	r3,	#0b11
	msr	control,	r3
	isb

	bx	lr

.type systick_handler, %function
.global systick_handler
systick_handler:
	//save lr (EXC_RETURN) to main stack
	push {lr} //先把lr儲存起來 （exc return）

	//save r4-r11 to user stack
	mrs	r0,	psp // 把psp的指標處存在r0 ，push的概念是 先下降 再把該處的資料存起來 1把前一個task的狀況儲存起來
	stmdb	r0!,	{r4-r11} // store multiple decrease before  //透過r0的指標來把r4~r11的值存起來 ！ ＝>r0會被更新成最後一筆位址 若不加驚嘆號 則r0便不會改變 

	//pass psp of curr task by r0 and get psp of the next task
	bl	sw_task //告知下一個task在那
	//psp of the next task is now in r0


	//restore r4~r11 from stack of the next task
    ldmia r0!,{r4-r11} // load multiple decrease after 

	//modify psp
	msr psp,r0  //更新psp

	//restore lr (EXC_RETURN)
	pop {lr} //pop 回來lr

	bx	lr //根據exc_return 去找到psp 在透過psp去找到下一個task 
