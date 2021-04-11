/**********************************************************************
 * Copyright (c) 2018
 *	Sang-Hoon Kim <sanghoonkim@ajou.ac.kr>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTIABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 **********************************************************************/
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include "config.h"


/**
 * Skeleton data structures to implement the buddy system allocator
 */

/**
 * Data structure to represent an order-@order pages. To the rest of this file,
 * consecutive pages will be represented in @start:@order notation.
 * E.g., 16:3 is 8(2^3)  consecutive pages (or say order-3 page) starting from
 * page frame 16.
 */
struct chunk {
	/**
	 * TODO: Modify this structure as you need.
	 */
	unsigned int start;
	unsigned int order;
	struct chunk *chunkPointer;
};


/**
 * Data structure to maintain order-@order free chunks.
 * NOTE that chunk_list SHOULD WORK LIKE THE QUEUE; the firstly added chunk
 * should be used first, otherwise the grading system will fail.
 */
struct chunk_list {
	/**
	 * TODO: Modify this structure as you need
	 */
	struct chunk *first;
	struct chunk *last; // use for FIFO;

	unsigned int order;
};
//not allocate memory, only insert chunk to the list
void insertChunkList(struct chunk* newChunk, struct chunk_list* l)
{

	if(l->first == NULL && l->last == NULL) //if there is no node exsists
	{
		l->first = newChunk;
		l->last = newChunk;
	}
	else
	{
		l->last->chunkPointer = newChunk;
		l->last = newChunk;
	}
}
//not free memory, only detach chunk from the list
struct chunk* removeChunkList(struct chunk_list* l)
{
	struct chunk *rChunk;
	rChunk = l->first;
	if(l->first == NULL && l->last == NULL) return NULL;
	if(l->first == l->last) //if the node is last.
	{
		l->first = NULL;
		l->last = NULL;

		return rChunk;
	}
	else
	{
		l->first = l->first->chunkPointer;
		return rChunk;
	}
}

/**
 * Data structure to realize the buddy system allocator
 */
struct buddy {
	/**
	 * TODO: Modify this example data structure as you need
	 */

	/**
	 * Free chunk list in the buddy system allocator.
	 *
	 * @NR_ORDERS is @MAX_ORDER + 1 (considering order-0 pages) and deifned in
	 * config.h. @MAX_ORDER is set in the Makefile. MAKE SURE your buddy
	 * implementation can handle order-0 to order-@MAX_ORDER pages.
	 */
	struct chunk_list chunks[NR_ORDERS];

	unsigned int allocated;	/* Number of pages that are allocated */
	unsigned int free;		/* Number of pages that are free */
};


/**
 * This is your buddy system allocator instance!
 */
static struct buddy buddy;


/**
 *    Your buddy system allocator should manage from order-0 to
 *  order-@MAX_ORDER. In the following example, assume your buddy system
 *  manages page 0 to 0x1F (0 -- 31, thus @nr_pages is 32) and pages from
 *  20 to 23 and 28 (0x14 - 0x17, 0x1C) are allocated by alloc_pages()
 *  by some orders.
 *  At this moment, the buddy system will split the address space into;
 *
 *      0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F
 * 0x00 <-------------------4-------------------------->
 * 0x10 <----2----->X  X  X  X  <-----2---->X  <0><-1-->
 *                  ^  ^  ^  ^              ^
 *                  |--|--|--|--------------|
 *                     allocated
 *
 *   Thus, the buddy system will maintain the free chunk lists like;
 *
 * Order     | Start addresses of free chunks
 * ----------------------------------------------
 * MAX_ORDER |
 *    ...    |
 *     4     | 0x00
 *     3     |
 *     2     | 0x10, 0x18
 *     1     | 0x1e
 *     0     | 0x1d
 */


/**
 * Allocate 2^@order contiguous pages.
 *
 * Description:
 *    For example, when @order=0, allocate a single page, @order=2 means
 *  to allocate 4 consecutive pages, and so forth.
 *    From the example state above, alloc_pages(2) gives 0x10 through @*page
 *  and the corresponding entry is removed from the free chunk. NOTE THAT the
 *  free chunk lists should be maintained as 'FIFO' so alloc_pages(2) returns 
 *  0x10, not 0x18. 
 *    To hanle alloc_pages(3), the order-4 chunk (0x00 -- 0x0f) should
 *  be broken into smaller chunks (say 0x00 -- 0x07, 0x08 -- 0x0f), and
 *  the LEFT BUDDY will be returned through @page whereas RIGHT BUDDY
 *  will be put into the order-3 free chunk list.
 *
 * Return:
 *   0      : On successful allocation. @*page will contain the starting
 *            page number of the allocated chunk
 *  -EINVAL : When @order < 0 or @order > MAX_ORDER
 *  -ENOMEM : When order-@order contiguous chunk is not available in the system
 */
