#include <Elementary.h>
#include <EWebKit.h>
#include <efl_extension.h>
#include <bundle.h>
#include <dlog.h>
#include "gearbrowser.h"
#include "utils.h"
#include "search_layout.h"
#include "bookmark_layout.h"
#include "web_layout.h"
#include "settings.h"

static const char *SIGNAL_BACK_BUTTON_CLICKED = "signal.btn.back.clicked";
static const char *SIGNAL_FORWARD_BUTTON_CLICKED = "signal.btn.forward.clicked";
static const char *SIGNAL_SEARCH_BUTTON_CLICKED = "signal.btn.search.clicked";
static const char *SIGNAL_BOOKMARK_BUTTON_CLICKED = "signal.btn.bookmark.clicked";

typedef struct web_data {
	Evas_Object *navi;
	Evas_Object *web;
	Evas_Object *layout;
	Evas_Object *progressbar;
} WebData;

WebData *gWebData = NULL;

void
web_layout_destroy(void) {
	dlog_print(DLOG_DEBUG, LOG_TAG, "[web_layout_destroy]");
	if (gWebData != NULL) {
		free(gWebData);
		gWebData = NULL;
	}
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
	search_layout_destroy();

	return EINA_TRUE;
}

static Eina_Bool
bookmark_result_cb(void *data, Elm_Object_Item *it) {
	bundle *result = data;
	char *query = NULL;
	bundle_get_str(result, "result", &query);
	dlog_print(DLOG_DEBUG, LOG_TAG, "[bookmark_result_cb] result:%s", query);
	if (query != NULL) {
		ewk_view_url_set(gWebData->web, query);
	}

	elm_naviframe_item_pop_cb_set(it, NULL, NULL);
	bundle_free(result);
	bookmark_layout_destroy();

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
		elm_naviframe_item_pop_cb_set(item, bookmark_result_cb, result);
	}
}

static void
web_url_change_cb(void *data, Evas_Object *obj, void *event_info) {
	dlog_print(DLOG_DEBUG, LOG_TAG, "[web_url_change_cb] %s", event_info);
	if (event_info != NULL) {
		settings_value_set(PREF_KEY_LAST_URL, event_info);
	}
}

static void
web_load_progress_cb(void *data, Evas_Object *obj, void *event_info) {
	double *progress = event_info;
	elm_progressbar_value_set(gWebData->progressbar, *progress);
	if (*progress >= 1.0) {
		elm_progressbar_value_set(gWebData->progressbar, 0);
		evas_object_hide(gWebData->progressbar);
	}
	else evas_object_show(gWebData->progressbar);
}

static void
web_load_finished_cb(void *data, Evas_Object *obj, void *event_info) {
	dlog_print(DLOG_DEBUG, LOG_TAG, "[web_load_finished_cb]");
	const char *sig;
	if (ewk_view_back_possible(gWebData->web)) sig = "signal,back,enabled";
	else sig = "signal,back,disabled";
	elm_layout_signal_emit(gWebData->layout, sig, "mycode");

	if (ewk_view_forward_possible(gWebData->web)) sig = "signal,forward,enabled";
	else sig = "signal,forward,disabled";
	elm_layout_signal_emit(gWebData->layout, sig, "mycode");
}

Elm_Object_Item*
web_layout_open(Evas_Object *navi) {
	if (gWebData != NULL) {
		dlog_print(DLOG_ERROR, LOG_TAG, "[web_layout_open] Do not allowed to open repeatedly.");
		return NULL;
	}

	gWebData = malloc(sizeof(WebData));
	gWebData->navi = navi;

	dlog_print(DLOG_DEBUG, LOG_TAG, "[web_layout_open]");
	char edj_path[PATH_MAX] = {0, };
	app_get_resource("edje/web_layout.edj", edj_path);

	Evas_Object *layout = elm_layout_add(navi);
	elm_layout_file_set(layout, edj_path, "group.web");
	gWebData->layout = layout;

	Evas *evas = evas_object_evas_get(layout);
	Evas_Object *web = ewk_view_add(evas);
	elm_object_part_content_set(layout, "part.web", web);
	ewk_view_url_set(web, settings_value_get_n(PREF_KEY_LAST_URL));
	gWebData->web = web;

	elm_layout_signal_emit(gWebData->layout, "signal,back,disabled", "mycode");
	elm_layout_signal_emit(gWebData->layout, "signal,forward,disabled", "mycode");

	elm_object_signal_callback_add(layout, SIGNAL_BACK_BUTTON_CLICKED, "*", web_button_click_cb, NULL);
	elm_object_signal_callback_add(layout, SIGNAL_FORWARD_BUTTON_CLICKED, "*", web_button_click_cb, NULL);
	elm_object_signal_callback_add(layout, SIGNAL_SEARCH_BUTTON_CLICKED, "*", web_button_click_cb, NULL);
	elm_object_signal_callback_add(layout, SIGNAL_BOOKMARK_BUTTON_CLICKED, "*", web_button_click_cb, NULL);

	evas_object_smart_callback_add(web, "url,changed", web_url_change_cb, NULL);
	evas_object_smart_callback_add(web, "load,progress", web_load_progress_cb, NULL);
	evas_object_smart_callback_add(web, "load,finished", web_load_finished_cb, NULL);

	Evas_Object *progressbar = elm_progressbar_add(layout);
	elm_object_part_content_set(layout, "part.progress", progressbar);
	gWebData->progressbar = progressbar;

	return elm_naviframe_item_simple_push(navi, layout);
}


