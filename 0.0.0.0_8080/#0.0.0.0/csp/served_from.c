// ============================================================================
// C servlet sample for the G-WAN Web Application Server (http://trustleap.ch/)
// ----------------------------------------------------------------------------
// servedfrom.c: return "Served from 192.168.142.16:8080" (with your details)
//
//               Useful to check Virtual Host settings.
//
// Based on an idea coded in Python (see below) and describbed in:
// http://www.linuxjournal.com/magazine/nginx-high-performance-web-server-
//                                              and-reverse-proxy?page=0,2
//
//   import sys,BaseHTTPServer as B
//   class Handler(B.BaseHTTPRequestHandler):
//     def do_GET(self):
//       self.wfile.write("Served from port %s" % port)
//     def log_message(self, *args):
//       pass
//   if __name__ == '__main__':
//     host,port = sys.argv[1:3]
//     server = B.HTTPServer((host,int(port)), Handler)
//     server.serve_forever()
//
// Python does not look like a simpler (or clearer) language than ANSI C, and 
// G-WAN certainly makes it easier to serve dynamic content than Nginx+Python
// (see the configuration detailed in the 'Linux Journal' article above).
// ============================================================================
// imported functions:
//     get_env(): get the specified 'environment' variable from the server
//   get_reply(): get a pointer on the 'reply' dynamic buffer from the server
//   xbuf_xcat(): like sprintf(), but it works in the specified dynamic buffer 
// ----------------------------------------------------------------------------
#include "gwan.h" // G-WAN exported functions

