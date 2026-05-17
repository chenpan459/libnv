#include "nv_ini.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

typedef struct nv_ini_entry_s {
    char *section;
    char *key;
    char *value;
    struct nv_ini_entry_s *next;
} nv_ini_entry_t;

struct nv_ini_s {
    nv_ini_entry_t *head;
    int count;
};

static char *nv_ini_strdup(const char *s)
{
    size_t n;
    char  *p;

    if (!s) {
        return NULL;
    }
    n = strlen(s);
    p = (char *)malloc(n + 1);
    if (!p) {
        return NULL;
    }
    memcpy(p, s, n + 1);
    return p;
}

static char *nv_ini_trim_inplace(char *s)
{
    char *end;

    if (!s) {
        return s;
    }
    while (*s && isspace((unsigned char)*s)) {
        s++;
    }
    if (*s == 0) {
        return s;
    }
    end = s + strlen(s) - 1;
    while (end > s && isspace((unsigned char)*end)) {
        *end-- = '\0';
    }
    return s;
}

static void nv_ini_strip_comment(char *line)
{
    char *semi = strchr(line, ';');
    if (semi) {
        *semi = '\0';
        nv_ini_trim_inplace(line);
    }
}

static int nv_ini_add_entry(nv_ini_t *ini, const char *section,
                            const char *key, const char *value)
{
    nv_ini_entry_t *entry;

    if (!ini || !key || !value) {
        return -1;
    }

    entry = (nv_ini_entry_t *)calloc(1, sizeof(*entry));
    if (!entry) {
        return -1;
    }

    entry->section = nv_ini_strdup(section ? section : "");
    entry->key     = nv_ini_strdup(key);
    entry->value   = nv_ini_strdup(value);
    if (!entry->section || !entry->key || !entry->value) {
        free(entry->section);
        free(entry->key);
        free(entry->value);
        free(entry);
        return -1;
    }

    entry->next = ini->head;
    ini->head   = entry;
    ini->count++;
    return 0;
}

static const nv_ini_entry_t *nv_ini_find(const nv_ini_t *ini,
                                         const char *section, const char *key)
{
    const nv_ini_entry_t *entry;
    const char *sec = (section && section[0]) ? section : "";

    if (!ini || !key) {
        return NULL;
    }

    for (entry = ini->head; entry; entry = entry->next) {
        const char *esec = entry->section ? entry->section : "";
        if (strcasecmp(esec, sec) == 0 && strcmp(entry->key, key) == 0) {
            return entry;
        }
    }
    return NULL;
}

nv_ini_t *nv_ini_load(const char *path)
{
    FILE           *fp;
    char            line[1024];
    char            section[128];
    nv_ini_t       *ini;

    if (!path) {
        errno = EINVAL;
        return NULL;
    }

    ini = (nv_ini_t *)calloc(1, sizeof(*ini));
    if (!ini) {
        return NULL;
    }

    fp = fopen(path, "r");
    if (!fp) {
        free(ini);
        return NULL;
    }

    section[0] = '\0';

    while (fgets(line, sizeof(line), fp)) {
        char *p = line;
        char *eq;
        char *rb;

        nv_ini_strip_comment(p);
        p = nv_ini_trim_inplace(p);
        if (*p == '\0') {
            continue;
        }

        if (*p == '[') {
            rb = strchr(p, ']');
            if (!rb) {
                continue;
            }
            *rb = '\0';
            snprintf(section, sizeof(section), "%s", p + 1);
            nv_ini_trim_inplace(section);
            continue;
        }

        eq = strchr(p, '=');
        if (!eq) {
            continue;
        }

        *eq = '\0';
        {
            char *key = nv_ini_trim_inplace(p);
            char *val = nv_ini_trim_inplace(eq + 1);
            if (*key == '\0') {
                continue;
            }
            nv_ini_add_entry(ini, section, key, val);
        }
    }

    fclose(fp);
    return ini;
}

void nv_ini_free(nv_ini_t *ini)
{
    nv_ini_entry_t *entry;
    nv_ini_entry_t *next;

    if (!ini) {
        return;
    }

    for (entry = ini->head; entry; entry = next) {
        next = entry->next;
        free(entry->section);
        free(entry->key);
        free(entry->value);
        free(entry);
    }
    free(ini);
}

int nv_ini_has_key(nv_ini_t *ini, const char *section, const char *key)
{
    return nv_ini_find(ini, section, key) != NULL;
}

const char *nv_ini_get_string(nv_ini_t *ini, const char *section,
                              const char *key, const char *default_val)
{
    const nv_ini_entry_t *entry = nv_ini_find(ini, section, key);
    return entry ? entry->value : default_val;
}

int nv_ini_get_bool(nv_ini_t *ini, const char *section,
                    const char *key, int default_val)
{
    const char *value = nv_ini_get_string(ini, section, key, NULL);

    if (!value) {
        return default_val;
    }
    if (strcasecmp(value, "true") == 0 || strcasecmp(value, "yes") == 0 ||
        strcasecmp(value, "on") == 0 || strcmp(value, "1") == 0) {
        return 1;
    }
    if (strcasecmp(value, "false") == 0 || strcasecmp(value, "no") == 0 ||
        strcasecmp(value, "off") == 0 || strcmp(value, "0") == 0) {
        return 0;
    }
    return default_val;
}

int nv_ini_get_int(nv_ini_t *ini, const char *section,
                   const char *key, int default_val)
{
    const char *value = nv_ini_get_string(ini, section, key, NULL);
    char       *end;

    if (!value) {
        return default_val;
    }
    return (int)strtol(value, &end, 0);
}

int64_t nv_ini_get_int64(nv_ini_t *ini, const char *section,
                         const char *key, int64_t default_val)
{
    const char *value = nv_ini_get_string(ini, section, key, NULL);
    char       *end;

    if (!value) {
        return default_val;
    }
    return (int64_t)strtoll(value, &end, 0);
}

double nv_ini_get_double(nv_ini_t *ini, const char *section,
                         const char *key, double default_val)
{
    const char *value = nv_ini_get_string(ini, section, key, NULL);
    char       *end;

    if (!value) {
        return default_val;
    }
    return strtod(value, &end);
}
