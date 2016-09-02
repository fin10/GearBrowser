#include <app.h>
#include <Elementary.h>
#include "utils.h"

const char *
app_get_resource_n(const char *file_name) {
	char output[PATH_MAX] = {0, };
	char *res_path = app_get_resource_path();
	if (res_path) {
		snprintf(output, PATH_MAX, "%s%s", res_path, file_name);
		free(res_path);
	}

	return output;
}
