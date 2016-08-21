#include <Elementary.h>
#include <dlog.h>
#include <bundle.h>
#include "gearbrowser.h"
#include "utils.h"
#include "add_bookmark_layout.h"
#include "bookmark_model.h"
#include "entry_layout.h"

static const char *SIGNAL_TITLE_BUTTON_CLICKED = "signal.title.btn.clicked";
static const char *SIGNAL_URL_BUTTON_CLICKED = "signal.url.btn.clicked";
static const char *PART_ADD_BOOKMARK_TITLE = "part.add_bookmark.title";
static const char *PART_ADD_BOOKMARK_URL = "part.add_bookmark.url";

typedef struct add_bookmark_data {
	Evas_Object *navi;
	Evas_Object *layout;
	bundle *result;
	char *title;
	char *url;
} AddBookmarkData;

AddBookmarkData *gAddBookmarkData = NULL;

void
add_bookmark_layout_release(void) {
	dlog_print(DLOG_DEBUG, LOG_TAG, "[add_bookmark_layout_release]");
	if (gAddBookmarkData != NULL) {
		free(gAddBookmarkData);
		gAddBookmarkData = NULL;
	}
}

static void
save_button_click_cb(void *data, Evas_Object *obj, void *event_info) {
	const char *title = elm_layout_text_get(gAddBookmarkData->layout, "part.add_bookmark.title");
	const char *url = elm_layout_text_get(gAddBookmarkData->layout, "part.add_bookmark.url");
	dlog_print(DLOG_DEBUG, LOG_TAG, "[save_button_click_cb] %s, %s", title, url);
	if (title != NULL && strcmp(title, "") && url != NULL && strcmp(url, "")) {
		Eina_Bool result = bookmark_model_add(title, url);
		if (result) {
			elm_naviframe_item_pop(gAddBookmarkData->navi);
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
		elm_layout_text_set(gAddBookmarkData->layout, part, entry);
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
		const char *title = elm_layout_text_get(gAddBookmarkData->layout, PART_ADD_BOOKMARK_TITLE);
		if (title != NULL) {
			bundle_add_str(result, "text", title);
		}

		Elm_Object_Item *item = entry_layout_open(gAddBookmarkData->navi, result);
		elm_naviframe_item_pop_cb_set(item, entry_result_cb, result);
	} else if (!strcmp(emission, SIGNAL_URL_BUTTON_CLICKED)) {
		bundle *result = bundle_create();
		bundle_add_str(result, "_part", PART_ADD_BOOKMARK_URL);
		const char *url = elm_layout_text_get(gAddBookmarkData->layout, PART_ADD_BOOKMARK_URL);
		if (url != NULL) {
			bundle_add_str(result, "text", url);
		}

		Elm_Object_Item *item = entry_layout_open(gAddBookmarkData->navi, result);
		elm_naviframe_item_pop_cb_set(item, entry_result_cb, result);
	}
}

Elm_Object_Item*
add_bookmark_layout_open(Evas_Object *navi, bundle *result) {
	if (gAddBookmarkData != NULL) {
		dlog_print(DLOG_ERROR, LOG_TAG, "[add_bookmark_layout_open] Do not allowed to open repeatedly.");
		return NULL;
	}

	gAddBookmarkData = malloc(sizeof(AddBookmarkData));
	gAddBookmarkData->navi = navi;
	gAddBookmarkData->result = result;
	bundle_get_str(result, "title", &gAddBookmarkData->title);
	bundle_get_str(result, "url", &gAddBookmarkData->url);
	dlog_print(DLOG_DEBUG, LOG_TAG, "[add_bookmark_layout_open] %s, %s", gAddBookmarkData->title, gAddBookmarkData->url);

	char edj_path[PATH_MAX] = {0, };
	app_get_resource("edje/add_bookmark_layout.edj", edj_path);

	Evas_Object *layout = elm_layout_add(navi);
	elm_layout_file_set(layout, edj_path, "group.add_bookmark");
	gAddBookmarkData->layout = layout;

	elm_layout_text_set(layout, PART_ADD_BOOKMARK_TITLE, gAddBookmarkData->title);
	elm_layout_text_set(layout, PART_ADD_BOOKMARK_URL, gAddBookmarkData->url);

	Evas_Object *button = elm_button_add(layout);
	elm_object_style_set(button, "bottom");
	elm_object_part_content_set(layout, "elm.swallow.button", button);
	elm_object_text_set(button, "Save");
	evas_object_smart_callback_add(button, "clicked", save_button_click_cb, NULL);

	elm_object_signal_callback_add(layout, SIGNAL_TITLE_BUTTON_CLICKED, "*", text_click_cb, NULL);
	elm_object_signal_callback_add(layout, SIGNAL_URL_BUTTON_CLICKED, "*", text_click_cb, NULL);

	return elm_naviframe_item_simple_push(navi, layout);
}

