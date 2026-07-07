#include "cli/cli.h"

#include "foundation/compat_fs.h"
#include "foundation/constants.h"
#include "sqlite3.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct cbm_config {
    sqlite3 *db;
    char get_buf[CBM_SZ_4K];
};

static const char *g_version = "dev";

static char *dup_text(const char *s) {
    if (!s) {
        s = "";
    }
    size_t n = strlen(s) + 1;
    char *out = malloc(n);
    if (out) {
        memcpy(out, s, n);
    }
    return out;
}

static void set_error(char **err_out, const char *msg) {
    if (err_out) {
        *err_out = dup_text(msg);
    }
}

void cbm_cli_set_version(const char *ver) {
    g_version = (ver && *ver) ? ver : "dev";
}

const char *cbm_cli_get_version(void) {
    return g_version;
}

char *cbm_cli_build_args_json(const char *tool_name, int argc, char **argv, char **err_out) {
    (void)tool_name;
    if (err_out) {
        *err_out = NULL;
    }
    if (argc == 0) {
        return dup_text("{}");
    }
    if (argc == 1 && argv && argv[0] && (argv[0][0] == '{' || argv[0][0] == '[')) {
        return dup_text(argv[0]);
    }
    set_error(err_out, "code-index cli expects a single JSON argument object");
    return NULL;
}

int cbm_cli_print_tool_help(const char *tool_name) {
    static const char *tools[] = {
        "index_repository", "index_status",   "list_projects",    "delete_project",
        "search_graph",     "search_code",    "trace_path",       "detect_changes",
        "query_graph",      "get_graph_schema", "get_code_snippet", "get_architecture",
        "manage_adr",       "ingest_traces",  NULL,
    };
    if (!tool_name || !*tool_name) {
        return 1;
    }
    for (int i = 0; tools[i]; i++) {
        if (strcmp(tool_name, tools[i]) == 0) {
            printf("Usage: codebase-memory-mcp cli %s '<json-args>'\n", tool_name);
            printf("Arguments must be a single JSON object, for example: {}\n");
            return 0;
        }
    }
    return 1;
}

cbm_config_t *cbm_config_open(const char *cache_dir) {
    if (!cache_dir || !*cache_dir) {
        return NULL;
    }
    if (!cbm_mkdir_p(cache_dir, 0755)) {
        return NULL;
    }

    char path[CBM_SZ_4K];
    int n = snprintf(path, sizeof(path), "%s/_config.db", cache_dir);
    if (n <= 0 || n >= (int)sizeof(path)) {
        return NULL;
    }

    cbm_config_t *cfg = calloc(1, sizeof(*cfg));
    if (!cfg) {
        return NULL;
    }
    if (sqlite3_open(path, &cfg->db) != SQLITE_OK) {
        cbm_config_close(cfg);
        return NULL;
    }
    const char *sql = "CREATE TABLE IF NOT EXISTS config (key TEXT PRIMARY KEY, value TEXT NOT NULL)";
    if (sqlite3_exec(cfg->db, sql, NULL, NULL, NULL) != SQLITE_OK) {
        cbm_config_close(cfg);
        return NULL;
    }
    return cfg;
}

void cbm_config_close(cbm_config_t *cfg) {
    if (!cfg) {
        return;
    }
    if (cfg->db) {
        sqlite3_close(cfg->db);
    }
    free(cfg);
}

const char *cbm_config_get(cbm_config_t *cfg, const char *key, const char *default_val) {
    if (!cfg || !cfg->db || !key) {
        return default_val;
    }
    sqlite3_stmt *stmt = NULL;
    const char *sql = "SELECT value FROM config WHERE key = ?";
    if (sqlite3_prepare_v2(cfg->db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return default_val;
    }
    sqlite3_bind_text(stmt, 1, key, -1, SQLITE_STATIC);
    const char *result = default_val;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        const unsigned char *val = sqlite3_column_text(stmt, 0);
        if (val) {
            snprintf(cfg->get_buf, sizeof(cfg->get_buf), "%s", (const char *)val);
            result = cfg->get_buf;
        }
    }
    sqlite3_finalize(stmt);
    return result;
}

bool cbm_config_get_bool(cbm_config_t *cfg, const char *key, bool default_val) {
    const char *val = cbm_config_get(cfg, key, NULL);
    if (!val) {
        return default_val;
    }
    return strcmp(val, "true") == 0 || strcmp(val, "1") == 0 || strcmp(val, "on") == 0;
}

int cbm_config_get_int(cbm_config_t *cfg, const char *key, int default_val) {
    const char *val = cbm_config_get(cfg, key, NULL);
    if (!val || !*val) {
        return default_val;
    }
    char *end = NULL;
    long parsed = strtol(val, &end, 10);
    if (!end || *end != '\0') {
        return default_val;
    }
    return (int)parsed;
}

int cbm_config_set(cbm_config_t *cfg, const char *key, const char *value) {
    if (!cfg || !cfg->db || !key || !value) {
        return -1;
    }
    sqlite3_stmt *stmt = NULL;
    const char *sql = "INSERT OR REPLACE INTO config(key,value) VALUES(?,?)";
    if (sqlite3_prepare_v2(cfg->db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return -1;
    }
    sqlite3_bind_text(stmt, 1, key, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, value, -1, SQLITE_STATIC);
    int rc = sqlite3_step(stmt) == SQLITE_DONE ? 0 : -1;
    sqlite3_finalize(stmt);
    return rc;
}

int cbm_config_delete(cbm_config_t *cfg, const char *key) {
    if (!cfg || !cfg->db || !key) {
        return -1;
    }
    sqlite3_stmt *stmt = NULL;
    const char *sql = "DELETE FROM config WHERE key = ?";
    if (sqlite3_prepare_v2(cfg->db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return -1;
    }
    sqlite3_bind_text(stmt, 1, key, -1, SQLITE_STATIC);
    int rc = sqlite3_step(stmt) == SQLITE_DONE ? 0 : -1;
    sqlite3_finalize(stmt);
    return rc;
}
