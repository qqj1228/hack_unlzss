// hack_unlzss.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "hack_unlzss.h"

// Global Variables:
HINSTANCE g_hInst;
FILE *fpIn,*fpOut;
HWND hWndPB;
DWORD g_sign;
bool blG_sign=false,blDeSize=true,blEnSize=false;
int match_position, match_length;
unsigned long int textsize = 0, codesize = 0, printcount = 0;
char szInfo[MAX_PATH];
//标准LZSS用
unsigned char text_buf_S[N_S + F_S - 1];
int lson_S[N_S + 1], rson_S[N_S + 257], dad_S[N_S + 1];
//LZSS Prinny变种用
unsigned char text_buf_P[N_P+F_P-1];
int son_P[N_P + 257], dad_P[N_P + 1];

int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	g_hInst=hInstance;
	DialogBox(g_hInst,(LPCSTR)IDD_DIALOG_MAIN,NULL,(DLGPROC)DlgProc);
	ExitProcess(NULL);
	return 0;
}

BOOL CALLBACK DlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	OPENFILENAME ofn;
	static char szFile[MAX_PATH],szFileName[MAX_PATH];
	RECT rcRect;
	int i;

	switch (message)
	{
	case WM_INITDIALOG:
		//创建进度条
		InitCommonControls();
		GetClientRect(hDlg, &rcRect);
		hWndPB=CreateWindow(PROGRESS_CLASS, "PB", WS_CHILD | WS_VISIBLE, 5, rcRect.bottom-20-5, rcRect.right-10, 20, hDlg, NULL, g_hInst, NULL);

		CheckRadioButton(hDlg, IDC_RADIO1, IDC_RADIO2, IDC_RADIO1);
		return TRUE;
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDC_BUTTON1:
			ZeroMemory(szFile, sizeof(szFile[MAX_PATH]));
			ZeroMemory(&ofn, sizeof(OPENFILENAME));
			ofn.lStructSize = sizeof(OPENFILENAME);
			ofn.hwndOwner = hDlg;
			ofn.lpstrFile = szFile;
			ofn.nMaxFile = sizeof(szFile);
			ofn.lpstrFilter = "tex File(*.tex)\0*.tex\0All Files(*.*)\0*.*\0";
			ofn.nFilterIndex = 2;
			ofn.lpstrFileTitle = szFileName;
			ofn.nMaxFileTitle = sizeof(szFileName);
			ofn.lpstrInitialDir = NULL;
			//ofn.lpstrDefExt = "vlg";
			ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
			
			if (GetOpenFileName(&ofn)!=TRUE) 
				break;
			SetDlgItemText(hDlg, IDC_EDIT1, szFile);
			break;
		case IDC_BUTTON2:
			DialogBox(g_hInst,(LPCSTR)IDD_DIALOG_SETUP,hDlg,(DLGPROC)DlgSetupProc);
			break;
		case IDOK:
			GetDlgItemText(hDlg, IDC_EDIT1, szFile, MAX_PATH);
			if (fopen_s(&fpIn,szFile,"rb")!=NULL)
			{
				MessageBox(hDlg,"打开文件失败！","错误",MB_OK | MB_ICONERROR | MB_TOPMOST);
				break;
			}
			if(BST_CHECKED == IsDlgButtonChecked(hDlg, IDC_RADIO1))//解压
			{
				if(BST_CHECKED == IsDlgButtonChecked(hDlg, IDC_CHECKBOX1))//Prinny变种
				{
					strcat_s(szFile,".bin");
					if(fopen_s(&fpOut,szFile,"wb")!=NULL)
					{
						MessageBox(hDlg,"创建文件失败！","错误",MB_OK | MB_ICONERROR | MB_TOPMOST);
						break;
					}
					if(DeLZSS_Prinny())
						MessageBox(hDlg,"解压LZSS Prinny变种成功！","成功",MB_OK | MB_ICONINFORMATION | MB_TOPMOST);
					else
						MessageBox(hDlg,"文件类型不符或解压文件大小不符！","错误",MB_OK | MB_ICONERROR | MB_TOPMOST);
					fclose(fpIn);
					fclose(fpOut);
				}
				else//标准
				{
					strcat_s(szFile,".txt");
					if(fopen_s(&fpOut,szFile,"wb")!=NULL)
					{
						MessageBox(hDlg,"创建文件失败！","错误",MB_OK | MB_ICONERROR | MB_TOPMOST);
						break;
					}
					if(DeLZSS())
						MessageBox(hDlg,"解压LZSS成功！","成功",MB_OK | MB_ICONINFORMATION | MB_TOPMOST);
					else
						MessageBox(hDlg,"文件类型不符或解压文件大小不符！","错误",MB_OK | MB_ICONERROR | MB_TOPMOST);
					fclose(fpIn);
					fclose(fpOut);
				}
			}
			else//压缩
			{
				if(BST_CHECKED == IsDlgButtonChecked(hDlg, IDC_CHECKBOX1))//Prinny变种
				{
					strcat_s(szFile,".lzs");
					if(fopen_s(&fpOut,szFile,"wb")!=NULL)
					{
						MessageBox(hDlg,"创建文件失败！","错误",MB_OK | MB_ICONERROR | MB_TOPMOST);
						break;
					}
					//预写文件头
					for(i=0;i<16;i++)
						fputc(0x00, fpOut);

					EnLZSS_Prinny();

					//填充文件头
					rewind(fpOut);
					fwrite("dat", 4, 1, fpOut);
					fwrite(&textsize, 4, 1, fpOut);
					codesize-=4;
					fwrite(&codesize, 4, 1, fpOut);
					putc(FLAG, fpOut);

					strcat_s(szInfo,"压缩LZSS Prinny变种成功！");
					MessageBox(hDlg, szInfo, "成功", MB_OK | MB_ICONINFORMATION | MB_TOPMOST);
					fclose(fpIn);
					fclose(fpOut);
				}
				else//标准
				{
					strcat_s(szFile,".lzs");
					if(fopen_s(&fpOut,szFile,"wb")!=NULL)
					{
						MessageBox(hDlg,"创建文件失败！","错误",MB_OK | MB_ICONERROR | MB_TOPMOST);
						break;
					}
					//预写文件头
					if(blG_sign)
						fwrite(&g_sign, 4, 1, fpOut);
					if(blDeSize)
						fwrite("\0\0\0", 1, 4, fpOut);
					if(blEnSize)
						fwrite("\0\0\0", 1, 4, fpOut);

					EnLZSS();

					//填充文件头
					fseek(fpOut, 4L, 0);
					if(blDeSize)
						fwrite(&textsize, 4, 1, fpOut);
					if(blEnSize)
						fwrite(&codesize, 4, 1, fpOut);

					strcat_s(szInfo,"压缩LZSS成功！");
					MessageBox(hDlg, szInfo, "成功", MB_OK | MB_ICONINFORMATION | MB_TOPMOST);
					fclose(fpIn);
					fclose(fpOut);
				}
			}
			break;
		/*case IDCANCEL:
			EndDialog(hDlg, LOWORD(wParam));
			break;*/
		}
		return (INT_PTR)TRUE;
	case WM_CLOSE:
		EndDialog(hDlg, 0);
		return TRUE;
	}
	return FALSE;
}

