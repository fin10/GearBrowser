#ifndef __common_H__
#define __common_H__

#ifdef  LOG_TAG
#undef  LOG_TAG
#endif
#define LOG_TAG "gear_browser"

#if !defined(PACKAGE)
#define PACKAGE "com.fin10.tizen.gearbrowser"
#endif

void app_get_resource(const char *edj_file_in, char *edj_path_out);

#endif /* __common_H__ */
