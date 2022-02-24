#ifndef FLINT_PARSER_H
#define FLINT_PARSER_H

#include "conf_parser.h"
#include <cjson/cJSON.h>

enum METHOD {
	GET,
	PUT,
	POST,
	DELETE,
	PATCH
};

enum DIRECTION {
	UP,
	DOWN
};

cJSON* flint_build_tree(void);
int flint_append_device_ctrl_from_chirpstack(cJSON* flint_tree, cJSON* original_tree);
int flint_append_input_ctrl(cJSON* flint_tree, cJSON* original_tree, _configuration* config);
int flint_append_adapter_ctrl(cJSON* flint_tree, _configuration* config);
int flint_set_method(cJSON* flint_tree, enum METHOD method);
enum METHOD flint_get_method(cJSON* flint_tree);
int flint_set_direction(cJSON* flint_tree, enum DIRECTION direction);

#endif