BOOL CALLBACK DlgSetupProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_INITDIALOG:
		if(blG_sign)
			CheckDlgButton(hDlg, IDC_CHECKBOX1, BST_CHECKED);
		if(blDeSize)
			CheckDlgButton(hDlg, IDC_CHECKBOX2, BST_CHECKED);
		if(blEnSize)
			CheckDlgButton(hDlg, IDC_CHECKBOX3, BST_CHECKED);
		return TRUE;
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDOK:
			if(BST_CHECKED == IsDlgButtonChecked(hDlg, IDC_CHECKBOX1))
			{
				blG_sign=true;
				GetDlgItemText(hDlg, IDC_EDIT1, (LPSTR)&g_sign, 4);
			}
			else
				blG_sign=false;

			if(BST_CHECKED == IsDlgButtonChecked(hDlg, IDC_CHECKBOX2))
				blDeSize=true;
			else
				blDeSize=false;

			if(BST_CHECKED == IsDlgButtonChecked(hDlg, IDC_CHECKBOX3))
				blEnSize=true;
			else
				blEnSize=false;
		case IDCANCEL:
			EndDialog(hDlg, LOWORD(wParam));
			break;
		}
		return TRUE;
	case WM_CLOSE:
		EndDialog(hDlg, LOWORD(wParam));
		return TRUE;
	}
	return FALSE;
}

