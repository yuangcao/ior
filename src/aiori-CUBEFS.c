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
#include "cJSON.h"

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
      {0, "cubefs.conf", "Config file path for the cubefs client", OPTION_OPTIONAL_ARGUMENT, 's', & o.conf},
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
static int64_t cubefs_client_id;

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

char* read_file(const char *filename) {
    FILE *file = NULL;
    long length = 0;
    char *content = NULL;
    size_t read_chars = 0;

    /* open in read binary mode */
    file = fopen(filename, "rb");
    if (file == NULL)
    {
        goto cleanup;
    }

    /* get the length */
    if (fseek(file, 0, SEEK_END) != 0)
    {
        goto cleanup;
    }
    length = ftell(file);
    if (length < 0)
    {
        goto cleanup;
    }
    if (fseek(file, 0, SEEK_SET) != 0)
    {
        goto cleanup;
    }

    /* allocate content buffer */
    content = (char*)malloc((size_t)length + sizeof(""));
    if (content == NULL)
    {
        goto cleanup;
    }

    /* read the file into memory */
    read_chars = fread(content, sizeof(char), (size_t)length, file);
    if ((long)read_chars != length)
    {
        free(content);
        content = NULL;
        goto cleanup;
    }
    content[read_chars] = '\0';


cleanup:
    if (file != NULL)
    {
        fclose(file);
    }

    return content;
}

static cJSON *parse_file(const char *filename)
{
    cJSON *parsed = NULL;
    char *content = read_file(filename);

    parsed = cJSON_Parse(content);

    if (content != NULL)
    {
        free(content);
    }

    return parsed;
}

static void CUBEFS_Init()
{
        // printf("CUBEFS_Init\n");
        cubefs_client_id = cfs_new_client();
        
        int statusVal;
        cJSON *tree = NULL;
        tree = parse_file(o.conf);
        if (tree->type != cJSON_Object) { printf("[ERROR] file content is not a json object.\n"); }

        cJSON *current_item = tree->child;
        while (current_item) {
                if (current_item->type != cJSON_String) { printf("[ERROR] json item's type must be string.\n"); }

                statusVal = cfs_set_client(cubefs_client_id, current_item->string, current_item->valuestring);
                if (statusVal != 0) { printf("Error\n"); CUBEFS_ERR("Error in cfs_set_client", statusVal); }

                current_item = current_item->next;
        }
        if (tree != NULL) {
                cJSON_Delete(tree);
        }

        statusVal = cfs_start_client(cubefs_client_id);
        if (statusVal != 0) { printf("Error 8\n"); CUBEFS_ERR("Unable to start client", statusVal); }
        
        // statusVal = cfs_open(cubefs_client_id, "/test.txt.6", O_CREAT | O_WRONLY, 0664);
        // if (statusVal < 0) { printf("Error 9, statusVal = %d\n", statusVal); CUBEFS_ERR("Unable to open file", statusVal); }

        return;
}

static void CUBEFS_Final()
{
        // printf("CUBEFS_Final\n");
        cfs_close_client(cubefs_client_id);
        return;
}

static aiori_fd_t *CUBEFS_Create(char *path, int flags, aiori_mod_opt_t *options)
{
        // printf("CUBEFS_Create: %s\n", path);
        return CUBEFS_Open(path, flags | IOR_CREAT, options);
}

static aiori_fd_t *CUBEFS_Open(char *path, int flags, aiori_mod_opt_t *options)
{
        // printf("CUBEFS_Open: path = %s\n", path);

        int* fd;
        fd = (int *)malloc(sizeof(int));

        mode_t mode = 0664;
        int cubefs_flags = (int) 0;

        /* set IOR file flags to CubeFS flags (same as POSIX flags) */
        /* -- file open flags -- */
        if (flags & IOR_RDONLY) {
                cubefs_flags |= O_RDONLY;
        }
        if (flags & IOR_WRONLY) {
                cubefs_flags |= O_WRONLY;
        }
        if (flags & IOR_RDWR) {
                cubefs_flags |= O_RDWR;
        }
        if (flags & IOR_APPEND) {
                cubefs_flags |= O_APPEND;
        }
        if (flags & IOR_CREAT) {
                cubefs_flags |= O_CREAT;
        }
        if (flags & IOR_EXCL) {
                cubefs_flags |= O_EXCL;
        }
        if (flags & IOR_TRUNC) {
                cubefs_flags |= O_TRUNC;
        }
        // if (flags & IOR_DIRECT) {
        //         cubefs_flags |= O_DIRECT;
        // }
        *fd = cfs_open(cubefs_client_id, path, cubefs_flags, mode);
        if (*fd < 0) {
                CUBEFS_ERR("cubefs_open failed", *fd);
        }

        return (void *) fd;
}

