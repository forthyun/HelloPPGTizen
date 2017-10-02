#include "helloppg.h"

/* global variables */
sensor_h gSensorHRM, gSensorPPG;
sensor_listener_h gListenerHRM, gListenerPPG;
bool gSensorSupported[2];
int gCount = 0, gHRMCount = 0, gPPGCount = 0;
unsigned long long gHRMTimeArray[BIG_NUMBER], gPPGTimeArray[BIG_NUMBER];
float gHRMBeatArray[BIG_NUMBER];
int gPPGLightArray[BIG_NUMBER];

/* Define callback */
void
example_sensor_callback(sensor_h sensor, sensor_event_s *event, appdata_s *ad)
{
	char tmp_txt[100];
    /* If a callback function is used to listen to different sensor types, it can check the sensor type */
    sensor_type_e type;
    sensor_get_type(sensor, &type);

    if (type == SENSOR_HRM) {
        unsigned long long timestamp = event->timestamp;
        float beat = event->values[0];
        sprintf(tmp_txt, "BPM: %.2f", beat);
        elm_object_text_set(ad->txt_bpm, tmp_txt);
        gHRMTimeArray[gHRMCount] = timestamp;
        gHRMBeatArray[gHRMCount] = beat;
        gHRMCount = gHRMCount + 1;
    }

    if (type == SENSOR_HRM_LED_GREEN) {
        unsigned long long timestamp = event->timestamp;
        int light = event->values[0];
        sprintf(tmp_txt, "PPG: %d", light);
        elm_object_text_set(ad->txt_ppg, tmp_txt);
        gPPGTimeArray[gPPGCount] = timestamp;
        gPPGLightArray[gPPGCount] = light;
        gPPGCount = gPPGCount + 1;
    }
}

static void bt_save_cb(appdata_s *ad, Evas_Object *obj, void *event_info) {
	char tmp_txt[100];
	char buffer[255];
	char DOCUMENTS_PATH[] = "/opt/usr/media/Documents/";

	gCount = gCount + 1;
	sprintf(tmp_txt, "%stest_hrm_%d.csv",DOCUMENTS_PATH, gCount);
	FILE *fp = fopen(tmp_txt, "w");
	for(int i = 0; i < gHRMCount; i++ ) {
		fprintf(fp, "%llu,%f\n", gHRMTimeArray[i], gHRMBeatArray[i]);
	}
	fclose(fp);
	sprintf(tmp_txt, "%stest_ppg_%d.csv",DOCUMENTS_PATH, gCount);
	fp = fopen(tmp_txt, "w");
	for(int i = 0; i < gPPGCount; i++ ) {
		fprintf(fp, "%llu,%d\n", gPPGTimeArray[i], gPPGLightArray[i]);
	}
	fclose(fp);
	sprintf(tmp_txt, "Saved:(%d, %d, %d)", gCount, gHRMCount, gPPGCount);
	elm_object_text_set(ad->txt_msg, tmp_txt);
	dlog_print(DLOG_INFO, LOG_TAG, tmp_txt);
}

static void bt_exit_cb(appdata_s *ad, Evas_Object *obj, void *event_info) {
	ui_app_exit();
}

static void
win_delete_request_cb(appdata_s *ad, Evas_Object *obj, void *event_info)
{
	ui_app_exit();
}

static void
win_back_cb(void *data, Evas_Object *obj, void *event_info)
{
	appdata_s *ad = data;
	/* Let window go to hide state. */
	elm_win_lower(ad->win);
}

