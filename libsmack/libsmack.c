/*
 * This file is part of libsmack
 *
 * Copyright (C) 2010 Nokia Corporation
 * Copyright (C) 2011 Intel Corporation
 * Copyright (C) 2012 Samsung Electronics Co.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * version 2.1 as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA
 *
 * Authors:
 * Jarkko Sakkinen <jarkko.sakkinen@intel.com>
 * Brian McGillion <brian.mcgillion@intel.com>
 * Rafal Krypa <r.krypa@samsung.com>
 */

#include "sys/smack.h"
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/xattr.h>
#include <unistd.h>

#define ACC_LEN 5
#define LOAD_LEN (2 * (SMACK_LABEL_LEN + 1) + 2 * ACC_LEN + 1)

#define KERNEL_FORMAT "%s %s %s"
#define KERNEL_FORMAT_MODIFY "%s %s %s %s"
#define READ_BUF_SIZE LOAD_LEN + 10
#define SMACKFS_MNT "/smack"
#define SELF_LABEL_FILE "/proc/self/attr/current"

typedef int (*getxattr_func)(void*, const char*, void*, size_t);
typedef int (*setxattr_func)(const void*, const char*, const void*, size_t, int);
typedef int (*removexattr_func)(void*, const char*);

struct smack_rule {
	char subject[SMACK_LABEL_LEN + 1];
	char object[SMACK_LABEL_LEN + 1];
	int is_modify;
	char access_set[ACC_LEN + 1];
	char access_add[ACC_LEN + 1];
	char access_del[ACC_LEN + 1];
	struct smack_rule *next;
};

struct smack_accesses {
	struct smack_rule *first;
	struct smack_rule *last;
};

static int accesses_apply(struct smack_accesses *handle, int clear);
static inline void parse_access_type(const char *in, char out[ACC_LEN + 1]);
static inline char* get_xattr_name(enum smack_label_type type);

int smack_accesses_new(struct smack_accesses **accesses)
{
	struct smack_accesses *result;

	result = calloc(sizeof(struct smack_accesses), 1);
	if (result == NULL)
		return -1;

	*accesses = result;
	return 0;
}

void smack_accesses_free(struct smack_accesses *handle)
{
	if (handle == NULL)
		return;

	struct smack_rule *rule = handle->first;
	struct smack_rule *next_rule = NULL;

	while (rule != NULL) {
		next_rule = rule->next;
		free(rule);
		rule = next_rule;
	}

	free(handle);
}

int smack_accesses_save(struct smack_accesses *handle, int fd)
{
	struct smack_rule *rule = handle->first;
	FILE *file;
	int ret;
	int newfd;

	newfd = dup(fd);
	if (newfd == -1)
		return -1;

	file = fdopen(newfd, "w");
	if (file == NULL) {
		close(newfd);
		return -1;
	}

	while (rule) {
		if (rule->is_modify) {
			ret = fprintf(file, "%s %s %s %s\n",
				      rule->subject, rule->object,
				      rule->access_add, rule->access_del);
		} else {
			ret = fprintf(file, "%s %s %s\n",
				      rule->subject, rule->object,
				      rule->access_set);
		}

		if (ret < 0) {
			fclose(file);
			return -1;
		}

		rule = rule->next;
	}

	fclose(file);
	return 0;
}

int smack_accesses_apply(struct smack_accesses *handle)
{
	return accesses_apply(handle, 0);
}

int smack_accesses_clear(struct smack_accesses *handle)
{
	return accesses_apply(handle, 1);
}

int smack_accesses_add(struct smack_accesses *handle, const char *subject,
		       const char *object, const char *access_type)
{
	struct smack_rule *rule = NULL;

	rule = calloc(sizeof(struct smack_rule), 1);
	if (rule == NULL)
		return -1;

	strncpy(rule->subject, subject, SMACK_LABEL_LEN + 1);
	strncpy(rule->object, object, SMACK_LABEL_LEN + 1);
	parse_access_type(access_type, rule->access_set);

	if (handle->first == NULL) {
		handle->first = handle->last = rule;
	} else {
		handle->last->next = rule;
		handle->last = rule;
	}

	return 0;
}

int smack_accesses_add_modify(struct smack_accesses *handle, const char *subject,
		       const char *object, const char *access_add, const char *access_del)
{
	struct smack_rule *rule = NULL;

	rule = calloc(sizeof(struct smack_rule), 1);
	if (rule == NULL)
		return -1;

	strncpy(rule->subject, subject, SMACK_LABEL_LEN + 1);
	strncpy(rule->object, object, SMACK_LABEL_LEN + 1);
	parse_access_type(access_add, rule->access_add);
	parse_access_type(access_del, rule->access_del);
	rule->is_modify = 1;

	if (handle->first == NULL) {
		handle->first = handle->last = rule;
	} else {
		handle->last->next = rule;
		handle->last = rule;
	}

	return 0;
}

