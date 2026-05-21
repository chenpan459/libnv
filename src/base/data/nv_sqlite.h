#ifndef _NV_SQLITE_H_INCLUDED_
#define _NV_SQLITE_H_INCLUDED_

#ifdef __cplusplus
extern "C" {
#endif

#include <nv_config.h>
#include <nv_base.h>
#include <stddef.h>
#include <stdint.h>

struct sqlite3;
struct sqlite3_stmt;

typedef struct sqlite3 nv_sqlite_t;
typedef struct sqlite3_stmt nv_sqlite_stmt_t;

/* ---------- 连接 ---------- */

/** 打开数据库文件（不存在则创建） */
int nv_sqlite_open(const char *path, nv_sqlite_t **db);

/** 嵌入式友好打开：WAL + synchronous=NORMAL + 较小 cache */
int nv_sqlite_open_embedded(const char *path, nv_sqlite_t **db);

/** 内存数据库 */
int nv_sqlite_open_memory(nv_sqlite_t **db);

void nv_sqlite_close(nv_sqlite_t *db);

const char *nv_sqlite_errmsg(nv_sqlite_t *db);
int nv_sqlite_errno(nv_sqlite_t *db);

/* ---------- 执行 SQL ---------- */

/** 执行无结果 SQL（DDL / INSERT / UPDATE 等） */
int nv_sqlite_exec(nv_sqlite_t *db, const char *sql);

int nv_sqlite_begin(nv_sqlite_t *db);
int nv_sqlite_commit(nv_sqlite_t *db);
int nv_sqlite_rollback(nv_sqlite_t *db);

int64_t nv_sqlite_last_insert_rowid(nv_sqlite_t *db);
int nv_sqlite_changes(nv_sqlite_t *db);

/* ---------- 预编译语句 ---------- */

int nv_sqlite_prepare(nv_sqlite_t *db, const char *sql, nv_sqlite_stmt_t **stmt);
void nv_sqlite_stmt_finalize(nv_sqlite_stmt_t *stmt);

/** 单步执行：有数据行返回 NV_OK，结束返回 NV_DONE，错误返回 NV_ERROR */
int nv_sqlite_step(nv_sqlite_stmt_t *stmt);
int nv_sqlite_reset(nv_sqlite_stmt_t *stmt);

int nv_sqlite_bind_null(nv_sqlite_stmt_t *stmt, int index);
int nv_sqlite_bind_int(nv_sqlite_stmt_t *stmt, int index, int value);
int nv_sqlite_bind_int64(nv_sqlite_stmt_t *stmt, int index, int64_t value);
int nv_sqlite_bind_double(nv_sqlite_stmt_t *stmt, int index, double value);
int nv_sqlite_bind_text(nv_sqlite_stmt_t *stmt, int index, const char *value);

int nv_sqlite_column_count(nv_sqlite_stmt_t *stmt);
const char *nv_sqlite_column_name(nv_sqlite_stmt_t *stmt, int index);
int nv_sqlite_column_int(nv_sqlite_stmt_t *stmt, int index);
int64_t nv_sqlite_column_int64(nv_sqlite_stmt_t *stmt, int index);
double nv_sqlite_column_double(nv_sqlite_stmt_t *stmt, int index);
const char *nv_sqlite_column_text(nv_sqlite_stmt_t *stmt, int index);

#ifdef __cplusplus
}
#endif

#endif
