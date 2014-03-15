// ============================================================================
// C servlet sample for the G-WAN Web Application Server (http://trustleap.ch/)
// ----------------------------------------------------------------------------
// cairo.c: generates a PNG bitmap with the "Hello World" string rendered with
//          the selected font and RGB 24-bit color.
// ----------------------------------------------------------------------------
// Cairo is a 2D graphics library with support for multiple output devices:
//
//   - the X Window System (via both Xlib and XCB), 
//   - Quartz, 
//   - Win32,
//   - image buffers,
//   - PostScript,
//   - PDF,
//   - OpenGL,
//   - SVG file output.
//
// Cairo support stroking and filling cubic Bézier splines, transforming and 
// compositing translucent images, and antialiased text rendering. All drawing 
// operations can be transformed by any affine transformation (scale, rotation, 
// shear, etc.). More on http://cairographics.org/
//
// Examples of use:
//
//  - GNUplot and PLplot are 2D and 3D graph plotting utility using Cairo,
//  - Firefox 3.0 uses cairo for rendering all content and UI,
//  - The GTK+ widget toolkit version 2.8+ relies on Cairo.
//
// Installation: sudo apt-get install libcairo2 libcairo2-dev
// ----------------------------------------------------------------------------
#pragma link "cairo"
#pragma include "/usr/include/cairo"
#include "cairo.h"

#include "gwan.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


static cairo_status_t
write_png_stream_to_buffer (void *in_closure, const unsigned char *data,
                                                unsigned int length)
{
   xbuf_ncat(in_closure, (char*) data, length);
   return CAIRO_STATUS_SUCCESS;
}



// ----------------------------------------------------------------------------
int main(int argc, char **argv)
{
   xbuf_t *reply = get_reply(argv);

   // -------------------------------------------------------------------------
   // get the URI parameter
   // -------------------------------------------------------------------------
   char *file = ""; get_arg("file=", &file, argc, argv);
   if(!file || !*file)
   {
      //xbuf_cat(reply, "<p>usage: http://127.0.0.1:8080/?minime.c&file=index.html</p>");
      printf ("No filename given");
      return 400;
   }
   
   // -------------------------------------------------------------------------
   // build the local file name   
   // -------------------------------------------------------------------------
   char *wwwpath = (char*)get_env(argv, WWW_ROOT);
   char filename[512] = {0};
   snprintf(filename, sizeof(filename), "%s/%s", wwwpath, file);
   u32 i = 1;
   for(; i < argc; i++) // just in case we have subdirectories
   {
       strcat(filename, "/");
       strcat(filename, argv[i]);
   }

   // -------------------------------------------------------------------------
   // try to load the file
   // -------------------------------------------------------------------------
   //printf("loading '%s' file...\n", filename);
   //xbuf_t f; xbuf_init(&f);
   //xbuf_frfile(&f, filename);
   //printf("loaded %d bytes\n", f.len);
   //if(!f.len) return 200;

   // -------------------------------------------------------------------------
   // build the bitmap using Cairo
   // -------------------------------------------------------------------------
   //int stride = cairo_format_stride_for_width (CAIRO_FORMAT_ARGB32, 640);
   //cairo_surface_t *surface_source = cairo_image_surface_create_for_data((unsigned char *)f.ptr, CAIRO_FORMAT_ARGB32, 640, 480, stride);
   cairo_surface_t *surface_source = cairo_image_surface_create_from_png(filename);
   cairo_t *cr_source = cairo_create (surface_source);

   //cairo_select_font_face(cr_source, "serif", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
   //cairo_set_font_size   (cr_source, 32.0);
   //cairo_set_source_rgb  (cr_source, 0.0, 0.0, 1.0);
   //cairo_move_to         (cr_source, 10.0, 50.0);
   //cairo_show_text       (cr_source, "Hello, world");

   const double factor = 0.5;

   int source_w = cairo_image_surface_get_width (surface_source);
   int source_h = cairo_image_surface_get_height (surface_source);

   printf("%f", source_w * factor);

   cairo_surface_t *surface_dest = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, source_w * factor, source_h * factor);
   cairo_t *cr_dest = cairo_create (surface_dest);

   cairo_scale(cr_dest, factor, factor);

   cairo_set_source_surface(cr_dest, surface_source, 0, 0);
   cairo_paint(cr_dest);

   cairo_destroy         (cr_source);
   cairo_destroy         (cr_dest);
   
   // -------------------------------------------------------------------------
   // allocate memory for a raw bitmap
   // -------------------------------------------------------------------------
   //int   w = 800, h = 800, nbcolors = 256, wXh = w * h;
   //u8 *bmp = (u8*)malloc(wXh);
   //if(!bmp) return 503; // service unavailable


   // -------------------------------------------------------------------------
   // specify a MIME type so we don't have to build custom HTTP headers
   // -------------------------------------------------------------------------
   char *mime = (char*)get_env(argv, REPLY_MIME_TYPE);
   // note that we setup the FILE EXTENTION, not the MIME type:
   mime[0] = '.'; mime[1] = 'p'; mime[2] = 'n'; mime[3] = 'g'; mime[4] = 0;

   // -------------------------------------------------------------------------
   // make sure that we have enough space in the 'reply' buffer
   // (we are going to fill it directly from gif_build(), not via xbuf_xxx)
   // -------------------------------------------------------------------------
   // (if we have not enough memory, we will get a 'graceful' crash)
   //if(reply->allocated < (wXh / 10)) // very gross approximation
   //{
   //   if(!xbuf_growto(reply, wXh / 10)) // resize reply
   //   {
   //      xbuf_init(reply);
   //      xbuf_ncat(reply, " ", 1);
   //      reply->len = 0; // discart pointless data, keep allocated memory
   //      return 503; // error: we could not allocate enough memory
   //   }
   //}

   int rc;

   rc = cairo_surface_write_to_png_stream (surface_dest, write_png_stream_to_buffer, reply);
   cairo_surface_destroy(surface_dest);

   if(rc == CAIRO_STATUS_SUCCESS){
      return 200;
   } else {
      return 500;
   }
}
// ============================================================================
// End of Source Code
// ============================================================================
