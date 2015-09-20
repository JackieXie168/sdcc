/*-------------------------------------------------------------------------
   free.c - deallocate memory.

   Copyright (C) 2015, Philipp Klaus Krause, pkk@spth.de

   This library is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published by the
   Free Software Foundation; either version 2, or (at your option) any
   later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License 
   along with this library; see the file COPYING. If not, write to the
   Free Software Foundation, 51 Franklin Street, Fifth Floor, Boston,
   MA 02110-1301, USA.

   As a special exception, if you link this library with other files,
   some of which are compiled with SDCC, to produce an executable,
   this library does not by itself cause the resulting executable to
   be covered by the GNU General Public License. This exception does
   not however invalidate any other reasons why the executable file
   might be covered by the GNU General Public License.
-------------------------------------------------------------------------*/

#include <stdlib.h>
#include <stddef.h>

#if defined(__SDCC_mcs51) || defined(__SDCC_ds390) || defined(__SDCC_ds400)
#define XDATA __xdata
#else
#define XDATA
#endif

typedef struct header XDATA header_t;

struct header
{
	header_t *next;
	header_t *next_free;
};

extern header_t *XDATA __sdcc_heap_free;

void free(void *ptr)
{
	header_t *h, *next_free;
	header_t *XDATA *f, *XDATA *p;

	if(!ptr)
		return;

	for(h = __sdcc_heap_free, f = &__sdcc_heap_free; h < ptr; f = &(h->next_free), h = h->next_free);
	next_free = *f;
	for(h = *f, p = f; h < ptr; p = &(h->next), h = h->next);

	h = (void XDATA *)((char XDATA *)(ptr) - offsetof(struct header, next_free));

	// Insert into free list.
	h->next_free = next_free;
	*f = h;

	if(next_free == h->next) // Merge with next block
	{
		h->next_free = h->next->next_free;
		h->next = h->next->next;
	}

	if ((char XDATA *)f == (char XDATA *)p + offsetof(struct header, next_free) - offsetof(struct header, next)) // Merge with previous block
	{
		*p = h->next;
		*f = h->next_free;
	}
}

