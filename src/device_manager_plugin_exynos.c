/*
* Copyright (c) 2012 Samsung Electronics Co., Ltd All Rights Reserved
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/


#include <string.h>
#include <sys/types.h>
#include <stdio.h>
#include <dirent.h>
#include <errno.h>
#include <devman_plugin_intf.h>

#include "devman_define_node_path.h"

#define EXPORT_API  __attribute__((visibility("default")))

#define BUFF_MAX	255
#define MAX_NAME 255


/* TODO: Add APIs has (char *) params */

#define GENERATE_ACCESSORS_INT_RW(_suffix, _item)	\
int OEM_sys_get_##_suffix(int *value)			\
{						\
	return sys_get_int(_item, value);	\
}						\
						\
int OEM_sys_set_##_suffix(int value)	\
{						\
	return sys_set_int(_item, value);	\
}

#define GENERATE_ACCESSORS_INT_R(_suffix, _item)	\
int OEM_sys_get_##_suffix(int *value)			\
{						\
	return sys_get_int(_item, value);	\
}

#define GENERATE_ACCESSORS_INT_W(_suffix, _item)	\
int OEM_sys_set_##_suffix(int value)			\
{						\
	return sys_set_int(_item, value);	\
}

/*
GENERATE_ACCESSORS_INT_R(backlight_max_brightness, BACKLIGHT_MAX_BRIGHTNESS_PATH)
GENERATE_ACCESSORS_INT_RW(backlight_brightness, BACKLIGHT_BRIGHTNESS_PATH)
GENERATE_ACCESSORS_INT_RW(backlight_acl_control, LCD_ACL_CONTROL_PATH)
GENERATE_ACCESSORS_INT_RW(lcd_power, LCD_POWER_PATH)
*/
#if defined(DEVMGR_LOG)
#define devmgr_log(fmt, args...) \
	do { \
		printf("%s:"fmt"\n", __func__, ##args); \
	}while(0);
#else
#define devmgr_log(fmt, args...)
#endif

enum display_type
{
	DISP_MAIN = 0,
	DISP_SUB,
	DISP_MAX
};

enum lux_status {
	decrement,
	no_change,
	increment,
};

struct display_info
{
	enum display_type etype; /* FIXME:!! Main LCD or Sub LCD node */
	char bl_name[MAX_NAME]; /* backlight name */
	char lcd_name[MAX_NAME]; /* lcd name */
};

#define MAX_CANDELA_CRITERION	300
#define PWR_SAVING_CANDELA_CRITERION	20

/* FIXME:!! change to global_ctx */
int lcd_index;
struct display_info disp_info[DISP_MAX];

/* This function is not supported */
int OEM_sys_get_backlight_brightness_by_lux(unsigned int lux, int *value)
{
	/* TODO */

	return 0;
}

static int OEM_sys_display_info(struct display_info *disp_info)
{
	struct dirent *dent;
	DIR *dirp;
	int i, index;
	const char * bl_path = BACKLIGHT_PATH;
	const char * lcd_path = LCD_PATH;

	/* Backlight */
	index = 0;
	dirp = opendir(bl_path);
	if (dirp) {
		while(dent = readdir(dirp)) {
			if (index >= DISP_MAX) {
				devmgr_log("supports %d display node", DISP_MAX);
				break;
			}

			if (!strcmp(".", dent->d_name) || !strcmp("..", dent->d_name))
				continue;
			else {
				strcpy(disp_info[index].bl_name, dent->d_name);
				index++;
			}
		}
		closedir(dirp);
	}

	for (i = 0; i < index; i++)
		devmgr_log("bl_name[%s]", disp_info[i].bl_name);

	/* LCD */
	index = 0;
	dirp = opendir(lcd_path);
	if (dirp) {
		while(dent = readdir(dirp)) {
			if (index >= DISP_MAX) {
				devmgr_log("supports %d display node", DISP_MAX);
				break;
			}

			if (!strcmp(".", dent->d_name) || !strcmp("..", dent->d_name))
				continue;
			else {
				strcpy(disp_info[index].lcd_name, dent->d_name);
				index++;
			}
		}
		closedir(dirp);
	}

	for (i = 0; i < index; i++)
		devmgr_log("lcd_name[%s]", disp_info[i].lcd_name);

	lcd_index = index;
}

int OEM_sys_get_display_count(int *value)
{
	int ret = -1;

	/* TODO: We should implement to find out current number of display */
	*value = lcd_index;
	ret = 0;
	/* ********************* */

	devmgr_log("value[%d]", *value);

	return ret;
}

int OEM_sys_get_backlight_max_brightness(int index, int *value)
{
	int ret = -1;
	char path[MAX_NAME+1];

	if (index >= DISP_MAX) {
		devmgr_log("supports %d display node", DISP_MAX);
		return ret;
	}

	snprintf(path, MAX_NAME, BACKLIGHT_MAX_BRIGHTNESS_PATH, disp_info[index].bl_name);
	ret = sys_get_int(path, value);
	devmgr_log("path[%s]value[%d]", path, *value);

	return ret;
}

int OEM_sys_get_backlight_min_brightness(int index, int *value)
{
	int ret = -1;
	char path[MAX_NAME+1];

	if (index >= DISP_MAX) {
		devmgr_log("supports %d display node", DISP_MAX);
		return ret;
	}

	snprintf(path, MAX_NAME, BACKLIGHT_MIN_BRIGHTNESS_PATH, disp_info[index].bl_name);
	ret = sys_get_int(path, value);

        /* s6e8ax0 driver doesn't support min_brightness node - return default/empirical value */
        if (ret < 0) {
                *value = 0;
                devmgr_log("Can't read min_brightness node[%s] default value[%d]", path, *value);
                return 0;
        }

	devmgr_log("path[%s]value[%d]", path, *value);

	return ret;
}

int OEM_sys_get_backlight_brightness(int index, int *value, int power_saving)
{
	int ret = -1;
	char path[MAX_NAME+1];
	int max_brightness;
	int pwr_saving_offset;

	if (index >= DISP_MAX) {
		devmgr_log("supports %d display node", DISP_MAX);
		return ret;
	}

	snprintf(path, MAX_NAME, BACKLIGHT_BRIGHTNESS_PATH, disp_info[index].bl_name);
	ret = sys_get_int(path, value);
	devmgr_log("path[%s]value[%d]power_saving[%d]", path, *value, power_saving);

	if (power_saving){
		snprintf(path, MAX_NAME, BACKLIGHT_MAX_BRIGHTNESS_PATH, disp_info[index].bl_name);
		ret = sys_get_int(path, &max_brightness);
		if (ret)
		{
			devmgr_log("Can't read max_brightness node[%s]", path);
			return ret;
		}
		pwr_saving_offset = (PWR_SAVING_CANDELA_CRITERION * max_brightness / MAX_CANDELA_CRITERION) + 0.5;

		if (*value > max_brightness - pwr_saving_offset)
			*value = max_brightness;
		else
			*value = *value + pwr_saving_offset;

		devmgr_log("power_saving result[%d]", *value);
	}

	return ret;
}

/* FIXME: s6e8ax0 driver doesn't support dimming */
int OEM_sys_set_backlight_dimming(int index, int value)
{
	int ret = -1;
	char path[MAX_NAME+1];

	devmgr_log("index is %d, value is %d",  index, value);
	if (index >= DISP_MAX) {
		devmgr_log("supports %d display node", DISP_MAX);
		return ret;
	}

	snprintf(path, MAX_NAME, BACKLIGHT_DIMMING_PATH, disp_info[index].bl_name);
	devmgr_log("path[%s]value[%d]", path, value);
	ret = sys_set_int(path, value);

	return ret;
}

int OEM_sys_set_backlight_brightness(int index, int value, int power_saving)
{
	int ret = -1;
	char path[MAX_NAME+1];
	int max_brightness;
	int pwr_saving_offset;

	if (index >= DISP_MAX) {
		devmgr_log("supports %d display node", DISP_MAX);
		return ret;
	}

	devmgr_log("path[%s]value[%d]power_saving[%d]", path, value, power_saving);

	if (power_saving){
		snprintf(path, MAX_NAME, BACKLIGHT_MAX_BRIGHTNESS_PATH, disp_info[index].bl_name);
		ret = sys_get_int(path, &max_brightness);
		if (ret)
		{
			devmgr_log("Can't read max_brightness node[%s]", path);
			return ret;
		}
		pwr_saving_offset = (PWR_SAVING_CANDELA_CRITERION * max_brightness / MAX_CANDELA_CRITERION) + 0.5;

		if (value < pwr_saving_offset)
			value = 0;
		else
			value = value - pwr_saving_offset;

		devmgr_log("power_saving result[%d]", value);
	}

	snprintf(path, MAX_NAME, BACKLIGHT_BRIGHTNESS_PATH, disp_info[index].bl_name);
	ret = sys_set_int(path, value);

	return ret;
}

int OEM_sys_get_backlight_acl_control(int index, int *value)
{
	int ret = -1;
	char path[MAX_NAME+1];

	if (index >= DISP_MAX) {
		devmgr_log("supports %d display node", DISP_MAX);
		return ret;
	}

	snprintf(path, MAX_NAME, LCD_ACL_CONTROL_PATH, disp_info[index].lcd_name);
	ret = sys_get_int(path, value);
	devmgr_log("path[%s]value[%d]", path, *value);

	return ret;
}

int OEM_sys_set_backlight_acl_control(int index, int value)
{
	int ret = -1;
	char path[MAX_NAME+1];

	if (index >= DISP_MAX) {
		devmgr_log("supports %d display node", DISP_MAX);
		return ret;
	}

	snprintf(path, MAX_NAME, LCD_ACL_CONTROL_PATH, disp_info[index].lcd_name);
	devmgr_log("path[%s]value[%d]", path, value);
	ret = sys_set_int(path, value);

	return ret;
}

int OEM_sys_get_lcd_power(int index, int *value)
{
	int ret = -1;
	char path[MAX_NAME+1];

	if (index >= DISP_MAX) {
		devmgr_log("supports %d display node", DISP_MAX);
		return ret;
	}

	snprintf(path, MAX_NAME, LCD_POWER_PATH, disp_info[index].lcd_name);
	ret = sys_get_int(path, value);
	devmgr_log("path[%s]value[%d]", path, *value);

	return ret;
}

int OEM_sys_set_lcd_power(int index, int value)
{
	int ret = -1;
	char path[MAX_NAME+1];

	if (index >= DISP_MAX) {
		devmgr_log("supports %d display node", DISP_MAX);
		return ret;
	}

	snprintf(path, MAX_NAME, LCD_POWER_PATH, disp_info[index].lcd_name);
	devmgr_log("path[%s]value[%d]", path, value);
	ret = sys_set_int(path, value);

	return ret;
}

/* image_enhance */
/* mode - dynamic, standard, natural, movie */
enum image_enhance_mode {
	MODE_DYNAMIC = 0,
	MODE_STANDARD,
	MODE_NATURAL,
	MODE_MOVIE,
};

/* scenario - ui, gallery, video, vtcall, camera, browser, negative, bypass */
enum image_enhance_scenario {
	SCENARIO_UI = 0,
	SCENARIO_GALLERY,
	SCENARIO_VIDEO,
	SCENARIO_VTCALL,
	SCENARIO_CAMERA,
	SCENARIO_BROWSER,
	SCENARIO_NEGATIVE,
	SCENARIO_BYPASS,
};

/* tone - normal, warm, cold */
enum image_enhance_tone {
	TONE_NORMAL = 0,
	TONE_WARM,
	TONE_COLD,
};

/* tone browser - tone1, tone2, tone3 */
enum image_enhance_tone_br {
	TONE_1 = 0,
	TONE_2,
	TONE_3,
};

/* outdoor - off, on */
enum image_enhance_outdoor {
	OUTDOOR_OFF = 0,
	OUTDOOR_ON,
};

/* index - mode, scenario, tone, outdoor, tune */
enum image_enhance_index {
	INDEX_MODE,
	INDEX_SCENARIO,
	INDEX_TONE,
	INDEX_OUTDOOR,
	INDEX_TUNE,
	INDEX_MAX,
};

const char *image_enhance_str[INDEX_MAX] = {
	"mode",
	"scenario",
	"tone",
	"outdoor",
	"tune",
};

struct image_enhance_info {
	enum image_enhance_mode mode;
	enum image_enhance_scenario scenario;
	enum image_enhance_tone tone;
	enum image_enhance_outdoor outdoor;
};

int OEM_sys_get_image_enhance_save(void *image_enhance)
{
	int ret = -1;
	char path[MAX_NAME+1];
	struct image_enhance_info *image_enhance_save = (struct image_enhance_info *)image_enhance;

	snprintf(path, MAX_NAME, IMAGE_ENHANCE_PATH, image_enhance_str[INDEX_MODE]);
	ret = sys_get_int(path, &image_enhance_save->mode);
	snprintf(path, MAX_NAME, IMAGE_ENHANCE_PATH, image_enhance_str[INDEX_SCENARIO]);
	ret = sys_get_int(path, &image_enhance_save->scenario);
	snprintf(path, MAX_NAME, IMAGE_ENHANCE_PATH, image_enhance_str[INDEX_TONE]);
	ret = sys_get_int(path, &image_enhance_save->tone);
	snprintf(path, MAX_NAME, IMAGE_ENHANCE_PATH, image_enhance_str[INDEX_OUTDOOR]);
	ret = sys_get_int(path, &image_enhance_save->outdoor);
	devmgr_log("path[%s]mode[%d]scenario[%d]tone[%d]outdoor[%d]", path, image_enhance_save->mode,
		image_enhance_save->scenario, image_enhance_save->tone, image_enhance_save->outdoor);

	return ret;
}

int OEM_sys_set_image_enhance_restore(void *image_enhance)
{
	int ret = -1;
	char path[MAX_NAME+1];
	struct image_enhance_info *image_enhance_restore = (struct image_enhance_info *)image_enhance;

	devmgr_log("path[%s]mode[%d]scenario[%d]tone[%d]outdoor[%d]", path, image_enhance_restore->mode,
		image_enhance_restore->scenario, image_enhance_restore->tone, image_enhance_restore->outdoor);
	snprintf(path, MAX_NAME, IMAGE_ENHANCE_PATH, image_enhance_str[INDEX_MODE]);
	ret = sys_set_int(path, image_enhance_restore->mode);
	snprintf(path, MAX_NAME, IMAGE_ENHANCE_PATH, image_enhance_str[INDEX_SCENARIO]);
	ret = sys_set_int(path, image_enhance_restore->scenario);
	snprintf(path, MAX_NAME, IMAGE_ENHANCE_PATH, image_enhance_str[INDEX_TONE]);
	ret = sys_set_int(path, image_enhance_restore->tone);
	snprintf(path, MAX_NAME, IMAGE_ENHANCE_PATH, image_enhance_str[INDEX_OUTDOOR]);
	ret = sys_set_int(path, image_enhance_restore->outdoor);

	return ret;
}

int OEM_sys_get_image_enhance_mode(int *value)
{
	int ret = -1;
	char path[MAX_NAME+1];

	snprintf(path, MAX_NAME, IMAGE_ENHANCE_PATH, image_enhance_str[INDEX_MODE]);
	ret = sys_get_int(path, value);
	devmgr_log("path[%s]value[%d]", path, *value);

	return ret;
}

int OEM_sys_set_image_enhance_mode(int value)
{
	int ret = -1;
	char path[MAX_NAME+1];

	snprintf(path, MAX_NAME, IMAGE_ENHANCE_PATH, image_enhance_str[INDEX_MODE]);
	devmgr_log("path[%s]value[%d]", path, value);
	ret = sys_set_int(path, value);

	return ret;
}

int OEM_sys_get_image_enhance_scenario(int *value)
{
	int ret = -1;
	char path[MAX_NAME+1];

	snprintf(path, MAX_NAME, IMAGE_ENHANCE_PATH, image_enhance_str[INDEX_SCENARIO]);
	ret = sys_get_int(path, value);
	devmgr_log("path[%s]value[%d]", path, *value);

	return ret;
}

int OEM_sys_set_image_enhance_scenario(int value)
{
	int ret = -1;
	char path[MAX_NAME+1];

	snprintf(path, MAX_NAME, IMAGE_ENHANCE_PATH, image_enhance_str[INDEX_SCENARIO]);
	devmgr_log("path[%s]value[%d]", path, value);
	ret = sys_set_int(path, value);

	return ret;
}

int OEM_sys_get_image_enhance_tone(int *value)
{
	int ret = -1;
	char path[MAX_NAME+1];

	snprintf(path, MAX_NAME, IMAGE_ENHANCE_PATH, image_enhance_str[INDEX_TONE]);
	ret = sys_get_int(path, value);
	devmgr_log("path[%s]value[%d]", path, *value);

	return ret;
}

int OEM_sys_set_image_enhance_tone(int value)
{
	int ret = -1;
	char path[MAX_NAME+1];

	snprintf(path, MAX_NAME, IMAGE_ENHANCE_PATH, image_enhance_str[INDEX_TONE]);
	devmgr_log("path[%s]value[%d]", path, value);
	ret = sys_set_int(path, value);

	return ret;
}

int OEM_sys_get_image_enhance_outdoor(int *value)
{
	int ret = -1;
	char path[MAX_NAME+1];

	snprintf(path, MAX_NAME, IMAGE_ENHANCE_PATH, image_enhance_str[INDEX_OUTDOOR]);
	ret = sys_get_int(path, value);
	devmgr_log("path[%s]value[%d]", path, *value);

	return ret;
}

int OEM_sys_set_image_enhance_outdoor(int value)
{
	int ret = -1;
	char path[MAX_NAME+1];

	snprintf(path, MAX_NAME, IMAGE_ENHANCE_PATH, image_enhance_str[INDEX_OUTDOOR]);
	devmgr_log("path[%s]value[%d]", path, value);
	ret = sys_set_int(path, value);

	return ret;
}

int OEM_sys_get_image_enhance_tune(int *value)
{
	int ret = -1;
	char path[MAX_NAME+1];

	snprintf(path, MAX_NAME, IMAGE_ENHANCE_PATH, image_enhance_str[INDEX_TUNE]);
	ret = sys_get_int(path, value);
	devmgr_log("path[%s]value[%d]", path, *value);

	return ret;
}

int OEM_sys_set_image_enhance_tune(int value)
{
	int ret = -1;
	char path[MAX_NAME+1];

	snprintf(path, MAX_NAME, IMAGE_ENHANCE_PATH, image_enhance_str[INDEX_TUNE]);
	devmgr_log("path[%s]value[%d]", path, value);
	ret = sys_set_int(path, value);

	return ret;
}

int OEM_sys_image_enhance_info(int *value)
{
	DIR *dir_info;
	struct dirent *dir_entry;
	int ret = -1;
	const char * image_enhance_path_info = IMAGE_ENHANCE_PATH_INFO;

	dir_info = opendir(image_enhance_path_info);

	if (NULL != dir_info) {
		*value = 1;
		ret = 0;
	} else {
		*value = 0;
		ret = -ENOENT;
	}

	closedir(dir_info);
	return ret;
}

int OEM_sys_set_display_frame_rate(int value)
{
	if(value){
		devmgr_log("Display frame rate limited to 40Hz");
		return sys_set_str(DISPLAY_FRAME_RATE_PATH, "40");
	}else{
		devmgr_log("Display frame rate change 40Hz and 60Hz");
		return sys_set_str(DISPLAY_FRAME_RATE_PATH, "60");
	}

	return -1;
}

GENERATE_ACCESSORS_INT_RW(haptic_motor_level, HAPTIC_MOTOR_LEVEL_PATH)
GENERATE_ACCESSORS_INT_R(haptic_motor_level_max, HAPTIC_MOTOR_LEVEL_MAX_PATH)
GENERATE_ACCESSORS_INT_W(haptic_motor_enable, HAPTIC_MOTOR_ENABLE_PATH)
GENERATE_ACCESSORS_INT_W(haptic_motor_oneshot, HAPTIC_MOTOR_ONESHOT_PATH)

GENERATE_ACCESSORS_INT_R(battery_capacity, BATTERY_CAPACITY_PATH)
GENERATE_ACCESSORS_INT_R(battery_charge_full, BATTERY_CHARGE_FULL_PATH)
GENERATE_ACCESSORS_INT_R(battery_charge_now, BATTERY_CHARGE_NOW_PATH)
GENERATE_ACCESSORS_INT_R(battery_present, BATTERY_PRESENT_PATH)

/* FIXME: max170xx_battery driver doesn't support capacity_raw */
int OEM_sys_get_battery_capacity_raw(int *value)
{
	int ret;

	ret = sys_get_int(BATTERY_CAPACITY_RAW_PATH, value);
	if (ret == -1) {
		return -ENODEV;
	}

	return ret;
}

static char *health_text[] = {
	"Unknown", "Good", "Overheat", "Dead", "Over voltage",
	"Unspecified failure", "Cold",
};

int OEM_sys_get_battery_health(int *value)
{
	char buf[BUFF_MAX] = {0};
	int ret = 0;
	int i = 0;

	ret = sys_get_str(BATTERY_HEALTH_PATH, buf);
	if (ret == -1)
		return -1;

	for (i = 0; i < BATTERY_HEALTH_MAX; i++) {
		if (strncmp(buf, health_text[i], strlen(health_text[i])) == 0) {
			*value = i;
			return 0;
		}
	}

	return -1;
}

int OEM_sys_get_battery_polling_required(int *value)
{
	*value = 0;

	return 0;
}

int OEM_sys_get_battery_support_insuspend_charging(int *value)
{
	*value = 1;

	return 0;
}

static char uart_node_path[MAX_NAME];
static char usb_node_path[MAX_NAME];

/* find uart/usb node path */
static int OEM_sys_muic_node_path_info()
{
	int err = -1;

	err = sys_check_node(UART_PATH);
	if (!err)
		sys_get_node(UART_PATH, uart_node_path);
	else {
		err = sys_check_node(UART_PATH_TRATS);
		if (err) {
			devmgr_log("uart path node not found");
			return -1;
		}
		sys_get_node(UART_PATH_TRATS, uart_node_path);
	}

	err = sys_check_node(USB_PATH);
	if (!err)
		sys_get_node(USB_PATH, usb_node_path);
	else {
		err = sys_check_node(USB_PATH_TRATS);
		if (err) {
			devmgr_log("usb path node not found");
			return -1;
		}
		sys_get_node(USB_PATH_TRATS, usb_node_path);
	}
	return 0;
}

int OEM_sys_get_uart_path(int *value)
{
	char buf[BUFF_MAX] = {0};
	int ret = 0;

	ret = sys_get_str(uart_node_path, buf);
	if (ret == -1)
		return -1;

	if (strncmp(buf, "CP", 2) == 0) {
		*value = PATH_CP;
		return 0;
	} else if (strncmp(buf, "AP", 2) == 0) {
		*value = PATH_AP;
		return 0;
	}

	return -1;
}

int OEM_sys_set_uart_path(int value)
{
	switch (value) {
	case PATH_CP:
		return sys_set_str(uart_node_path, "CP");
	case PATH_AP:
		return sys_set_str(uart_node_path, "AP");
	}

	return -1;
}


int OEM_sys_get_usb_path(int *value)
{
	char buf[BUFF_MAX] = {0};
	int ret = 0;

	ret = sys_get_str(usb_node_path, buf);
	if (ret == -1)
		return -1;

	if (strncmp(buf, "PDA", 3) == 0) {
		*value = PATH_AP;
		return 0;
	} else if (strncmp(buf, "MODEM", 5) == 0) {
		*value = PATH_CP;
		return 0;
	}

	return -1;
}

int OEM_sys_set_usb_path(int value)
{
	switch (value) {
	case PATH_CP:
		return sys_set_str(usb_node_path, "MODEM");
	case PATH_AP:
		return sys_set_str(usb_node_path, "PDA");
	}

	return -1;
}

GENERATE_ACCESSORS_INT_R(jack_charger_online, JACK_CHARGER_ONLINE_PATH)
GENERATE_ACCESSORS_INT_R(jack_earjack_online, JACK_EARJACK_ONLINE_PATH)
GENERATE_ACCESSORS_INT_R(jack_earkey_online, JACK_EARKEY_ONLINE_PATH)
GENERATE_ACCESSORS_INT_R(jack_hdmi_online, JACK_HDMI_ONLINE_PATH)
GENERATE_ACCESSORS_INT_R(jack_usb_online, JACK_USB_ONLINE_PATH)
GENERATE_ACCESSORS_INT_R(jack_cradle_online, JACK_CRADLE_ONLINE_PATH)
GENERATE_ACCESSORS_INT_R(jack_tvout_online, JACK_TVOUT_ONLINE_PATH)

int OEM_sys_get_jack_keyboard_online(int *value)
{
	/* Currently, We don't provide SLP Based platform with keyboard I/F */
	int ret = -1;
	/*return sys_get_int(JACK_KEYBOARD_ONLINE_PATH, value);*/
	return ret;
}

int OEM_sys_get_hdmi_support(int *value)
{
	*value = 1;

	return 0;
}

GENERATE_ACCESSORS_INT_R(leds_torch_max_brightness, LEDS_TORCH_MAX_BRIGHTNESS_PATH)
GENERATE_ACCESSORS_INT_RW(leds_torch_brightness, LEDS_TORCH_BRIGHTNESS_PATH)

int OEM_sys_set_power_state(int value)
{
	switch (value) {
	case POWER_STATE_SUSPEND:
		return sys_set_str(POWER_STATE_PATH, "mem");
	case POWER_STATE_PRE_SUSPEND:
		return sys_set_str(POWER_STATE_PATH, "pre_suspend");
	case POWER_STATE_POST_RESUME:
		return sys_set_str(POWER_STATE_PATH, "post_resume");
	}

	return -1;
}

GENERATE_ACCESSORS_INT_RW(power_wakeup_count, POWER_WAKEUP_COUNT_PATH)

GENERATE_ACCESSORS_INT_W(memnotify_threshold_lv1, MEMNOTIFY_THRESHOLD_LV1_PATH)
GENERATE_ACCESSORS_INT_W(memnotify_threshold_lv2, MEMNOTIFY_THRESHOLD_LV2_PATH)

GENERATE_ACCESSORS_INT_R(cpufreq_cpuinfo_max_freq, CPUFREQ_CPUINFO_MAX_FREQ_PATH)
GENERATE_ACCESSORS_INT_R(cpufreq_cpuinfo_min_freq, CPUFREQ_CPUINFO_MIN_FREQ_PATH)
GENERATE_ACCESSORS_INT_RW(cpufreq_scaling_max_freq, CPUFREQ_SCALING_MAX_FREQ_PATH)
GENERATE_ACCESSORS_INT_RW(cpufreq_scaling_min_freq, CPUFREQ_SCALING_MIN_FREQ_PATH)

#define GENERATE_ACCESSORS_INT_R_NO_CONVERT(_suffix, _item)	\
int OEM_sys_get_##_suffix(int *value)			\
{						\
	return sys_get_int_wo_convert(_item, value);	\
}

#define GENERATE_ACCESSORS_INT_W_NO_CONVERT(_suffix, _item)	\
int OEM_sys_set_##_suffix(int value)			\
{						\
	return sys_set_int_wo_convert(_item, value);	\
}

GENERATE_ACCESSORS_INT_R_NO_CONVERT(memnotify_victim_task, MEMNOTIFY_VICTIM_TASK_PATH)
GENERATE_ACCESSORS_INT_W_NO_CONVERT(process_monitor_mp_pnp, PROCESS_MONITOR_MP_PNP_PATH)
GENERATE_ACCESSORS_INT_W_NO_CONVERT(process_monitor_mp_vip, PROCESS_MONITOR_MP_VIP_PATH)

#define GENERATE_ACCESSORS_GET_NODE_PATH(_suffix, _item)	\
int OEM_sys_get_##_suffix(char *node)			\
{						\
	return sys_get_node(_item, node);	\
}

GENERATE_ACCESSORS_GET_NODE_PATH(touch_event, TOUCH_EVENT_NODE)
GENERATE_ACCESSORS_GET_NODE_PATH(memnotify_node, MEMNOTIFY_NODE)
GENERATE_ACCESSORS_GET_NODE_PATH(process_monitor_node, PROCESS_MONITOR_NODE)

static OEM_sys_devman_plugin_interface devman_plugin_interface_exynos;

EXPORT_API const OEM_sys_devman_plugin_interface *OEM_sys_get_devman_plugin_interface()
{
	/* Light interfaces */
	devman_plugin_interface_exynos.OEM_sys_get_display_count = &OEM_sys_get_display_count;
	devman_plugin_interface_exynos.OEM_sys_get_backlight_min_brightness = &OEM_sys_get_backlight_min_brightness;
	devman_plugin_interface_exynos.OEM_sys_get_backlight_max_brightness = &OEM_sys_get_backlight_max_brightness;
	devman_plugin_interface_exynos.OEM_sys_get_backlight_brightness = &OEM_sys_get_backlight_brightness;
	devman_plugin_interface_exynos.OEM_sys_set_backlight_brightness = &OEM_sys_set_backlight_brightness;
	devman_plugin_interface_exynos.OEM_sys_set_backlight_dimming = &OEM_sys_set_backlight_dimming;
	devman_plugin_interface_exynos.OEM_sys_get_backlight_acl_control = &OEM_sys_get_backlight_acl_control;
	devman_plugin_interface_exynos.OEM_sys_set_backlight_acl_control = &OEM_sys_set_backlight_acl_control;

	devman_plugin_interface_exynos.OEM_sys_get_lcd_power = &OEM_sys_get_lcd_power;
	devman_plugin_interface_exynos.OEM_sys_set_lcd_power = &OEM_sys_set_lcd_power;

	/* Image Ehnhace interfaces */
	devman_plugin_interface_exynos.OEM_sys_get_image_enhance_mode = &OEM_sys_get_image_enhance_mode;
	devman_plugin_interface_exynos.OEM_sys_set_image_enhance_mode = &OEM_sys_set_image_enhance_mode;
	devman_plugin_interface_exynos.OEM_sys_get_image_enhance_scenario = &OEM_sys_get_image_enhance_scenario;
	devman_plugin_interface_exynos.OEM_sys_set_image_enhance_scenario = &OEM_sys_set_image_enhance_scenario;
	devman_plugin_interface_exynos.OEM_sys_get_image_enhance_tone = &OEM_sys_get_image_enhance_tone;
	devman_plugin_interface_exynos.OEM_sys_set_image_enhance_tone = &OEM_sys_set_image_enhance_tone;
	devman_plugin_interface_exynos.OEM_sys_get_image_enhance_outdoor = &OEM_sys_get_image_enhance_outdoor;
	devman_plugin_interface_exynos.OEM_sys_set_image_enhance_outdoor = &OEM_sys_set_image_enhance_outdoor;

	devman_plugin_interface_exynos.OEM_sys_get_image_enhance_tune = &OEM_sys_get_image_enhance_tune;
	devman_plugin_interface_exynos.OEM_sys_set_image_enhance_tune = &OEM_sys_set_image_enhance_tune;

	devman_plugin_interface_exynos.OEM_sys_image_enhance_info = &OEM_sys_image_enhance_info;

	devman_plugin_interface_exynos.OEM_sys_set_display_frame_rate = &OEM_sys_set_display_frame_rate;

	/* UART path interfaces */
	devman_plugin_interface_exynos.OEM_sys_get_uart_path = &OEM_sys_get_uart_path;
	devman_plugin_interface_exynos.OEM_sys_set_uart_path = &OEM_sys_set_uart_path;

	/* USB path interfaces */
	devman_plugin_interface_exynos.OEM_sys_get_usb_path = &OEM_sys_get_usb_path;
	devman_plugin_interface_exynos.OEM_sys_set_usb_path = &OEM_sys_set_usb_path;

	/* Vibrator interfaces */
	devman_plugin_interface_exynos.OEM_sys_get_haptic_vibetones_level_max = &OEM_sys_get_haptic_motor_level_max;
	devman_plugin_interface_exynos.OEM_sys_get_haptic_vibetones_level = &OEM_sys_get_haptic_motor_level;
	devman_plugin_interface_exynos.OEM_sys_set_haptic_vibetones_level = &OEM_sys_set_haptic_motor_level;
	devman_plugin_interface_exynos.OEM_sys_set_haptic_vibetones_enable = &OEM_sys_set_haptic_motor_enable;
	devman_plugin_interface_exynos.OEM_sys_set_haptic_vibetones_oneshot = &OEM_sys_set_haptic_motor_oneshot;

	/* Battery interfaces */
	devman_plugin_interface_exynos.OEM_sys_get_battery_capacity = &OEM_sys_get_battery_capacity;
	devman_plugin_interface_exynos.OEM_sys_get_battery_capacity_raw = &OEM_sys_get_battery_capacity_raw;
	devman_plugin_interface_exynos.OEM_sys_get_battery_charge_full = &OEM_sys_get_battery_charge_full;
	devman_plugin_interface_exynos.OEM_sys_get_battery_charge_now = &OEM_sys_get_battery_charge_now;
	devman_plugin_interface_exynos.OEM_sys_get_battery_present = &OEM_sys_get_battery_present;
	devman_plugin_interface_exynos.OEM_sys_get_battery_health = &OEM_sys_get_battery_health;
	devman_plugin_interface_exynos.OEM_sys_get_battery_polling_required= &OEM_sys_get_battery_polling_required;
	devman_plugin_interface_exynos.OEM_sys_get_battery_support_insuspend_charging = &OEM_sys_get_battery_support_insuspend_charging;

	/* Connection interfaces  */
	devman_plugin_interface_exynos.OEM_sys_get_jack_charger_online = &OEM_sys_get_jack_charger_online;
	devman_plugin_interface_exynos.OEM_sys_get_jack_earjack_online = &OEM_sys_get_jack_earjack_online;
	devman_plugin_interface_exynos.OEM_sys_get_jack_earkey_online = &OEM_sys_get_jack_earkey_online;
	devman_plugin_interface_exynos.OEM_sys_get_jack_hdmi_online = &OEM_sys_get_jack_hdmi_online;
	devman_plugin_interface_exynos.OEM_sys_get_jack_usb_online = &OEM_sys_get_jack_usb_online;
	devman_plugin_interface_exynos.OEM_sys_get_jack_cradle_online = &OEM_sys_get_jack_cradle_online;
	devman_plugin_interface_exynos.OEM_sys_get_jack_tvout_online = &OEM_sys_get_jack_tvout_online;
	devman_plugin_interface_exynos.OEM_sys_get_jack_keyboard_online = &OEM_sys_get_jack_keyboard_online;

	devman_plugin_interface_exynos.OEM_sys_get_hdmi_support = &OEM_sys_get_hdmi_support;

	/* Torch interfaces */
	devman_plugin_interface_exynos.OEM_sys_get_leds_torch_max_brightness = &OEM_sys_get_leds_torch_max_brightness;
	devman_plugin_interface_exynos.OEM_sys_get_leds_torch_brightness = &OEM_sys_get_leds_torch_brightness;
	devman_plugin_interface_exynos.OEM_sys_set_leds_torch_brightness = &OEM_sys_set_leds_torch_brightness;

	/* Power management interfaces */
	devman_plugin_interface_exynos.OEM_sys_set_power_state = &OEM_sys_set_power_state;
	devman_plugin_interface_exynos.OEM_sys_get_power_wakeup_count = &OEM_sys_get_power_wakeup_count;
	devman_plugin_interface_exynos.OEM_sys_set_power_wakeup_count = &OEM_sys_set_power_wakeup_count;

	/* OOM interfaces */
	devman_plugin_interface_exynos.OEM_sys_get_memnotify_node = &OEM_sys_get_memnotify_node;
	devman_plugin_interface_exynos.OEM_sys_get_memnotify_victim_task = &OEM_sys_get_memnotify_victim_task;
	devman_plugin_interface_exynos.OEM_sys_set_memnotify_threshold_lv1 = &OEM_sys_set_memnotify_threshold_lv1;
	devman_plugin_interface_exynos.OEM_sys_set_memnotify_threshold_lv2 = &OEM_sys_set_memnotify_threshold_lv2;

	/* Process monitor interfaces */
	devman_plugin_interface_exynos.OEM_sys_get_process_monitor_node = &OEM_sys_get_process_monitor_node;
	devman_plugin_interface_exynos.OEM_sys_set_process_monitor_mp_pnp = &OEM_sys_set_process_monitor_mp_pnp;
	devman_plugin_interface_exynos.OEM_sys_set_process_monitor_mp_vip = &OEM_sys_set_process_monitor_mp_vip;

	/* UART path interfaces */
	devman_plugin_interface_exynos.OEM_sys_get_cpufreq_cpuinfo_max_freq = &OEM_sys_get_cpufreq_cpuinfo_max_freq;
	devman_plugin_interface_exynos.OEM_sys_get_cpufreq_cpuinfo_min_freq = &OEM_sys_get_cpufreq_cpuinfo_min_freq;
	devman_plugin_interface_exynos.OEM_sys_get_cpufreq_scaling_max_freq = &OEM_sys_get_cpufreq_scaling_max_freq;
	devman_plugin_interface_exynos.OEM_sys_set_cpufreq_scaling_max_freq = &OEM_sys_set_cpufreq_scaling_max_freq;
	devman_plugin_interface_exynos.OEM_sys_get_cpufreq_scaling_min_freq = &OEM_sys_get_cpufreq_scaling_min_freq;
	devman_plugin_interface_exynos.OEM_sys_set_cpufreq_scaling_min_freq = &OEM_sys_set_cpufreq_scaling_min_freq;

	devman_plugin_interface_exynos.OEM_sys_get_backlight_brightness_by_lux = &OEM_sys_get_backlight_brightness_by_lux;
	OEM_sys_display_info(disp_info);
	OEM_sys_muic_node_path_info();

	return &devman_plugin_interface_exynos;
}
