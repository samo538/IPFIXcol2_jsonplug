//Author: Samuel Luptak
//Data: 27.5.2024

#include <stdio.h>
#include <stdlib.h>
#include <libfds.h>
#include "selectorlib.h"


int parse_field(struct fds_selector_item *item, fds_selector_field_t *field, const fds_iemgr_t *iemgr);
int parse_alias(struct fds_selector_item *item, fds_selector_field_t *field, const fds_iemgr_t *iemgr);
struct fds_selector_idpen *idpen_add(struct fds_selector_idpen *idpen, int index, int pen, int id);

//********************************************EXPOSED FUNCS*************************************

fds_selector_t *fds_selector_create(const fds_iemgr_t *iemgr){
	fds_selector_t *selector;

	selector = malloc(sizeof(fds_selector_t));
	if (selector == NULL){
		return NULL; //TODO some error code here
	}
	
	selector->iemgr = iemgr;
	selector->item = NULL;
	selector->item_size = 0;

	return selector;
}

void fds_selector_destroy(fds_selector_t *selector){
	for(int i = 0; i < selector->item_size; i++){
		free(selector->item[i].idpen);
	}
	free(selector->item);
	free(selector);
}


int fds_selector_add(fds_selector_t *selector, fds_selector_field_t *field){
	int index = selector->item_size;
	selector->item_size++;

	selector->item = realloc(selector->item, sizeof(struct fds_selector_item) * selector->item_size);
	if (selector->item == NULL){
		return -1; //TODO some error code here
	}

	int ret;
	ret = parse_field(&selector->item[index], field, selector->iemgr);
	if (ret >= 0){
		return index;
	}
	ret = parse_alias(&selector->item[index], field, selector->iemgr);
	if (ret >= 0){
		return index;
	}

	selector->item_size--;
	return -1; //TODO some error code here
}

int fds_selector_find(
		fds_selector_t *selector,
		struct fds_drec *drec,
		int index,
		fds_selector_fn fn,
		void *context
		)
{
	if (index >= selector->item_size){
		return -1; //TODO some error code
	}

	struct fds_selector_idpen *idpen = selector->item[index].idpen;
	size_t idpen_size = selector->item[index].idpen_size; 

	int ret = 0;
	int fn_ret = 0;
	struct fds_drec_field field;
	for (int i = 0; i < idpen_size; i++){
		ret = fds_drec_find(drec, idpen[i].pen, idpen[i].id, &field);
		if (ret >= 0){
			fn_ret = fn(&field, context);
		}
		if (fn_ret < 0){
			return ret;
		}
	}

	return FDS_OK; //TODO some good code	
}

size_t fds_selector_get_count(const fds_selector_t *selector){
	return selector->item_size;
}

char *fds_selector_get_name(const fds_selector_t *selector, int index){
	return selector->item[index].name;
}

//********************************************PRIVATE FUNCS*************************************

//This function parses a field and adds it to the selector
int parse_field(struct fds_selector_item *item, fds_selector_field_t *field, const fds_iemgr_t *iemgr){
	const struct fds_iemgr_elem *found_field;

	found_field = fds_iemgr_elem_find_name(iemgr, field->name);
	
	item->idpen = NULL;
	item->idpen_size = 0;

	if (found_field != NULL){
		item->name = field->name;
		item->flag = field->flag;
		item->idpen = idpen_add(item->idpen, 0, found_field->scope->pen, found_field->id);
		item->idpen_size = 1;
		return 0; //TODO good code
	}
	return -1; //TODO error code
}

//This function parses a fields (if it is a alias) and adds it to the selector
int parse_alias(struct fds_selector_item *item, fds_selector_field_t *field, const fds_iemgr_t *iemgr){
	const struct fds_iemgr_alias *found_alias;

	found_alias = fds_iemgr_alias_find(iemgr, field->name);

	item->idpen = NULL;
	item->idpen_size = 0;

	if(found_alias != NULL){
		item->name = field->name;
		item->flag = field->flag;
		for (int i = 0; i < found_alias->sources_cnt; i++){
			item->idpen = idpen_add(item->idpen, i, found_alias->sources[i]->scope->pen, found_alias->sources[i]->id);
			item->idpen_size += 1;
		}
		return 0; //TODO good code
	}
	return -1; //TODO error code
}

//This function adds (or enlarges) an idpen to an item
struct fds_selector_idpen *idpen_add(struct fds_selector_idpen *idpen, int index, int pen, int id){
	idpen = realloc(idpen, sizeof(struct fds_selector_idpen) * (index + 1));
	if (idpen == NULL){
		return NULL;
	}

	idpen[index].id = id;
	idpen[index].pen = pen;
	return idpen;
}
