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


#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <devman_plugin_intf.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>

#define EXPORT_API  __attribute__((visibility("default")))

#define BUFF_MAX	255
#define SUCCESS		0
#define	FAIL		1

int sys_check_node(char *path)
{
	int fd = -1;

	fd = open(path, O_RDONLY);

	if (fd == -1)
		return -1;

	close(fd);
	return 0;
}

int sys_get_node(char *path, char *node)
{
	if (0 >= snprintf(node, strlen(path) + 1, "%s", path))
		return -1;

	return 0;
}

int sys_get_int_wo_convert(char *path, int *value)
{
	int fd = -1;

	fd = open(path, O_RDONLY);
	if (fd == -1) {
		return -1;
	}

	if (0 > read(fd, value, sizeof(int))) {
		close(fd);
		return -1;
	}
	close (fd);

	return 0;
}

int sys_set_int_wo_convert(char *path, int value)
{
	int fd = -1;

	fd = open(path, O_WRONLY);
	if (fd == -1) {
		return -1;
	}

	if (0 > write(fd, &value, sizeof(int))) {
		close(fd);
		return -1;
	}
	close(fd);

	return 0;
}

static int sys_read_buf(char *file, char *buf)
{
	int fd;
	int r;

	fd = open(file, O_RDONLY);
	if (fd == -1) {
		return -ENOENT;
	}

	r = read(fd, buf, BUFF_MAX);
	if ((r >= 0) && (r < BUFF_MAX))
		buf[r] = '\0';
	else {
		return -EIO;
	}

	close(fd);

	return 0;
}

static int sys_write_buf(char *file, char *buf)
{
	int fd;
	int r;

	fd = open(file, O_WRONLY);
	if (fd == -1) {
		return -1;
	}

	r = write(fd, buf, strlen(buf));
	close(fd);
	if (r < 0) {
		return -1;
	}

	return 0;
}

int sys_get_int(char *fname, int *val)
{
	char buf[BUFF_MAX];

	if (sys_read_buf(fname, buf) == 0) {
		*val = atoi(buf);
		return 0;
	} else {
		*val = -1;
		return -1;
	}
}

int sys_get_str(char *fname, char *str)
{
	char buf[BUFF_MAX] = {0};

	if (sys_read_buf(fname, buf) == 0)
	{
		strncpy(str, buf, strlen(buf));
		return 0;
	}

	return -1;
}

int sys_set_int(char *fname, int val)
{
	char buf[BUFF_MAX];
	int r = -1;
	snprintf(buf, sizeof(buf), "%d", val);

	if (sys_write_buf(fname, buf) == 0)
		r = 0;

	return r;
}

int sys_set_str(char *fname, char *val)
{
	int r = -1;

	if (val != NULL) {
		if (sys_write_buf(fname, val) == 0)
			r = 0;
	}

	return r;
}