static IOR_offset_t CUBEFS_Xfer(int access, aiori_fd_t *file, IOR_size_t *buffer,
                           IOR_offset_t length, IOR_offset_t offset, aiori_mod_opt_t *options)
{
        // printf("CUBEFS_Xfer\n");
        return 0;
}

static void CUBEFS_Fsync(aiori_fd_t *file, aiori_mod_opt_t *options)
{
        // printf("CUBEFS_Fsync\n");
        return;
}

static void CUBEFS_Close(aiori_fd_t *file, aiori_mod_opt_t *options)
{
        // printf("CUBEFS_Close\n");
        int fd = *(int *) file;
        cfs_close(cubefs_client_id, fd);
        free(file);
        return;
}

static void CUBEFS_Delete(char *path, aiori_mod_opt_t *options)
{
        printf("CUBEFS_Delete: %s\n", path);
        int statusVal = cfs_unlink(cubefs_client_id, path);
        if (statusVal != 0) { printf("Error in CUBEFS_Delete\n"); CUBEFS_ERR("Error occured when deleting a file", statusVal); }
        return;
}

static IOR_offset_t CUBEFS_GetFileSize(aiori_mod_opt_t *options, char *path)
{
        // printf("CUBEFS_GetFileSize: %s\n", path);
        // This method is only called by ior.
        // Not implentmented.
        return 0;
}

static int CUBEFS_StatFS(const char *path, ior_aiori_statfs_t *stat_buf, aiori_mod_opt_t *options)
{
        // printf("CUBEFS_StatFS: %s\n", path);
        return 0;
}

static int CUBEFS_MkDir(const char *path, mode_t mode, aiori_mod_opt_t *options)
{
        printf("CUBEFS_MkDir: %s\n", path);
        return cfs_mkdirs(cubefs_client_id, path, mode); // not sure about the return value. In mdtest.c, -1 is the error return value.
}

static int CUBEFS_RmDir(const char *path, aiori_mod_opt_t *options)
{
        printf("CUBEFS_RmDir: %s\n", path);
        return cfs_rmdir(cubefs_client_id, path); // not sure about the return value. In mdtest.c, -1 is the error return value.
}

static int CUBEFS_Access(const char *path, int mode, aiori_mod_opt_t *options)
{
        // printf("CUBEFS_Access: %s\n", path);
        struct cfs_stat_info buf;
        return cfs_getattr(cubefs_client_id, path, &buf);
}

static int CUBEFS_Stat(const char *path, struct stat *buf, aiori_mod_opt_t *options)
{
        // printf("CUBEFS_Stat: %s\n", path);
        struct cfs_stat_info cfs_buf;
        int statusVal = cfs_getattr(cubefs_client_id, path, &cfs_buf);
        // int statusVal = cfs_getattr(cubefs_client_id, path, buf);
        
        // transfer cfs_stat_info to posix stat
        buf->st_atime = cfs_buf.atime;
        // buf->st_atimensec = cfs_buf.atime_nsec;
        buf->st_blksize = cfs_buf.blk_size;
        buf->st_blocks = cfs_buf.blocks;
        buf->st_ctime = cfs_buf.ctime;
        // buf->st_ctimensec = cfs_buf.ctime_nsec;
        // buf->st_dev; // ??
        buf->st_gid = cfs_buf.gid;
        buf->st_ino = cfs_buf.ino;
        buf->st_mode = cfs_buf.mode;
        buf->st_mtime = cfs_buf.mtime;
        // buf->st_mtimensec = cfs_buf.mtime_nsec;
        buf->st_nlink = cfs_buf.nlink;
        // buf->st_rdev; // ??
        buf->st_size = cfs_buf.size;
        buf->st_uid = cfs_buf.uid;
        
        return statusVal;
}

static void CUBEFS_Sync(aiori_mod_opt_t *options)
{
        // printf("CUBEFS_Sync\n");
        return;
}