void InitTree()
{
	int  i;

	for (i = N_S + 1; i <= N_S + 256; i++)
		rson_S[i] = NIL_S;
	for (i = 0; i < N_S; i++)
		dad_S[i] = NIL_S;
}

void InsertNode(int r)
{
	int  i, p, cmp;
	unsigned char  *key;

	cmp = 1;
	key = &text_buf_S[r];
	p = N_S + 1 + key[0];
	rson_S[r] = lson_S[r] = NIL_S;
	match_length = 0;
	for ( ; ; )
	{
		if (cmp >= 0)
		{
			if (rson_S[p] != NIL_S)
				p = rson_S[p];
			else
			{  
				rson_S[p] = r;
				dad_S[r] = p;
				return;
			}
		}
		else
		{
			if (lson_S[p] != NIL_S)
				p = lson_S[p];
			else
			{  
				lson_S[p] = r;
				dad_S[r] = p;
				return;
			}
		}
		for (i = 1; i < F_S; i++)
		{
			if ((cmp = key[i] - text_buf_S[p + i]) != 0)
				break;
		}
		if (i > match_length)
		{
			match_position = p;
			if ((match_length = i) >= F_S)
				break;
		}
	}
	dad_S[r] = dad_S[p];
	lson_S[r] = lson_S[p];
	rson_S[r] = rson_S[p];
	dad_S[lson_S[p]] = r;
	dad_S[rson_S[p]] = r;
	if (rson_S[dad_S[p]] == p)
		rson_S[dad_S[p]] = r;
	else
		lson_S[dad_S[p]] = r;
	dad_S[p] = NIL_S;
}

void DeleteNode(int p)
{
	int  q;
	
	if (dad_S[p] == NIL_S)
		return;
	if (rson_S[p] == NIL_S)
		q = lson_S[p];
	else if (lson_S[p] == NIL_S)
			q = rson_S[p];
	else
	{
		q = lson_S[p];
		if (rson_S[q] != NIL_S)
		{
			do	{
				q = rson_S[q];
			} while (rson_S[q] != NIL_S);
			rson_S[dad_S[q]] = lson_S[q];
			dad_S[lson_S[q]] = dad_S[q];
			lson_S[q] = lson_S[p];
			dad_S[lson_S[p]] = q;
		}
		rson_S[q] = rson_S[p];
		dad_S[rson_S[p]] = q;
	}
	dad_S[q] = dad_S[p];
	if (rson_S[dad_S[p]] == p)
		rson_S[dad_S[p]] = q;
	else
		lson_S[dad_S[p]] = q;
	dad_S[p] = NIL_S;
}

