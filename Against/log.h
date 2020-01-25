#pragma once

#include "error.h"

int log_init ();
int log_info (const char* category, const char* message);
int log_error (AGAINSTRESULT Result);
int log_exit ();