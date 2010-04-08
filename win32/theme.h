#ifndef _theme
#define _theme

#include <uxtheme.h>
#include <tmschema.h>

HTHEME open_theme_data(HWND,LPCWSTR);
HRESULT close_theme_data(HTHEME);
HRESULT draw_theme_background(HTHEME,HDC,int,int,const RECT*,const RECT*);
HRESULT draw_theme_text(HTHEME,HDC,int,int,LPCWSTR,int,DWORD,DWORD,const RECT*);
HRESULT get_theme_text_extent(HTHEME,HDC,int,int,LPCWSTR,int,DWORD,const RECT*,RECT*);
BOOL is_theme_active();

void draw_sunken_box(HDC dc, RECT *r);

#endif
