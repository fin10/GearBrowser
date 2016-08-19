#include <Elementary.h>
#include <dlog.h>
#include <bundle.h>
#include "gearbrowser.h"
#include "utils.h"
#include "bookmark_layout.h"
#include "bookmark_model.h"
#include "entry_layout.h"

static const char *SIGNAL_TITLE_BUTTON_CLICKED = "signal.title.btn.clicked";
static const char *SIGNAL_URL_BUTTON_CLICKED = "signal.url.btn.clicked";
static const char *PART_ADD_BOOKMARK_TITLE = "part.add_bookmark.title";
static const char *PART_ADD_BOOKMARK_URL = "part.add_bookmark.url";

typedef struct bookmark_data {
	Evas_Object *navi;
	Evas_Object *layout;
	bundle *result;
	GPtrArray *items;
	char *title;
	char *url;
} BookmarkData;

BookmarkData *gBookmarkData = NULL;

void
bookmark_layout_release(void) {
	dlog_print(DLOG_DEBUG, LOG_TAG, "[bookmark_layout_release]");
	if (gBookmarkData != NULL) {
		g_ptr_array_free(gBookmarkData->items, TRUE);
		free(gBookmarkData);
		gBookmarkData = NULL;
	}
}

static char*
gl_title_text_get(void *data, Evas_Object *obj, const char *part) {
	return strdup("Favorite");
}

static char*
gl_text_get(void *data, Evas_Object *obj, const char *part) {
	BookmarkModel *item = data;
//	dlog_print(DLOG_DEBUG, LOG_TAG, "[gl_text_get] %s", part);
	if (!strcmp(part, "elm.text")) return strdup(item->title);
	else if (!strcmp(part, "elm.text.1")) return strdup(item->url);
	else return strdup("empty");
}

static void
gl_item_sel(void *data, Evas_Object *obj, void *event_info) {
	BookmarkModel *item = data;
	dlog_print(DLOG_DEBUG, LOG_TAG, "[gl_item_sel] %s, %s", item->title, item->url);
	bundle_add_str(gBookmarkData->result, "result", item->url);
	elm_naviframe_item_pop(gBookmarkData->navi);
}

static Evas_Object*
create_genlist(Evas_Object *parent, GPtrArray *items) {
	Evas_Object *genlist = elm_genlist_add(parent);
	elm_genlist_mode_set(genlist, ELM_LIST_COMPRESS);

	Elm_Genlist_Item_Class *title_cls = elm_genlist_item_class_new();
	title_cls->item_style = "title";
	title_cls->func.text_get = gl_title_text_get;

	Elm_Genlist_Item_Class *item_cls = elm_genlist_item_class_new();
	item_cls->item_style = "2text";
	item_cls->func.text_get = gl_text_get;

	Elm_Genlist_Item_Class *padding_cls = elm_genlist_item_class_new();
	padding_cls->item_style = "padding";

	elm_genlist_item_append(genlist, title_cls, NULL, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);

	guint size = items->len;
	for (int i = 0; i < size; ++i)
	{
		BookmarkModel *item = g_ptr_array_index(items, i);
		elm_genlist_item_append(genlist, item_cls, item, NULL, ELM_GENLIST_ITEM_NONE, gl_item_sel, item);
	}

	elm_genlist_item_append(genlist, padding_cls, NULL, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);

	elm_genlist_item_class_free(title_cls);
	elm_genlist_item_class_free(item_cls);
	elm_genlist_item_class_free(padding_cls);
	title_cls = NULL;
	item_cls = NULL;
	padding_cls = NULL;

	return genlist;
}

static void
save_button_click_cb(void *data, Evas_Object *obj, void *event_info) {
	const char *title = elm_layout_text_get(gBookmarkData->layout, "part.add_bookmark.title");
	const char *url = elm_layout_text_get(gBookmarkData->layout, "part.add_bookmark.url");
	dlog_print(DLOG_DEBUG, LOG_TAG, "[save_button_click_cb] %s, %s", title, url);
	if (title != NULL && strcmp(title, "") && url != NULL && strcmp(url, "")) {
		Eina_Bool result = bookmark_model_add(title, url);
		if (result) {
			elm_naviframe_item_pop(gBookmarkData->navi);
		}
	}
}

