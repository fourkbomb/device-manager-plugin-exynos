/*
 * device-node
 *
 * Copyright (c) 2015 Samsung Electronics Co., Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the License);
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <linux/limits.h>

#include <hw/backlight.h>
#include "shared.h"

#ifndef BACKLIGHT_PATH
#define BACKLIGHT_PATH	"/sys/class/backlight/s6e8aa0-bl"
#endif

static int get_max_brightness(int *val)
{
	static int max = -1;
	int r;

	if (!val)
		return -EINVAL;

	if (max < 0) {
		r = sys_get_int(BACKLIGHT_PATH"/max_brightness", &max);
		if (r < 0)
			return r;
	}

	*val = max;
	return 0;
}

static int backlight_get_brightness(int *brightness)
{
	int r, v;

	if (!brightness) {
		_E("wrong parameter");
		return -EINVAL;
	}

	r = sys_get_int(BACKLIGHT_PATH"/brightness", &v);
	if (r < 0) {
		_E("fail to get brightness : %s", strerror(r));
		return r;
	}

	*brightness = v;
	return 0;
}

static int backlight_set_brightness(int brightness)
{
	int r, v, max;

	if (brightness < 0 || brightness > 100) {
		_E("wrong parameter");
		return -EINVAL;
	}

	r = get_max_brightness(&max);
	if (r < 0) {
		_E("fail to get max brightness : %s", strerror(r));
		return r;
	}

	v = brightness/100.f*max;
	r = sys_set_int(BACKLIGHT_PATH"/brightness", v);
	if (r < 0) {
		_E("fail to set brightness : %s", strerror(r));
		return r;
	}

	return 0;
}

static int backlight_get_mode(enum backlight_mode *mode)
{
	if (!mode) {
		_E("wrong parameter");
		return -EINVAL;
	}

	*mode = BACKLIGHT_MANUAL;
	return 0;
}

static int backlight_set_mode(enum backlight_mode mode)
{
	if (mode == BACKLIGHT_SENSOR) {
		_E("not supported option");
		return -ENOTSUP;
	}

	return 0;
}

static int backlight_open(struct hw_info *info,
		const char *id, struct hw_common **common)
{
	struct backlight_device *backlight_dev;

	if (!info || !common)
		return -EINVAL;

	backlight_dev = calloc(1, sizeof(struct backlight_device));
	if (!backlight_dev)
		return -ENOMEM;

	backlight_dev->common.info = info;
	backlight_dev->get_brightness = backlight_get_brightness;
	backlight_dev->set_brightness = backlight_set_brightness;
	backlight_dev->get_mode = backlight_get_mode;
	backlight_dev->set_mode = backlight_set_mode;

	*common = (struct hw_common *)backlight_dev;
	return 0;
}

static int backlight_close(struct hw_common *common)
{
	if (!common)
		return -EINVAL;

	free(common);
	return 0;
}

EXPORT
struct hw_info HARDWARE_INFO_SYM = {
	.magic = HARDWARE_INFO_TAG,
	.hal_version = HARDWARE_INFO_VERSION,
	.device_version = BACKLIGHT_HARDWARE_DEVICE_VERSION,
	.id = BACKLIGHT_HARDWARE_DEVICE_ID,
	.name = "Default Backlight",
	.author = "Jiyoung Yun <jy910.yun@samsung.com>",
	.open = backlight_open,
	.close = backlight_close,
};
