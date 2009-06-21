#include <kos.h>

static void
palette_grey_ramp (void)
{
  unsigned int i;
  
  pvr_set_pal_format (PVR_PAL_ARGB8888);
  
  for (i = 0; i < 256; i++)
    {
      unsigned int palentry;
      
      /* Grey ramp with solid alpha.  */
      palentry = 0xff000000 | (i << 16) | (i << 8) | i;
      pvr_set_pal_entry (i, palentry);
    }
}
