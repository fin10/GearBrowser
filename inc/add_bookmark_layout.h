#ifndef ADD_BOOKMARK_LAYOUT_H_
#define ADD_BOOKMARK_LAYOUT_H_

#include <Elementary.h>
#include <bundle.h>

void add_bookmark_layout_release(void);
Elm_Object_Item *add_bookmark_layout_open(Evas_Object *navi, bundle *result);

#endif /* ADD_BOOKMARK_LAYOUT_H_ */