int smack_accesses_add_from_file(struct smack_accesses *accesses, int fd)
{
	FILE *file = NULL;
	char buf[READ_BUF_SIZE];
	char *ptr;
	const char *subject, *object, *access, *access2;
	int newfd;
	int ret;

	newfd = dup(fd);
	if (newfd == -1)
		return -1;

	file = fdopen(newfd, "r");
	if (file == NULL) {
		close(newfd);
		return -1;
	}

	while (fgets(buf, READ_BUF_SIZE, file) != NULL) {
		if (strcmp(buf, "\n") == 0)
			continue;
		subject = strtok_r(buf, " \t\n", &ptr);
		object = strtok_r(NULL, " \t\n", &ptr);
		access = strtok_r(NULL, " \t\n", &ptr);
		access2 = strtok_r(NULL, " \t\n", &ptr);

		if (subject == NULL || object == NULL || access == NULL ||
		    strtok_r(NULL, " \t\n", &ptr) != NULL) {
			errno = EINVAL;
			fclose(file);
			return -1;
		}

		if (access2 == NULL)
			ret = smack_accesses_add(accesses, subject, object, access);
		else
			ret = smack_accesses_add_modify(accesses, subject, object, access, access2);

		if (ret) {
			fclose(file);
			return -1;
		}
	}

	if (ferror(file)) {
		fclose(file);
		return -1;
	}

	fclose(file);
	return 0;
}

int smack_have_access(const char *subject, const char *object,
		      const char *access_type)
{
	char buf[LOAD_LEN + 1];
	char access_type_k[ACC_LEN + 1];
	int ret;
	int fd;

	parse_access_type(access_type, access_type_k);

	ret = snprintf(buf, LOAD_LEN + 1, KERNEL_FORMAT, subject, object,
		       access_type_k);
	if (ret < 0)
		return -1;

	fd = open(SMACKFS_MNT "/access2", O_RDWR);
	if (fd < 0)
		return -1;

	ret = write(fd, buf, LOAD_LEN);
	if (ret < 0) {
		close(fd);
		return -1;
	}

	ret = read(fd, buf, 1);
	close(fd);
	if (ret < 0)
		return -1;

	return buf[0] == '1';
}

int smack_new_label_from_self(char **label)
{
	char *result;
	int fd;
	int ret;

	result = calloc(SMACK_LABEL_LEN + 1, 1);
	if (result == NULL)
		return -1;

	fd = open(SELF_LABEL_FILE, O_RDONLY);
	if (fd < 0) {
		free(result);
		return -1;
	}

	ret = read(fd, result, SMACK_LABEL_LEN);
	close(fd);
	if (ret < 0) {
		free(result);
		return -1;
	}

	*label = result;
	return 0;
}

int smack_new_label_from_socket(int fd, char **label)
{
	char dummy;
	int ret;
	socklen_t length = 1;
	char *result;

	ret = getsockopt(fd, SOL_SOCKET, SO_PEERSEC, &dummy, &length);
	if (ret < 0 && errno != ERANGE)
		return -1;

	result = calloc(length, 1);
	if (result == NULL)
		return -1;

	ret = getsockopt(fd, SOL_SOCKET, SO_PEERSEC, result, &length);
	if (ret < 0) {
		free(result);
		return -1;
	}

	*label = result;
	return 0;
}

int smack_set_label_for_self(const char *label)
{
	int len;
	int fd;
	int ret;

	len = strnlen(label, SMACK_LABEL_LEN + 1);
	if (len > SMACK_LABEL_LEN)
		return -1;

	fd = open(SELF_LABEL_FILE, O_WRONLY);
	if (fd < 0)
		return -1;

	ret = write(fd, label, len);
	close(fd);

	return (ret < 0) ? -1 : 0;
}

int smack_revoke_subject(const char *subject)
{
	int ret;
	int fd;

	fd = open(SMACKFS_MNT "/revoke-subject", O_RDWR);
	if (fd < 0)
		return -1;

	ret = write(fd, subject, strnlen(subject, SMACK_LABEL_LEN));
	close(fd);

	return (ret < 0) ? -1 : 0;
}

static int internal_getlabel(void* file, char** label,
		enum smack_label_type type,
		getxattr_func getfunc)
{
	char* xattr_name = get_xattr_name(type);
	char value[SMACK_LABEL_LEN + 1];
	int ret;

	ret = getfunc(file, xattr_name, value, SMACK_LABEL_LEN + 1);
	if (ret == -1) {
		if (errno == ENODATA) {
			*label = NULL;
			return 0;
		}
		return -1;
	}

	value[ret] = '\0';
	*label = calloc(ret + 1, 1);
	if (*label == NULL)
		return -1;
	strncpy(*label, value, ret);
	return 0;
}

