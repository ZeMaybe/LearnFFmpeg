#include "common.h"

char* get_input_file_path(int argc, char* argv[])
{
    static char default_in_path[] = "input.mp4";

    if (argc >= 2)
    {
        return argv[1];
    }
    else
    {
        return default_in_path;
    }
}

char* get_output_file_path(int argc, char* argv[])
{
    static char default_out_path[] = "output.yuv";
    if (argc >= 3)
    {
        return argv[2];
    }
    else
    {
        return default_out_path;
    }
}