int alloc_pages(unsigned int *page, const unsigned int order)
{
	unsigned int x;
	struct chunk* cp;
	unsigned int highOrder;
	struct chunk *lcp, *rcp;
	int flag;

	if(order < 0 || order > MAX_ORDER) return -EINVAL;
	else
	{
		if(buddy.chunks[order].first == NULL && buddy.chunks[order].last == NULL)
		{
			if(order == MAX_ORDER) return ENOMEM;
			flag = 0;
			for(int i = order+1; i <= MAX_ORDER; i++)
			{
				if(buddy.chunks[i].first != NULL)
				{
					highOrder = i;
					flag = 1;
					break;
				}
			} //find first splitied order

			if(flag == 0) return EINVAL; //No return chunk exists...

			for(int i = highOrder; i > order; i--)
			{
				if(i == highOrder) cp = removeChunkList(&(buddy.chunks[i]));
				else cp = lcp;

				lcp = (struct chunk*)malloc(sizeof(struct chunk));
				rcp = (struct chunk*)malloc(sizeof(struct chunk));

				lcp->chunkPointer = NULL; rcp->chunkPointer = NULL;
				lcp->order = cp->order-1; rcp->order = cp->order-1;
				lcp->start = cp->start;
				rcp->start = cp->start + (1 << (cp->order-1));
				PRINTF("SPLIT 0x%x:%u -> 0x%x:%u + 0x%x:%u\n", cp->start, cp->order, lcp->start, lcp->order, rcp->start, rcp->order);
				free(cp);
				

				insertChunkList(rcp, &(buddy.chunks[i-1]));
				PRINTF("PUT   0x%x:%u\n", rcp->start, rcp->order);
			}

			*page = lcp->start;
			PRINTF("ALLOC 0x%x:%x\n", *page, lcp->order);
			free(lcp);
			
			buddy.allocated += (1 << order);
			buddy.free -= (1 << order);
			return 0;

		}
		else
		{
			cp = removeChunkList(&(buddy.chunks[order])); 
			*page = cp->start;
			x = cp->order;
			free(cp);
			PRINTF("ALLOC 0x%x:%x\n", *page, x);
			buddy.allocated += (1 << order);
			buddy.free -= (1 << order);
			return 0;
		}
	}

	/**
	 * Your implementation will look (but not limited to) like;
	 *
	 * Check whether a chunk is available from chunk_list of @order
	 * if (exist) {
	 *    allocate the chunk from the list; Done!
	 * } else {
	 *    Make an order-@order chunk by breaking a higher-order chunk(s)
	 *    - Find the smallest free chunk that can satisfy the request
	 *    - Break the LEFT chunk until it is small enough
	 *    - Put remainders into the free chunk list
	 *
	 *    Return the allocated chunk via @*page
	 * }
	 *
	 *----------------------------------------------------------------------
	 * Print out below message using PRINTF upon each events. Note it is
	 * possible for multiple events to be happened to handle a single
	 * alloc_pages(). Also, MAKE SURE TO USE 'PRINTF', _NOT_ printf, otherwise
	 * the grading procedure will fail.
	 *
	 * - Split an order-@x chunk starting from @page into @left and @right:
	 *   PRINTF("SPLIT 0x%x:%u -> 0x%x:%u + 0x%x:%u\n",
	 *			page, x, left, x-1, right, x-1);
	 *
	 *
	 * - Put an order-@x chunk starting from @page into the free list:
	 *   PRINTF("PUT   0x%x:%u\n", page, x);
	 *
	 * - Allocate an order-@x chunk starting from @page for serving the request:
	 *   PRINTF("ALLOC 0x%x:%x\n", page, x);
	 *
	 * Example: A order-4 chunk starting from 0 is split into 0:3 and 8:3,
	 * and 0:3 is split again to 0:2 and 4:2 to serve an order-2 allocation.
	 * And then 0:2 is allocated:
	 *
	 * SPLIT 0x0:4 -> 0x0:3 + 0x8:3
	 * PUT   0x8:3
	 * SPLIT 0x0:3 -> 0x0:2 + 0x4:2
	 * PUT   0x4:2
	 * ALLOC 0x0:2
	 *
	 *       OR
	 *
	 * SPLIT 0x0:4 -> 0x0:3 + 0x8:3
	 * SPLIT 0x0:3 -> 0x0:2 + 0x4:2
	 * PUT   0x8:3
	 * PUT   0x4:2
	 * ALLOC 0x0:2
	 *
	 *       OR
	 *
	 * SPLIT 0x0:4 -> 0x0:3 + 0x8:3
	 * SPLIT 0x0:3 -> 0x0:2 + 0x4:2
	 * PUT   0x4:2
	 * PUT   0x8:3
	 * ALLOC 0x0:2
	 *----------------------------------------------------------------------
	 */

	buddy.allocated += (1 << order);
	buddy.free -= (1 << order);
	return -ENOMEM;
}