static void
create_base_gui(appdata_s *ad)
{
	/* Window */
	/* Create and initialize elm_win.
	   elm_win is mandatory to manipulate window. */
	ad->win = elm_win_util_standard_add(PACKAGE, PACKAGE);
	elm_win_autodel_set(ad->win, EINA_TRUE);

	if (elm_win_wm_rotation_supported_get(ad->win)) {
		int rots[4] = { 0, 90, 180, 270 };
		elm_win_wm_rotation_available_rotations_set(ad->win, (const int *)(&rots), 4);
	}

	evas_object_smart_callback_add(ad->win, "delete,request", win_delete_request_cb, NULL);
	eext_object_event_callback_add(ad->win, EEXT_CALLBACK_BACK, win_back_cb, ad);

	/* Conformant */
	/* Create and initialize elm_conformant.
	   elm_conformant is mandatory for base gui to have proper size
	   when indicator or virtual keypad is visible. */
	ad->conform = elm_conformant_add(ad->win);
	elm_win_indicator_mode_set(ad->win, ELM_WIN_INDICATOR_SHOW);
	elm_win_indicator_opacity_set(ad->win, ELM_WIN_INDICATOR_OPAQUE);
	evas_object_size_hint_weight_set(ad->conform, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_win_resize_object_add(ad->win, ad->conform);
	evas_object_show(ad->conform);

	/* Grid and objects */
	ad->grid = elm_grid_add(ad->conform);
	evas_object_size_hint_weight_set(ad->grid, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_grid_size_set(ad->grid, 9, 9);
	elm_object_content_set(ad->conform, ad->grid);

	ad->txt_bpm = elm_label_add(ad->grid);
	//elm_object_text_set(ad->txt_bpm, "<align=center>BPM: number</align>");
	elm_object_text_set(ad->txt_bpm, "BPM: number");
	evas_object_size_hint_weight_set(ad->txt_bpm, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(ad->txt_bpm,  EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_grid_pack(ad->grid, ad->txt_bpm, 1, 2, 7, 1);
	evas_object_show(ad->txt_bpm);

	ad->txt_ppg = elm_label_add(ad->grid);
	elm_object_text_set(ad->txt_ppg, "PPG: number");
	evas_object_size_hint_weight_set(ad->txt_ppg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(ad->txt_ppg,  EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_grid_pack(ad->grid, ad->txt_ppg, 1, 3, 7, 1);
	evas_object_show(ad->txt_ppg);

	ad->txt_msg = elm_label_add(ad->grid);
	elm_object_text_set(ad->txt_msg, "");
	evas_object_size_hint_weight_set(ad->txt_msg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(ad->txt_msg,  EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_grid_pack(ad->grid, ad->txt_msg, 1, 4, 7, 1);
	evas_object_show(ad->txt_msg);

	ad->bt_save = elm_button_add(ad->grid);
	elm_object_text_set(ad->bt_save, "Save");
	evas_object_size_hint_weight_set(ad->bt_save, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(ad->bt_save, EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_grid_pack(ad->grid, ad->bt_save, 1, 6, 3, 1);
	evas_object_smart_callback_add(ad->bt_save, "clicked", bt_save_cb, ad);
	evas_object_show(ad->bt_save);

	ad->bt_exit = elm_button_add(ad->grid);
	elm_object_text_set(ad->bt_exit, "Exit");
	evas_object_size_hint_weight_set(ad->bt_exit, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(ad->bt_exit, EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_grid_pack(ad->grid, ad->bt_exit, 5, 6, 3, 1);
	evas_object_smart_callback_add(ad->bt_exit, "clicked", bt_exit_cb, ad);
	evas_object_show(ad->bt_exit);

	/* Show window after base gui is set up */
	evas_object_show(ad->win);
}

static bool
app_create(void *data)
{
	/* Hook to take necessary actions before main event loop starts
		Initialize UI resources and application's data
		If this function returns true, the main loop of application starts
		If this function returns false, the application is terminated */
	appdata_s *ad = data;

	create_base_gui(ad);

	//Create sensor listener
	/* get the default sensor of heart rate monitor */
	bool supported = false;
	sensor_is_supported(SENSOR_HRM, &supported);
	if(supported) {
		gSensorSupported[0] = true;
		sensor_get_default_sensor(SENSOR_HRM, &gSensorHRM);
		sensor_create_listener(gSensorHRM, &gListenerHRM);
		sensor_listener_set_event_cb(gListenerHRM, 0, example_sensor_callback, ad);
		sensor_listener_start(gListenerHRM);
	} else {
		gSensorSupported[0] = false;
		elm_object_text_set(ad->txt_bpm, "BPM: not supported");
	}

	sensor_is_supported(SENSOR_HRM_LED_GREEN, &supported);
	if(supported) {
		gSensorSupported[1] = true;
		sensor_get_default_sensor(SENSOR_HRM_LED_GREEN, &gSensorPPG);
		sensor_create_listener(gSensorPPG, &gListenerPPG);
		sensor_listener_set_event_cb(gListenerPPG, 0, example_sensor_callback, ad);
		sensor_listener_start(gListenerPPG);
	} else {
		gSensorSupported[0] = false;
		elm_object_text_set(ad->txt_ppg, "PPG: not supported");
	}

	return true;
}

static void
app_control(app_control_h app_control, void *data)
{
	/* Handle the launch request. */
}

static void
app_pause(void *data)
{
	/* Take necessary actions when application becomes invisible. */
}

static void
app_resume(void *data)
{
	/* Take necessary actions when application becomes visible. */
}

static void
app_terminate(void *data)
{
	/* Release all resources. */
	if(gSensorSupported[0] == true) {
		sensor_listener_stop(gListenerHRM);
		sensor_destroy_listener(gListenerHRM);
	}
	if(gSensorSupported[1] == true) {
		sensor_listener_stop(gListenerPPG);
		sensor_destroy_listener(gListenerPPG);
	}

}

static void
ui_app_lang_changed(app_event_info_h event_info, void *user_data)
{
	/*APP_EVENT_LANGUAGE_CHANGED*/
	char *locale = NULL;
	system_settings_get_value_string(SYSTEM_SETTINGS_KEY_LOCALE_LANGUAGE, &locale);
	elm_language_set(locale);
	free(locale);
	return;
}

static void
ui_app_orient_changed(app_event_info_h event_info, void *user_data)
{
	/*APP_EVENT_DEVICE_ORIENTATION_CHANGED*/
	return;
}

static void
ui_app_region_changed(app_event_info_h event_info, void *user_data)
{
	/*APP_EVENT_REGION_FORMAT_CHANGED*/
}

static void
ui_app_low_battery(app_event_info_h event_info, void *user_data)
{
	/*APP_EVENT_LOW_BATTERY*/
}

static void
ui_app_low_memory(app_event_info_h event_info, void *user_data)
{
	/*APP_EVENT_LOW_MEMORY*/
}

int
main(int argc, char *argv[])
{
	appdata_s ad = {0,};
	int ret = 0;

	ui_app_lifecycle_callback_s event_callback = {0,};
	app_event_handler_h handlers[5] = {NULL, };

	event_callback.create = app_create;
	event_callback.terminate = app_terminate;
	event_callback.pause = app_pause;
	event_callback.resume = app_resume;
	event_callback.app_control = app_control;

	ui_app_add_event_handler(&handlers[APP_EVENT_LOW_BATTERY], APP_EVENT_LOW_BATTERY, ui_app_low_battery, &ad);
	ui_app_add_event_handler(&handlers[APP_EVENT_LOW_MEMORY], APP_EVENT_LOW_MEMORY, ui_app_low_memory, &ad);
	ui_app_add_event_handler(&handlers[APP_EVENT_DEVICE_ORIENTATION_CHANGED], APP_EVENT_DEVICE_ORIENTATION_CHANGED, ui_app_orient_changed, &ad);
	ui_app_add_event_handler(&handlers[APP_EVENT_LANGUAGE_CHANGED], APP_EVENT_LANGUAGE_CHANGED, ui_app_lang_changed, &ad);
	ui_app_add_event_handler(&handlers[APP_EVENT_REGION_FORMAT_CHANGED], APP_EVENT_REGION_FORMAT_CHANGED, ui_app_region_changed, &ad);

	ret = ui_app_main(argc, argv, &event_callback, &ad);
	if (ret != APP_ERROR_NONE) {
		dlog_print(DLOG_ERROR, LOG_TAG, "app_main() is failed. err = %d", ret);
	}

	return ret;
}
