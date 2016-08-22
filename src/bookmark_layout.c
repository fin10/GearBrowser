#include <Elementary.h>
#include <dlog.h>
#include <bundle.h>
#include <efl_extension.h>
#include "gearbrowser.h"
#include "utils.h"
#include "bookmark_layout.h"
#include "bookmark_model.h"
#include "entry_layout.h"
#include "add_bookmark_layout.h"

typedef struct bookmark_data {
	Evas_Object *navi;
	Evas_Object *layout;
	Evas_Object *genlist;
	Evas_Object *emptyLayout;
	Eext_Circle_Surface *surface;
	Evas_Object *moreOption;
	bundle *result;
	GPtrArray *items;
	char *title;
	char *url;
} BookmarkData;

BookmarkData *gBookmarkData = NULL;

static void genlist_refresh(void);

void
bookmark_layout_release(void) {
	dlog_print(DLOG_DEBUG, LOG_TAG, "[bookmark_layout_release]");
	if (gBookmarkData != NULL) {
		eext_circle_surface_del(gBookmarkData->surface);
		g_ptr_array_free(gBookmarkData->items, TRUE);
		free(gBookmarkData);
		gBookmarkData = NULL;
	}
}

static BookmarkModel *
get_current_model() {
	Evas_Coord x, y, w, h;
	evas_object_geometry_get(gBookmarkData->genlist, &x, &y, &w, &h);
	int posRet = 0;
	Elm_Object_Item *it = elm_genlist_at_xy_item_get(gBookmarkData->genlist, w/2 - x, h/2 - y, &posRet);
	int index = elm_genlist_item_index_get(it);
	dlog_print(DLOG_DEBUG, LOG_TAG, "[get_current_model] index:%d", index);
	return g_ptr_array_index(gBookmarkData->items, index - 2);
}

static void
delete_popup_ok_cb(void *data, Evas_Object *obj, void *event_info) {
	bookmark_model_remove(get_current_model());
	elm_popup_dismiss(data);
	eext_more_option_opened_set(gBookmarkData->moreOption, EINA_FALSE);
	genlist_refresh();
}

static void
delete_popup_hide_cb(void *data, Evas_Object *obj, void *event_info) {
	dlog_print(DLOG_DEBUG, LOG_TAG, "[delete_popup_hide_cb]");
	Evas_Object *popup = data;
	elm_popup_dismiss(popup);
}

static void
delete_popup_hide_finished_cb(void *data, Evas_Object *obj, void *event_info) {
	dlog_print(DLOG_DEBUG, LOG_TAG, "[delete_popup_hide_finished_cb]");
	Evas_Object *popup = data;
	eext_object_event_callback_del(popup, EEXT_CALLBACK_BACK, delete_popup_hide_cb);
	eext_object_event_callback_del(popup, EEXT_CALLBACK_BACK, delete_popup_hide_finished_cb);
	evas_object_del(popup);
}