#include <assert.h>
#include <string.h>
// ----------------------------------------------------------------------------
// see enum HTTP_Method {} and char *szHTTP_Method[] in gwan.h
static char *szHTTP_Method[] =
{
   [HTTP_ANY]        = "?",
   [HTTP_GET]        = "GET",
   [HTTP_HEAD]       = "HEAD",
   [HTTP_POST]       = "POST",
   [HTTP_PUT]        = "PUT",
   [HTTP_DELETE]     = "DELETE",
   [HTTP_OPTIONS]    = "OPTIONS",
   // G-WAN currently supports this list until 'OPTIONS'
   [HTTP_CONNECT]    = "CONNECT",
   [HTTP_TRACE]      = "TRACE",
   [HTTP_PATCH]      = "PATCH",     // no RFC (remove?)
   [HTTP_PROPFIND]   = "PROPFIND",  // RFC 2518: WebDAV
   [HTTP_PROPPATCH]  = "PROPPATCH",
   [HTTP_MKCOL]      = "MKCOL",
   [HTTP_COPY]       = "COPY",
   [HTTP_MOVE]       = "MOVE",
   [HTTP_LOCK]       = "LOCK",
   [HTTP_UNLOCK]     = "UNLOCK",
   [HTTP_VERSION_CONTROL]="VERSION_CONTROL",
   [HTTP_CHECKOUT]   = "CHECKOUT",
   [HTTP_UNCHECKOUT] = "UNCHECKOUT",
   [HTTP_CHECKIN]    = "CHECKIN",
   [HTTP_UPDATE]     = "UPDATE",
   [HTTP_LABEL]      = "LABEL",
   [HTTP_REPORT]     = "REPORT",
   [HTTP_MKWORKSPACE]= "MKWORKSPACE",
   [HTTP_MKACTIVITY] = "MKACTIVITY",
   [HTTP_BASELINE_CONTROL]="BASELINE_CONTROL",
   [HTTP_MERGE]      = "MERGE",
   [HTTP_INVALID]    = "INVALID"    // RFC 3253: WebDAV versioning
   ""
};
// ----------------------------------------------------------------------------
int main(int argc, char *argv[])
{
   xbuf_t *reply = get_reply(argv);

   char *srv_host = (char*)get_env(argv, SERVER_NAME);
   char *srv_ip   = (char*)get_env(argv, SERVER_ADDR);
   u32 srv_port   = (u32)  get_env(argv, SERVER_PORT);
   char *vhost    = (char*)get_env(argv, VHOST_ROOT);
   char *cli_ip   = (char*)get_env(argv, REMOTE_ADDR);
   u32 cli_port   = (u32)  get_env(argv, REMOTE_PORT);
   u32   method   = (u32)  get_env(argv, REQUEST_METHOD);
   char *cli_id   = (char*)get_env(argv, USER_AGENT);
   char *request  = (char*)get_env(argv, REQUEST);
   char *query    = (char*)get_env(argv, QUERY_STRING);
   char *fragment = (char*)get_env(argv, FRAGMENT_ID);
   char *entity   = (char*)get_env(argv, REQ_ENTITY);
   u32  cont_enc  = (u32)  get_env(argv, CONTENT_ENCODING);
   u32  cont_len  = (u32)  get_env(argv, CONTENT_LENGTH);
   u32  cont_type = (u32)  get_env(argv, CONTENT_TYPE);

   xbuf_xcat(reply,
             "\n<pre>\n"
             "This page was processed...\n\n"
             "<b>Using get_env(argv, XXX) for each value displayed here</b>:\n\n"
             "by the Server:    %s:%u (hostname: %s)\n"
             "Virtual Host:     %s\n"
             "HTTP method:      %s\n"
             "HTTP request:     %s\n"
             "HTTP query:       %s\n"
             "HTTP fragment:    %s\n"
             "HTTP entity:      %s\n"
             "Content-Encoding: %u\n"
             "Content-Length:   %U\n"
             "Content-Type:     %u\n"
             "for the Client:   %s:%u\n"
             "%s\n",
             srv_ip, srv_port, srv_host,
             vhost,
             szHTTP_Method[method],
             request, query, fragment, 
             entity, 
             cont_enc, cont_len, cont_type,
             cli_ip, cli_port, cli_id);
             
   // an alternate, faster method to get the same information:
   // (see typedef struct http_t in gwan.h)
   http_t *http = (http_t*)get_env(argv, HTTP_HEADERS);
   
   char acpt_encodings[80] = {0};
   if(http->h_accept_encoding & ENC_GZIP)     strcat(acpt_encodings, "|GZIP");
   if(http->h_accept_encoding & ENC_DEFLATE)  strcat(acpt_encodings, "|DEFLATE");
   if(http->h_accept_encoding & ENC_COMPRESS) strcat(acpt_encodings, "|COMPRESS");
   if(http->h_accept_encoding & ENC_CHUNKED)  strcat(acpt_encodings, "|CHUNKED");
   if(http->h_accept_encoding & ENC_SDCH)     strcat(acpt_encodings, "|SDCH");

   // -------------------------------------------------------------------------
   // with G-WAN v3.10+ the http_t fields (below) are offsets (not pointers):
   // -------------------------------------------------------------------------
   // they are offsets of the read_buffer start (coping with buffer resizes)
   xbuf_t *read_buf  = (xbuf_t*)get_env(argv, READ_XBUF);
   char *p = read_buf->ptr;
   entity                = http->h_entity          ? p + http->h_entity : 0;
   srv_host              = http->h_host            ? p + http->h_host : 0;
   char *useragent       = http->h_useragent       ? p + http->h_useragent : 0;
   char *referer         = http->h_referer         ? p + http->h_referer : 0;
   char *accept_language = http->h_accept_language ? p + http->h_accept_language : 0;
   char *cookies         = http->h_cookies         ? p + http->h_cookies : 0;
   
   xbuf_xcat(reply,
             "\n" 
             "<b>Using get_env(argv, HTTP_HEADERS); to get more information</b>:\n\n"
             "by the Server:    %s:%u (hostname: %s)\n"
             "HTTP method:      %s\n"
             "HTTP request:     %s\n"
             "HTTP query:       %s\n"
             "HTTP fragment:    %s\n"
             "HTTP entity:      %s\n"
             "Content-Encoding: %u\n"
             "Content-Length:   %U\n"
             "Content-Type:     %u\n"
             "for the Client:   %s:%u\n"
             "%s\n"
             "Acpt-Language:    %s\n"
             "Acpt-Encoding:    %u: %s\n"
             "Cookies:          %s\n"
             "Referer:          %s\n</pre>\n\n",
             srv_host, http->h_port, srv_host, 
             szHTTP_Method[http->h_method],
             request, query, fragment,
             http->h_content_encoding,
             entity, http->h_content_length, http->h_content_type,
             cli_ip, cli_port, useragent,
             accept_language,
             http->h_accept_encoding, acpt_encodings,
             cookies, 
             referer);
   
   return 200;
}
// ============================================================================
// End of Source Code
// ============================================================================
