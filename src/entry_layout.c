#include <Elementary.h>
#include <bundle.h>
#include <dlog.h>
#include "gearbrowser.h"
#include "utils.h"
#include "entry_layout.h"

typedef struct entry_data {
	Evas_Object* navi;
	bundle* result;
} EntryData;

EntryData* gEntryData = NULL;

void
entry_layout_release() {
	dlog_print(DLOG_DEBUG, LOG_TAG, "[entry_layout_release]");
	if (gEntryData != NULL) {
		free(gEntryData);
		gEntryData = NULL;
	}
}

static void
entry_done_cb(void* data, Evas_Object* obj, void* event_info) {
	const char* text = elm_entry_entry_get(obj);
	dlog_print(DLOG_DEBUG, LOG_TAG, "[entry_done_cb] %s", text);
	bundle_add_str(gEntryData->result, "result", text);

	elm_naviframe_item_pop(gEntryData->navi);
}

Elm_Object_Item*
entry_layout_open(Evas_Object* navi, bundle* result) {
	if (gEntryData != NULL) {
		dlog_print(DLOG_ERROR, LOG_TAG, "[entry_layout_open] Do not allowed to open repeatedly.");
		return NULL;
	}

	dlog_print(DLOG_DEBUG, LOG_TAG, "[entry_layout_open]");
	char edj_path[PATH_MAX] = {0, };
	app_get_resource("edje/entry_layout.edj", edj_path);

	Evas_Object* layout = elm_layout_add(navi);
	elm_layout_file_set(layout, edj_path, "group.entry");

	Evas_Object* entry = elm_entry_add(layout);
	elm_object_part_content_set(layout, "part.entry", entry);
	elm_entry_single_line_set(entry, EINA_TRUE);
	elm_entry_scrollable_set(entry, EINA_TRUE);
	elm_entry_input_panel_return_key_type_set(entry, ELM_INPUT_PANEL_RETURN_KEY_TYPE_SEARCH);

	gEntryData = malloc(sizeof(EntryData));
	gEntryData->navi = navi;
	gEntryData->result = result;

	evas_object_smart_callback_add(entry, "activated", entry_done_cb, NULL);

	return elm_naviframe_item_simple_push(navi, layout);
}
