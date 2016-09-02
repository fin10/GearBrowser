#ifndef BOOKMARK_MODEL_H_
#define BOOKMARK_MODEL_H_

#include <eina_types.h>
#include <eina_array.h>

typedef struct bookmark_model {
	const char *title;
	const char *url;
} BookmarkModel;

Eina_Bool bookmark_model_add(const char *title, const char *url);
Eina_Bool bookmark_model_remove(BookmarkModel *item);
Eina_Array *bookmark_model_get_list_n(void);

#endif /* BOOKMARK_MODEL_H_ */
