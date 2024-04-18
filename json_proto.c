//
#include <ipfixcol2.h>
#include <libfds/drec.h>
#include <libfds/iemgr.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//

#define BUFF_SIZE 64

/** Plugin description */
IPX_API struct ipx_plugin_info ipx_plugin_info = {
    // Plugin type
    .type = IPX_PT_OUTPUT,
    // Plugin identification name
    .name = "json_proto",
    // Brief description of plugin
    .dsc = "This is a prototype",
    // Configuration flags (reserved for future use)
    .flags = 0,
    // Plugin version string (like "1.2.3")
    .version = "0.0.1",
    // Minimal IPFIXcol version string (like "1.2.3")
    .ipx_min = "2.0.0"
};

char *param[] = {"iana:sourceIPv4Address", "iana:destinationIPv4Address", "iana:destinationTransportPort", "iana:sourceTransportPort", "ip"};

//********************************STRUCTS************************************

struct elm_id {
	int pen;
	int id;
};

struct elm_convert{
	char *name;
	struct elm_id *elm_arr;
	size_t elm_arr_size;
};

struct instance{
	struct elm_convert* elm_inst_arr;
	size_t elm_inst_arr_size;
};

struct str_arr{
	char **matched;
	size_t matched_size;
};

//********************************FUNCTION DECLARATIONS************************************

// main init functions
int element_converter_init(struct instance *inst_ctx, char **params_arr, size_t params_arr_size, const fds_iemgr_t *iemgr); 
// init helper functions
int parse_elem(struct elm_convert *convert_arr, char *element , const fds_iemgr_t *iemgr);
int parse_alias(struct elm_convert *convert_arr, char *element , const fds_iemgr_t *iemgr);

//TODO main convertor function
int element_converter_convert();

// main cleanup finction
void element_converter_cleanup(struct instance *inst_ctx);

// othen helper functions
struct elm_id *elm_id_add(struct elm_id *arr, int position, int pen, int id);
void *struct_create(int struct_size, int num_of);

//********************************PLUGIN MAIN************************************

int 
ipx_plugin_init(
    ipx_ctx_t *ctx,
    const char *params)
{
	const fds_iemgr_t *iemgr; 
	iemgr = ipx_ctx_iemgr_get(ctx);

	size_t param_size = sizeof(param) / sizeof(char *); 
	int ret;
		
	struct instance *inst_ctx;
	inst_ctx = struct_create(sizeof(struct instance), 1);
	if (inst_ctx == NULL){
		return IPX_ERR_DENIED;
	}

	ret = element_converter_init(inst_ctx, param, param_size, iemgr);
	if (ret < 0){
		element_converter_cleanup(inst_ctx);
		return IPX_ERR_DENIED;
	}

	ipx_ctx_private_set(ctx, inst_ctx);

    return IPX_OK;
}

void
ipx_plugin_destroy(    
    ipx_ctx_t *ctx,
    void *cfg)
{
	struct instance *inst_ctx;
	inst_ctx = (struct instance *) cfg;

	element_converter_cleanup(inst_ctx);

}

int
ipx_plugin_process(   
    ipx_ctx_t *ctx,
    void *cfg,
    ipx_msg_t *msg)
{	
	struct instance *inst_ctx;
	inst_ctx = (struct instance *) cfg;

	//TODO Do something with this spagetty	
	struct ipx_ipfix_record *ipfix_rec;
	struct fds_drec_field result;

	struct str_arr str;

	struct elm_convert smp;
	char buffer[BUFF_SIZE] = {};
	int cnt;
	int ret;

	ipx_msg_ipfix_t* ipfix_msg = ipx_msg_base2ipfix(msg);
	cnt =  ipx_msg_ipfix_get_drec_cnt(ipfix_msg);

	for (int i = 0; i < cnt; i++){
		ipfix_rec = ipx_msg_ipfix_get_drec(ipfix_msg, i);
		
		for (int x = 0; x < inst_ctx->elm_inst_arr_size; x++){ //iterate through every entry in main elm_convert
															   
			smp = inst_ctx->elm_inst_arr[x]; //simplifing the path to wanted variables

			str.matched = malloc(sizeof(char *) * (smp.elm_arr_size + 1));	
			if (str.matched == NULL){
				printf("Memory error\n");
				return 1; //insert some fancy error code here
			}
			str.matched_size = 0;

			for (int y = 0; y < smp.elm_arr_size; y++){ //iterate through every entry in sub elm_arr
				ret = fds_drec_find(&ipfix_rec->rec, smp.elm_arr[y].pen, smp.elm_arr[y].id, &result);
				if(ret > 0){
					ret = fds_field2str_be(result.data, result.size, result.info->def->data_type, buffer, BUFF_SIZE);
					str.matched[str.matched_size] = malloc(sizeof(char) * BUFF_SIZE);
					strcpy(str.matched[str.matched_size], buffer);
					str.matched_size++;
				}
			}
			
			if (str.matched_size > 0){
				printf("%s:", smp.name);
				for (int z = 0; z < str.matched_size; z++){
					printf("%s ", str.matched[z]);	
					free(str.matched[z]);
				}
				printf("\n");
			}
			free(str.matched);
		}
		printf("-------------------------------------------------\n");
	}
	//end of TODO

    return IPX_OK;
}

