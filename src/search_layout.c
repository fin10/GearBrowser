#include <Elementary.h>
#include <dlog.h>
#include <bundle.h>
#include "gearbrowser.h"
#include "utils.h"
#include "search_layout.h"
#include "entry_layout.h"

static const char* SIGNAL_ENTRY_BUTTON_CLICKED = "signal.btn.entry.clicked";

static const char* URL_SEARCH_GOOGLE = "https://www.google.co.kr/search?q=";
static const char* URL_SEARCH_NAVER = "https://m.search.naver.com/search.naver?query=";

typedef struct search_data {
	Evas_Object* navi;
	Evas_Object* layout;
	bundle* result;
} SearchData;

SearchData* gSearchData = NULL;

static Eina_Bool
startsWith(const char* pre, const char* str)
{
    size_t lenpre = strlen(pre), lenstr = strlen(str);
    return lenstr < lenpre ? EINA_FALSE : strncmp(pre, str, lenpre) == 0;
}

void
search_layout_release() {
	dlog_print(DLOG_DEBUG, LOG_TAG, "[search_layout_release]");
	if (gSearchData != NULL) {
		free(gSearchData);
		gSearchData = NULL;
	}
}

static Eina_Bool
entry_result_cb(void* data, Elm_Object_Item* it) {
	bundle* result = data;
	char* entry = NULL;
	bundle_get_str(result, "result", &entry);
	dlog_print(DLOG_DEBUG, LOG_TAG, "[entry_result_cb] result:%s", entry);

	elm_layout_text_set(gSearchData->layout, "part.search.text", entry);

	elm_naviframe_item_pop_cb_set(it, NULL, NULL);
	bundle_free(result);
	entry_layout_release();

	return EINA_TRUE;
}

static void
search_button_click_cb(void* data, Evas_Object* obj, const char* emission, const char* source) {
	const char* text = elm_layout_text_get(gSearchData->layout, "part.search.text");
	if (text != NULL && strcmp(text, "")) {
		if (startsWith("http://", text) || startsWith("https://", text)) {
			int size = strlen(text);
			char* query = malloc(sizeof(char) * size);
			dlog_print(DLOG_DEBUG, LOG_TAG, "[search_button_click_cb] query:%s", query);
			bundle_add_str(gSearchData->result, "result", query);
		} else {
			int size = strlen(text) + strlen(URL_SEARCH_GOOGLE) + 1;
			char* query = malloc(sizeof(char) * size);
			snprintf(query, size, "%s%s", URL_SEARCH_GOOGLE, text);
			dlog_print(DLOG_DEBUG, LOG_TAG, "[search_button_click_cb] query:%s", query);
			bundle_add_str(gSearchData->result, "result", query);
		}
	}

	elm_naviframe_item_pop(gSearchData->navi);
}

static void
entry_click_cb(void* data, Evas_Object* obj, const char* emission, const char* source) {
	dlog_print(DLOG_DEBUG, LOG_TAG, "[entry_click_cb] %s", emission);
	if (!strcmp(emission, SIGNAL_ENTRY_BUTTON_CLICKED)) {
		bundle* result = bundle_create();
		Elm_Object_Item* item = entry_layout_open(gSearchData->navi, result);
		elm_naviframe_item_pop_cb_set(item, entry_result_cb, result);
	}
}

Elm_Object_Item*
search_layout_open(Evas_Object* navi, bundle* result) {
	if (gSearchData != NULL) {
		dlog_print(DLOG_ERROR, LOG_TAG, "[search_layout_open] Do not allowed to open repeatedly.");
		return NULL;
	}

	dlog_print(DLOG_DEBUG, LOG_TAG, "[search_layout_open]");
	char edj_path[PATH_MAX] = {0, };
	app_get_resource("edje/search_layout.edj", edj_path);

	Evas_Object* layout = elm_layout_add(navi);
	elm_layout_file_set(layout, edj_path, "group.search");

	Evas_Object* button = elm_button_add(layout);
	elm_object_style_set(button, "bottom");
	elm_object_part_content_set(layout, "elm.swallow.button", button);
	elm_object_text_set(button, "Search");
	evas_object_smart_callback_add(button, "clicked", search_button_click_cb, NULL);
	evas_object_show(button);

	gSearchData = malloc(sizeof(SearchData));
	gSearchData->navi = navi;
	gSearchData->layout = layout;
	gSearchData->result = result;

	elm_object_signal_callback_add(layout, SIGNAL_ENTRY_BUTTON_CLICKED, "*", entry_click_cb, NULL);

	return elm_naviframe_item_simple_push(navi, layout);
}
