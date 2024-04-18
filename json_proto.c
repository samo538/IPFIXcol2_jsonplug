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


char *test_field[] = {"iana:sourceIPv4Address", "iana:destinationIPv4Address", "iana:destinationTransportPort", "iana:sourceTransportPort", "ip"};
size_t test_field_size = 5;

// main functions
int element_converter_init(struct elm_convert *ptr, const fds_iemgr_t *iemgr, char **elm_list, size_t elm_list_size); 
int element_converter_convert();
void element_converter_cleanup(struct elm_convert *ptr,size_t elm_list_size);

//helper functions
struct elm_id *elm_id_add(struct elm_id *arr, int position, int pen, int id);
void elm_id_remove(struct elm_id *arr);
void *struct_create(int struct_size, int num_of);
struct str_arr * str_arr_create();
int str_arr_add(struct str_arr, char *buffer);
void str_arr_destroy(struct str_arr);





int 
ipx_plugin_init(         // Constructor
    ipx_ctx_t *ctx,
    const char *params)
{
	printf("Initializing json_proto\n"); 

	const fds_iemgr_t *iemgr; // const paradox 
	iemgr = ipx_ctx_iemgr_get(ctx);

 	//struct elm_convert elm_convert_arr[test_field_size];

	struct elm_convert *elm_convert_arr;
	elm_convert_arr = struct_create(sizeof(struct elm_convert), test_field_size);
	if (elm_convert_arr == NULL){
		return 1; //input some error code here later
	}

	struct instance *inst_ctx;
	inst_ctx = struct_create(sizeof(struct instance), 1);
	if (inst_ctx == NULL){
		return 1; //input some error code here later
	}

	inst_ctx->elm_inst_arr = elm_convert_arr;
	inst_ctx->elm_inst_arr_size = test_field_size;

	element_converter_init(elm_convert_arr, iemgr, test_field, test_field_size);
	
	ipx_ctx_private_set(ctx, inst_ctx);

    return IPX_OK;
}

void
ipx_plugin_destroy(      // Destructor
    ipx_ctx_t *ctx,
    void *cfg)
{
	struct instance *inst_ctx;

	//first, test the elm_convert_arr in plugin_process

	/*inst_ctx = (struct instance *) cfg;
	for (int i = 0; i < inst_ctx->elms_arr_size; i++){
		
	}	*/
}

int
ipx_plugin_process(      // Function processing every IPX packet and extracting ids that are specified
    ipx_ctx_t *ctx,
    void *cfg,
    ipx_msg_t *msg)
{	
	struct instance *inst_ctx;
	inst_ctx = (struct instance *) cfg;
	
	struct ipx_ipfix_record *ipfix_rec;
	struct fds_drec_field result;

	struct str_arr str;

	char buffer[BUFF_SIZE] = {};
	int cnt;
	int ret;

	printf("processing flow...\n");

	ipx_msg_ipfix_t* ipfix_msg = ipx_msg_base2ipfix(msg);
	cnt =  ipx_msg_ipfix_get_drec_cnt(ipfix_msg);

	for (int i = 0; i < cnt; i++){
		ipfix_rec = ipx_msg_ipfix_get_drec(ipfix_msg, i);
		
		for (int x = 0; x < inst_ctx->elm_inst_arr_size; x++){ //iterate through every entry in main elm_convert
			//add a const variable to simplify the madness bellow
						
			str.matched = malloc(sizeof(char *) * (inst_ctx->elm_inst_arr[x].elm_arr_size + 1));	
			if (str.matched == NULL){
				printf("Memory error\n");
				return 1; //insert some fancy error code here
			}
			str.matched_size = 0;

			for (int y = 0; y < inst_ctx->elm_inst_arr[x].elm_arr_size; y++){ //iterate through every entry in sub elm_arr
				ret = fds_drec_find(&ipfix_rec->rec, inst_ctx->elm_inst_arr[x].elm_arr[y].pen, inst_ctx->elm_inst_arr[x].elm_arr[y].id, &result);
				if(ret > 0){
					ret = fds_field2str_be(result.data, result.size, result.info->def->data_type, buffer, BUFF_SIZE);
					str.matched[str.matched_size] = malloc(sizeof(char) * BUFF_SIZE);
					strcpy(str.matched[str.matched_size], buffer);
					str.matched_size++;
				}
			}
			
			if (str.matched_size > 0){
				printf("%s:", inst_ctx->elm_inst_arr[x].name);
				for (int z = 0; z < str.matched_size; z++){
					printf(" %s ", str.matched[z]);	
				}
				printf("\n");
			}

			free(str.matched);
		}
		printf("-------------------------------------------------\n");
	}

    return IPX_OK;
}


int element_converter_init(struct elm_convert *ptr, const fds_iemgr_t *iemgr, char **elm_list, size_t elm_list_size){
 	const struct fds_iemgr_elem *found_elem;
	const struct fds_iemgr_alias *found_alias;

	int tmp = 0; //testing
	

	for(int i = 0; i < elm_list_size; i++){ //mainloop
	 	found_elem = fds_iemgr_elem_find_name(iemgr, elm_list[i]);
		ptr[i].elm_arr = NULL;
		ptr[i].elm_arr_size = 0;

		if(found_elem != NULL){
			ptr[i].name = elm_list[i];
			ptr[i].elm_arr = elm_id_add(ptr[i].elm_arr, 0, found_elem->scope->pen, found_elem->id);
			ptr[i].elm_arr_size += 1;			
		}	
		else{
			found_alias = fds_iemgr_alias_find(iemgr, elm_list[i]);

		   	if (found_alias != NULL){
				ptr[i].name = elm_list[i];
				for (int j = 0; j < found_alias->sources_cnt; j++){
					ptr[i].elm_arr = elm_id_add(ptr[i].elm_arr, j, found_alias->sources[j]->scope->pen, found_alias->sources[j]->id);
					ptr[i].elm_arr_size += 1;
				}
			}	
		}

		tmp++; //testing
	}

	for(int i = 0; i < tmp; i++){ // testing for
		printf("%s\n", ptr[i].name);
		printf("%zu\n", ptr[i].elm_arr_size);

		for (int j = 0; j < ptr[i].elm_arr_size; j++){
			printf("Id: %d\n", ptr[i].elm_arr[j].id);
			printf("Pen: %d\n",ptr[i].elm_arr[j].pen);
		}
	} // end of testing for

	return 0;
}


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

void elm_id_remove(struct elm_id *arr){ //maybe uprage so it takes the whole struct as its parameter
	free(arr);
}



void *struct_create(int struct_size, int num_of){
	void *ptr;
	ptr = malloc(struct_size * num_of);
	if (ptr == NULL){
		printf("Memory error\n");
		return NULL; //input some error code here later
	}
	return ptr;
}