//********************************_INIT MAIN AND HELPER************************************

int element_converter_init(struct instance *inst_ctx, char **params_arr, size_t params_arr_size, const fds_iemgr_t *iemgr){
	int ret;

	struct elm_convert *elm_convert_arr;
	elm_convert_arr = struct_create(sizeof(struct elm_convert), params_arr_size);
	if (elm_convert_arr == NULL){
		return -1; 
	}

	inst_ctx->elm_inst_arr = elm_convert_arr;
	inst_ctx->elm_inst_arr_size = params_arr_size;

	for(int i = 0; i < inst_ctx->elm_inst_arr_size; i++){ //mainloop
		ret = parse_elem(&inst_ctx->elm_inst_arr[i], params_arr[i], iemgr);
		if (ret < 0){
			ret = parse_alias(&inst_ctx->elm_inst_arr[i], params_arr[i], iemgr);
			if (ret < 0){
				printf("the element \"%s\" is invalid!\n", params_arr[i]);
				return -1;
			}
		}
	}
	return 0;
}

int parse_elem(struct elm_convert *convert_arr, char *element , const fds_iemgr_t *iemgr){
 	const struct fds_iemgr_elem *found_elem;
	found_elem = fds_iemgr_elem_find_name(iemgr, element);
	convert_arr->elm_arr = NULL;
	convert_arr->elm_arr_size = 0;

	if(found_elem != NULL){
		convert_arr->name = element;
		convert_arr->elm_arr = elm_id_add(convert_arr->elm_arr, 0, found_elem->scope->pen, found_elem->id);
		convert_arr->elm_arr_size += 1;		
		return 0;	
	}	
	return -1;
}

int parse_alias(struct elm_convert *convert_arr, char *element , const fds_iemgr_t *iemgr){
	const struct fds_iemgr_alias *found_alias;
	found_alias = fds_iemgr_alias_find(iemgr, element);
	convert_arr->elm_arr = NULL;
	convert_arr->elm_arr_size = 0;

	if (found_alias != NULL){
		convert_arr->name = element;
		for (int j = 0; j < found_alias->sources_cnt; j++){
			convert_arr->elm_arr = elm_id_add(convert_arr->elm_arr, j, found_alias->sources[j]->scope->pen, found_alias->sources[j]->id);
			convert_arr->elm_arr_size += 1;
		}
		return 0;
	}	
	return -1;
}

//********************************_CLEANUP MAIN************************************

void element_converter_cleanup(struct instance *inst_ctx){
	for (int i = 0; i < inst_ctx->elm_inst_arr_size; i++){
		free(inst_ctx->elm_inst_arr[i].elm_arr);
	}
	free(inst_ctx->elm_inst_arr);
	free(inst_ctx);
}


//********************************_CONVERT MAIN AND HELPER************************************





//********************************OTHER HELPER************************************

struct elm_id *elm_id_add(struct elm_id *arr, int position, int pen, int id){
	int num = position + 1;

	arr = realloc(arr, sizeof(struct elm_id) * num);
	if (arr == NULL){
		printf("Allocation failed!\n");
		return NULL;
	}
	arr[position].id = id;
	arr[position].pen = pen;
	return arr;
}

void *struct_create(int struct_size, int num_of){
	void *ptr;
	ptr = malloc(struct_size * num_of);
	if (ptr == NULL){
		printf("Memory error\n");
		return NULL;
	}
	return ptr;
}


