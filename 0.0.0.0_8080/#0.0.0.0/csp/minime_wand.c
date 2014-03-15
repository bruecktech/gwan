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

#pragma link "MagickWand"
#pragma include "/usr/include/ImageMagick"

#include "gwan.h"
#include <stdio.h>
#include <string.h>
#include <wand/MagickWand.h>


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

   MagickWand *m_wand = NULL;
	
   int width,height;	
   MagickWandGenesis();

   m_wand = NewMagickWand();
   MagickReadImage(m_wand, filename);


   // Get the image's width and height
   width = MagickGetImageWidth(m_wand);
   height = MagickGetImageHeight(m_wand);
   
   // Cut them in half but make sure they don't underflow
   if((width /= 2) < 1)width = 1;
   if((height /= 2) < 1)height = 1;
   
   // Resize the image using the Lanczos filter
   // The blur factor is a "double", where > 1 is blurry, < 1 is sharp
   // I haven't figured out how you would change the blur parameter of MagickResizeImage
   // on the command line so I have set it to its default of one.
   MagickResizeImage(m_wand,width,height,LanczosFilter,1);
   
   // Set the compression quality to 95 (high quality = low compression)
   MagickSetImageCompressionQuality(m_wand,95);
   

   // -------------------------------------------------------------------------
   // specify a MIME type so we don't have to build custom HTTP headers
   // -------------------------------------------------------------------------
   char *mime = (char*)get_env(argv, REPLY_MIME_TYPE);
   // note that we setup the FILE EXTENTION, not the MIME type:
   mime[0] = '.'; mime[1] = 'j'; mime[2] = 'p'; mime[3] = 'g'; mime[4] = 0;
 
   size_t src_len = 0;
   u8 *src = MagickGetImagesBlob(m_wand, &src_len);

   xbuf_ncat(reply, (char *)src, src_len);

   /* Clean up */
   if(m_wand)m_wand = DestroyMagickWand(m_wand);
   
   MagickWandTerminus();

   return 200;
}
// ============================================================================
// End of Source Code
// ============================================================================
