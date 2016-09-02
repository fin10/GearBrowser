#include <app_preference.h>
#include "settings.h"

void
settings_init(void) {
	bool existing;
	preference_is_existing(PREF_KEY_BOOKMARKS, &existing);
	if (!existing) preference_set_string(PREF_KEY_BOOKMARKS, "[]");
	preference_is_existing(PREF_KEY_SEARCH_ENGINE, &existing);
	if (!existing) preference_set_string(PREF_KEY_SEARCH_ENGINE, "https://www.google.co.kr/search?q=");
	preference_is_existing(PREF_KEY_LAST_SEARCH_KEYWORD, &existing);
	if (!existing) preference_set_string(PREF_KEY_LAST_SEARCH_KEYWORD, "");
	preference_is_existing(PREF_KEY_LAST_URL, &existing);
	if (!existing) preference_set_string(PREF_KEY_LAST_URL, "http://www.google.com");
}

int
settings_clear(void) {
	return preference_remove_all();
}

const char *
settings_value_get_n(const char *key) {
	char *value = NULL;
	preference_get_string(key, &value);
	return value;
}

int
settings_value_set(const char *key, const char *value) {
	return preference_set_string(key, value);
}