void EnLZSS()
{
	int  i, c, len, r, s, last_match_length, code_buf_ptr;
	unsigned char  code_buf[17], mask;
	
	InitTree();
	code_buf[0] = 0;
	code_buf_ptr = mask = 1;
	s = 0;
	r = N_S - F_S;

	//初始化进度条
	fseek(fpIn, 0L, 2);
	SendMessage(hWndPB, PBM_SETRANGE32, 0, ftell(fpIn)/1024);
    SendMessage(hWndPB, PBM_SETSTEP, (WPARAM) 1, 0);
	SendMessage(hWndPB, PBM_SETPOS, (WPARAM) 0, 0);
	rewind(fpIn);
	printcount = 1024;

	for (i = s; i < r; i++)
		text_buf_S[i] = 0x00;//' ';
	for (len = 0; len < F_S && (c = getc(fpIn)) != EOF; len++)
		text_buf_S[r + len] = c;
	if ((textsize = len) == 0)
		return;
	for (i = 1; i <= F_S; i++)
		InsertNode(r - i);
	InsertNode(r);
	do {
		if (match_length > len)
			match_length = len;
		if (match_length <= THRESHOLD_S)
		{
			match_length = 1;
			code_buf[0] |= mask;
			code_buf[code_buf_ptr++] = text_buf_S[r];
		}
		else
		{
			code_buf[code_buf_ptr++] = (unsigned char) match_position;
			code_buf[code_buf_ptr++] = (unsigned char) (((match_position >> 4) & 0xf0) | (match_length - (THRESHOLD_S + 1)));
		}
		if ((mask <<= 1) == 0)
		{
			for (i = 0; i < code_buf_ptr; i++)
				putc(code_buf[i], fpOut);
			codesize += code_buf_ptr;
			code_buf[0] = 0;
			code_buf_ptr = mask = 1;
		}
		last_match_length = match_length;
		for (i = 0; i < last_match_length && (c = getc(fpIn)) != EOF; i++)
		{
			DeleteNode(s);
			text_buf_S[s] = c;
			if (s < F_S - 1)
				text_buf_S[s + N_S] = c;
			s = (s + 1) & (N_S - 1);
			r = (r + 1) & (N_S - 1);
			InsertNode(r);
		}
		if ((textsize += i) > printcount)
		{
			SendMessage(hWndPB, PBM_STEPIT, 0, 0);//进度条步进1个单位
			printcount += 1024;
		}
		while (i++ < last_match_length)
		{
			DeleteNode(s);
			s = (s + 1) & (N_S - 1);
			r = (r + 1) & (N_S - 1);
			if (--len) InsertNode(r);
		}
	} while (len > 0);
	if (code_buf_ptr > 1)
	{
		for (i = 0; i < code_buf_ptr; i++)
			putc(code_buf[i], fpOut);
		codesize += code_buf_ptr;
	}
	codesize=ftell(fpOut);
	sprintf(szInfo, "In : %ld bytes\nOut: %ld bytes\nOut/In: %.3f\n", textsize, codesize, (double)codesize / textsize);
}

bool DeLZSS(void)
{
	int  i, j, k, r, c, step;
	unsigned int  flags;
	DWORD sign, dwDeSize, dwEnSize;
	
	if(blG_sign)
	{
		fread(&sign, 4, 1, fpIn);
		if(g_sign!=sign)
			return false;
	}
	if(blDeSize)
		fread(&dwDeSize, 4, 1, fpIn);
	if(blEnSize)
		fread(&dwEnSize, 4, 1, fpIn);

	//初始化进度条
	SendMessage(hWndPB, PBM_SETRANGE32, 0, dwDeSize/1024);
    SendMessage(hWndPB, PBM_SETSTEP, (WPARAM) 1, 0);
	SendMessage(hWndPB, PBM_SETPOS, (WPARAM) 0, 0);

	for (i = 0; i < N_S - F_S; i++)
		text_buf_S[i] = 0x00;//' ';
	r = N_S - F_S;
	flags = 0;
	step=0;
	for ( ; ; )
	{
		if (((flags >>= 1) & 256) == 0)
		{
			if ((c = getc(fpIn)) == EOF)
				break;
			flags = c | 0xff00;		/* uses higher byte cleverly */
		}							/* to count eight */
		if (flags & 1)
		{
			if ((c = getc(fpIn)) == EOF)
				break;
			putc(c, fpOut);
			text_buf_S[r++] = c;
			r &= (N_S - 1);
			step++;
			if((step%1024)==1023)
				SendMessage(hWndPB, PBM_STEPIT, 0, 0);//进度条步进1个单位
		}
		else
		{
			if ((i = getc(fpIn)) == EOF)
				break;
			if ((j = getc(fpIn)) == EOF)
				break;
			i |= ((j & 0xf0) << 4);
			j = (j & 0x0f) + THRESHOLD_S;
			for (k = 0; k <= j; k++)
			{
				c = text_buf_S[(i + k) & (N_S - 1)];
				putc(c, fpOut);
				text_buf_S[r++] = c;
				r &= (N_S - 1);
				step++;
				if((step%1024)==1023)
					SendMessage(hWndPB, PBM_STEPIT, 0, 0);//进度条步进1个单位
			}
		}
	}
	if(dwDeSize!=ftell(fpOut))
		return false;
	else
		return true;
}

