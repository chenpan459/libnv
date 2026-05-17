#include <nv_sqlite.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

static int g_fail;

static void check(int cond, const char *msg)
{
    if (!cond) {
        fprintf(stderr, "FAIL: %s\n", msg);
        g_fail++;
    }
}

int main(void)
{
    const char *path = "/tmp/libnv_test_nv_sqlite.db";
    nv_sqlite_t *db = NULL;
    nv_sqlite_stmt_t *stmt = NULL;
    int64_t rowid;

    unlink(path);

    check(nv_sqlite_open(path, &db) == NV_OK, "open file db");
    check(nv_sqlite_exec(db, "CREATE TABLE kv (k TEXT PRIMARY KEY, v TEXT)") == NV_OK, "create table");

    check(nv_sqlite_prepare(db, "INSERT INTO kv(k,v) VALUES(?,?)", &stmt) == NV_OK, "prepare insert");
    check(nv_sqlite_bind_text(stmt, 1, "name") == NV_OK, "bind k");
    check(nv_sqlite_bind_text(stmt, 2, "libnv") == NV_OK, "bind v");
    check(nv_sqlite_step(stmt) == NV_DONE, "insert step");
    rowid = nv_sqlite_last_insert_rowid(db);
    check(rowid > 0, "last insert rowid");
    nv_sqlite_stmt_finalize(stmt);
    stmt = NULL;

    check(nv_sqlite_prepare(db, "SELECT v FROM kv WHERE k=?", &stmt) == NV_OK, "prepare select");
    check(nv_sqlite_bind_text(stmt, 1, "name") == NV_OK, "bind select key");
    check(nv_sqlite_step(stmt) == NV_OK, "select row");
    check(strcmp(nv_sqlite_column_text(stmt, 0), "libnv") == 0, "select value");
    check(nv_sqlite_step(stmt) == NV_DONE, "select done");
    nv_sqlite_stmt_finalize(stmt);
    stmt = NULL;

    check(nv_sqlite_prepare(db, "UPDATE kv SET v=? WHERE k=?", &stmt) == NV_OK, "prepare update");
    check(nv_sqlite_bind_text(stmt, 1, "libnv2") == NV_OK, "bind new v");
    check(nv_sqlite_bind_text(stmt, 2, "name") == NV_OK, "bind key");
    check(nv_sqlite_step(stmt) == NV_DONE, "update step");
    check(nv_sqlite_changes(db) == 1, "one row changed");
    nv_sqlite_stmt_finalize(stmt);
    stmt = NULL;

    nv_sqlite_close(db);
    db = NULL;

    check(nv_sqlite_open(path, &db) == NV_OK, "reopen db");
    check(nv_sqlite_prepare(db, "SELECT v FROM kv WHERE k='name'", &stmt) == NV_OK, "reload select");
    check(nv_sqlite_step(stmt) == NV_OK, "reload row");
    check(strcmp(nv_sqlite_column_text(stmt, 0), "libnv2") == 0, "persisted value");
    nv_sqlite_stmt_finalize(stmt);
    nv_sqlite_close(db);

    check(nv_sqlite_open_memory(&db) == NV_OK, "memory db");
    check(nv_sqlite_begin(db) == NV_OK, "begin");
    check(nv_sqlite_exec(db, "CREATE TABLE t(n INTEGER)") == NV_OK, "mem create");
    check(nv_sqlite_commit(db) == NV_OK, "commit");
    nv_sqlite_close(db);

    unlink(path);

    if (g_fail) {
        fprintf(stderr, "%d test(s) failed\n", g_fail);
        return 1;
    }
    printf("test_nv_sqlite: ok\n");
    return 0;
}
