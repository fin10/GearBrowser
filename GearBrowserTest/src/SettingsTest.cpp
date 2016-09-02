#include <app_preference.h>
#include <gtest/gtest.h>

extern "C" {
	#include "settings.h"
}

class SettingsTestSuite : public testing::Test {
	protected:
	virtual void SetUp() {
	}

	virtual void TearDown() {
		settings_clear();
	}
};

TEST_F(SettingsTestSuite, test_settings_value_set) {
	int ret = settings_value_set("test", "value");
	ASSERT_EQ(ret, PREFERENCE_ERROR_NONE);

	const char *value = settings_value_get_n("test");
	ASSERT_STREQ(value, "value");
}

TEST_F(SettingsTestSuite, test_settings_value_get) {
	settings_value_set("test", "value");

	const char *value = settings_value_get_n("test");
	ASSERT_STREQ(value, "value");
}
