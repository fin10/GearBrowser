#ifndef SETTINGS_H_
#define SETTINGS_H_

static const char *PREF_KEY_BOOKMARKS = "pref_key_bookmarks";
static const char *PREF_KEY_SEARCH_ENGINE = "pref_key_search_engine";
static const char *PREF_KEY_LAST_SEARCH_KEYWORD = "pref_key_last_search_keyword";
static const char *PREF_KEY_LAST_URL = "pref_key_last_url";

void settings_init(void);
const char *settings_value_get_n(const char *key);
int settings_value_set(const char *key, const char *value);

#endif /* SETTINGS_H_ */
