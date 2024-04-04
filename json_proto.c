#include <ipfixcol2.h>
#include <libfds/iemgr.h>
#include <stdio.h>
#include <stdlib.h>

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
	int elm_arr_size;
};

char *test_field[] = {"iana:protocolIdentifier", "iana:sourceIPv4Address", "iana:destinationIPv4Address", "iana:destinationTransportPort", "iana:sourceTransportPort"};
int test_field_size = 5;

// main functions
int element_converter_init(struct elm_convert *ptr, fds_iemgr_t *iemgr, char **elm_list, int elm_list_size); 
int element_converter_convert();
void element_converter_cleanup();

//helper functions
struct elm_id *elm_id_add(struct elm_id *arr, int position, int pen, int id);
void elm_id_remove();

int 
ipx_plugin_init(         // Constructor
    ipx_ctx_t *ctx,
    const char *params)
{
	printf("Initializing json_proto\n"); 
	
	fds_iemgr_t *iemgr;
	iemgr = ipx_ctx_iemgr_get(ctx);

 	struct elm_convert elm_convert_arr[test_field_size];

	element_converter_init(elm_convert_arr, iemgr, test_field, test_field_size);

    return IPX_OK;
}

void
ipx_plugin_destroy(      // Destructor
    ipx_ctx_t *ctx,
    void *cfg)
{
	
}

int
ipx_plugin_process(      // Function processing every IPX packet and extracting src/dst ip, src/dst port and protocol
    ipx_ctx_t *ctx,
    void *cfg,
    ipx_msg_t *msg)
{


    return IPX_OK;
}


int element_converter_init(struct elm_convert *ptr, fds_iemgr_t *iemgr, char **elm_list, int elm_list_size){
 	struct fds_iemgr_elem *found_elem;

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
			printf("Not Found!, maybe aliased\n");
		}

		tmp++; //testing
	}

	for(int i = 0; i < tmp; i++){ // testing for
		printf("%s\n", ptr[i].name);
		printf("%d\n", ptr[i].elm_arr_size);

		for (int j = 0; j < ptr[i].elm_arr_size; j++){
			printf("Id: %d\n", ptr[i].elm_arr[0].id);
			printf("Pen: %d\n",ptr[i].elm_arr[0].pen);
		}
	}

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
