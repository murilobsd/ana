/*
 * Copyright (c) 2019 Murilo Ijanc' <mbsd@m0x.ru>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */
#include <sys/queue.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <dirent.h>
#include <err.h>
#include <errno.h>
#include <fnmatch.h>
#include <limits.h>
#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <lowdown.h>

#include "ana.h"

FILE *fp1, *fp2, *fp3;
int numfiles, status;
struct stat src, dst;
struct lines *ignore_file, *cache_file;

__dead void usage(void);
void *xmalloc(size_t);
static char *remove_slash(char *);
void push_line(char *, int);
void read_file(char *, int);
void read_all_files(char *, char *);
static void init_mkdir(const char *, mode_t);
static int selectfile(const struct dirent *);
void process(char *, char *);
void process_html(char *, char *);
void process_md(char *, char *);
void copy_asset(char *, char *);
static int check_ext(char *, const char *);
static int check_f(char *, int);
void fname_html(char *, char *);

int
main(int argc, char *argv[])
{
        clock_t t;
        double time_taken;

	argc -= 1;
	argv += 1;
#ifndef __OpenBSD__
	if (pledge("stdio rpath cpath wpath", NULL) == -1)
		err(2, "pledge");
#endif        
	if (argc != 2)
		usage();
	if (stat(argv[0], &src) != 0)
		err(2, "%s", argv[0]);
	if (stat(argv[1], &dst) != 0)
		err(2, "%s", argv[1]);
	if (S_ISDIR(src.st_mode) && S_ISDIR(dst.st_mode)) {
	        printf("[ana] %s --> %s\n", argv[0], argv[1]);
                argv[0] = remove_slash(argv[0]);
                argv[1] = remove_slash(argv[1]);
                read_all_files(argv[0], argv[1]);
                t = clock();
		process(argv[0], argv[1]);
                t = clock() - t;
                time_taken = (double)t / CLOCKS_PER_SEC;
                printf("Processing %d on %.2f seconds\n", numfiles, time_taken);

                if (fp1 != NULL) fclose(fp1);
                if (fp2 != NULL) fclose(fp2);
                if (fp3 != NULL) fclose(fp3);
        } else
		errx(2, "please pass directories");

	return (status);
}

static int
check_ext(char *p, const char *ext) {
        const char *fname = basename(p);
        const char *dot = strrchr(fname, '.');

        if(!dot || dot == fname) return 0;
        if (strcmp(dot+1, ext) == 0)
                return 1;
        return 0;
}

void
process(char *p1, char *p2)
{
        struct dirent *dent1, **dp1, **edp1, **dirp1 = NULL;
        size_t dirl1, dirl2; 
        char path1[PATH_MAX], path2[PATH_MAX];
        int pos;

        dirl1 = strlcpy(path1, *p1 ? p1 : ".", sizeof(path1));
        if (dirl1 >= sizeof(path1) - 1) {
                warnc(ENAMETOOLONG, "%s", p1);
                status |= 2;
                return;
        }
        if(path1[dirl1 - 1] != '/') {
                path1[dirl1++] = '/';
                path1[dirl1] = '\0';
        }


        dirl2 = strlcpy(path2, *p2 ? p2 : ".", sizeof(path1));
        if (dirl2 >= sizeof(path2) - 1) {
                warnc(ENAMETOOLONG, "%s", p1);
                status |= 2;
                return;
        }
        if(path2[dirl2 - 1] != '/') {
                path2[dirl2++] = '/';
                path2[dirl2] = '\0';
        }

        pos = scandir(path1, &dirp1, selectfile, alphasort);
        if (pos == -1) {
                if (errno == ENOENT)
                        pos = 0;
                else {
                        warn("%s", path1);
                        goto closem;
                }
        }
        dp1 = dirp1;
        edp1 = dirp1 + pos;

        //printf("[ana] Directory: %s --> %s\n", path1, path2);

        while (dp1 != edp1) {
                dent1 = dp1 != edp1 ? *dp1 : NULL;
                strlcpy(path1 + dirl1, dent1->d_name, PATH_MAX - dirl1);
                strlcpy(path2 + dirl2, dent1->d_name, PATH_MAX - dirl2);

                if (stat(path1, &src) != 0) {
                        if (errno != ENOENT)
                                warn("%s", path1);
                        memset(&src, 0, sizeof(src));
                }
                if (S_ISDIR(src.st_mode)) {
                        if (check_f(path1, 0) == 1) {
                                init_mkdir(path2, 0777);
                                process(path1, path2);
                        }
                }
                if (!S_ISDIR(src.st_mode) && S_ISREG(src.st_mode)) {
                        if (check_f(path1, 0) == 1) {
                                //printf("[ana]: processing %s --> %s\n", path1, path2);
                                if (check_ext(path1, "html") == 1) {
                                        process_html(path1, path2); 
                                        numfiles++;
                                } else if(check_ext(path1, "md") == 1) {
                                        process_md(path1, path2); 
                                        numfiles++;
                                } else
                                        copy_asset(path1, path2);
                        }
                }
                memset(&src, 0, sizeof(src));
                dp1++;
        }

       /* for (excl = ignore_file; excl != NULL; excl = excl->next)
                printf("%s\n", excl->line);
        */

closem:
        if (dirp1 != NULL) {
                for (dp1 = dirp1; dp1 < edp1; dp1++)
                        free(*dp1);
                free(dirp1);
        }
}