void InitTree_P()
{
	int  i;

	for (i = N_P + 1; i <= N_P + 256; i++)
		son_P[i] = NIL_P;
	for (i = 0; i < N_P; i++)
		dad_P[i] = NIL_P;
}

void InsertNode_P(int r, int j)
{
	int  i, p;
	unsigned char  *key;

	key = &text_buf_P[r];
	p = N_P + 1 + key[0];
	son_P[r] = NIL_P;
	match_length = 0;
	for ( ; ; )
	{
		if (son_P[p] != NIL_P)
		{
			p = son_P[p];
			if(((r-p)&(N_P-1))>=0xFF)
				continue;
		}
		else
		{  
			son_P[p] = r;
			dad_P[r] = p;
			return;
		}
		for (i = 1; i < F_P-j; i++)
		{
			if ((key[i] - text_buf_P[p + i]) != 0 || ((p+i)&(N_P-1))==r)
				break;
		}
		if (i >= match_length)
		{
			match_position = (r-p)&(N_P-1);
			if(match_position>=FLAG)
				match_position++;
			if ((match_length = i) >= F_P)
				break;
		}
	}
	dad_P[r] = dad_P[p];
	son_P[r] = son_P[p];
	dad_P[son_P[p]] = r;
	son_P[dad_P[p]] = r;
	dad_P[p] = NIL_P;
}

void DeleteNode_P(int p)
{
	if (dad_P[p] == NIL_P)
		return;
	if (son_P[p] == NIL_P)
	{
		son_P[dad_P[p]]=NIL_P;
		dad_P[p]=NIL_P;
	}
	else
	{
		son_P[dad_P[p]]=son_P[p];
		dad_P[son_P[p]]=dad_P[p];
		dad_P[p]=NIL_P;
	}
}