static int internal_setlabel(void* file, const char* label,
		enum smack_label_type type,
		setxattr_func setfunc, removexattr_func removefunc)
{
	char* xattr_name = get_xattr_name(type);

	/* Check validity of labels for LABEL_TRANSMUTE */
	if (type == SMACK_LABEL_TRANSMUTE && label != NULL) {
		if (!strcmp(label, "0"))
			label = NULL;
		else if (!strcmp(label, "1"))
			label = "TRUE";
		else
			return -1;
	}

	if (label == NULL || label[0] == '\0') {
		return removefunc(file, label);
	} else {
		int len = strnlen(label, SMACK_LABEL_LEN + 1);
		if (len > SMACK_LABEL_LEN)
			return -1;
		return setfunc(file, xattr_name, label, len, 0);
	}
}

int smack_getlabel(const char *path, char** label,
		enum smack_label_type type)
{
	return internal_getlabel((void*) path, label, type,
			(getxattr_func) getxattr);
}

int smack_lgetlabel(const char *path, char** label,
		enum smack_label_type type)
{
	return internal_getlabel((void*) path, label, type,
			(getxattr_func) lgetxattr);
}

int smack_fgetlabel(int fd, char** label,
		enum smack_label_type type)
{
	return internal_getlabel((void*) fd, label, type,
			(getxattr_func) fgetxattr);
}

int smack_setlabel(const char *path, const char* label,
		enum smack_label_type type)
{
	return internal_setlabel((void*) path, label, type,
			(setxattr_func) setxattr, (removexattr_func) removexattr);
}

int smack_lsetlabel(const char *path, const char* label,
		enum smack_label_type type)
{
	return internal_setlabel((void*) path, label, type,
			(setxattr_func) lsetxattr, (removexattr_func) lremovexattr);
}

int smack_fsetlabel(int fd, const char* label,
		enum smack_label_type type)
{
	return internal_setlabel((void*) fd, label, type,
			(setxattr_func) fsetxattr, (removexattr_func) fremovexattr);
}

static int accesses_apply(struct smack_accesses *handle, int clear)
{
	char buf[LOAD_LEN + 1];
	struct smack_rule *rule;
	int ret;
	int fd;
	int load_fd;
	int change_fd;
	int size;

	load_fd = open(SMACKFS_MNT "/load2", O_WRONLY);
	if (load_fd < 0)
		return -1;

	change_fd = open(SMACKFS_MNT "/change-rule", O_WRONLY);
	if (change_fd < 0) {
		close(load_fd);
		return -1;
	}

	for (rule = handle->first; rule != NULL; rule = rule->next) {
		fd = load_fd;
		if (clear) {
			size = snprintf(buf, LOAD_LEN + 1, KERNEL_FORMAT, rule->subject, rule->object, "-----");
		} else {

			if (rule->is_modify) {
				fd = change_fd;
				size = snprintf(buf, LOAD_LEN + 1, KERNEL_FORMAT_MODIFY,
						rule->subject, rule->object, rule->access_add, rule->access_del);

			} else {
				size = snprintf(buf, LOAD_LEN + 1, KERNEL_FORMAT,
						rule->subject, rule->object, rule->access_set);
			}
		}

		if (size == -1 || size > LOAD_LEN) {
			close(load_fd);
			close(change_fd);
			return -1;
		}

		ret = write(fd, buf, size);
		if (ret < 0) {
			close(load_fd);
			close(change_fd);
			return -1;
		}
	}

	close(load_fd);
	close(change_fd);
	return 0;
}

static inline void parse_access_type(const char *in, char out[ACC_LEN + 1])
{
	int i;

	for (i = 0; i < ACC_LEN; ++i)
		out[i] = '-';
	out[ACC_LEN] = '\0';

	for (i = 0; i < ACC_LEN && in[i] != '\0'; i++)
		switch (in[i]) {
		case 'r':
		case 'R':
			out[0] = 'r';
			break;
		case 'w':
		case 'W':
			out[1] = 'w';
			break;
		case 'x':
		case 'X':
			out[2] = 'x';
			break;
		case 'a':
		case 'A':
			out[3] = 'a';
			break;
		case 't':
		case 'T':
			out[4] = 't';
			break;
		default:
			break;
		}
}

static inline char* get_xattr_name(enum smack_label_type type)
{
	switch (type) {
	case SMACK_LABEL_ACCESS:
		return "security.SMACK64";
	case SMACK_LABEL_EXEC:
		return "security.SMACK64EXEC";
	case SMACK_LABEL_MMAP:
		return "security.SMACK64MMAP";
	case SMACK_LABEL_TRANSMUTE:
		return "security.SMACK64TRANSMUTE";
	case SMACK_LABEL_IPIN:
		return "security.SMACK64IPIN";
	case SMACK_LABEL_IPOUT:
		return "security.SMACK64IPOUT";
	default:
		/* Should not reach this point */
		return NULL;
	}

}
