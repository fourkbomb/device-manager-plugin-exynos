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


#ifndef __DEVMAN_DEFINE_NODE_PATH_H
#define __DEVMAN_DEFINE_NODE_PATH_H

#define BACKLIGHT_PATH "/sys/class/backlight/"
#define BACKLIGHT_MAX_BRIGHTNESS_PATH	BACKLIGHT_PATH"%s/max_brightness"
#define BACKLIGHT_MIN_BRIGHTNESS_PATH	BACKLIGHT_PATH"%s/min_brightness"
#define BACKLIGHT_BRIGHTNESS_PATH 	BACKLIGHT_PATH"%s/brightness"
#define BACKLIGHT_DIMMING_PATH	BACKLIGHT_PATH"%s/dimming"
#define LCD_PATH  "/sys/class/lcd/"
#define LCD_ACL_CONTROL_PATH	LCD_PATH"%s/acl_control"
#define LCD_POWER_PATH			LCD_PATH"%s/lcd_power"
#define IMAGE_ENHANCE_PATH			"/sys/class/extension/mdnie/%s"
#define IMAGE_ENHANCE_PATH_INFO			"/sys/class/extension/mdnie"

#define DISPLAY_FRAME_RATE_PATH			"/sys/devices/platform/exynos4412-display/devfreq/exynos4412-display/max_freq"

#define UART_PATH			"/sys/devices/virtual/sec/switch/uart_sel"
#define USB_PATH			"/sys/devices/virtual/sec/switch/usb_sel"
#define UART_PATH_TRATS			"/sys/devices/platform/uart-select/path"
#define USB_PATH_TRATS			"/sys/devices/platform/usb-select/path"

#define HAPTIC_MOTOR_LEVEL_PATH	"/sys/class/haptic/motor/level"
#define HAPTIC_MOTOR_LEVEL_MAX_PATH	"/sys/class/haptic/motor/level_max"
#define HAPTIC_MOTOR_ENABLE_PATH	"/sys/class/haptic/motor/enable"
#define HAPTIC_MOTOR_ONESHOT_PATH	"/sys/class/haptic/motor/oneshot"

#define BATTERY_CAPACITY_PATH		"/sys/class/power_supply/battery/capacity"
#define BATTERY_CAPACITY_RAW_PATH	"/sys/class/power_supply/battery/capacity_raw"
#define BATTERY_CHARGE_FULL_PATH	"/sys/class/power_supply/battery/charge_full"
#define BATTERY_CHARGE_NOW_PATH		"/sys/class/power_supply/battery/charge_now"
#define BATTERY_PRESENT_PATH		"/sys/class/power_supply/battery/present"
#define BATTERY_HEALTH_PATH		"/sys/class/power_supply/battery/health"

#define JACK_CHARGER_ONLINE_PATH	"/sys/devices/platform/jack/charger_online"
#define JACK_EARJACK_ONLINE_PATH	"/sys/devices/platform/jack/earjack_online"
#define JACK_EARKEY_ONLINE_PATH		"/sys/devices/platform/jack/earkey_online"
#define JACK_HDMI_ONLINE_PATH		"/sys/devices/platform/jack/hdmi_online"
#define JACK_USB_ONLINE_PATH		"/sys/devices/platform/jack/usb_online"
#define JACK_CRADLE_ONLINE_PATH		"/sys/devices/platform/jack/cradle_online"
#define JACK_TVOUT_ONLINE_PATH		"/sys/devices/platform/jack/tvout_online"
#define JACK_KEYBOARD_ONLINE_PATH		"/sys/devices/platform/jack/keyboard_online"

#define LEDS_TORCH_MAX_BRIGHTNESS_PATH	"/sys/class/camera/flash/max_brightness"
#define LEDS_TORCH_BRIGHTNESS_PATH	"/sys/class/camera/flash/rear_flash"

#define POWER_STATE_PATH		"/sys/power/state"
#define POWER_WAKEUP_COUNT_PATH		"/sys/power/wakeup_count"

#define MEMNOTIFY_NODE			"/dev/memnotify"
#define MEMNOTIFY_VICTIM_TASK_PATH	"/sys/class/memnotify/victim_task"
#define MEMNOTIFY_THRESHOLD_LV1_PATH	"/sys/class/memnotify/threshold_lv1"
#define MEMNOTIFY_THRESHOLD_LV2_PATH	"/sys/class/memnotify/threshold_lv2"

#define TOUCH_EVENT_NODE		"/dev/event2"

#define PROCESS_MONITOR_NODE		"/dev/pmon"
#define PROCESS_MONITOR_MP_PNP_PATH	"/sys/class/pmon/mp_pnp"
#define PROCESS_MONITOR_MP_VIP_PATH	"/sys/class/pmon/mp_vip"

#define CPUFREQ_CPUINFO_MAX_FREQ_PATH	"/sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_max_freq"
#define CPUFREQ_CPUINFO_MIN_FREQ_PATH	"/sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_min_freq"
#define CPUFREQ_SCALING_MAX_FREQ_PATH	"/sys/devices/system/cpu/cpu0/cpufreq/scaling_max_freq"
#define CPUFREQ_SCALING_MIN_FREQ_PATH	"/sys/devices/system/cpu/cpu0/cpufreq/scaling_min_freq"

#endif /* __DEVMAN_DEFINE_NODE_PATH_H */
