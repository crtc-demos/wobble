#include <kos.h>
#include <limits.h>

#include "restrip.h"

/***
  1___3___5
  /\  /\  /
 /__\/__\/
 0   2   4
***/

void
restrip_list (strip *strips_in, strip_classify_fn fn, strip *strip_starts[],
	      strip *strip_ends[], restrip_allocator_fn allocfn)
{
  strip *strip_ptr;
  
  for (strip_ptr = strips_in; strip_ptr; strip_ptr = strip_ptr->next)
    {
      unsigned int i;
      int current_class = -1;
      unsigned int strip_length = 0;
      float (*strip_start)[][3] = strip_ptr->start;
      unsigned int start_pos = 0;
      
      for (i = 0; i < strip_ptr->length - 2; i++)
        {
	  vertex_attrs *attrs = &strip_ptr->v_attrs[i];
	  int class = fn (&(*strip_ptr->start)[i], (i & 1) == 1, attrs);
	  
	  if (class == current_class)
	    strip_length++;
	  else
	    {
	      if (current_class >= 0)
	        {
		  strip *newptr = allocfn (sizeof (strip));
		  
		  if (strip_ends[current_class] != NULL)
		    strip_ends[current_class]->next = newptr;

		  newptr->next = 0;
		  
		  newptr->start = strip_start;
		  newptr->length = strip_length + 2;

		  if ((start_pos & 1) == 1)
		    newptr->inverse = !strip_ptr->inverse;
		  else
		    newptr->inverse = strip_ptr->inverse;

		  newptr->v_attrs = &strip_ptr->v_attrs[start_pos];
		  newptr->s_attrs = strip_ptr->s_attrs;
		  newptr->normals = (strip_ptr->normals)
		    ? (float (*)[][3]) &(*strip_ptr->normals)[start_pos]
		    : NULL;
		  newptr->texcoords = (strip_ptr->texcoords)
		    ? (float (*)[][2]) &(*strip_ptr->texcoords)[start_pos]
		    : NULL;
		  
		  strip_ends[current_class] = newptr;
		  if (strip_starts[current_class] == NULL)
		    strip_starts[current_class] = newptr;
		}
	      
	      strip_length = 1;
	      strip_start = (float (*)[][3]) &(*strip_ptr->start)[i];
	      start_pos = i;
	      current_class = class;
	    }
	}

      if (strip_length != 0 && current_class >= 0)
        {
	  strip *newptr = allocfn (sizeof (strip));
	  
	  if (strip_ends[current_class] != NULL)
	    strip_ends[current_class]->next = newptr;

	  newptr->next = 0;
	  
	  newptr->start = strip_start;
	  newptr->length = strip_length + 2;

	  if ((start_pos & 1) == 1)
	    newptr->inverse = !strip_ptr->inverse;
	  else
	    newptr->inverse = strip_ptr->inverse;

	  newptr->v_attrs = &strip_ptr->v_attrs[start_pos];
	  newptr->s_attrs = strip_ptr->s_attrs;
	  newptr->normals = (strip_ptr->normals)
	    ? (float (*)[][3]) &(*strip_ptr->normals)[start_pos] : NULL;
	  newptr->texcoords = (strip_ptr->texcoords)
	    ? (float (*)[][2]) &(*strip_ptr->texcoords)[start_pos] : NULL;
	  
	  strip_ends[current_class] = newptr;
	  if (strip_starts[current_class] == NULL)
	    strip_starts[current_class] = newptr;
	}
    }  
}
