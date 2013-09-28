/*
 * cloudfs: bucket header
 *	By Benjamin Kittridge. Copyright (C) 2013, All rights reserved.
 *
 */

#pragma once

////////////////////////////////////////////////////////////////////////////////
// Section:     Required includes

#include "store.h"

////////////////////////////////////////////////////////////////////////////////
// Section:     Macros

#define BUCKET_MAX	256

////////////////////////////////////////////////////////////////////////////////
// Section:     Bucket operation table definition

struct bucket_oper {
	const char *name;
	void (*func)();
};

////////////////////////////////////////////////////////////////////////////////
// Section:     Bucket initialization

void bucket_load();
void bucket_unload();

////////////////////////////////////////////////////////////////////////////////
// Section:     Bucket operation functions

void bucket_create();
void bucket_list();
void bucket_delete();

////////////////////////////////////////////////////////////////////////////////
// Section:     Bucket selection

const char *bucket_get_selected();
