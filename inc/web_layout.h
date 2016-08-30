#ifndef WEB_PAGE_H_
#define WEB_PAGE_H_

void web_layout_destroy(void);
Elm_Object_Item *web_layout_open(Evas_Object *navi);
void web_layout_pause(void);
void web_layout_resume(void);

#endif /* WEB_PAGE_H_ */
