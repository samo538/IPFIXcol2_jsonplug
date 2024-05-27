//Author: Samuel Luptak
//Date: 27.5.2024


#ifndef __LIBSELECT__
#define __LIBSELECT__

#include <libfds.h>

enum fds_selector_flags{
	FDS_SELECTOR_FIRST,
	FDS_SELECTOR_ALL
};

//This struct is used to fill the selector
struct fds_selector_field{
	char *name;
	enum fds_selector_flags flag;
};

//This struct is used to store the id and pen
struct fds_selector_idpen{
	int id;
	int pen;
};

//This struct is used to store the name and (if nessesary multiple) id and pen information
struct fds_selector_item{
	char *name;
	enum fds_selector_flags flag;
	size_t idpen_size;
	struct fds_selector_idpen *idpen;
};

//This is the main selector struct, it holds the items and also stores the manager
struct fds_selector{
	const fds_iemgr_t *iemgr;
	size_t item_size;
	struct fds_selector_item *item;
};

typedef struct fds_selector fds_selector_t;
typedef struct fds_selector_field fds_selector_field_t;

//This will be a special, user defined, function which will be used in fds_selector_find(...)
//fds_selector_find(...) will call this function with every fds_drec_field it matches
//This function MUST return FDS_OK on success and < 0 on fail
typedef int (*fds_selector_fn)(const struct fds_drec_field *field, void *context);

//Functions used to create and destroy the selector
fds_selector_t *fds_selector_create(const fds_iemgr_t *iemgr);
void fds_selector_destroy(fds_selector_t *selector);

//Function used to add an item to the selector, ..._field_t is used to do this
//returns: on success returns the index, on fail returns < 0
int fds_selector_add(fds_selector_t *selector, fds_selector_field_t *field);

//The main function for the selector
//Parameters:
//selector - Pointer to an already created selector
//drec - Pointer to a drec that you want to select fields from 
//index - Selects one specific item which will be selected from the drec
//fn - user defined function
//ctx - user given ctx which is passed down to the Function
//returns: on success returns > 0, on fail propagates error from fn
int fds_selector_find(
		fds_selector_t *selector, 
		struct fds_drec *drec,
		int index,
		fds_selector_fn fn,
		void *context
		);

//This function returns the number of indexes
size_t fds_selector_get_count(const fds_selector_t *selector);

//This function returns the name of an item on index index
char *fds_selector_get_name(const fds_selector_t *selector, int index);

#endif
