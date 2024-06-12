/* -*- mode: c; c-basic-offset: 8; indent-tabs-mode: nil; -*-
 * vim:expandtab:shiftwidth=8:tabstop=8:
 */
/******************************************************************************\
*
* Implement abstract I/O interface for CUBEFS.
*
\******************************************************************************/

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include "libcfs.h"

#include "ior.h"
#include "iordef.h"
#include "aiori.h"
#include "utilities.h"

/************************** O P T I O N S *****************************/
struct cubefs_options{
  char * conf;
};

static struct cubefs_options o = {
  .conf = NULL,
};

static option_help options [] = {
      {0, "cubefs.conf", "Config file for the cubefs cluster", OPTION_OPTIONAL_ARGUMENT, 's', & o.conf},
      LAST_OPTION
};

/**************************** P R O T O T Y P E S *****************************/
static void CUBEFS_Init();
static void CUBEFS_Final();
void CUBEFS_xfer_hints(aiori_xfer_hint_t * params);
static aiori_fd_t *CUBEFS_Create(char *path, int flags, aiori_mod_opt_t *options);
static aiori_fd_t *CUBEFS_Open(char *path, int flags, aiori_mod_opt_t *options);
static IOR_offset_t CUBEFS_Xfer(int access, aiori_fd_t *file, IOR_size_t *buffer,
                           IOR_offset_t length, IOR_offset_t offset, aiori_mod_opt_t *options);
static void CUBEFS_Close(aiori_fd_t *, aiori_mod_opt_t *);
static void CUBEFS_Delete(char *path, aiori_mod_opt_t *);
static void CUBEFS_Fsync(aiori_fd_t *, aiori_mod_opt_t *);
static IOR_offset_t CUBEFS_GetFileSize(aiori_mod_opt_t *, char *);
static int CUBEFS_StatFS(const char *path, ior_aiori_statfs_t *stat, aiori_mod_opt_t *options);
static int CUBEFS_MkDir(const char *path, mode_t mode, aiori_mod_opt_t *options);
static int CUBEFS_RmDir(const char *path, aiori_mod_opt_t *options);
static int CUBEFS_Access(const char *path, int mode, aiori_mod_opt_t *options);
static int CUBEFS_Stat(const char *path, struct stat *buf, aiori_mod_opt_t *options);
static void CUBEFS_Sync(aiori_mod_opt_t *);
static option_help * CUBEFS_options();

static aiori_xfer_hint_t * hints = NULL;

/************************** D E C L A R A T I O N S ***************************/
ior_aiori_t cubefs_aiori = {
        .name = "CUBEFS",
        .name_legacy = NULL,
        .initialize = CUBEFS_Init,
        .finalize = CUBEFS_Final,
        .create = CUBEFS_Create,
        .open = CUBEFS_Open,
        .xfer = CUBEFS_Xfer,
        .close = CUBEFS_Close,
        .delete = CUBEFS_Delete,
        .get_options = CUBEFS_options,
        .get_version = aiori_get_version,
        .xfer_hints = CUBEFS_xfer_hints,
        .fsync = CUBEFS_Fsync,
        .get_file_size = CUBEFS_GetFileSize,
        .statfs = CUBEFS_StatFS,
        .mkdir = CUBEFS_MkDir,
        .rmdir = CUBEFS_RmDir,
        .access = CUBEFS_Access,
        .stat = CUBEFS_Stat,
        .sync = CUBEFS_Sync,
        .enable_mdtest = true
};

#define CUBEFS_ERR(__err_str, __ret) do { \
        errno = -__ret; \
        ERR(__err_str); \
} while(0)

/***************************** F U N C T I O N S ******************************/

void CUBEFS_xfer_hints(aiori_xfer_hint_t * params)
{
  hints = params;
}

static option_help * CUBEFS_options(){
  return options;
}

static void CUBEFS_Init()
{
        printf("CUBEFS_Init\n");
        return;
}

static void CUBEFS_Final()
{
        printf("CUBEFS_Final\n");
        return;
}

static aiori_fd_t *CUBEFS_Create(char *path, int flags, aiori_mod_opt_t *options)
{
        return CUBEFS_Open(path, flags | IOR_CREAT, options);
}

static aiori_fd_t *CUBEFS_Open(char *path, int flags, aiori_mod_opt_t *options)
{
        int* fd;
        fd = (int *)malloc(sizeof(int));
        *fd = 0;
        return (void *) fd;
}

static IOR_offset_t CUBEFS_Xfer(int access, aiori_fd_t *file, IOR_size_t *buffer,
                           IOR_offset_t length, IOR_offset_t offset, aiori_mod_opt_t *options)
{
        return 0;
}

static void CUBEFS_Fsync(aiori_fd_t *file, aiori_mod_opt_t *options)
{
        return;
}

static void CUBEFS_Close(aiori_fd_t *file, aiori_mod_opt_t *options)
{
        return;
}

static void CUBEFS_Delete(char *path, aiori_mod_opt_t *options)
{
        return;
}

static IOR_offset_t CUBEFS_GetFileSize(aiori_mod_opt_t *options, char *path)
{
        return 0;
}

static int CUBEFS_StatFS(const char *path, ior_aiori_statfs_t *stat_buf, aiori_mod_opt_t *options)
{
        return 0;
}

static int CUBEFS_MkDir(const char *path, mode_t mode, aiori_mod_opt_t *options)
{
        return 0;
}

static int CUBEFS_RmDir(const char *path, aiori_mod_opt_t *options)
{
        return 0;
}

static int CUBEFS_Access(const char *path, int mode, aiori_mod_opt_t *options)
{
        return 0;
}

static int CUBEFS_Stat(const char *path, struct stat *buf, aiori_mod_opt_t *options)
{
        return 0;
}

static void CUBEFS_Sync(aiori_mod_opt_t *options)
{
        return;

}
