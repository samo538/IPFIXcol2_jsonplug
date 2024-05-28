//
#include <ipfixcol2.h>
#include <libfds.h>
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

int fn(const struct fds_drec_field *field, void *context){
	char buffer[BUFF_SIZE + 1];
	char *name = (char *)context;

	fds_field2str_be(field->data, field->size, field->info->def->data_type, buffer, BUFF_SIZE);
	printf("%s: %s\n",name, buffer);
	return FDS_OK;
}

int 
ipx_plugin_init(
    ipx_ctx_t *ctx,
    const char *params)
{
	size_t param_size = 5;
	fds_selector_field_t *param = malloc(sizeof(fds_selector_field_t) * param_size);

	if(param == NULL){
		return IPX_ERR_DENIED;
	}

	param[0].name = "iana:sourceIPv4Address";
	param[0].flag = FDS_SELECTOR_ALL;
	param[1].name = "iana:destinationIPv4Address";
	param[1].flag = FDS_SELECTOR_ALL;
	param[2].name = "iana:destinationTransportPort";
	param[2].flag = FDS_SELECTOR_FIRST;
	param[3].name = "iana:sourceTransportPort";
	param[3].flag = FDS_SELECTOR_ALL;
	param[4].name = "ip";
	param[4].flag = FDS_SELECTOR_FIRST;


	const fds_iemgr_t *iemgr; 
	iemgr = ipx_ctx_iemgr_get(ctx);

	fds_selector_t *selector;
	int ret;
	selector = fds_selector_create(iemgr);
	
	for (int i = 0; i < param_size; i++){
		ret = fds_selector_add(selector, param + i);	
		if (ret < 0){
			return IPX_ERR_DENIED;
		}
	}

	ipx_ctx_private_set(ctx, selector);


	for (int i = 0; i < selector->item_size; i++){
		printf("%s: ", selector->item[i].name);
		for(int j = 0; j < selector->item[i].idpen_size; j++){
			printf("%d %d\n", selector->item[i].idpen[j].id, selector->item[i].idpen[j].pen);
		}
	}

    return IPX_OK;
}

void
ipx_plugin_destroy(    
    ipx_ctx_t *ctx,
    void *cfg)
{
	fds_selector_t *selector;
	selector = (fds_selector_t *)cfg;

	fds_selector_destroy(selector);
}

int
ipx_plugin_process(   
    ipx_ctx_t *ctx,
    void *cfg,
    ipx_msg_t *msg)
{	
	fds_selector_t *selector;
	selector = (fds_selector_t *)cfg;
	
	ipx_msg_ipfix_t *ipfix_msg = ipx_msg_base2ipfix(msg);
	int drec_cnt = ipx_msg_ipfix_get_drec_cnt(ipfix_msg);

	char *name;
	size_t selector_cnt = fds_selector_get_count(selector);
	struct ipx_ipfix_record *ipfix_rec;
	for (int i = 0; i < drec_cnt; i++){
		ipfix_rec = ipx_msg_ipfix_get_drec(ipfix_msg, i);
		for (int j = 0; j < selector_cnt; j++){
			name = fds_selector_get_name(selector, j);
			fds_selector_find(selector, &ipfix_rec->rec, j, fn, name);
		}
	}

    return IPX_OK;
}
