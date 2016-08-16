#include <Elementary.h>
#include <dlog.h>
#include <bundle.h>
#include <app_preference.h>
#include <json-glib.h>
#include "favorite_layout.h"
#include "common.h"

static const char* PREF_KEY_FAVORITES = "pref_key_favorites";

typedef struct appdata {
	Evas_Object* navi;
	Evas_Object* entry;
	bundle* result;
	const char* title;
	const char* url;
} appdata_s;

typedef struct favorite_item {
	const char* title;
	const char* url;
} favorite_item_s;

static const char*
get_preference_n(const char* key)
{
	char* value;
	bool existing;
	preference_is_existing(key, &existing);
	if (existing)
	{
		preference_get_string(key, &value);
	}
	else
	{
		value = strdup("[]");
	}

	return value;
}

static void
add_favorite(const char* json, const appdata_s* ad)
{
	GError* error = NULL;
	JsonParser* parser = json_parser_new();
	json_parser_load_from_data(parser, json, strlen(json), &error);
	if (error)
	{
		dlog_print(DLOG_ERROR, LOG_TAG, "[add_favorite] %s", error);
		g_error_free(error);
	}
	else
	{
		JsonObject* obj = json_object_new();
		json_object_set_string_member(obj, "title", ad->title);
		json_object_set_string_member(obj, "url", ad->url);

		JsonNode* root = json_parser_get_root(parser);
		JsonArray* items = json_node_get_array(root);
		json_array_add_object_element(items, obj);

		JsonGenerator* generator = json_generator_new();
		json_generator_set_root(generator, root);
		gchar* result = json_generator_to_data(generator, NULL);
		dlog_print(DLOG_DEBUG, LOG_TAG, "[add_favorite] result:%s", result);
		int ret = preference_set_string(PREF_KEY_FAVORITES, result);
		dlog_print(DLOG_DEBUG, LOG_TAG, "[add_favorite] preference:%d", ret);
	}
}

static GPtrArray*
get_favorites_n(const char* json)
{
	dlog_print(DLOG_DEBUG, LOG_TAG, "[get_favorites_n] %s", json);

	GPtrArray* result = g_ptr_array_new();
	GError* error = NULL;
	JsonParser* parser = json_parser_new();
	json_parser_load_from_data(parser, json, strlen(json), &error);
	if (error)
	{
		dlog_print(DLOG_ERROR, LOG_TAG, "[get_favorites_n] %s", error);
		g_error_free(error);
	}
	else
	{
		JsonNode* root = json_parser_get_root(parser);
		JsonArray* items = json_node_get_array(root);
		dlog_print(DLOG_DEBUG, LOG_TAG, "[get_favorites_n] size:%d", json_array_get_length(items));
	}

	return result;
}

static char*
gl_title_text_get(void *data, Evas_Object *obj, const char *part)
{
	dlog_print(DLOG_DEBUG, LOG_TAG, "[gl_title_text_get] %s", part);
	return strdup("Favorite");
}

static char*
gl_add_text_get(void *data, Evas_Object *obj, const char *part)
{
	dlog_print(DLOG_DEBUG, LOG_TAG, "[gl_add_text_get] %s", part);
	return strdup("Add to favorites");
}

static char*
gl_text_get(void *data, Evas_Object *obj, const char *part)
{
	dlog_print(DLOG_DEBUG, LOG_TAG, "[gl_text_get] %s", part);
	favorite_item_s* item = data;
	if (strcmp(part, "elm.text.1"))
	{
		return strdup(item->title);
	} else if (strcmp(part, "elm.text.2"))
	{
		return strdup(item->url);
	}

	return strdup("null");
}

static void
gl_add_sel(void *data, Evas_Object *obj, void *event_info)
{
	appdata_s* ad = data;
	const char* json = get_preference_n(PREF_KEY_FAVORITES);
	dlog_print(DLOG_DEBUG, LOG_TAG, "[gl_add_sel] %s", json);
	add_favorite(json, ad);

	free((char*) json);
}

static void
gl_item_sel(void *data, Evas_Object *obj, void *event_info)
{
	dlog_print(DLOG_DEBUG, LOG_TAG, "[gl_item_sel]");
	favorite_item_s* item = data;

}
Elm_Object_Item*
open_favorite_layout(Evas_Object* navi, Evas_Object* parent, bundle* result, const char* title, const char* url)
{
	appdata_s* ad = malloc(sizeof(appdata_s));
	ad->navi = navi;
	ad->result = result;
	ad->title = title;
	ad->url = url;
	dlog_print(DLOG_DEBUG, LOG_TAG, "[open_favorite_layout] %s", title);

	Evas_Object* genlist = elm_genlist_add(parent);

	Elm_Genlist_Item_Class* title_cls = elm_genlist_item_class_new();
	title_cls->item_style = "title";
	title_cls->func.text_get = gl_title_text_get;

	Elm_Genlist_Item_Class* add_cls = elm_genlist_item_class_new();
	add_cls->item_style = "1text";
	add_cls->func.text_get = gl_add_text_get;

	Elm_Genlist_Item_Class* item_cls = elm_genlist_item_class_new();
	item_cls->item_style = "2text";
	item_cls->func.text_get = gl_text_get;

	elm_genlist_item_append(genlist, title_cls, NULL, NULL, ELM_GENLIST_ITEM_NONE, NULL, ad);
	elm_genlist_item_append(genlist, add_cls, NULL, NULL, ELM_GENLIST_ITEM_NONE, gl_add_sel, ad);

	const char* json = get_preference_n(PREF_KEY_FAVORITES);
	GPtrArray* favorites = get_favorites_n(json);
	int size = favorites->len;
	dlog_print(DLOG_DEBUG, LOG_TAG, "size:%d", size);
	for (int i = 0; i < size; ++i)
	{
		favorite_item_s* item = g_ptr_array_index(favorites, i);
		elm_genlist_item_append(genlist, item_cls, item, NULL, ELM_GENLIST_ITEM_NONE, gl_item_sel, item);
	}

	free((char*) json);
	g_ptr_array_free(favorites, FALSE);

	elm_genlist_item_class_free(title_cls);
	elm_genlist_item_class_free(add_cls);
	elm_genlist_item_class_free(item_cls);
	title_cls = NULL;
	add_cls = NULL;
	item_cls = NULL;

	return elm_naviframe_item_simple_push(ad->navi, genlist);
}
