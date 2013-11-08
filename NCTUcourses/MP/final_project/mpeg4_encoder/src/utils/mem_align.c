#include "mem_align.h"
#include "../xvid.h"

/*****************************************************************************
 * xvid_malloc
 *
 * This function allocates 'size' bytes (usable by the user) on the heap and
 * takes care of the requested 'alignment'.
 * In order to align the allocated memory block, the xvid_malloc allocates
 * 'size' bytes + 'alignment' bytes. So try to keep alignment very small
 * when allocating small pieces of memory.
 *
 * NB : a block allocated by xvid_malloc _must_ be freed with xvid_free
 *      (the libc free will return an error)
 *
 * Returned value : - NULL on error
 *                  - Pointer to the allocated aligned block
 *
 ****************************************************************************/

void *
xvid_malloc(size_t size,
			uint8_t alignment)
{
	uint8_t *mem_ptr;

	if (!alignment) {

		/* We have not to satisfy any alignment */
		if ((mem_ptr = (uint8_t *) my_malloc(size + 1)) != NULL) {

			/* Store (mem_ptr - "real allocated memory") in *(mem_ptr-1) */
			*mem_ptr = 1;

			/* Return the mem_ptr pointer */
			return (void *)(mem_ptr+1);

		}

	} else {
		uint8_t *tmp;

		/*
		 * Allocate the required size memory + alignment so we
		 * can realign the data if necessary
		 */

		if ((tmp = (uint8_t *) my_malloc(size + alignment)) != NULL) {

			/* Align the tmp pointer */
			mem_ptr =
				(uint8_t *) ((ptr_t) (tmp + alignment - 1) &
							 (~(ptr_t) (alignment - 1)));

			/*
			 * Special case where malloc have already satisfied the alignment
			 * We must add alignment to mem_ptr because we must store
			 * (mem_ptr - tmp) in *(mem_ptr-1)
			 * If we do not add alignment to mem_ptr then *(mem_ptr-1) points
			 * to a forbidden memory space
			 */
			if (mem_ptr == tmp)
				mem_ptr += alignment;

			/*
			 * (mem_ptr - tmp) is stored in *(mem_ptr-1) so we are able to retrieve
			 * the real malloc block allocated and free it in xvid_free
			 */
			*(mem_ptr - 1) = (uint8_t) (mem_ptr - tmp);

			/* Return the aligned pointer */
			return (void *) mem_ptr;

		}
	}

	return NULL;

}

/*****************************************************************************
 * xvid_free
 *
 * Free a previously 'xvid_malloc' allocated block. Does not free NULL
 * references.
 *
 * Returned value : None.
 *
 ****************************************************************************/

void
xvid_free(void *mem_ptr)
{

	/* *(mem_ptr - 1) give us the offset to the real malloc block */
	if (mem_ptr)
		my_free((uint8_t *) mem_ptr - *((uint8_t *) mem_ptr - 1));

}