static Eina_Bool
entry_result_cb(void *data, Elm_Object_Item *it) {
	bundle *result = data;
	char *entry = NULL, *part = NULL;
	bundle_get_str(result, "result", &entry);
	bundle_get_str(result, "_part", &part);
	dlog_print(DLOG_DEBUG, LOG_TAG, "[url_entry_result_cb] part:%s, result:%s", part, entry);

	if (entry != NULL && part != NULL) {
		elm_layout_text_set(gBookmarkData->layout, part, entry);
	}

	elm_naviframe_item_pop_cb_set(it, NULL, NULL);
	bundle_free(result);
	entry_layout_release();

	return EINA_TRUE;
}

static void
text_click_cb(void *data, Evas_Object *obj, const char *emission, const char *source) {
	dlog_print(DLOG_DEBUG, LOG_TAG, "[text_click_cb] %s", emission);
	if (!strcmp(emission, SIGNAL_TITLE_BUTTON_CLICKED)) {
		bundle *result = bundle_create();
		bundle_add_str(result, "_part", PART_ADD_BOOKMARK_TITLE);
		const char *title = elm_layout_text_get(gBookmarkData->layout, PART_ADD_BOOKMARK_TITLE);
		if (title != NULL) {
			bundle_add_str(result, "text", title);
		}

		Elm_Object_Item *item = entry_layout_open(gBookmarkData->navi, result);
		elm_naviframe_item_pop_cb_set(item, entry_result_cb, result);
	} else if (!strcmp(emission, SIGNAL_URL_BUTTON_CLICKED)) {
		bundle *result = bundle_create();
		bundle_add_str(result, "_part", PART_ADD_BOOKMARK_URL);
		const char *url = elm_layout_text_get(gBookmarkData->layout, PART_ADD_BOOKMARK_URL);
		if (url != NULL) {
			bundle_add_str(result, "text", url);
		}

		Elm_Object_Item *item = entry_layout_open(gBookmarkData->navi, result);
		elm_naviframe_item_pop_cb_set(item, entry_result_cb, result);
	}
}

static Evas_Object*
create_add_bookmark_layout(Evas_Object *parent) {
	char edj_path[PATH_MAX] = {0, };
	app_get_resource("edje/add_bookmark_layout.edj", edj_path);

	Evas_Object *layout = elm_layout_add(parent);
	elm_layout_file_set(layout, edj_path, "group.add_bookmark");
	gBookmarkData->layout = layout;

	elm_layout_text_set(layout, PART_ADD_BOOKMARK_TITLE, gBookmarkData->title);
	elm_layout_text_set(layout, PART_ADD_BOOKMARK_URL, gBookmarkData->url);

	Evas_Object *button = elm_button_add(layout);
	elm_object_style_set(button, "bottom");
	elm_object_part_content_set(layout, "elm.swallow.button", button);
	elm_object_text_set(button, "Save");
	evas_object_smart_callback_add(button, "clicked", save_button_click_cb, NULL);

	elm_object_signal_callback_add(layout, SIGNAL_TITLE_BUTTON_CLICKED, "*", text_click_cb, NULL);
	elm_object_signal_callback_add(layout, SIGNAL_URL_BUTTON_CLICKED, "*", text_click_cb, NULL);

	return layout;
}

Elm_Object_Item*
bookmark_layout_open(Evas_Object *navi, bundle *result) {
	if (gBookmarkData != NULL) {
		dlog_print(DLOG_ERROR, LOG_TAG, "[bookmark_layout_open] Do not allowed to open repeatedly.");
		return NULL;
	}

	gBookmarkData = malloc(sizeof(BookmarkData));
	gBookmarkData->navi = navi;
	gBookmarkData->result = result;
	bundle_get_str(result, "title", &gBookmarkData->title);
	bundle_get_str(result, "url", &gBookmarkData->url);
	dlog_print(DLOG_DEBUG, LOG_TAG, "[bookmark_layout_open] %s, %s", gBookmarkData->title, gBookmarkData->url);

	GPtrArray *items = bookmark_model_get_list_n();
	gBookmarkData->items = items;
	dlog_print(DLOG_DEBUG, LOG_TAG, "[bookmark_layout_open] size:%d", items->len);

	Evas_Object *content = NULL;
	if (items->len == 0) {
		content = create_add_bookmark_layout(navi);
	} else {
		content = create_genlist(navi, items);
	}

	return elm_naviframe_item_simple_push(navi, content);
}