void EnLZSS_Prinny()
{
	int  i, j, c, len, r, s, last_match_length;
	long offsetOut;
	
	InitTree_P();
	s = 0;
	r = N_P - F_P;

	//初始化进度条
	fseek(fpIn, 0L, 2);
	SendMessage(hWndPB, PBM_SETRANGE32, 0, ftell(fpIn)/1024);
    SendMessage(hWndPB, PBM_SETSTEP, (WPARAM) 1, 0);
	SendMessage(hWndPB, PBM_SETPOS, (WPARAM) 0, 0);
	rewind(fpIn);
	printcount = 1024;

	for (i = s; i < r; i++)
		text_buf_P[i] = FLAG;
	for (len = 0; len < F_P && (c = getc(fpIn)) != EOF; len++)
		text_buf_P[r + len] = c;
	if ((textsize = len) == 0)
		return;
	//for (i = 1; i <= F_S; i++)
	//	InsertNode(r - i);
	InsertNode_P(r, 0);
	do
	{
		if (match_length > len)
			match_length = len;
		if (match_length <= THRESHOLD_P)
		{
			match_length = 1;
			putc(text_buf_P[r], fpOut);
			offsetOut=ftell(fpOut);
			if(FLAG == text_buf_P[r])
				putc(text_buf_P[r], fpOut);
		}
		else
		{
			putc(FLAG, fpOut);
			putc(match_position, fpOut);
			putc(match_length, fpOut);
			offsetOut=ftell(fpOut);
		}
		last_match_length = match_length;
		for (i = 0; i < last_match_length && (c = getc(fpIn)) != EOF; i++)
		{
			DeleteNode_P(s);
			text_buf_P[s] = c;
			if (s < F_P - 1)
				text_buf_P[s + N_P] = c;
			s = (s + 1) & (N_P - 1);
			r = (r + 1) & (N_P - 1);
			InsertNode_P(r, 0);
		}
		if ((textsize += i) > printcount)
		{
			SendMessage(hWndPB, PBM_STEPIT, 0, 0);//进度条步进1个单位
			printcount += 1024;
		}
		for (j=1;i++ < last_match_length;j++)
		{
			DeleteNode_P(s);
			s = (s + 1) & (N_P - 1);
			r = (r + 1) % (N_P - 1);
			if (--len) InsertNode_P(r, j);
		}
	} while (len > 0);
	codesize=ftell(fpOut);
	sprintf(szInfo, "In : %ld bytes\nOut: %ld bytes\nOut/In: %.3f\n", textsize, codesize, (double)codesize / textsize);
}

bool DeLZSS_Prinny()
{
	DWORD dwDeCodeSize,dwEnCodeSize,dwFlag;
	DWORD offsetIn,offsetOut;
	int r=N_P-F_P,p,i,count,byBuffer=0, step;

	fread(&dwFlag,4,1,fpIn);
	if(0x00746164!=dwFlag)
		return false;
	fread(&dwDeCodeSize,4,1,fpIn);
	fread(&dwEnCodeSize,4,1,fpIn);
	fread(&dwFlag,4,1,fpIn);

	for (i = 0; i < N_P; i++)
		text_buf_P[i] = 0x00;

	//初始化进度条
	SendMessage(hWndPB, PBM_SETRANGE32, 0, dwDeCodeSize/1024);
    SendMessage(hWndPB, PBM_SETSTEP, (WPARAM) 1, 0);
	SendMessage(hWndPB, PBM_SETPOS, (WPARAM) 0, 0);

	step=0;
	while((byBuffer=fgetc(fpIn))!=EOF)
	{
		if(dwFlag==byBuffer)
		{
			if((byBuffer=fgetc(fpIn))==EOF)
				break;
			if(dwFlag!=byBuffer)
			{
				if((unsigned int)byBuffer>dwFlag)
					byBuffer--;
				p=(r-byBuffer)&(N_P-1);
				if((count=fgetc(fpIn))==EOF)
					break;
				for(i=0;i<count;i++)
				{
					byBuffer=text_buf_P[(p+i)&(N_P-1)];
					fputc(byBuffer, fpOut);
					text_buf_P[r++]=(unsigned char)byBuffer;
					r&=(N_P-1);
					step++;
					if((step%1024)==1023)
						SendMessage(hWndPB, PBM_STEPIT, 0, 0);//进度条步进1个单位
				}
			}
			else
			{
				fputc(byBuffer, fpOut);
				text_buf_P[r++]=(unsigned char)byBuffer;
				r&=(N_P-1);
				step++;
				if((step%1024)==1023)
					SendMessage(hWndPB, PBM_STEPIT, 0, 0);//进度条步进1个单位
			}
		}
		else
		{
			fputc(byBuffer, fpOut);
			text_buf_P[r++]=(unsigned char)byBuffer;
			r&=(N_P-1);
			step++;
			if((step%1024)==1023)
				SendMessage(hWndPB, PBM_STEPIT, 0, 0);//进度条步进1个单位
			offsetOut=ftell(fpOut);
			offsetIn=ftell(fpIn);
		}
	}
	if(ftell(fpOut)==dwDeCodeSize)
		return true;
	else
		return false;
}