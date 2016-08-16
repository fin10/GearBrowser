#include <app.h>
#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include "common.h"

void
app_get_resource(const char *edj_file_in, char *edj_path_out)
{
	char* res_path = app_get_resource_path();
	if (res_path) {
		snprintf(edj_path_out, PATH_MAX, "%s%s", res_path, edj_file_in);
		free(res_path);
	}
}
