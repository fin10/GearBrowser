#include <app.h>
#include <Elementary.h>
#include <EWebKit.h>
#include <dlog.h>
#include <system_settings.h>
#include <efl_extension.h>
#include "gearbrowser.h"
#include "search_layout.h"
#include "common.h"

typedef struct appdata {
	Evas_Object* win;
	Evas_Object* conform;
	Evas_Object* navi;
} appdata_s;

static void
win_delete_request_cb(void *data, Evas_Object *obj, void *event_info)
{
	dlog_print(DLOG_DEBUG, LOG_TAG, "[win_delete_request_cb]");
	ui_app_exit();
}

static void
win_back_cb(void* data, Evas_Object* obj, void* event_info)
{
	dlog_print(DLOG_DEBUG, LOG_TAG, "[win_back_cb]");
	appdata_s* ad = data;
	Eina_List* items = elm_naviframe_items_get(ad->navi);
	if (items != NULL) {
		dlog_print(DLOG_DEBUG, LOG_TAG, "[win_back_cb] count:%d", items->accounting->count);
		if (items->accounting->count == 1) {
			elm_win_lower(ad->win);
		} else {
			elm_naviframe_item_pop(ad->navi);
		}
	}
}

static void
web_back_cb(void *data, Evas_Object *obj, const char *emission, const char *source)
{
	dlog_print(DLOG_DEBUG, LOG_TAG, "[web_back_cb]");
	Evas_Object* web = data;
	ewk_view_back(web);
}

static void
web_forward_cb(void *data, Evas_Object *obj, const char *emission, const char *source)
{
	dlog_print(DLOG_DEBUG, LOG_TAG, "[web_forward_cb]");
	Evas_Object* web = data;
	ewk_view_forward(web);
}

static void
web_search_cb(void *data, Evas_Object *obj, const char *emission, const char *source)
{
	dlog_print(DLOG_DEBUG, LOG_TAG, "[web_search_cb]");
	appdata_s* ad = data;
	open_searchview(ad->navi, ad->conform);
}

static void
web_favorite_cb(void *data, Evas_Object *obj, const char *emission, const char *source)
{
	dlog_print(DLOG_DEBUG, LOG_TAG, "[web_favorite_cb]");
}

static void
open_webview(appdata_s* ad, Evas_Object* parent)
{
	char edj_path[PATH_MAX] = {0, };
	app_get_resource("edje/web_view_layout.edj", edj_path);
	dlog_print(DLOG_DEBUG, LOG_TAG, "path:%s", edj_path);

	Evas_Object* layout = elm_layout_add(parent);
	elm_layout_file_set(layout, edj_path, "group.web");

	Evas* evas = evas_object_evas_get(layout);
	Evas_Object* web = ewk_view_add(evas);
	elm_object_part_content_set(layout, "part.web", web);
	ewk_view_url_set(web, "http://www.google.com");
	evas_object_show(web);

	elm_object_signal_callback_add(layout, "signal.btn.back.clicked", "*", web_back_cb, web);
	elm_object_signal_callback_add(layout, "signal.btn.forward.clicked", "*", web_forward_cb, web);
	elm_object_signal_callback_add(layout, "signal.btn.search.clicked", "*", web_search_cb, ad);
	elm_object_signal_callback_add(layout, "signal.btn.favorite.clicked", "*", web_favorite_cb, ad);

	elm_naviframe_item_simple_push(ad->navi, layout);
}

static void
create_base_gui(appdata_s* ad)
{
	dlog_print(DLOG_DEBUG, LOG_TAG, "[create_base_gui]");

	/* Window */
	ad->win = elm_win_util_standard_add(PACKAGE, PACKAGE);
	elm_win_autodel_set(ad->win, EINA_TRUE);

	if (elm_win_wm_rotation_supported_get(ad->win)) {
		int rots[4] = { 0, 90, 180, 270 };
		elm_win_wm_rotation_available_rotations_set(ad->win, (const int *)(&rots), 4);
	}

	evas_object_smart_callback_add(ad->win, "delete,request", win_delete_request_cb, NULL);
	eext_object_event_callback_add(ad->win, EEXT_CALLBACK_BACK, win_back_cb, ad);

	/* Conformant */
	ad->conform = elm_conformant_add(ad->win);
	elm_win_indicator_mode_set(ad->win, ELM_WIN_INDICATOR_SHOW);
	elm_win_indicator_opacity_set(ad->win, ELM_WIN_INDICATOR_OPAQUE);
	evas_object_size_hint_weight_set(ad->conform, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_win_resize_object_add(ad->win, ad->conform);
	evas_object_show(ad->conform);

	ad->navi = elm_naviframe_add(ad->conform);
	elm_object_content_set(ad->conform, ad->navi);
	open_webview(ad, ad->conform);

	/* Show window after base gui is set up */
	evas_object_show(ad->win);
}

static bool
app_create(void *data)
{
	/* Hook to take necessary actions before main event loop starts
		Initialize UI resources and application's data
		If this function returns true, the main loop of application starts
		If this function returns false, the application is terminated */
	appdata_s *ad = data;

	create_base_gui(ad);

	return true;
}

static void
app_control(app_control_h app_control, void *data)
{
	/* Handle the launch request. */
}

static void
app_pause(void *data)
{
	/* Take necessary actions when application becomes invisible. */
}

static void
app_resume(void *data)
{
	/* Take necessary actions when application becomes visible. */
}

static void
app_terminate(void *data)
{
	/* Release all resources. */
}

static void
ui_app_lang_changed(app_event_info_h event_info, void *user_data)
{
	/*APP_EVENT_LANGUAGE_CHANGED*/
	char *locale = NULL;
	system_settings_get_value_string(SYSTEM_SETTINGS_KEY_LOCALE_LANGUAGE, &locale);
	elm_language_set(locale);
	free(locale);
	return;
}

static void
ui_app_orient_changed(app_event_info_h event_info, void *user_data)
{
	/*APP_EVENT_DEVICE_ORIENTATION_CHANGED*/
	return;
}

static void
ui_app_region_changed(app_event_info_h event_info, void *user_data)
{
	/*APP_EVENT_REGION_FORMAT_CHANGED*/
}

static void
ui_app_low_battery(app_event_info_h event_info, void *user_data)
{
	/*APP_EVENT_LOW_BATTERY*/
}

static void
ui_app_low_memory(app_event_info_h event_info, void *user_data)
{
	/*APP_EVENT_LOW_MEMORY*/
}

int
main(int argc, char *argv[])
{
	appdata_s ad = {0,};
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
