#pragma once

#include "resource.h"
#include "commctrl.h"
#include "Commdlg.h"
#include "stdio.h"

#define MAX_LOADSTRING	100

//标准LZSS用
#define N_S				4096
#define F_S				18
#define THRESHOLD_S		2
#define NIL_S			N_S

//LZSS Prinny变种用
#define N_P				(2*256)
#define THRESHOLD_P		3
#define F_P				256
#define FLAG			0xD5
#define NIL_P			N_P

// Forward declarations of functions included in this code module:
BOOL CALLBACK	DlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK	DlgSetupProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
bool			DeLZSS();
bool			DeLZSS_Prinny();
void			InitTree();
void			InsertNode(int r);
void			DeleteNode(int p);
void			EnLZSS();
void			EnLZSS_Prinny();
void			InitTree_P();
void			InsertNode_P(int r, int j);
void			DeleteNode_P(int p);