#include "object.h"

#undef DEBUG

typedef enum {
  NOTHING,
  STRIP,
  VERTS,
  NORMS,
  TEXCOORDS
} state;

object *
load_object (const char *filename)
{
  FILE *f = fopen (filename, "r");
  char buf[256];
  strip *strips = NULL;
  state s = NOTHING;
  unsigned int verts = 0, norms = 0, texcoords = 0;
  
  while (!feof (f))
    {
      int slen;

      fgets (buf, sizeof (buf), f);
      
      switch (s)
        {
	case NOTHING:
	  if (sscanf (buf, "strip %d", &slen) > 0)
	    {
#ifdef DEBUG
	      printf ("consing strip of length: %d\n", slen);
#endif
	      strips = strip_cons (strips, slen, 0);
	      //strips->inverse = 1;
	      verts = norms = texcoords = 0;
	      s = STRIP;
	    }
	  break;
	
	case STRIP:
	  if (strncmp (buf, "verts", 5) == 0)
	    {
	      s = VERTS;
	      strips->start = malloc (slen * 3 * sizeof (float));
	    }
	  else if (strncmp (buf, "norms", 5) == 0)
	    {
	      s = NORMS;
	      strips->normals = malloc (slen * 3 * sizeof (float));
	    }
	  else if (strncmp (buf, "texcoords", 9) == 0)
	    {
	      s = TEXCOORDS;
	      strips->texcoords = malloc (slen * 2 * sizeof (float));
	    }
	  else if (strncmp (buf, "end", 3) == 0)
	    s = NOTHING;
	  break;
	
	case VERTS:
	  {
	    float x, y, z;

	    if (sscanf (buf, " %f %f %f", &x, &y, &z) > 0)
	      {
	        (*strips->start)[verts][0] = x;
		(*strips->start)[verts][1] = y;
		(*strips->start)[verts][2] = z;
		verts++;
		if (verts == slen)
		  {
#ifdef DEBUG
		    printf ("got %d vertices\n", slen);
#endif
		    s = STRIP;
		  }
	      }
	  }
	  break;
	
	case NORMS:
	  {
	    float x, y, z;
	    
	    if (sscanf (buf, " %f %f %f", &x, &y, &z) > 0)
	      {
	        (*strips->normals)[norms][0] = x;
		(*strips->normals)[norms][1] = y;
		(*strips->normals)[norms][2] = z;
		norms++;
		if (norms == slen)
		  {
#ifdef DEBUG
		    printf ("got %d normals\n", slen);
#endif
		    s = STRIP;
		  }
	      }
	  }
	  break;
	
	case TEXCOORDS:
	  {
	    float u, v;
	    
	    if (sscanf (buf, " %f %f", &u, &v) > 0)
	      {
	        (*strips->texcoords)[texcoords][0] = u;
		(*strips->texcoords)[texcoords][1] = v;
		texcoords++;
		if (texcoords == slen)
		  {
#ifdef DEBUG
		    printf ("got %d texcoords\n", texcoords);
#endif
		    s = STRIP;
		  }
	      }
	  }
	  break;
	}
    }
  
  fclose (f);
  
  return object_create_default (strips);
}
