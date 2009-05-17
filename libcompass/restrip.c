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
	      strip *strip_ends[], strip **stripbuf, unsigned int *capacity)
{
  strip *strip_ptr;
  unsigned int entries = 0;
  
  for (strip_ptr = strips_in; strip_ptr; strip_ptr = strip_ptr->next)
    {
      unsigned int i;
      int current_class = -1;
      unsigned int strip_length = 1;
      float (*strip_start)[][3] = strip_ptr->start;
      unsigned int start_pos = 0;
      
      for (i = 0; i < strip_ptr->length - 2; i++)
        {
	  void *extra
	    = &((char *)strip_ptr->extra)[strip_ptr->extra_elsize * i];
	  int class = fn (&(*strip_ptr->start)[i], (i & 1) == 1, extra);
	  
	  if (class == current_class)
	    strip_length++;
	  else
	    {
	      if (current_class >= 0)
	        {
		  strip *class_end = strip_ends[current_class], *newptr;
		  
		  if (entries == *capacity)
		    {
		      (*capacity) *= 2;
		      *stripbuf = realloc (*stripbuf,
					   sizeof (strip) * (*capacity));
		    }
		  
		  newptr = &(*stripbuf)[entries];
		  
		  class_end->next = newptr;
		  newptr->next = 0;
		  
		  newptr->start = strip_start;
		  newptr->length = strip_length + 2;
		  if ((start_pos & 1) == 1)
		    newptr->length = -newptr->length;
		  newptr->extra = &((char *) strip_ptr->extra)
				    [strip_ptr->extra_elsize * start_pos];
		  newptr->extra_elsize = strip_ptr->extra_elsize;
		  
		  strip_ends[current_class] = newptr;
		  if (strip_starts[current_class] == NULL)
		    strip_starts[current_class] = newptr;
		  
		  entries++;
		}
	      
	      strip_length = 1;
	      strip_start = (float (*)[][3]) &(*strip_ptr->start)[i];
	      start_pos = i;
	      current_class = class;
	    }
	}

      if (strip_length != 0 && current_class >= 0)
        {
	  strip *class_end = strip_ends[current_class], *newptr;
	  
	  if (entries == *capacity)
	    {
	      (*capacity) *= 2;
	      *stripbuf = realloc (*stripbuf, sizeof (strip) * (*capacity));
	    }
	  
	  newptr = &(*stripbuf)[entries];
	  
	  class_end->next = newptr;
	  newptr->next = 0;
	  
	  newptr->start = strip_start;
	  newptr->length = strip_length + 2;
	  if ((start_pos & 1) == 1)
	    newptr->length = -newptr->length;
	  newptr->extra = &((char *) strip_ptr->extra)
			    [strip_ptr->extra_elsize * start_pos];
	  newptr->extra_elsize = strip_ptr->extra_elsize;
	  
	  strip_ends[current_class] = newptr;
	  if (strip_starts[current_class] == NULL)
	    strip_starts[current_class] = newptr;
	  
	  entries++;
	}
    }  
}
