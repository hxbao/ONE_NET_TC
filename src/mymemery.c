/*
 * mymemery.c
 *
 *  Created on: 2017年12月1日
 *      Author: hxbao
 */
#include "mymemery.h"
#include "stm32f10x.h"
#include "stdio.h"

#define MEM_BLOCK_SIZE 512
#define MAX_MEM_SIZE 6*1024
#define LARGER_MEM_SIZE 2*1024
#define MEM_ALLOC_TABLE_SIZE    (MAX_MEM_SIZE/MEM_BLOCK_SIZE)

//申请的内存池
static uint8_t MEM_BASE_ADDRESS[MAX_MEM_SIZE];
//内存使用映射表
static uint8_t memory_map[MEM_ALLOC_TABLE_SIZE];
//是否已经初始化
static uint8_t isMemoryManagementReady = 0;

void *memset(void *s, uint8_t c, size_t count)
{
	uint8_t *xs = s;
	while (count--)
		*xs++ = c;
	return s;
}

void my_mem_init(void)
{
	memset(memory_map, 0, sizeof(memory_map));
	isMemoryManagementReady = 1;
}

int16_t my_mem_percentage_used()
{
	int used = 0;
	int i;
	for (i = 0; i < MEM_ALLOC_TABLE_SIZE; i++)
	{
		if (memory_map[i])
		{
			used++;
		}
	}
	return used * 100 / MEM_ALLOC_TABLE_SIZE;
}
//return 0:FAIL
//>=0:   return allocated address
uint8_t* my_malloc(uint32_t size)
{
	uint8_t nmemb; //需要分配的block块数
	int i,j;

	if (!isMemoryManagementReady)
	{
		my_mem_init();
	}

	if (size == 0)
	{
		return 0;
	}

	nmemb = size / MEM_BLOCK_SIZE;
	if (size % MEM_BLOCK_SIZE)
	{
		nmemb++;
	}

	//从映射表的开始往后搜寻
	for (i = 0; i < MEM_ALLOC_TABLE_SIZE - nmemb; i++)
	{
		if (!memory_map[i])
		{
			//找到一块可以分配的地址
			int vacantSize = 0;
			for (vacantSize = 0;
					vacantSize < nmemb && !memory_map[i + vacantSize];
					vacantSize++)
				;
			if (vacantSize == nmemb)
			{
				for (j = 0; j < nmemb; j++)
				{
					memory_map[i + j] = nmemb;
				}
				return (MEM_BASE_ADDRESS + i * MEM_BLOCK_SIZE);
			} //如果沒有一快可以完整可以用大内存存下，跳過
		}
	}

//	//如果是大内存分配，从后面分配
//	if (nmemb >= LARGER_MEM_SIZE)
//	{
//
//	}
//	else
//	{
//
//
//	}

	return NULL;
}
//return 0:OK
//return 1:Out of bound
uint8_t my_free(uint8_t* ptr)
{
	int i;
	uint32_t offset;

	if (!isMemoryManagementReady)
	{
		my_mem_init();
		return 1;
	}
	offset = ptr - MEM_BASE_ADDRESS;
	if (offset < MAX_MEM_SIZE)
	{
		int index = offset / MEM_BLOCK_SIZE;
		int nmemb = memory_map[index];
		for (i = 0; i < nmemb; i++)
		{
			memory_map[index + i] = 0;
		}
		return 0;
	}
	else
	{
		return 1; //out of bound
	}
}
