#include <Elementary.h>
#include <dlog.h>
#include <bundle.h>
#include <efl_extension.h>
#include "gearbrowser.h"
#include "utils.h"
#include "bookmark_layout.h"
#include "bookmark_model.h"
#include "entry_layout.h"

typedef struct bookmark_data {
	Evas_Object *navi;
	Evas_Object *layout;
	Evas_Object *genlist;
	Eext_Circle_Surface *surface;
	bundle *result;
	GPtrArray *items;
	char *title;
	char *url;
} BookmarkData;

BookmarkData *gBookmarkData = NULL;

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

static void
mo_item_clicked(void *data, Evas_Object *obj, void *event_info) {
	Eext_Object_Item *item = (Eext_Object_Item *)event_info;
	const char *mainText = eext_more_option_item_part_text_get(item, "selector,main_text");
	dlog_print(DLOG_DEBUG, LOG_TAG, "[mo_item_clicked] %s", mainText);
	if (!strcmp(mainText, "Bookmark")) {
		bundle_add_str(gBookmarkData->result, "action", "add_bookmark");
		elm_naviframe_item_pop(gBookmarkData->navi);
	} else if (!strcmp(mainText, "Delete")) {
		Evas_Coord x, y, w, h;
		evas_object_geometry_get(gBookmarkData->genlist, &x, &y, &w, &h);
		dlog_print(DLOG_DEBUG, LOG_TAG, "[mo_item_clicked] x:%d, y:%d, w:%d, h:%d", x, y, w, h);
		int posRet = 0;
		Elm_Object_Item *it = elm_genlist_at_xy_item_get(gBookmarkData->genlist, w/2, h/2, &posRet);
		int index = elm_genlist_item_index_get(it);
		dlog_print(DLOG_DEBUG, LOG_TAG, "[mo_item_clicked] index:%d", index);
		BookmarkModel *model = g_ptr_array_index(gBookmarkData->items, index - 2);
		if (model != NULL) {
			bookmark_model_remove(model);
			elm_naviframe_item_pop(gBookmarkData->navi);
		}
	}
}

static Evas_Object*
create_more_option(Evas_Object* layout) {
	Evas_Object *moreOption = eext_more_option_add(layout);
	elm_object_part_content_set(layout, "part.bookmark.more", moreOption);

	evas_object_smart_callback_add(moreOption, "item,clicked", mo_item_clicked, NULL);

	Eext_Object_Item *item = eext_more_option_item_append(moreOption);
	eext_more_option_item_part_text_set(item, "selector,main_text", "Bookmark");
	eext_more_option_item_part_text_set(item, "selector,sub_text", "this page");

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
create_genlist(Evas_Object *parent, GPtrArray *items) {
	char edj_path[PATH_MAX] = {0, };
	app_get_resource("edje/bookmark_layout.edj", edj_path);

	Evas_Object *layout = elm_layout_add(parent);
	elm_layout_file_set(layout, edj_path, "group.bookmark");
	gBookmarkData->layout = layout;

	Evas_Object *genlist = elm_genlist_add(layout);
	elm_object_part_content_set(layout, "part.bookmark.bg", genlist);
	elm_genlist_mode_set(genlist, ELM_LIST_COMPRESS);
	gBookmarkData->genlist = genlist;

	Eext_Circle_Surface *surface = eext_circle_surface_naviframe_add(parent);
	Evas_Object *circleGenlist = eext_circle_object_genlist_add(genlist, surface);
	eext_circle_object_genlist_scroller_policy_set(circleGenlist, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_AUTO);
	eext_rotary_object_event_activated_set(circleGenlist, EINA_TRUE);
	gBookmarkData->surface = surface;

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
	for (int i = 0; i < size; ++i)
	{
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

	create_more_option(layout);

	return layout;
}

Elm_Object_Item*
bookmark_layout_open(Evas_Object *navi, bundle *result) {
	if (gBookmarkData != NULL) {
		dlog_print(DLOG_ERROR, LOG_TAG, "[bookmark_layout_open] Do not allowed to open repeatedly.");
		return NULL;
	}

	GPtrArray *items = bookmark_model_get_list_n();
	dlog_print(DLOG_DEBUG, LOG_TAG, "[bookmark_layout_open] size:%d", items->len);
	if (items->len == 0) {
		g_ptr_array_free(items, TRUE);
		bundle_add_str(result, "action", "add_bookmark");
		return NULL;
	} else {
		gBookmarkData = malloc(sizeof(BookmarkData));
		gBookmarkData->navi = navi;
		gBookmarkData->result = result;
		gBookmarkData->items = items;
		bundle_get_str(result, "title", &gBookmarkData->title);
		bundle_get_str(result, "url", &gBookmarkData->url);
		dlog_print(DLOG_DEBUG, LOG_TAG, "[bookmark_layout_open] %s, %s", gBookmarkData->title, gBookmarkData->url);

		Evas_Object *content = create_genlist(navi, items);
		return elm_naviframe_item_simple_push(navi, content);
	}
}
