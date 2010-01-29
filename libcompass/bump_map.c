#include "bumpmap.h"
#include "vector.h"
#include "object.h"

/* Calculate "orientation" vectors for bump mapping.  These are the vectors
   corresponding to "+u" in texture space, but in the space of the object (and
   normalised).  These vectors are per-strip, so care should be taken that
   bump-map textures don't "twist" too much over each strip (if accurate
   results are desired).  */

void
bumpmap_auto_uv_orient (object *obj)
{
  strip *str;
  
  for (str = obj->striplist; str; str = str->next)
    {
      float u[3], a_minus_b[3], a_minus_c[3];
      const int a = 0, b = (str->inverse) ? 2 : 1;
      const int c = 3 - b;
      
      if (str->length < 3)
        {
	  fprintf (stderr, "Warning: strip too short for auto UV "
		   "orientation\n");
          continue;
	}

      if (!str->texcoords)
        {
	  fprintf (stderr, "Warning: no texture coordinates for auto UV "
		   "orientation\n");
	  continue;
	}

      if (!str->s_attrs)
        {
	  fprintf (stderr, "Warning: strip has no attributes block\n");
	  continue;
	}

      vec_sub (a_minus_b, (*str->start)[a], (*str->start)[b]);
      vec_scale (a_minus_b, a_minus_b,
		 (*str->texcoords)[a][1] - (*str->texcoords)[c][1]);
      vec_sub (a_minus_c, (*str->start)[a], (*str->start)[c]);
      vec_scale (a_minus_c, a_minus_c,
		 (*str->texcoords)[a][1] - (*str->texcoords)[b][1]);
      vec_sub (u, a_minus_b, a_minus_c);

      /* Not really sure if this is right/needed.  */
      if ((((*str->texcoords)[a][0] - (*str->texcoords)[b][0])
            * ((*str->texcoords)[a][1] - (*str->texcoords)[c][1])
	   - ((*str->texcoords)[a][0] - (*str->texcoords)[c][0])
	     * ((*str->texcoords)[a][1] - (*str->texcoords)[b][1])) < 0.0f)
	vec_scale (u, u, -1.0f);

      vec_normalize (str->s_attrs->uv_orient, u);
    }
}

/* Allocate memory for raw bumpmap, and load from FILENAME.  (Should probably
   hack dcbumpgen to output kimg files instead.)  */

pvr_ptr_t
bumpmap_load_raw (const char *filename, unsigned int xsize, unsigned int ysize)
{
  void *data = malloc (xsize * ysize * 2);
  pvr_ptr_t pvraddr;
  FILE *fp = fopen (filename, "rb");
  
  if (!fp)
    {
      fprintf (stderr, "Couldn't load bump map '%s'\n", filename);
      return NULL;
    }
  
  fread (data, 1, xsize * ysize * 2, fp);
  fclose (fp);
  
  pvraddr = pvr_mem_malloc (xsize * ysize * 2);
  pvr_txr_load (data, pvraddr, xsize * ysize * 2);
  free (data);
  
  return pvraddr;
}
