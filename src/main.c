/*
 * cloudfs: main source
 *   By Benjamin Kittridge. Copyright (C) 2013, All rights reserved.
 *
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <getopt.h>
#include "main.h"
#include "config.h"
#include "log.h"
#include "store.h"
#include "bucket.h"
#include "crypt.h"
#include "volume.h"
#include "mt.h"

////////////////////////////////////////////////////////////////////////////////
// Class:       main
// Description: Handles program options and startup

////////////////////////////////////////////////////////////////////////////////
// Section:     Options

enum opt_value {
  OPT_VERSION,
  OPT_HELP,
  OPT_LOG,
  OPT_CONFIG,
  OPT_EXCL,
  OPT_NRML,
};

static struct option opt_field[] = {
  { "store",               1,  NULL,  OPT_NRML    },
  { "bucket",              1,  NULL,  OPT_NRML    },
  { "volume",              1,  NULL,  OPT_NRML    },

  { "version",             0,  NULL,  OPT_VERSION },
  { "help",                0,  NULL,  OPT_HELP    },
  { "log",                 1,  NULL,  OPT_LOG     },
  { "config",              1,  NULL,  OPT_CONFIG  },
  { "password",            1,  NULL,  OPT_NRML    },
  { "password-prompt",     0,  NULL,  OPT_NRML    },
  { "readonly",            0,  NULL,  OPT_NRML    },
  { "nofork",              0,  NULL,  OPT_NRML    },
  { "norandom",            0,  NULL,  OPT_NRML    },
  { "force",               0,  NULL,  OPT_NRML    },
  { "use-https",           0,  NULL,  OPT_NRML    },

  { "cache-type",          1,  NULL,  OPT_NRML    },
  { "cache-max",           1,  NULL,  OPT_NRML    },

  { "create-bucket",       0,  NULL,  OPT_EXCL    },
  { "auto-create-bucket",  0,  NULL,  OPT_NRML    },
  { "delete-bucket",       0,  NULL,  OPT_EXCL    },

  { "create",              0,  NULL,  OPT_EXCL    },
  { "mount",               1,  NULL,  OPT_EXCL    },
  { "unmount",             1,  NULL,  OPT_EXCL    },
  { "list",                0,  NULL,  OPT_EXCL    },
  { "fsck",                0,  NULL,  OPT_EXCL    },
  { "delete",              0,  NULL,  OPT_EXCL    },

  { "format",              1,  NULL,  OPT_NRML    },
  { "size",                1,  NULL,  OPT_NRML    },

  { "amazon-key",          1,  NULL,  OPT_NRML    },
  { "amazon-secret",       1,  NULL,  OPT_NRML    },

  { "google-client-id",    1,  NULL,  OPT_NRML    },
  { "google-client-secret",1,  NULL,  OPT_NRML    },
  { "google-project-id",   1,  NULL,  OPT_NRML    },
  { "google-token-file",   1,  NULL,  OPT_NRML    },

  { "dummy-path",          1,  NULL,  OPT_NRML    },
  { NULL,                  0,  NULL,  0           }
};

////////////////////////////////////////////////////////////////////////////////
// Section:     Help

void version() {
  fprintf(stderr, "cloudfs v"VERSION" built on "__DATE__" "__TIME__"\n");
  fprintf(stderr, "Author: Benjamin Kittridge, bysin@bysin.net\n");
}

void usage() {
  version();
  fprintf(stderr, "Usage: cloudfs [OPTIONS]...\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "Required Arguments:\n");
  fprintf(stderr, "\t%-25s Storage service, must be one of:\n", "--store [service]");
  fprintf(stderr, "\t%-25s     dummy, amazon, google\n",        "");
  fprintf(stderr, "\t%-25s Bucket\n",                           "--bucket [name]");
  fprintf(stderr, "\t%-25s Volume\n",                           "--volume [name]");
  fprintf(stderr, "\n");
  fprintf(stderr, "Optional Arguments:\n");
  fprintf(stderr, "\t%-25s Displays this message\n",            "--help");
  fprintf(stderr, "\t%-25s Write to log file\n",                "--log [file]");
  fprintf(stderr, "\t%-25s Config file\n",                      "--config [file]");
  fprintf(stderr, "\t%-25s Encryption password\n",              "--password [key]");
  fprintf(stderr, "\t%-25s Prompt for encryption password\n",   "--password-prompt");
  fprintf(stderr, "\t%-25s Read only\n",                        "--readonly");
  fprintf(stderr, "\t%-25s Do not fork into background\n",      "--nofork");
  fprintf(stderr, "\t%-25s Do not use /dev/random\n",           "--norandom");
  fprintf(stderr, "\t%-25s Force mounting volume\n",            "--force");
  fprintf(stderr, "\t%-25s Use HTTPS for storage requests\n",   "--use-https");
  fprintf(stderr, "\n");
  fprintf(stderr, "Cache Arguments:\n");
  fprintf(stderr, "\t%-25s Cache type, must be one of:\n",      "--cache-type [type]");
  fprintf(stderr, "\t%-25s     memory, file\n",                 "");
  fprintf(stderr, "\t%-25s Maximum size of cache\n",            "--cache-max [size]");
  fprintf(stderr, "\t%-25s Path to store cache\n",              "--cache-path [path]");
  fprintf(stderr, "\n");
  fprintf(stderr, "Bucket operations:\n");
  fprintf(stderr, "\t%-25s Create bucket\n",                    "--create-bucket");
  fprintf(stderr, "\t%-25s Create bucket if non-existent\n",    "--auto-create-bucket");
  fprintf(stderr, "\t%-25s Delete bucket\n",                    "--delete-bucket");
  fprintf(stderr, "\n");
  fprintf(stderr, "Volume operations:\n");
  fprintf(stderr, "\t%-25s Create volume\n",                    "--create");
  fprintf(stderr, "\t%-25s Mount volume to target path\n",      "--mount [path]");
  fprintf(stderr, "\t%-25s Unmount volume from target path\n",  "--unmount [path]");
  fprintf(stderr, "\t%-25s List volumes\n",                     "--list");
  fprintf(stderr, "\t%-25s Check filesystem\n",                 "--fsck");
  fprintf(stderr, "\t%-25s Delete volume\n",                    "--delete");
  fprintf(stderr, "\n");
  fprintf(stderr, "Arguments for volume creation:\n");
  fprintf(stderr, "\t%-25s Volume format, must be one of:\n",   "--format [format]");
  fprintf(stderr, "\t%-25s     vfs, block\n",                   "");
  fprintf(stderr, "\t%-25s Size of volume\n",                   "--size [size]");
  fprintf(stderr, "\n");
  fprintf(stderr, "Arguments for amazon storage:\n");
  fprintf(stderr, "\t%-25s Access key ID\n",                    "--amazon-key [key]");
  fprintf(stderr, "\t%-25s Secret access key\n",                "--amazon-secret [key]");
  fprintf(stderr, "\n");
  fprintf(stderr, "Arguments for google storage:\n");
  fprintf(stderr, "\t%-25s Client ID\n",                        "--google-client-id [id]");
  fprintf(stderr, "\t%-25s Client Secret\n",                    "--google-client-secret [key]");
  fprintf(stderr, "\t%-25s Project ID\n",                       "--google-project-id [id]");
  fprintf(stderr, "\t%-25s File to write auth token\n",         "--google-token-file [file]");
  fprintf(stderr, "\n");
  fprintf(stderr, "Arguments for dummy storage:\n");
  fprintf(stderr, "\t%-25s Path to storage directory\n",        "--dummy-path [path]");
  fprintf(stderr, "\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "Setup:\n");
  fprintf(stderr, "\t1) Copy ${BUILD_PATH}/bin/cloudfs.conf to ~/.cloudfs.conf\n");
  fprintf(stderr, "\t2) Edit ~/.cloudfs.conf with required information\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "Example volume operations (assuming you followed the setup):\n");
  fprintf(stderr, "\tListing:     cloudfs --list\n");
  fprintf(stderr, "\tCreate:      cloudfs --volume [volume] --create\n");
  fprintf(stderr, "\tMounting:    cloudfs --volume [volume] --mount [directory]\n");
  fprintf(stderr, "\tUnmounting:  cloudfs --volume [volume] --unmount [directory]\n");
  fprintf(stderr, "\tDeleting:    cloudfs --volume [volume] --delete\n");
  fprintf(stderr, "\n");
}

////////////////////////////////////////////////////////////////////////////////
// Section:     Initialization

int main(int argc, char **argv) {
  int32_t ch, index, excl_index;
  const char *name;
  bool load_default, exclusive;

  load_default = true;
  exclusive = false;

  index = 0;
  while (1) {
    if ((ch = getopt_long_only(argc, argv, "", opt_field, &index)) < 0)
      break;
    switch (ch) {
      case OPT_LOG:
        log_load(optarg);
        break;

      case OPT_CONFIG:
        config_load(optarg);
        load_default = false;
        break;

      case OPT_EXCL:
        if (exclusive)
          error("Cannot use --%s with --%s",
              opt_field[index].name,
              opt_field[excl_index].name);
        exclusive = true;
        excl_index = index;

      case OPT_NRML:
        name = opt_field[index].name;
        if (opt_field[index].has_arg)
          config_set(name, optarg);
        else
          config_set(name, "true");
        break;

      case OPT_VERSION:
        version();
        return 1;

      case OPT_HELP:
      default:
        usage();
        return 1;
    }
  }

  if (load_default)
    config_default();

  mt_init();

  store_load();
  bucket_load();
  crypt_load();
  volume_load();
  return 0;
}
