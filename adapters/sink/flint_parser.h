#ifndef FLINT_PARSER_H
#define FLINT_PARSER_H

#include "conf_parser.h"
#include <cjson/cJSON.h>

cJSON* flint_build_tree(void);
void flint_append_device_ctrl_from_chirpstack(cJSON* flint_tree, cJSON* original_tree);
void flint_append_input_ctrl(cJSON* flint_tree, cJSON* original_tree, _configuration* config);
void flint_append_adapter_ctrl(cJSON* flint_tree, _configuration* config);

#endif