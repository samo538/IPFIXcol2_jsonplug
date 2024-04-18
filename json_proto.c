//
#include <ipfixcol2.h>
#include <libfds/iemgr.h>
#include <stdio.h>
#include <stdlib.h>
//
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
	struct elm_convert* elms_arr;
	size_t elms_arr_size;
};

char *test_field[] = {"iana:protocolIdentifier", "iana:sourceIPv4Address", "iana:destinationIPv4Address", "iana:destinationTransportPort", "iana:sourceTransportPort", "srcip"};
size_t test_field_size = 6;

// main functions
int element_converter_init(struct elm_convert *ptr, fds_iemgr_t *iemgr, char **elm_list, size_t elm_list_size); 
int element_converter_convert();
void element_converter_cleanup(struct elm_convert *ptr,size_t elm_list_size);

//helper functions
struct elm_id *elm_id_add(struct elm_id *arr, int position, int pen, int id);
void elm_id_remove(struct elm_id *arr);

int 
ipx_plugin_init(         // Constructor
    ipx_ctx_t *ctx,
    const char *params)
{
	printf("Initializing json_proto\n"); 

	fds_iemgr_t *iemgr; // const paradox 
	iemgr = ipx_ctx_iemgr_get(ctx);

 	struct elm_convert elm_convert_arr[test_field_size];

	struct instance *inst_ctx;
	inst_ctx = malloc(sizeof(struct instance));
	if (inst_ctx == NULL){
		printf("Memory error\n");
		return 1;
	}
	inst_ctx->elms_arr = elm_convert_arr;
	inst_ctx->elms_arr_size = test_field_size;

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
	printf("processing flow...\n");

	struct instance *inst_ctx;


    return IPX_OK;
}


int element_converter_init(struct elm_convert *ptr, fds_iemgr_t *iemgr, char **elm_list, size_t elm_list_size){
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
			printf("Not Found!, maybe aliased, finding alias...\n");

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

void elm_id_remove(struct elm_id *arr){
	free(arr);
}
