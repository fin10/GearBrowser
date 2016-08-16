#include <Elementary.h>
#include <dlog.h>
#include "search_layout.h"
#include "common.h"

typedef struct appdata {
	Evas_Object* navi;
	Evas_Object* entry;
} appdata_s;

static void
search_cb(void* data, Evas_Object* obj, const char* emission, const char* source)
{
	appdata_s* ad = data;
	const char* text = elm_entry_entry_get(ad->entry);
	dlog_print(DLOG_DEBUG, LOG_TAG, "[search_cb] %s", text);
	elm_naviframe_item_pop(ad->navi);
	free(ad);
}

void
open_searchview(Evas_Object* navi, Evas_Object* parent)
{
	dlog_print(DLOG_DEBUG, LOG_TAG, "[open_searchview]");
	char edj_path[PATH_MAX] = {0, };
	app_get_resource("edje/search_layout.edj", edj_path);
	dlog_print(DLOG_DEBUG, LOG_TAG, "path:%s", edj_path);

	appdata_s* ad = malloc(sizeof(appdata_s));
	ad->navi = navi;

	Evas_Object* layout = elm_layout_add(parent);
	elm_layout_file_set(layout, edj_path, "group.search");

	ad->entry = elm_entry_add(layout);
	elm_object_part_content_set(layout, "part.search.entry", ad->entry);
	evas_object_show(ad->entry);

	elm_object_signal_callback_add(layout, "signal.btn.search.clicked", "*", search_cb, ad);
	elm_naviframe_item_simple_push(ad->navi, layout);
}
