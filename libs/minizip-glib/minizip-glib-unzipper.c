#include "minizip-glib/minizip.h"

int main (int argc, char **argv)
{
  GFile *src;
  GFile *dest;
  GInputStream *src_stream;

  g_type_init ();

  if (argc != 3)
    return 1;

  src = g_file_new_for_commandline_arg (argv[1]);
  dest = g_file_new_for_commandline_arg (argv[2]);

  src_stream = G_INPUT_STREAM (g_file_read (src, NULL, NULL));

  if (!minizip_unzip (src_stream, dest))
    return 1;

  g_input_stream_close (src_stream, NULL, NULL);
  g_object_unref (src);
  g_object_unref (dest);

  g_print ("Successfully extracted unzipped file!\n");

  return 0;
}
