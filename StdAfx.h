#pragma once
#define WIN32_LEAN_AND_MEAN
#define WINVER       0x0601 // _WIN32_WINNT_WIN7
#define _WIN32_WINNT 0x0601
#include <Windows.h>
#include <cstdlib>
#include <intrin.h>
#pragma intrinsic(memset, memcpy, strcat, strcmp, strcpy, strlen)

// IDA libs
#define USE_DANGEROUS_FUNCTIONS
#define NO_OBSOLETE_FUNCS
#include <ida.hpp>
#include <auto.hpp>
#include <loader.hpp>
#include <funcs.hpp>
#pragma warning(push)
#pragma warning(disable:4244) // "conversion from 'size_t' to 'xxx', possible loss of data"
#pragma warning(disable:4267) // "conversion from 'size_t' to 'xxx', possible loss of data"
#include <typeinf.hpp>
#pragma warning(pop)

#include "Utility.h"

// Tick IDA's Qt message pump so it will show msg() output
//#define refreshUI() WaitBox::processIdaEvents()

#define MY_VERSION MAKEWORD(7, 1) // Low, high, 0 to 99
