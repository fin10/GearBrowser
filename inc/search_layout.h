#ifndef __search_layout_H__
#define __search_layout_H__

#include <Elementary.h>
#include <bundle.h>

void search_layout_destroy(void);
Elm_Object_Item *search_layout_open(Evas_Object *navi, bundle *result);

#endif /* __search_layout_H__ */
