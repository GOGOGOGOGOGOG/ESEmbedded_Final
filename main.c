#include <stdint.h>
#include <stdio.h>
#include "reg.h"
#include "blink.h"
#include "usart.h"
#include "asm_func.h"

#define TASK_NUM 4
#define PSTACK_SIZE_WORDS 1024 //user stack size = 4 kB

static uint32_t *psp_array[TASK_NUM];

void setup_systick(uint32_t ticks);

void init_task(unsigned int task_id, uint32_t *task_addr, uint32_t *psp_init)
{
	*(psp_init-1) = (uint32_t)0x1000000 ;	  //xPSR (bit 24第25個bits, T bit, has to be 1 in Thumb state) 所以是用thumb bits instruction
	*(psp_init-2) = (uint32_t)task_addr; //Return Address is being initialized to the task entry
	psp_array[task_id] = psp_init-16;	//initialize psp_array (stack frame: 8 + r4 ~ r11: 8)
}

void task0(void)
{
	while (1)
	{
		printf("[Task0] Start in unprivileged thread mode.\r\n\n");
	printf("[Task0] Control: 0x%x \r\n", (unsigned int)read_ctrl());

	blink_count(LED_BLUE,15); //should not return
	 for(int i=0;i<=10;i++)
    ;
	}
	
	
}

void task1(void)
{
	while(1){
	printf("[Task1] Start in unprivileged thread mode.\r\n\n");
	printf("[Task1] Control: 0x%x \r\n", (unsigned int)read_ctrl());

	blink_count(LED_GREEN,15); //should not return
	 for(int i=0;i<=10;i++)
    ;
	}
}

void task2(void)
{
	while(1){
	printf("[Task2] Start in unprivileged thread mode.\r\n\n");
	printf("[Task2] Control: 0x%x \r\n", (unsigned int)read_ctrl());

	blink_count(LED_ORANGE,15); //should not return
    for(int i=0;i<=10;i++)
    ;

	}
}

void task3(void)

{
int t1 = 0, t2 = 1, nextTerm = 0, n=UINT16_MAX;

while(nextTerm <= n)
    {
        //printf("%d, ",nextTerm);
		 
        t1 = t2;
        t2 = nextTerm;
        nextTerm = t1 + t2;
		printf("Fib: %d %d\n\r", t1, t2);
        
		for(int i=0;i<=100;i++)
    ;
            if(t1>n)
			{
             t1 = 0;
			 t2=1;
			 nextTerm = 0;
			 blink_count(LED_RED,15);

			 for(int j=0;j<=100;j++)
    ;
			continue; 
			}
	 
    }
    



}

int main(void)
{
	init_usart1();

	uint32_t user_stacks[TASK_NUM][PSTACK_SIZE_WORDS];

	//init user tasks
	init_task(0, (uint32_t*)task0, user_stacks[0]+1024);
	init_task(1, (uint32_t*)task1, user_stacks[1]+1024);
	init_task(2, (uint32_t*)task2, user_stacks[2]+1024);
	init_task(3,(uint32_t*)task3, user_stacks[3]+1024);

	printf("[Kernel] Start in privileged thread mode.\r\n\n");

	printf("[Kernel] Setting systick...\r\n\n");
	setup_systick(168e6 / 8 / 100); //10 ms

	//start user task
	printf("[Kernel] Switch to unprivileged thread mode & start user task0 with psp.\r\n\n");
	start_user((uint32_t *)task0, user_stacks[0]);

	while (1) //should not go here
		;
}

void setup_systick(uint32_t ticks)
{
	// set reload value
	WRITE_BITS(SYST_BASE + SYST_RVR_OFFSET, SYST_RELOAD_23_BIT, SYST_RELOAD_0_BIT, ticks - 1);

	// uses external reference clock
	CLEAR_BIT(SYST_BASE + SYST_CSR_OFFSET, SYST_CLKSOURCE_BIT);

	// enable systick exception
	SET_BIT(SYST_BASE + SYST_CSR_OFFSET, SYST_TICKINT_BIT);

	// enable systick
	SET_BIT(SYST_BASE + SYST_CSR_OFFSET, SYST_ENABLE_BIT);
}

uint32_t *sw_task(uint32_t *psp)
{
	static unsigned int curr_task_id = 0;

	psp_array[curr_task_id] = psp; //save current psp

	if (++curr_task_id > TASK_NUM - 1) //get next task id
		curr_task_id = 0;

	return psp_array[curr_task_id]; //return next psp
}





//task 0 把之前的資料存在r4~r11 stack systick handler 儲存上一個task的資料 再去call switchtask函式  

