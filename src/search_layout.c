#include <Elementary.h>
#include <dlog.h>
#include <bundle.h>
#include "gearbrowser.h"
#include "utils.h"
#include "search_layout.h"
#include "entry_layout.h"
#include "settings.h"

static const char *SIGNAL_ENTRY_BUTTON_CLICKED = "signal.btn.entry.clicked";
static const char *SIGNAL_GOOGLE_BUTTON_CLICKED = "signal.btn.google.clicked";
static const char *SIGNAL_NAVER_BUTTON_CLICKED = "signal.btn.naver.clicked";

static const char *URL_SEARCH_GOOGLE = "https://www.google.co.kr/search?q=";
static const char *URL_SEARCH_NAVER = "https://m.search.naver.com/search.naver?query=";

typedef struct search_data {
	Evas_Object *navi;
	Evas_Object *layout;
	bundle *result;
	const char *searchEngine;
} SearchData;

SearchData *gSearchData = NULL;

static Eina_Bool
startsWith(const char *pre, const char *str) {
    size_t lenpre = strlen(pre), lenstr = strlen(str);
    return lenstr < lenpre ? EINA_FALSE : strncmp(pre, str, lenpre) == 0;
}

void
search_layout_destroy(void) {
	dlog_print(DLOG_DEBUG, LOG_TAG, "[search_layout_destroy]");
	if (gSearchData != NULL) {
		free(gSearchData);
		gSearchData = NULL;
	}
}

static Eina_Bool
entry_result_cb(void *data, Elm_Object_Item *it) {
	bundle *result = data;
	char *entry = NULL;
	bundle_get_str(result, "result", &entry);
	dlog_print(DLOG_DEBUG, LOG_TAG, "[entry_result_cb] result:%s", entry);

	if (entry != NULL) {
		elm_layout_text_set(gSearchData->layout, "part.search.text", entry);
		settings_value_set(PREF_KEY_LAST_SEARCH_KEYWORD, entry);
	}

	elm_naviframe_item_pop_cb_set(it, NULL, NULL);
	bundle_free(result);
	entry_layout_destroy();

	return EINA_TRUE;
}

static void
search_button_click_cb(void *data, Evas_Object *obj, void *event_info) {
	const char *text = elm_layout_text_get(gSearchData->layout, "part.search.text");
	if (text != NULL && strcmp(text, "")) {
		if (startsWith("http://", text) || startsWith("https://", text)) {
			dlog_print(DLOG_DEBUG, LOG_TAG, "[search_button_click_cb] query:%s", text);
			bundle_add_str(gSearchData->result, "result", text);
		} else {
			int size = strlen(text) + strlen(gSearchData->searchEngine) + 1;
			char *query = malloc(sizeof(char) * size);
			snprintf(query, size, "%s%s", gSearchData->searchEngine, text);
			dlog_print(DLOG_DEBUG, LOG_TAG, "[search_button_click_cb] query:%s", query);
			bundle_add_str(gSearchData->result, "result", query);
		}
	}

	elm_naviframe_item_pop(gSearchData->navi);
}

static void
button_click_cb(void *data, Evas_Object *obj, const char *emission, const char *source) {
	dlog_print(DLOG_DEBUG, LOG_TAG, "[button_click_cb] %s", emission);
	if (!strcmp(emission, SIGNAL_ENTRY_BUTTON_CLICKED)) {
		bundle *result = bundle_create();
		const char *text = elm_layout_text_get(gSearchData->layout, "part.search.text");
		if (text != NULL) {
			bundle_add_str(result, "text", text);
		}

		Elm_Object_Item *item = entry_layout_open(gSearchData->navi, result);
		elm_naviframe_item_pop_cb_set(item, entry_result_cb, result);
	} else if (!strcmp(emission, SIGNAL_GOOGLE_BUTTON_CLICKED)) {
		elm_layout_signal_emit(gSearchData->layout, "signal,google,selected", "mycode");
		elm_layout_signal_emit(gSearchData->layout, "signal,naver,normal", "mycode");
		gSearchData->searchEngine = URL_SEARCH_GOOGLE;
		settings_value_set(PREF_KEY_SEARCH_ENGINE, gSearchData->searchEngine);
	} else if (!strcmp(emission, SIGNAL_NAVER_BUTTON_CLICKED)) {
		elm_layout_signal_emit(gSearchData->layout, "signal,google,normal", "mycode");
		elm_layout_signal_emit(gSearchData->layout, "signal,naver,selected", "mycode");
		gSearchData->searchEngine = URL_SEARCH_NAVER;
		settings_value_set(PREF_KEY_SEARCH_ENGINE, gSearchData->searchEngine);
	}
}

Elm_Object_Item*
search_layout_open(Evas_Object *navi, bundle *result) {
	if (gSearchData != NULL) {
		dlog_print(DLOG_ERROR, LOG_TAG, "[search_layout_open] Do not allowed to open repeatedly.");
		return NULL;
	}

	gSearchData = malloc(sizeof(SearchData));
	gSearchData->navi = navi;
	gSearchData->result = result;

	dlog_print(DLOG_DEBUG, LOG_TAG, "[search_layout_open]");
	const char *edj_path = app_get_resource_n("edje/search_layout.edj");

	Evas_Object *layout = elm_layout_add(navi);
	elm_layout_file_set(layout, edj_path, "group.search");
	gSearchData->layout = layout;

	Evas_Object *button = elm_button_add(layout);
	elm_object_style_set(button, "bottom");
	elm_object_part_content_set(layout, "elm.swallow.button", button);
	elm_object_text_set(button, "Search");
	evas_object_smart_callback_add(button, "clicked", search_button_click_cb, NULL);

	const char *keyword = settings_value_get_n(PREF_KEY_LAST_SEARCH_KEYWORD);
	if (!strcmp(keyword, "")) {
		free((void *)keyword);
		keyword = NULL;
	} else {
		elm_object_part_text_set(layout, "part.search.text", keyword);
	}

	elm_object_signal_callback_add(layout, SIGNAL_ENTRY_BUTTON_CLICKED, "*", button_click_cb, NULL);
	elm_object_signal_callback_add(layout, SIGNAL_GOOGLE_BUTTON_CLICKED, "*", button_click_cb, NULL);
	elm_object_signal_callback_add(layout, SIGNAL_NAVER_BUTTON_CLICKED, "*", button_click_cb, NULL);

	gSearchData->searchEngine = settings_value_get_n(PREF_KEY_SEARCH_ENGINE);
	if (!strcmp(gSearchData->searchEngine, URL_SEARCH_GOOGLE)) {
		elm_layout_signal_emit(gSearchData->layout, "signal,google,selected", "mycode");
		elm_layout_signal_emit(gSearchData->layout, "signal,naver,normal", "mycode");
	} else if (!strcmp(gSearchData->searchEngine, URL_SEARCH_NAVER)) {
		elm_layout_signal_emit(gSearchData->layout, "signal,google,normal", "mycode");
		elm_layout_signal_emit(gSearchData->layout, "signal,naver,selected", "mycode");
	}

	return elm_naviframe_item_simple_push(navi, layout);
}