// TODO: fix size output
void
fname_html(char *p, char *out) 
{
        const char *dot = strrchr(p, '.');

        if(!dot) return;
        snprintf(out, 1024, "%.*s.html", (int)(dot - p), p);
        //printf("Filename: %.*s\n", (int)(dot - p), p);
        //printf("Filename: %s\n", out);
}

static int
check_f(char *p, int n) {
        struct lines *excl;
        if (n == 0) {
                /* ignore */
                for (excl = ignore_file; excl != NULL; excl = excl->next) {
                        if (strcmp(excl->line, p) == 0 ||
                            strcmp(excl->line, dirname(p)) == 0)
                                return (0);
                }
                return (1);
         }
                /* cache */
                for (excl = cache_file; excl != NULL; excl = excl->next)
                        if  (strcmp(excl->line, p) != 0) 
                                return (0);
                return (1);
}

void process_md(char *p1, char *p2)
{
        FILE *f1, *f2;
        char ch;
        char *ret = NULL;
        char out[PATH_MAX];
        size_t  i, retsz = 0, msz = 0, size = 0;
        struct lowdown_opts opts;
        struct lowdown_meta *m = NULL;

        memset(&opts, 0, sizeof(struct lowdown_opts));
        opts.type = LOWDOWN_HTML;
        opts.feat = LOWDOWN_METADATA | LOWDOWN_AUTOLINK;
        opts.oflags = LOWDOWN_HTML_SKIP_HTML;

        fname_html(p2, out);
        //printf("Filename: %s\n", out);

        if ((f1 = fopen(p1, "r")) == NULL)
                err(2, "%s", p1);
        if ((f2 = fopen(out, "w")) == NULL)
                err(2, "%s", p2);

        /* header */
        if (fp1 != NULL) {
                while ((ch = fgetc(fp1)) != EOF) {
                        fputc(ch, f2);
                }
        } else
                if (fputs(D_HEADER, f2) == EOF)
                        err(1, "fputs");
        /* content */
        lowdown_file(&opts, f1, &ret, &retsz, &m, &msz);
        fwrite(ret, size+1, retsz, f2);

         
        /* footer */
        if (fp2 != NULL)
                while ((ch = fgetc(fp2)) != EOF)
                        fputc(ch, f2);
        else
                if (fputs(D_FOOTER, f2) == EOF)
                        err(1, "fputs");
        for (i = 0; i < msz; i++) {
                free(m[i].key);
                free(m[i].value);
        }

	if (fp1 != NULL) fseek(fp1, 0, SEEK_SET);
	if (fp2 != NULL) fseek(fp2, 0, SEEK_SET);
        free(ret);
        fclose(f1);
        fclose(f2);
}

void process_html(char *p1, char *p2)
{
        FILE *f1, *f2;
        char ch;

        if ((f1 = fopen(p1, "r")) == NULL)
                err(2, "%s", p1);
        if ((f2 = fopen(p2, "w")) == NULL)
                err(2, "%s", p2);

        /* header */
        if (fp1 != NULL)
                while ((ch = fgetc(fp1)) != EOF) {
                        fputc(ch, f2);
                }
        else
                if (fputs(D_HEADER, f2) == EOF)
                        err(1, "fputs");
        /* content */
        while ((ch = fgetc(f1)) != EOF)
                fputc(ch, f2);
        /* footer */
        if (fp2 != NULL)
                while ((ch = fgetc(fp2)) != EOF)
                        fputc(ch, f2);
        else
                if (fputs(D_FOOTER, f2) == EOF)
                        err(1, "fputs");
	if (fp1 != NULL) fseek(fp1, 0, SEEK_SET);
	if (fp2 != NULL) fseek(fp2, 0, SEEK_SET);
        fclose(f1);
        fclose(f2);
}

