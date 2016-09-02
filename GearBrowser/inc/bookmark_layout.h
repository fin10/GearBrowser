#ifndef __bookmark_layout_H__
#define __bookmark_layout_H__

#include <Elementary.h>
#include <bundle.h>

void bookmark_layout_destroy(void);
Elm_Object_Item *bookmark_layout_open(Evas_Object *navi, bundle *result);

#endif /*__bookmark_layout_H__ */
