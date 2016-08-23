#include <Elementary.h>
#include <bundle.h>
#include <dlog.h>
#include "gearbrowser.h"
#include "utils.h"
#include "entry_layout.h"

typedef struct entry_data {
	Evas_Object *navi;
	bundle *result;
} EntryData;

EntryData *gEntryData = NULL;

void
entry_layout_destroy(void) {
	dlog_print(DLOG_DEBUG, LOG_TAG, "[entry_layout_destroy]");
	if (gEntryData != NULL) {
		free(gEntryData);
		gEntryData = NULL;
	}
}

static void
entry_done_cb(void *data, Evas_Object *obj, void *event_info) {
	const char *text = elm_entry_entry_get(obj);
	dlog_print(DLOG_DEBUG, LOG_TAG, "[entry_done_cb] %s", text);
	bundle_add_str(gEntryData->result, "result", text);

	elm_naviframe_item_pop(gEntryData->navi);
}

Elm_Object_Item*
entry_layout_open(Evas_Object *navi, bundle *result) {
	if (gEntryData != NULL) {
		dlog_print(DLOG_ERROR, LOG_TAG, "[entry_layout_open] Do not allowed to open repeatedly.");
		return NULL;
	}

	char edj_path[PATH_MAX] = {0, };
	app_get_resource("edje/entry_layout.edj", edj_path);

	Evas_Object *layout = elm_layout_add(navi);
	elm_layout_file_set(layout, edj_path, "group.entry");

	Evas_Object *entry = elm_entry_add(layout);
	elm_entry_single_line_set(entry, EINA_TRUE);
	elm_entry_scrollable_set(entry, EINA_TRUE);
	elm_scroller_policy_set(entry, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_AUTO);
	evas_object_smart_callback_add(entry, "activated", entry_done_cb, NULL);

	char *text = NULL;
	bundle_get_str(result, "text", &text);
	dlog_print(DLOG_DEBUG, LOG_TAG, "[entry_layout_open] input:%s", text);
	if (text != NULL) {
		elm_entry_entry_set(entry, text);
		elm_entry_cursor_end_set(entry);
	}

	elm_object_part_content_set(layout, "part.entry", entry);

	gEntryData = malloc(sizeof(EntryData));
	gEntryData->navi = navi;
	gEntryData->result = result;

	return elm_naviframe_item_simple_push(navi, layout);
}