void copy_asset(char *p1, char *p2)
{
        FILE *f1, *f2;
        char ch;

        if ((f1 = fopen(p1, "r")) == NULL)
                err(2, "%s", p1);
        if ((f2 = fopen(p2, "w")) == NULL)
                err(2, "%s", p2);

        while ((ch = fgetc(f1)) != EOF)
                fputc(ch, f2);
        
        fclose(f1);
        fclose(f2);
}

static int
selectfile(const struct dirent *dp)
{
        if (dp->d_fileno == 0)
                return (0);
        if (dp->d_name[0] == '.') 
                return (0);
        if ((strcmp(dp->d_name, "_header.html")) == 0 ||
            (strcmp(dp->d_name, "_footer.html")) == 0) 
                return (0);
        return (1);
}


void
read_all_files(char *p1, char *p2)
{
        read_file(p1, 0);       /* header file */
        read_file(p1, 1);       /* footer file */
        read_file(p1, 2);       /* footer file */
        read_file(p2, 3);       /* footer file */
}

void
read_file(char *p1, int n)
{
        FILE *fp;
        int ret;
        char *buf, *lbuf;
        char file[PATH_MAX + 11];
        size_t len;

        if (n != 0 && n != 1 && n != 2 && n != 3)
                err(2, "read file not accepted n = %d", n);
        if (n == 0) {
                ret = snprintf(file, sizeof(file), D_HEADER_F, p1);
                if (ret < 0 || ret >= sizeof(file))
                        err(2, "ignore file path toolong");
                if ((fp1 = fopen(file, "r")) == NULL) {
                        warnx("not found: %s", file);
                        return;
                }
                printf("[ana] read header file\n");
        } else if (n == 1) {
                ret = snprintf(file, sizeof(file), D_FOOTER_F, p1);
                if (ret < 0 || ret >= sizeof(file))
                        err(2, "ignore file path toolong");
                if ((fp2 = fopen(file, "r")) == NULL) {
                        warnx("not found: %s", file);
                        return;
                }
                printf("[ana] read footer file\n");
        } else if (n == 2) {
                ret = snprintf(file, sizeof(file), D_IGNORE_F, p1);
                if (ret < 0 || ret >= sizeof(file))
                        err(2, "ignore file path toolong");
                if ((fp = fopen(file, "r")) == NULL) {
                        warnx("not found: %s", file);
                        return;
                }
                while ((buf = fgetln(fp, &len)) != NULL) {
                        if (buf[len - 1] == '\n')
                                len--;
                        lbuf = xmalloc(len + 1);
                        memcpy(lbuf, buf, len);
                        lbuf[len] = '\0';
                        push_line(lbuf, n);
                }
                printf("[ana] read ignore file\n");
                fclose(fp);
        } else {
                ret = snprintf(file, sizeof(file), D_CACHE_F, p1);
                if (ret < 0 || ret >= sizeof(file))
                        err(2, "ignore file path toolong");
                if ((fp = fopen(file, "r")) == NULL) {
                        warnx("not found: %s", file);
                        return;
                }
                while ((buf = fgetln(fp, &len)) != NULL) {
                        if (buf[len - 1] == '\n')
                                len--;
                        lbuf = xmalloc(len + 1);
                        memcpy(lbuf, buf, len);
                        lbuf[len] = '\0';
                        push_line(lbuf, n);
                }
                printf("[ana] read cache file\n");
                fclose(fp);
        }
}

void
push_line(char *line, int n)
{
        struct lines *entry;
        if (n != 2 && n != 3)
                err(2, "push line not aceppted n = %d", n);
        
        entry = xmalloc(sizeof(*entry));
        entry->line = line;
        if (n == 2) {
                entry->next = ignore_file;
                ignore_file = entry;
        } else {
                entry->next = cache_file;
                cache_file = entry;
        }
}

static void
init_mkdir(const char *path, mode_t mode)
{
        struct stat sb;

         if (stat(path, &sb) == -1) {
                   if (errno != ENOENT)
                              return;
                     if (mkdir(path, mode | S_IWUSR | S_IXUSR) == -1)
                                return;
                      }
          else if (!S_ISDIR(sb.st_mode))
                    return;

           return;

}

static char *
remove_slash(char *p)
{
        size_t dirlen;

        dirlen = strlen(p);
        if (dirlen > 1 && p[dirlen - 1] == '/') {
               p[dirlen - 1] = '\0'; 
        }
        return (p);
}

__dead void
usage(void)
{
	(void)fprintf(stderr, "usage: ana src dst\n");
	exit(2);
}