/**
 * Free @page which are contiguous for 2^@order pages
 *
 * Description:
 *    Assume @page was allocated by alloc_pages(@order) above. 
 *  WARNING: When handling free chunks, put them into the free chunk list
 *  carefully so that free chunk lists work in FIFO.
 */
void free_pages(unsigned int page, const unsigned int order)
{
	struct chunk* cp = (struct chunk*)malloc(sizeof(struct chunk));
	cp->chunkPointer = NULL; cp->order = order; cp->start = page;
	struct chunk* myBuddy; 
	unsigned int myBuddyStart; 
	int currentOrder;
	struct chunk* frontChunk;


	if(buddy.chunks[order].first == NULL && buddy.chunks[order].last == NULL)
	{
		insertChunkList(cp, &(buddy.chunks[order]));
		PRINTF("PUT  : 0x%x:%u\n", cp->start, cp->order);
	}
	else
	{
		
		currentOrder = order;
		while(1)
		{
			if((cp->start % (1 << (cp->order+1))) == 0) myBuddyStart = cp->start + (1 << cp->order); //left
			else myBuddyStart = cp->start - ( 1<< cp->order); //right
			myBuddy = buddy.chunks[currentOrder].first;
			frontChunk = NULL;
			
			while(1)
			{
				if(myBuddy == NULL) break;
				if(myBuddy->start == myBuddyStart) break;
				frontChunk = myBuddy;
				myBuddy = myBuddy->chunkPointer;
			}

			
			if(myBuddy == NULL || currentOrder >= MAX_ORDER) // cannot find buddy.
			{ 
				
				insertChunkList(cp, &(buddy.chunks[currentOrder]));
				PRINTF("PUT  : 0x%x:%u\n", cp->start, cp->order);
				return;
				
			}
			else //merge
			{
				
				if(cp->start < myBuddyStart) //left side
				{

					cp->order += 1;
					PRINTF("MERGE : 0x%x:%u + 0x%x:%u -> 0x%x:%u\n", cp->start, currentOrder, myBuddy->start, currentOrder, cp->start, currentOrder+1);
				}
				else//right side
				{
					PRINTF("MERGE : 0x%x:%u + 0x%x:%u -> 0x%x:%u\n", myBuddy->start, currentOrder, cp->start, currentOrder, myBuddy->start, currentOrder+1);
					cp->order +=1;
					cp->start = myBuddy->start;
				}
				
				if(frontChunk != NULL) frontChunk->chunkPointer = myBuddy->chunkPointer;
				
				
				if(buddy.chunks[currentOrder].first == myBuddy)
				{
					buddy.chunks[currentOrder].first = myBuddy->chunkPointer;
				}
				if(buddy.chunks[currentOrder].last == myBuddy)
				{
					buddy.chunks[currentOrder].last = frontChunk;
				}
				
				free(myBuddy);
				currentOrder += 1;
				
			}
			
		}
	}
	/**
	 * Your implementation will look (but not limited to) like;
	 *
	 * Find the buddy chunk from this @order.
	 * if (buddy does not exist in this order-@order free list) {
	 *    put into the TAIL of this chunk list. Problem solved!!!
	 * } else {
	 *    Merge with the buddy
	 *    Promote the merged chunk into the higher-order chunk list
	 *
	 *    Consider the cascading case as well; in the higher-order list, there
	 *    might exist its buddy again, and again, again, ....
	 * }
	 *
	 *----------------------------------------------------------------------
	 * Similar to alloc_pages() above, print following messages using PRINTF
	 * when the event happens;
	 *
	 * - Merge order-$x buddies starting from $left and $right:
	 *   PRINTF("MERGE : 0x%x:%u + 0x%x:%u -> 0x%x:%u\n",
	 *			left, x, right, x, left, x+1);
	 *
	 * - Put an order-@x chunk starting from @page into the free list:
	 *   PRINTF("PUT  : 0x%x:%u\n", page, x);
	 *
	 * Example: Two buddies 0:2 and 4:2 (0:2 indicates an order-2 chunk
	 * starting from 0) are merged to 0:3, and it is merged again with 8:3,
	 * producing 0:4. And then finally the chunk is put into the order-4 free
	 * chunk list:
	 *
	 * MERGE : 0x0:2 + 0x4:2 -> 0x0:3
	 * MERGE : 0x0:3 + 0x8:3 -> 0x0:4
	 * PUT   : 0x0:4
	 *----------------------------------------------------------------------
	 */
	buddy.allocated -= (1 << order);
	buddy.free += (1 << order);
}


/**
 * Print out the order-@order free chunk list
 *
 *  In the example above, print_free_pages(0) will print out:
 *  0x1d:0
 *
 *  print_free_pages(2):
 *    0x10:2
 *    0x18:2
 */
