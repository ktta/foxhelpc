#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <png.h>
#include <errno.h>
#include "pngdim.h"

#define SIG_BYTES 8

char *get_png_dimensions (const char *fn, unsigned int *Rw, unsigned int *Rh)
{
  FILE *fp;
  unsigned char header[SIG_BYTES];
  png_structp png_ptr;
  png_infop info_ptr;
  png_infop end_info;
  png_uint_32 ww, hh;
  int bit_depth, color_type, interlace_type;
  char ebuf[2000];

  fp = fopen (fn, "rb");
  if (!fp)
  {
    snprintf(ebuf,sizeof(ebuf)-1,"can not open file %s: %s",
                                  fn, strerror (errno));
    goto eret;
  }

  if (fread (header, 1, SIG_BYTES, fp) != SIG_BYTES)
  {
      snprintf(ebuf,sizeof(ebuf)-1,
                "can not read signature bytes from file %s", fn);
      goto err_ret_close;
  }

  if (png_sig_cmp (header, 0, SIG_BYTES))
  {
     snprintf(ebuf,sizeof(ebuf)-1, "file %s is not a PNG file.", fn);
     goto err_ret_close;
  }

  png_ptr = png_create_read_struct (PNG_LIBPNG_VER_STRING, 0, 0, 0);

  if (!png_ptr)
  {
     snprintf(ebuf,sizeof(ebuf)-1,
              "internal error. libpng: can not allocate PNG struct.");
     goto err_ret_close;
  }

  info_ptr = png_create_info_struct (png_ptr);
  if (!info_ptr)
  {
    snprintf(ebuf,sizeof(ebuf)-1,
               "internal error. libpng: can not allocate first info struct.");
    goto err_ret_destroy0;
  }

  end_info = png_create_info_struct (png_ptr);
  if (!end_info)
  {
    snprintf(ebuf,sizeof(ebuf)-1,
	"internal error. libpng: can not allocate second info struct.");
    goto err_ret_destroy1;
  }

  if (setjmp (png_jmpbuf (png_ptr)))
  {
    snprintf(ebuf,sizeof(ebuf)-1,
              "internal error. libpng jumped back.");
    goto err_ret_destroy2;
  }

  png_init_io (png_ptr, fp);
  png_set_sig_bytes (png_ptr, SIG_BYTES);

  png_read_info (png_ptr, info_ptr);

  png_get_IHDR (png_ptr, info_ptr, &ww, &hh, &bit_depth, &color_type,
		&interlace_type, NULL, NULL);

  png_destroy_read_struct (&png_ptr, &info_ptr, &end_info);

  fclose(fp);

  *Rw= ww;
  *Rh= hh;

  return NULL;

err_ret_destroy0:
  png_destroy_read_struct (&png_ptr, 0, 0);
  goto err_ret_close;
err_ret_destroy1:
  png_destroy_read_struct (&png_ptr, &info_ptr, 0);
  goto err_ret_close;
err_ret_destroy2:
  png_destroy_read_struct (&png_ptr, &info_ptr, &end_info);
err_ret_close:
  fclose (fp);
eret:
  ebuf[sizeof(ebuf)-1]= 0;
  return strdup(ebuf);
}
