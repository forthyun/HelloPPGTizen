#ifndef __helloppg_H__
#define __helloppg_H__

#include <app.h>
#include <Elementary.h>
#include <system_settings.h>
#include <efl_extension.h>
#include <dlog.h>
#include <sensor.h>
#include <storage.h>

#ifdef  LOG_TAG
#undef  LOG_TAG
#endif
#define LOG_TAG "helloppg"

#if !defined(PACKAGE)
#define PACKAGE "org.example.helloppg"
#endif

#define BIG_NUMBER 100000

typedef struct appdata {
	Evas_Object *win;
	Evas_Object *conform;
	Evas_Object *grid;
	Evas_Object *txt_bpm;
	Evas_Object *txt_ppg;
	Evas_Object *txt_msg;
	Evas_Object *bt_save;
	Evas_Object *bt_exit;
} appdata_s;

void
example_sensor_callback(sensor_h sensor, sensor_event_s *event, appdata_s *ad);

static void
bt_save_cb(appdata_s *ad, Evas_Object *obj, void *event_info);


static void
bt_exit_cb(appdata_s *ad, Evas_Object *obj, void *event_info);

#endif /* __helloppg_H__ */