void print_free_pages(const unsigned int order)
{
	unsigned int starting_page = 0x43; /* I love 43 because it's perfect!! */
	struct chunk *cp;
	cp = buddy.chunks[order].first;
	while(1)
	{
		if(cp == NULL)
		{
			break;
		} //last node or no node exists

		starting_page = cp->start;
		fprintf(stderr, "    0x%x:%u\n", starting_page, order);
		cp = cp->chunkPointer; //go next node
	}
	/**
	 * Your implementation should print out each free chunk from the beginning
	 * in the following format.
	 * WARNING: USE fprintf(stderr) NOT printf, otherwise the grading
	 * system will evaluate your implementation wrong.
	 */
	
}


/**
 * Return the unusable index(UI) of order-@order.
 *
 * Description:
 *    Return the unusable index of @order. In the above example, we have 27 free
 *  pages;
 *  # of free pages =
 *    sum(i = 0 to @MAX_ORDER){ (1 << i) * # of order-i free chunks }
 *
 *    and
 *
 *  UI(0) = 0 / 27 = 0.0 (UI of 0 is always 0 in fact).
 *  UI(1) = 1 (for 0x1d) / 27 = 0.037
 *  UI(2) = (1 (0x1d) + 2 (0x1e-0x1f)) / 27 = 0.111
 *  UI(3) = (1 (0x1d) + 2 (0x1e-0x1f) + 4 (0x10-0x13) + 4 (0x18-0x1b)) / 27
 *        = 0.407
 *  ...
 */
double get_unusable_index(unsigned int order)
{
	int allocatable = 0, inallocatable = 0; int chunkNum;
	struct chunk* cp; double returnVal = 0.0;

	for(int i =0; i< NR_ORDERS; i++)
	{
		chunkNum = 0;
		cp = buddy.chunks[i].first;

		while(1)
		{
			if(cp != NULL) chunkNum++;
			else break;

			cp = cp->chunkPointer;
		} // counting number of node

		if(i < order)
		{
			inallocatable += chunkNum*(1<<i);
		}//add inallocatable node
		else
		{
			allocatable += chunkNum*(1<<i);
		}//add allocatable node
	}
	
	returnVal = (double)inallocatable/(double)(allocatable + inallocatable);
	return returnVal;
}


/**
 * Initialize your buddy system.
 *
 * @nr_pages_in_order: number of pages in order-n notation to manage.
 * For instance, if @nr_pages_in_order = 13, the system should be able to
 * manage 8192 pages. You can set @nr_pages_in_order by using -n option while
 * launching the program;
 * ./pa4 -n 13       <-- will initiate the system with 2^13 pages.
 *
 * Return:
 *   0      : On successful initialization
 *  -EINVAL : Invalid arguments or when something goes wrong
 */
int init_buddy(unsigned int nr_pages_in_order)
{
	int i;
	
	buddy.allocated = 0;
	buddy.free = 1 << nr_pages_in_order;
	
	/* TODO: Do your initialization as you need */

	for (i = 0; i < NR_ORDERS; i++) {
		buddy.chunks[i].order = i;
	}

	unsigned int arrow = 0;
	struct chunk *c;
	while(1)
	{
		if(arrow == (1 << nr_pages_in_order)) 
		{
			break;
		}

		c = (struct chunk*)malloc(sizeof(struct chunk));
		c->chunkPointer = NULL;
		c->order = MAX_ORDER;
		c->start = arrow;
		insertChunkList(c, &(buddy.chunks[MAX_ORDER]));
		if(buddy.chunks[MAX_ORDER].first == NULL) buddy.chunks[MAX_ORDER].first = c;

		buddy.chunks[MAX_ORDER].last = c;
		arrow += 1 << MAX_ORDER;
	}
	/**
	 * TODO: Don't forget to initiate the free chunk list with
	 * order-@MAX_ORDER chunks. Note you might add multiple chunks if
	 * @nr_pages_in_order > @MAX_ORDER. For instance, when
	 * @nr_pages_in_order = 10 and @MAX_ORDER = 9, the initial free chunk
	 * lists will have two chunks; 0x0:9, 0x200:9.
	 */

	return 0;
}


/**
 * Return resources that your buddy system has been allocated. No other
 * function will be called after calling this function.
 */
void fini_buddy(void)
{
	/**
	 * TODO: Do your finalization if needed, and don't forget to release
	 * the initial chunks that you put in init_buddy().
	 */
	struct chunk* c = NULL;

	for(int i =0; i<NR_ORDERS; i++)
	{
		while(1)
		{
			c = removeChunkList(&(buddy.chunks[i]));
			if(c == NULL) break;
			else
			{
				//printf("free test : free chunk %d\n", c->start);
				free(c);
			}
		}
	}
}
