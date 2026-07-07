#ifndef CBM_CLI_H
#define CBM_CLI_H

#include <stdbool.h>

void cbm_cli_set_version(const char *ver);
const char *cbm_cli_get_version(void);

char *cbm_cli_build_args_json(const char *tool_name, int argc, char **argv, char **err_out);
int cbm_cli_print_tool_help(const char *tool_name);

typedef struct cbm_config cbm_config_t;

cbm_config_t *cbm_config_open(const char *cache_dir);
void cbm_config_close(cbm_config_t *cfg);
const char *cbm_config_get(cbm_config_t *cfg, const char *key, const char *default_val);
bool cbm_config_get_bool(cbm_config_t *cfg, const char *key, bool default_val);
int cbm_config_get_int(cbm_config_t *cfg, const char *key, int default_val);
int cbm_config_set(cbm_config_t *cfg, const char *key, const char *value);
int cbm_config_delete(cbm_config_t *cfg, const char *key);

#define CBM_CONFIG_AUTO_INDEX "auto_index"
#define CBM_CONFIG_AUTO_INDEX_LIMIT "auto_index_limit"
#define CBM_CONFIG_AUTO_WATCH "auto_watch"

#endif