static void
delete_popup_open(BookmarkModel *model) {
	Evas_Object *popup = elm_popup_add(gBookmarkData->layout);
	elm_object_style_set(popup, "circle");
	evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	eext_object_event_callback_add(popup, EEXT_CALLBACK_BACK, delete_popup_hide_cb, popup);
	evas_object_smart_callback_add(popup, "dismissed", delete_popup_hide_finished_cb, popup);

	Evas_Object *layout = elm_layout_add(popup);
	elm_layout_theme_set(layout, "layout", "popup", "content/circle/buttons2");
	elm_object_part_text_set(layout, "elm.text.title", model->title);
	elm_object_part_text_set(layout, "elm.text", "Delete this page?");
	elm_object_content_set(popup, layout);

	Evas_Object *btn = elm_button_add(popup);
	elm_object_style_set(btn, "popup/circle/left");
	evas_object_size_hint_weight_set(btn, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_object_part_content_set(popup, "button1", btn);
	evas_object_smart_callback_add(btn, "clicked", delete_popup_hide_cb, popup);

	char img_path[PATH_MAX] = {0, };
	app_get_resource("image/tw_ic_popup_btn_delete.png", img_path);
	Evas_Object *icon = elm_image_add(btn);
	elm_image_file_set(icon, img_path, NULL);
	evas_object_size_hint_weight_set(icon, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_object_part_content_set(btn, "elm.swallow.content", icon);

	btn = elm_button_add(popup);
	elm_object_style_set(btn, "popup/circle/right");
	evas_object_size_hint_weight_set(btn, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_object_part_content_set(popup, "button2", btn);
	evas_object_smart_callback_add(btn, "clicked", delete_popup_ok_cb, popup);

	app_get_resource("image/tw_ic_popup_btn_check.png", img_path);
	icon = elm_image_add(btn);
	elm_image_file_set(icon, img_path, NULL);
	evas_object_size_hint_weight_set(icon, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_object_part_content_set(btn, "elm.swallow.content", icon);

	evas_object_show(popup);
}

static Eina_Bool
add_bookmark_result_cb(void *data, Elm_Object_Item *it) {
	dlog_print(DLOG_DEBUG, LOG_TAG, "[add_bookmark_result_cb]");
	bundle *result = data;
	bundle_free(result);
	elm_naviframe_item_pop_cb_set(it, NULL, NULL);
	add_bookmark_layout_release();
	eext_more_option_opened_set(gBookmarkData->moreOption, EINA_FALSE);
	genlist_refresh();

	return EINA_TRUE;
}

static void
mo_item_clicked(void *data, Evas_Object *obj, void *event_info) {
	Eext_Object_Item *item = (Eext_Object_Item *)event_info;
	const char *mainText = eext_more_option_item_part_text_get(item, "selector,main_text");
	dlog_print(DLOG_DEBUG, LOG_TAG, "[mo_item_clicked] %s", mainText);
	if (!strcmp(mainText, "Add")) {
		bundle *result = bundle_create();
		bundle_add_str(result, "title", gBookmarkData->title);
		bundle_add_str(result, "url", gBookmarkData->url);
		Elm_Object_Item *item = add_bookmark_layout_open(gBookmarkData->navi, result);
		elm_naviframe_item_pop_cb_set(item, add_bookmark_result_cb, result);
	} else if (!strcmp(mainText, "Delete")) {
		delete_popup_open(get_current_model());
	}
}

static Evas_Object*
create_more_option(Evas_Object* parent) {
	Evas_Object *moreOption = eext_more_option_add(parent);
	evas_object_smart_callback_add(moreOption, "item,clicked", mo_item_clicked, NULL);

	Eext_Object_Item *item = eext_more_option_item_append(moreOption);
	eext_more_option_item_part_text_set(item, "selector,main_text", "Add");

	Evas_Object *img = elm_image_add(moreOption);
	char add_img_path[PATH_MAX] = {0, };
	app_get_resource("image/add.png", add_img_path);
	elm_image_file_set(img, add_img_path, NULL);
	eext_more_option_item_part_content_set(item, "item,icon", img);

	item = eext_more_option_item_append(moreOption);
	eext_more_option_item_part_text_set(item, "selector,main_text", "Delete");

	img = elm_image_add(moreOption);
	char delete_img_path[PATH_MAX] = {0, };
	app_get_resource("image/delete.png", delete_img_path);
	elm_image_file_set(img, delete_img_path, NULL);
	eext_more_option_item_part_content_set(item, "item,icon", img);

	return moreOption;
}

static char*
gl_title_text_get(void *data, Evas_Object *obj, const char *part) {
	return strdup("Bookmark");
}

static char*
gl_text_get(void *data, Evas_Object *obj, const char *part) {
	BookmarkModel *item = data;
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
create_genlist(Evas_Object *parent, Eext_Circle_Surface *surface) {
	Evas_Object *genlist = elm_genlist_add(parent);
	elm_genlist_mode_set(genlist, ELM_LIST_COMPRESS);

	Evas_Object *circleGenlist = eext_circle_object_genlist_add(genlist, surface);
	eext_circle_object_genlist_scroller_policy_set(circleGenlist, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_AUTO);
	eext_rotary_object_event_activated_set(circleGenlist, EINA_TRUE);

	return genlist;
}

static void
empty_clicked_cb(void *data, Evas_Object *obj, void *event_info) {
	bundle *result = bundle_create();
	bundle_add_str(result, "title", gBookmarkData->title);
	bundle_add_str(result, "url", gBookmarkData->url);
	Elm_Object_Item *item = add_bookmark_layout_open(gBookmarkData->navi, result);
	elm_naviframe_item_pop_cb_set(item, add_bookmark_result_cb, result);
}

static Evas_Object *
create_empty_layout(Evas_Object *parent) {
	Evas_Object *layout = elm_layout_add(parent);
	elm_layout_theme_set(layout, "layout", "nocontents", "default");

	char img_path[PATH_MAX] = {0, };
	app_get_resource("image/ic_bookmark_white_48dp.png", img_path);

	Evas_Object *img = elm_image_add(parent);
	elm_image_file_set(img, img_path, NULL);
	elm_object_part_content_set(layout, "elm.swallow.icon", img);
	elm_object_part_text_set(layout, "elm.text.title", "Bookmark");
	elm_object_part_text_set(layout, "elm.text", "Add this page");
	evas_object_smart_callback_add(img, "clicked", empty_clicked_cb, NULL);

	return layout;
}

static void
genlist_refresh(void) {
	Evas_Object *genlist = gBookmarkData->genlist;
	elm_genlist_clear(genlist);
	if (gBookmarkData->items != NULL) {
		g_ptr_array_free(gBookmarkData->items, TRUE);
	}

	GPtrArray *items = bookmark_model_get_list_n();
	gBookmarkData->items = items;
	dlog_print(DLOG_DEBUG, LOG_TAG, "[genlist_refresh] items:%d", items->len);

	if (items->len == 0) {
		elm_layout_signal_emit(gBookmarkData->layout, "signal,empty,show", "mycode");
		elm_layout_signal_emit(gBookmarkData->layout, "signal,more,hide", "mycode");
	} else {
		elm_layout_signal_emit(gBookmarkData->layout, "signal,empty,hide", "mycode");
		elm_layout_signal_emit(gBookmarkData->layout, "signal,more,show", "mycode");

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
		for (int i = 0; i < size; ++i) {
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
	}
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

	char edj_path[PATH_MAX] = {0, };
	app_get_resource("edje/bookmark_layout.edj", edj_path);

	Evas_Object *layout = elm_layout_add(navi);
	elm_layout_file_set(layout, edj_path, "group.bookmark");
	gBookmarkData->layout = layout;

	Evas_Object *emptyLayout = create_empty_layout(layout);
	elm_object_part_content_set(layout, "part.bookmark.empty", emptyLayout);
	gBookmarkData->emptyLayout = emptyLayout;

	Eext_Circle_Surface *surface = eext_circle_surface_naviframe_add(layout);
	gBookmarkData->surface = surface;

	Evas_Object *genlist = create_genlist(layout, surface);
	elm_object_part_content_set(layout, "part.bookmark.genlist", genlist);
	gBookmarkData->genlist = genlist;

	Evas_Object *moreOption = create_more_option(layout);
	gBookmarkData->moreOption = moreOption;
	elm_object_part_content_set(layout, "part.bookmark.more", moreOption);

	gBookmarkData->items = NULL;
	genlist_refresh();

	return elm_naviframe_item_simple_push(navi, layout);
}
