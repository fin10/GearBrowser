#include <app.h>
#include <Elementary.h>
#include <EWebKit.h>
#include <dlog.h>
#include <bundle.h>
#include <system_settings.h>
#include <efl_extension.h>
#include "gearbrowser.h"
#include "utils.h"
#include "web_layout.h"

static void
win_delete_request_cb(void *data, Evas_Object *obj, void *event_info) {
	dlog_print(DLOG_DEBUG, LOG_TAG, "[win_delete_request_cb]");
	ui_app_exit();
}

static void
win_back_cb(void *data, Evas_Object *obj, void *event_info) {
	dlog_print(DLOG_DEBUG, LOG_TAG, "[win_back_cb]");
	AppData *ad = data;
	Eina_List *items = elm_naviframe_items_get(ad->navi);
	if (items != NULL) {
		dlog_print(DLOG_DEBUG, LOG_TAG, "[win_back_cb] count:%d", items->accounting->count);
		if (items->accounting->count == 1) {
			elm_win_lower(ad->win);
		} else {
			elm_naviframe_item_pop(ad->navi);
		}
	}
}

static Eina_Bool
web_close_cb(void *data, Elm_Object_Item *it) {
	dlog_print(DLOG_DEBUG, LOG_TAG, "[web_close_cb]");
	elm_naviframe_item_pop_cb_set(it, NULL, NULL);
	web_layout_release();

	return EINA_TRUE;
}

static void
create_base_gui(AppData *appData) {
	dlog_print(DLOG_DEBUG, LOG_TAG, "[create_base_gui]");

	/* Window */
	appData->win = elm_win_util_standard_add(PACKAGE, PACKAGE);
	elm_win_autodel_set(appData->win, EINA_TRUE);

	if (elm_win_wm_rotation_supported_get(appData->win)) {
		int rots[4] = { 0, 90, 180, 270 };
		elm_win_wm_rotation_available_rotations_set(appData->win, (const int *)(&rots), 4);
	}

	evas_object_smart_callback_add(appData->win, "delete,request", win_delete_request_cb, NULL);

	/* Conformant */
	appData->conform = elm_conformant_add(appData->win);
	elm_win_indicator_mode_set(appData->win, ELM_WIN_INDICATOR_SHOW);
	elm_win_indicator_opacity_set(appData->win, ELM_WIN_INDICATOR_OPAQUE);
	evas_object_size_hint_weight_set(appData->conform, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_win_resize_object_add(appData->win, appData->conform);
	evas_object_show(appData->conform);

	appData->navi = elm_naviframe_add(appData->conform);
	elm_object_content_set(appData->conform, appData->navi);
	Elm_Object_Item *item = web_layout_open(appData->navi);
	elm_naviframe_item_pop_cb_set(item, web_close_cb, NULL);

	eext_object_event_callback_add(appData->navi, EEXT_CALLBACK_BACK, win_back_cb, appData);

	/* Show window after base gui is set up */
	evas_object_show(appData->win);
}

static bool
app_create(void *data) {
	AppData *ad = data;
	create_base_gui(ad);
	return true;
}

static void
app_control(app_control_h app_control, void *data) {
	/* Handle the launch request. */
}

static void
app_pause(void *data) {
	/* Take necessary actions when application becomes invisible. */
}

static void
app_resume(void *data) {
	/* Take necessary actions when application becomes visible. */
}

static void
app_terminate(void *data) {
	/* Release all resources. */
}

static void
ui_app_lang_changed(app_event_info_h event_info, void *user_data) {
	/*APP_EVENT_LANGUAGE_CHANGED*/
	char *locale = NULL;
	system_settings_get_value_string(SYSTEM_SETTINGS_KEY_LOCALE_LANGUAGE, &locale);
	elm_language_set(locale);
	free(locale);
}

static void
ui_app_orient_changed(app_event_info_h event_info, void *user_data) {
	/*APP_EVENT_DEVICE_ORIENTATION_CHANGED*/
	return;
}

static void
ui_app_region_changed(app_event_info_h event_info, void *user_data) {
	/*APP_EVENT_REGION_FORMAT_CHANGED*/
}

static void
ui_app_low_battery(app_event_info_h event_info, void *user_data) {
	/*APP_EVENT_LOW_BATTERY*/
}

static void
ui_app_low_memory(app_event_info_h event_info, void *user_data) {
	/*APP_EVENT_LOW_MEMORY*/
}

int
main(int argc, char *argv[]) {
	AppData *ad = {0,};
	int ret = 0;

	ui_app_lifecycle_callback_s event_callback = {0,};
	app_event_handler_h handlers[5] = {NULL, };

	event_callback.create = app_create;
	event_callback.terminate = app_terminate;
	event_callback.pause = app_pause;
	event_callback.resume = app_resume;
	event_callback.app_control = app_control;

	ui_app_add_event_handler(&handlers[APP_EVENT_LOW_BATTERY], APP_EVENT_LOW_BATTERY, ui_app_low_battery, &ad);
	ui_app_add_event_handler(&handlers[APP_EVENT_LOW_MEMORY], APP_EVENT_LOW_MEMORY, ui_app_low_memory, &ad);
	ui_app_add_event_handler(&handlers[APP_EVENT_DEVICE_ORIENTATION_CHANGED], APP_EVENT_DEVICE_ORIENTATION_CHANGED, ui_app_orient_changed, &ad);
	ui_app_add_event_handler(&handlers[APP_EVENT_LANGUAGE_CHANGED], APP_EVENT_LANGUAGE_CHANGED, ui_app_lang_changed, &ad);
	ui_app_add_event_handler(&handlers[APP_EVENT_REGION_FORMAT_CHANGED], APP_EVENT_REGION_FORMAT_CHANGED, ui_app_region_changed, &ad);
	ui_app_remove_event_handler(handlers[APP_EVENT_LOW_MEMORY]);

	ret = ui_app_main(argc, argv, &event_callback, &ad);
	if (ret != APP_ERROR_NONE) {
		dlog_print(DLOG_ERROR, LOG_TAG, "app_main() is failed. err = %d", ret);
	}

	return ret;
}
