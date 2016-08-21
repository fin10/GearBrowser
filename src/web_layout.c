#include <Elementary.h>
#include <EWebKit.h>
#include <bundle.h>
#include <dlog.h>
#include "gearbrowser.h"
#include "utils.h"
#include "search_layout.h"
#include "bookmark_layout.h"
#include "add_bookmark_layout.h"
#include "web_layout.h"

static const char *SIGNAL_BACK_BUTTON_CLICKED = "signal.btn.back.clicked";
static const char *SIGNAL_FORWARD_BUTTON_CLICKED = "signal.btn.forward.clicked";
static const char *SIGNAL_SEARCH_BUTTON_CLICKED = "signal.btn.search.clicked";
static const char *SIGNAL_BOOKMARK_BUTTON_CLICKED = "signal.btn.bookmark.clicked";

typedef struct web_data {
	Evas_Object *navi;
	Evas_Object *web;
} WebData;

WebData *gWebData = NULL;

void
web_layout_release(void) {
	dlog_print(DLOG_DEBUG, LOG_TAG, "[web_layout_release]");
	if (gWebData != NULL) {
		free(gWebData);
		gWebData = NULL;
	}
}

static Eina_Bool
add_bookmark_result_cb(void *data, Elm_Object_Item *it) {
	dlog_print(DLOG_DEBUG, LOG_TAG, "[add_bookmark_result_cb]");
	bundle *result = data;
	bundle_free(result);
	elm_naviframe_item_pop_cb_set(it, NULL, NULL);
	add_bookmark_layout_release();

	return EINA_TRUE;
}

static Eina_Bool
search_result_cb(void *data, Elm_Object_Item *it) {
	bundle *result = data;
	char *query = NULL;
	bundle_get_str(result, "result", &query);
	dlog_print(DLOG_DEBUG, LOG_TAG, "[search_result_cb] result:%s", query);

	if (query != NULL) {
		ewk_view_url_set(gWebData->web, query);
	}

	elm_naviframe_item_pop_cb_set(it, NULL, NULL);
	bundle_free(result);
	search_layout_release();

	return EINA_TRUE;
}

static Eina_Bool
bookmark_result_cb(void *data, Elm_Object_Item *it) {
	bundle *result = data;
	char *query = NULL, *action = NULL;
	bundle_get_str(result, "result", &query);
	bundle_get_str(result, "action", &action);
	dlog_print(DLOG_DEBUG, LOG_TAG, "[bookmark_result_cb] result:%s, action:%s", query, action);
	if (query != NULL) {
		ewk_view_url_set(gWebData->web, query);
	}

	if (action != NULL && !strcmp(action, "add_bookmark")) {
		bundle *result = bundle_create();
		bundle_add_str(result, "title", ewk_view_title_get(gWebData->web));
		bundle_add_str(result, "url", ewk_view_url_get(gWebData->web));
		Elm_Object_Item *item = add_bookmark_layout_open(gWebData->navi, result);
		elm_naviframe_item_pop_cb_set(item, add_bookmark_result_cb, result);
	}

	elm_naviframe_item_pop_cb_set(it, NULL, NULL);
	bundle_free(result);
	bookmark_layout_release();

	return EINA_TRUE;
}

static void
web_button_click_cb(void *data, Evas_Object *obj, const char *emission, const char *source) {
	dlog_print(DLOG_DEBUG, LOG_TAG, "[web_button_click_cb] %s", emission);
	if (!strcmp(emission, SIGNAL_BACK_BUTTON_CLICKED)) {
		ewk_view_back(gWebData->web);
	} else if (!strcmp(emission, SIGNAL_FORWARD_BUTTON_CLICKED)) {
		ewk_view_forward(gWebData->web);
	} else if (!strcmp(emission, SIGNAL_SEARCH_BUTTON_CLICKED)) {
		bundle *result = bundle_create();
		Elm_Object_Item *item = search_layout_open(gWebData->navi, result);
		elm_naviframe_item_pop_cb_set(item, search_result_cb, result);
	} else if (!strcmp(emission, SIGNAL_BOOKMARK_BUTTON_CLICKED)) {
		bundle *result = bundle_create();
		bundle_add_str(result, "title", ewk_view_title_get(gWebData->web));
		bundle_add_str(result, "url", ewk_view_url_get(gWebData->web));
		Elm_Object_Item *item = bookmark_layout_open(gWebData->navi, result);
		if (item != NULL) {
			elm_naviframe_item_pop_cb_set(item, bookmark_result_cb, result);
		} else {
			char *action = NULL;
			bundle_get_str(result, "action", &action);
			if (action != NULL && !strcmp(action, "add_bookmark")) {
				item = add_bookmark_layout_open(gWebData->navi, result);
				elm_naviframe_item_pop_cb_set(item, add_bookmark_result_cb, result);
			}
		}
	}
}

Elm_Object_Item*
web_layout_open(Evas_Object *navi) {
	if (gWebData != NULL) {
		dlog_print(DLOG_ERROR, LOG_TAG, "[web_layout_open] Do not allowed to open repeatedly.");
		return NULL;
	}

	dlog_print(DLOG_DEBUG, LOG_TAG, "[web_layout_open]");
	char edj_path[PATH_MAX] = {0, };
	app_get_resource("edje/web_layout.edj", edj_path);

	Evas_Object *layout = elm_layout_add(navi);
	elm_layout_file_set(layout, edj_path, "group.web");

	Evas *evas = evas_object_evas_get(layout);
	Evas_Object *web = ewk_view_add(evas);
	elm_object_part_content_set(layout, "part.web", web);
	ewk_view_url_set(web, "http://www.google.com");

	elm_object_signal_callback_add(layout, SIGNAL_BACK_BUTTON_CLICKED, "*", web_button_click_cb, NULL);
	elm_object_signal_callback_add(layout, SIGNAL_FORWARD_BUTTON_CLICKED, "*", web_button_click_cb, NULL);
	elm_object_signal_callback_add(layout, SIGNAL_SEARCH_BUTTON_CLICKED, "*", web_button_click_cb, NULL);
	elm_object_signal_callback_add(layout, SIGNAL_BOOKMARK_BUTTON_CLICKED, "*", web_button_click_cb, NULL);

	gWebData = malloc(sizeof(WebData));
	gWebData->navi = navi;
	gWebData->web = web;

	return elm_naviframe_item_simple_push(navi, layout);
}


