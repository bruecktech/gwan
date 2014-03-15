// ============================================================================
// C servlet sample for the G-WAN Web Application Server (http://trustleap.ch/)
// ----------------------------------------------------------------------------
// hello.c: just used with ab (ApacheBench) to benchmark a minimalist servlet
// ============================================================================
#include "gwan.h" // G-WAN exported functions

int main(int argc, char *argv[])
{
   static char msg[] = "Hello, ANSI C2!";
   xbuf_ncat(get_reply(argv), msg, sizeof(msg) - 1);
   
   return 200; // return an HTTP code (200:'OK')
}
// ============================================================================
// End of Source Code
// ============================================================================

