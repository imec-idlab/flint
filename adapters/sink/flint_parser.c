#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "flint_parser.h"

cJSON* flint_build_tree(void) {
	cJSON* tree = cJSON_CreateObject();
	cJSON* device_ctrl = cJSON_CreateObject();
	cJSON* input_ctrl = cJSON_CreateObject();
	cJSON* output_ctrl = cJSON_CreateObject();
	cJSON* adapter_ctrl = cJSON_CreateObject();

    if (device_ctrl == NULL || input_ctrl == NULL || output_ctrl == NULL || adapter_ctrl == NULL || tree == NULL) {
        goto end;
    }

    cJSON_AddItemToObject(tree, "device-ctrl", device_ctrl); // also transfer pointer ownership
    cJSON_AddItemToObject(tree, "input-ctrl", input_ctrl); // also transfer pointer ownership
    cJSON_AddItemToObject(tree, "output-ctrl", output_ctrl); // also transfer pointer ownership
    cJSON_AddItemToObject(tree, "adapter-ctrl", adapter_ctrl); // also transfer pointer ownership

    return tree;

end:
	cJSON_Delete(tree);
	cJSON_Delete(input_ctrl);
	cJSON_Delete(output_ctrl);
	cJSON_Delete(device_ctrl);
	cJSON_Delete(adapter_ctrl);

	return NULL;
}

int flint_append_device_ctrl_from_chirpstack(cJSON* flint_tree, cJSON* original_tree) {
	cJSON* device_ctrl = cJSON_GetObjectItem(flint_tree, "device-ctrl");
	cJSON* device_custom_ctrl = cJSON_CreateObject();

	cJSON* tx_info = cJSON_GetObjectItem(original_tree, "txInfo");
	if(tx_info == NULL) {
		goto end;
	}
	cJSON* tx = cJSON_Duplicate(tx_info, 1);
	cJSON_AddItemToObject(device_custom_ctrl, "txInfo", tx); // transfers pointer ownership, so duplicate first

	cJSON* rx_info = cJSON_GetObjectItem(original_tree, "rxInfo");
	if(rx_info == NULL) {
		goto end;
	}

	cJSON* rx = cJSON_Duplicate(rx_info, 1);
	cJSON_AddItemToObject(device_custom_ctrl, "rxInfo", rx);

	cJSON* data = cJSON_GetObjectItem(original_tree, "data");
	if(data == NULL) {
		goto end;
	}
	cJSON* d = cJSON_Duplicate(data, 1);
	cJSON_AddItemToObject(device_ctrl, "data", d);


	cJSON* dev_eui = cJSON_GetObjectItem(original_tree, "devEUI");
	if(dev_eui == NULL) {
		goto end;
	}
	
	char* eui = dev_eui->valuestring;

	cJSON* devEUI = cJSON_CreateString(eui);
	cJSON_AddItemToObject(device_ctrl, "dev-eui", devEUI);


	cJSON* fport = cJSON_GetObjectItem(original_tree, "fPort");
	if(fport == NULL) {
		goto end;
	}
	cJSON* fp = cJSON_Duplicate(fport, 1);
	cJSON_AddItemToObject(device_ctrl, "fPort", fp);

    cJSON_AddItemToObject(device_ctrl, "device-custom-ctrl", device_custom_ctrl); // also transfer pointer ownership

	return 0; // do not enter end

end:
	cJSON_Delete(device_custom_ctrl);
	return -1;
}


int flint_append_input_ctrl(cJSON* flint_tree, cJSON* original_tree, _configuration* config) {
	cJSON* input_ctrl = cJSON_GetObjectItem(flint_tree, "input-ctrl");

    cJSON* input_type = cJSON_CreateString(config->agent.name);
    if (input_type != NULL) {
    	cJSON_AddItemToObject(input_ctrl, "input-type", input_type); // also transfer pointer ownership
    }

    cJSON* ip = cJSON_CreateString(config->agent.ip);
    if (ip != NULL) {
    	cJSON_AddItemToObject(input_ctrl, "input-ip", ip);
    }

    cJSON* port = cJSON_CreateNumber(config->agent.port);
    if (port != NULL) {
    	cJSON_AddItemToObject(input_ctrl, "input-port", port);
    }

    cJSON* protocol = cJSON_CreateString(config->agent.protocol);
    if (protocol != NULL) {
    	cJSON_AddItemToObject(input_ctrl, "input-protocol", protocol);
    }

    return 0; // do not enter end

end:
	cJSON_Delete(input_ctrl);
	return -1;
}

int flint_set_method(cJSON* flint_tree, enum METHOD method) {
	cJSON* output_ctrl = cJSON_GetObjectItem(flint_tree, "output-ctrl");

	cJSON* output_method;
	switch(method) {
		case GET:
    		output_method = cJSON_CreateString("GET");
    	break;
		case PUT:
    		output_method = cJSON_CreateString("PUT");
    	break;
		case POST:
    		output_method = cJSON_CreateString("POST");
    	break;
		case DELETE:
    		output_method = cJSON_CreateString("DELETE");
    	break;
		case PATCH:
    		output_method = cJSON_CreateString("PATCH");
    	break;
    	default:
    		output_method = cJSON_CreateString("POST");
    	break;
	}

	if (output_method != NULL) {
    	cJSON_AddItemToObject(output_ctrl, "method", output_method);
    }

    return 0;
}

enum METHOD flint_get_method(cJSON* flint_tree) {
	enum METHOD m = -1;

	cJSON* output_ctrl = cJSON_GetObjectItem(flint_tree, "output-ctrl");

    cJSON* method = cJSON_GetObjectItem(output_ctrl, "method");
    char* method_string = method->valuestring;

    if(strcmp(method_string, "GET") == 0) {
    	m = GET;
    } else if(strcmp(method_string, "PUT") == 0) {
    	m = PUT;
    } else if(strcmp(method_string, "POST") == 0) {
    	m = POST;
    } else if(strcmp(method_string, "DELETE") == 0) {
    	m = DELETE;
    } else if(strcmp(method_string, "PATCH") == 0) {
    	m = PATCH;
    }
	
	return m;
}

int flint_append_adapter_ctrl(cJSON* flint_tree, _configuration* config) {
	cJSON* input_ctrl = cJSON_GetObjectItem(flint_tree, "adapter-ctrl");

    cJSON* id = cJSON_CreateString(config->sink.id);
    if (id != NULL) {
    	cJSON_AddItemToObject(input_ctrl, "from", id);
    }

    return 0;
}