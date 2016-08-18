#ifndef __gearbrowser_H__
#define __gearbrowser_H__

#include <bundle.h>

#ifdef  LOG_TAG
#undef  LOG_TAG
#endif
#define LOG_TAG "gear_browser"

#if !defined(PACKAGE)
#define PACKAGE "com.fin10.tizen.gearbrowser"
#endif

typedef struct app_data {
	Evas_Object* win;
	Evas_Object* conform;
	Evas_Object* navi;
} AppData;

typedef struct result_data {
	const AppData* appData;
	bundle* result;
} ResultData;

#endif /* __gearbrowser_H__ */
