/*
 * xpath.c - Path and file tools.
 */
#include <limits.h>
#include <logger/logger.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <xpath/xpath.h>

/*
 * Test if file exist using the effective user.
 */
int xpath_file_exist(const char * fn)
{
    FILE * fp;
    fp = fopen(fn, "r");
    if (fp == NULL)
    {
        return 0;
    }
    fclose(fp);
    return 1;
}

/*
 * Test if a path is a directory
 */
int xpath_is_dir(const char * path)
{
    struct stat st;
    memset(&st, 0, sizeof(struct stat));
    stat(path, &st);
    return S_ISDIR(st.st_mode);
}

/*
 * Returns the length of the content for a file and set buffer with the file
 * content. Note that malloc is used to allocate memory for the buffer.
 *
 * In case of an error -1 is returned and buffer will be set to NULL.
 */
ssize_t xpath_get_content(char ** buffer, const char * fn)
{
    ssize_t size = 0;
    *buffer = NULL;

    FILE * fp = fopen(fn, "r");
    if (fp == NULL)
    {
        log_critical("Cannot open file: '%s'", fn);
        return -1;
    }

    if (fseeko(fp, 0, SEEK_END) == 0 &&
        (size = ftello(fp)) != -1 &&
        fseeko(fp, 0, SEEK_SET) == 0)
    {
        *buffer = malloc(size);
        if (*buffer != NULL)
        {
            if (fread(*buffer, size, 1, fp) != 1)
            {
                log_critical("Could not get full content from '%s'", fn);
                free(*buffer);
                *buffer = NULL;
            }
        }
    }

    fclose(fp);
    return (*buffer == NULL) ? -1 : size;
}

/*
 * Get the current executable path.
 * (path should at least have size XPATH_MAX)
 *
 * Returns 0 if successful or -1 in case of an error.
 * (this functions writes logging in case of errors)
 */
int xpath_get_exec_path(char * path)
{
    char* path_end;

    if (readlink("/proc/self/exe", path, XPATH_MAX) == -1)
    {
        log_critical("Cannot read executable path");
        return -1;
    }

    /* find last / in path */
    path_end = strrchr(path, '/');

    if (path_end == NULL)
    {
        log_critical("Cannot find / in executable path");
        return -1;
    }

    *(++path_end) = '\0';

    return 0;
}

int xpath_rmdir(const char * path)
{
    DIR * d = opendir(path);
    if (!d)
        return -1;

    size_t bufsz = 0, path_len = strlen(path);
    const char * slash = (path[path_len - 1] == '/') ? "" : "/";
    struct dirent * p;
    char * buf = NULL;

    while ((p = readdir(d)))
    {
        size_t len;

        /* Skip the names "." and ".." as we don't want to recurse on them. */
        if (!strcmp(p->d_name, ".") || !strcmp(p->d_name, ".."))
            continue;

        len = path_len + strlen(p->d_name) + 2;
        if (len > bufsz)
        {
            bufsz = len;
            char * tmp = realloc(buf, bufsz);
            if (!tmp) goto stop;
            buf = tmp;
        }

        sprintf(buf, "%s%s%s", path, slash, p->d_name);

        if (xpath_is_dir(buf) ? xpath_rmdir(buf) : unlink(buf))
            goto stop;
    }

stop:
    free(buf);
    closedir(d);

    return rmdir(path);
}
