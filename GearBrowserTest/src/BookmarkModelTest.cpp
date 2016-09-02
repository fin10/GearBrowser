#include <gtest/gtest.h>

extern "C" {
	#include "settings.h"
	#include "bookmark_model.h"
}

class BookmarkModelTestSuite : public testing::Test {
	protected:
	virtual void SetUp() {
		settings_init();
	}

	virtual void TearDown() {
		Eina_Array *models = bookmark_model_get_list_n();
		unsigned int size = eina_array_count(models);
		for (int i = 0; i < size; ++i) {
			bookmark_model_remove((BookmarkModel *)eina_array_data_get(models, i));
		}

		for (int i = 0; i < size; ++i) {
			free(eina_array_data_get(models, i));
		}
		eina_array_free(models);
	}
};

static void
eina_array_clear(Eina_Array * arr) {
	unsigned int size = eina_array_count(arr);
	for (int i = 0; i < size; ++i) {
		free(eina_array_data_get(arr, i));
	}
	eina_array_free(arr);
}

TEST_F(BookmarkModelTestSuite, test_bookmark_add) {
	Eina_Bool ret = bookmark_model_add("title", "url");
	ASSERT_EQ(ret, EINA_TRUE);

	Eina_Array *models = bookmark_model_get_list_n();
	BookmarkModel *model = (BookmarkModel *)eina_array_data_get(models, 0);
	ASSERT_STREQ(model->title, "title");
	ASSERT_STREQ(model->url, "url");

	eina_array_clear(models);
}

TEST_F(BookmarkModelTestSuite, test_bookmark_remove) {
	bookmark_model_add("no_title", "empty");

	Eina_Array *before_models = bookmark_model_get_list_n();
	BookmarkModel *model = (BookmarkModel *)eina_array_data_get(before_models, 0);
	Eina_Bool ret = bookmark_model_remove(model);
	ASSERT_EQ(ret, EINA_TRUE);

	Eina_Array *after_models = bookmark_model_get_list_n();
	unsigned int size = eina_array_count(after_models);
	ASSERT_EQ(size, 0);
	eina_array_free(after_models);

	eina_array_clear(before_models);
}
