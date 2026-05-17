#include "nv_sqlite.h"

#include "sqlite3.h"

static int nv_sqlite_map_rc(int rc)
{
    if (rc == SQLITE_OK || rc == SQLITE_ROW) {
        return NV_OK;
    }
    if (rc == SQLITE_DONE) {
        return NV_DONE;
    }
    return NV_ERROR;
}

int nv_sqlite_open(const char *path, nv_sqlite_t **db)
{
    int rc;

    if (path == NULL || db == NULL) {
        return NV_ERROR;
    }
    *db = NULL;
    rc = sqlite3_open(path, db);
    return nv_sqlite_map_rc(rc);
}

int nv_sqlite_open_memory(nv_sqlite_t **db)
{
    return nv_sqlite_open(":memory:", db);
}

void nv_sqlite_close(nv_sqlite_t *db)
{
    if (db != NULL) {
        sqlite3_close(db);
    }
}

const char *nv_sqlite_errmsg(nv_sqlite_t *db)
{
    if (db == NULL) {
        return "invalid database handle";
    }
    return sqlite3_errmsg(db);
}

int nv_sqlite_errno(nv_sqlite_t *db)
{
    if (db == NULL) {
        return SQLITE_ERROR;
    }
    return sqlite3_errcode(db);
}

int nv_sqlite_exec(nv_sqlite_t *db, const char *sql)
{
    char *errmsg = NULL;
    int rc;

    if (db == NULL || sql == NULL) {
        return NV_ERROR;
    }
    rc = sqlite3_exec(db, sql, NULL, NULL, &errmsg);
    if (rc != SQLITE_OK && errmsg != NULL) {
        sqlite3_free(errmsg);
    }
    return nv_sqlite_map_rc(rc);
}

int nv_sqlite_begin(nv_sqlite_t *db)
{
    return nv_sqlite_exec(db, "BEGIN");
}

int nv_sqlite_commit(nv_sqlite_t *db)
{
    return nv_sqlite_exec(db, "COMMIT");
}

int nv_sqlite_rollback(nv_sqlite_t *db)
{
    return nv_sqlite_exec(db, "ROLLBACK");
}

int64_t nv_sqlite_last_insert_rowid(nv_sqlite_t *db)
{
    if (db == NULL) {
        return 0;
    }
    return sqlite3_last_insert_rowid(db);
}

int nv_sqlite_changes(nv_sqlite_t *db)
{
    if (db == NULL) {
        return 0;
    }
    return sqlite3_changes(db);
}

int nv_sqlite_prepare(nv_sqlite_t *db, const char *sql, nv_sqlite_stmt_t **stmt)
{
    int rc;

    if (db == NULL || sql == NULL || stmt == NULL) {
        return NV_ERROR;
    }
    *stmt = NULL;
    rc = sqlite3_prepare_v2(db, sql, -1, stmt, NULL);
    return nv_sqlite_map_rc(rc);
}

void nv_sqlite_stmt_finalize(nv_sqlite_stmt_t *stmt)
{
    if (stmt != NULL) {
        sqlite3_finalize(stmt);
    }
}

int nv_sqlite_step(nv_sqlite_stmt_t *stmt)
{
    if (stmt == NULL) {
        return NV_ERROR;
    }
    return nv_sqlite_map_rc(sqlite3_step(stmt));
}

int nv_sqlite_reset(nv_sqlite_stmt_t *stmt)
{
    if (stmt == NULL) {
        return NV_ERROR;
    }
    return nv_sqlite_map_rc(sqlite3_reset(stmt));
}

int nv_sqlite_bind_null(nv_sqlite_stmt_t *stmt, int index)
{
    if (stmt == NULL) {
        return NV_ERROR;
    }
    return nv_sqlite_map_rc(sqlite3_bind_null(stmt, index));
}

int nv_sqlite_bind_int(nv_sqlite_stmt_t *stmt, int index, int value)
{
    if (stmt == NULL) {
        return NV_ERROR;
    }
    return nv_sqlite_map_rc(sqlite3_bind_int(stmt, index, value));
}

int nv_sqlite_bind_int64(nv_sqlite_stmt_t *stmt, int index, int64_t value)
{
    if (stmt == NULL) {
        return NV_ERROR;
    }
    return nv_sqlite_map_rc(sqlite3_bind_int64(stmt, index, value));
}

int nv_sqlite_bind_double(nv_sqlite_stmt_t *stmt, int index, double value)
{
    if (stmt == NULL) {
        return NV_ERROR;
    }
    return nv_sqlite_map_rc(sqlite3_bind_double(stmt, index, value));
}

int nv_sqlite_bind_text(nv_sqlite_stmt_t *stmt, int index, const char *value)
{
    if (stmt == NULL) {
        return NV_ERROR;
    }
    if (value == NULL) {
        return nv_sqlite_bind_null(stmt, index);
    }
    return nv_sqlite_map_rc(sqlite3_bind_text(stmt, index, value, -1, SQLITE_TRANSIENT));
}

int nv_sqlite_column_count(nv_sqlite_stmt_t *stmt)
{
    if (stmt == NULL) {
        return 0;
    }
    return sqlite3_column_count(stmt);
}

const char *nv_sqlite_column_name(nv_sqlite_stmt_t *stmt, int index)
{
    if (stmt == NULL) {
        return NULL;
    }
    return sqlite3_column_name(stmt, index);
}

int nv_sqlite_column_int(nv_sqlite_stmt_t *stmt, int index)
{
    if (stmt == NULL) {
        return 0;
    }
    return sqlite3_column_int(stmt, index);
}

int64_t nv_sqlite_column_int64(nv_sqlite_stmt_t *stmt, int index)
{
    if (stmt == NULL) {
        return 0;
    }
    return sqlite3_column_int64(stmt, index);
}

double nv_sqlite_column_double(nv_sqlite_stmt_t *stmt, int index)
{
    if (stmt == NULL) {
        return 0.0;
    }
    return sqlite3_column_double(stmt, index);
}

const char *nv_sqlite_column_text(nv_sqlite_stmt_t *stmt, int index)
{
    const unsigned char *text;

    if (stmt == NULL) {
        return NULL;
    }
    text = sqlite3_column_text(stmt, index);
    return (const char *)text;
}
