﻿#include <unistd.h>
#include "fuse_options.hpp"
using namespace std;
using namespace rocksfs;
/*
* Command line options
*
* We can't set default values for the char* fields here because
* fuse_opt_parse would attempt to free() them when the user specifies
* different values on the command line.
*/
struct myfs_opts {
    char* dbpath = nullptr;
    int debugfs = 0;
    int is_help;
}myfs_opts;

#define MYFS_OPT(t,p){t,offsetof(struct myfs_opts,p),1}

static const char* usage =
"Usage: ./rocksdb-fuse [options] <mountpoint>\n\n"
"options:\n"
"    --help|-h		Print this help message\n"
"    --dbpath=<s>	The path for database files.\n"
"    -o allow_other	Allow other users to access the files."
"   -o allow_root	This option is similar to allow_other but file access is limited to the user mounting the filesystem and root."
"\n";

static const struct fuse_opt option_spec[] = {
    MYFS_OPT("--dbpath=%s",dbpath),
    MYFS_OPT("--debugfs",debugfs),
    FUSE_OPT_KEY("-h",	0),
    FUSE_OPT_KEY("--help",	0),
    FUSE_OPT_END
};
static int process_arg(void* data, const char* arg, int key, struct fuse_args* outargs)
{
    struct myfs_opts* param = static_cast<struct myfs_opts*>(data);

    (void)outargs;
    (void)arg;

    switch (key) {
    case 0:
        param->is_help = 1;
        fprintf(stderr, "%s", usage);
        //puts(usage);
        return fuse_opt_add_arg(outargs, "-ho");
    default:
        return 1;
    }
}

int main(int argc, char* argv[])
{
    struct fuse_args args = FUSE_ARGS_INIT(argc, argv);
    struct myfs_opts config {};
    if (fuse_opt_parse(&args, &config, option_spec, process_arg))
    {
        printf("failed to parse option\n");
        return 1;
    }
    if (config.is_help)
    {
        return 0;
    }
    else
    {
        if (!config.dbpath)
        {
            puts("dbpath not inputed");
            puts(usage);
            return 1;
        }
    }
    fuse_opt_add_arg(&args, "-oauto_unmount");
    if(config.debugfs)
    {
        fuse_opt_add_arg(&args, "-d");
    }
    else
    {
        fuse_opt_add_arg(&args, "-f");
        daemon(1, 0);
    }
	rocksfs::FileSystemOptions fs(config.dbpath);
    fs.Mount(args.argc, args.argv);
    return 0;
}
