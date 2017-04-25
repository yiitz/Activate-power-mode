//this file is part of notepad++
//Copyright (C)2003 Don HO <donho@altern.org>
//
//This program is free software; you can redistribute it and/or
//modify it under the terms of the GNU General Public License
//as published by the Free Software Foundation; either
//version 2 of the License, or (at your option) any later version.
//
//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with this program; if not, write to the Free Software
//Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

#include "PluginDefinition.h"

extern FuncItem funcItem[nbFunc];
extern NppData nppData;

static void shake(HWND hwndFrom);
static void boom(HWND hwndFrom);
static void drawParticles(HWND hwndFrom);

extern BOOL isActive;

#include "Particle.h"
#define PARTICLE_COUNT 16
Particle particles[PARTICLE_COUNT*4];
BOOL isAllDead = TRUE;

BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  reasonForCall, 
                       LPVOID lpReserved )
{
    switch (reasonForCall)
    {
      case DLL_PROCESS_ATTACH:
        pluginInit(hModule);
        break;

      case DLL_PROCESS_DETACH:
		commandMenuCleanUp();
        pluginCleanUp();
        break;

      case DLL_THREAD_ATTACH:
        break;

      case DLL_THREAD_DETACH:
        break;
    }

    return TRUE;
}


extern "C" __declspec(dllexport) void setInfo(NppData notpadPlusData)
{
	nppData = notpadPlusData;
	commandMenuInit();
}

extern "C" __declspec(dllexport) const TCHAR * getName()
{
	return NPP_PLUGIN_NAME;
}

extern "C" __declspec(dllexport) FuncItem * getFuncsArray(int *nbF)
{
	*nbF = nbFunc;
	return funcItem;
}

extern "C" __declspec(dllexport) void beNotified(SCNotification *notification)
{
	if (!isActive)return;
	if (SCN_MODIFIED == notification->nmhdr.code && (SC_MOD_DELETETEXT & notification->modificationType || SC_MOD_INSERTTEXT & notification->modificationType))
	{
		shake((HWND)notification->nmhdr.hwndFrom);
		boom((HWND)notification->nmhdr.hwndFrom);
	}
	else if (SCN_PAINTED == notification->nmhdr.code)
	{
		if (!isAllDead)
		{
			drawParticles((HWND)notification->nmhdr.hwndFrom);
		}
	}
}

#define SHAKE_TIMER_ID 1
#define SHAKE_INTERVAL_MS 75
#define SHAKE_OFFSET 2

static BOOL hasFinishedShake = TRUE;
static int lastOffsetX;
static int lastOffsetY;

void moveWindow(HWND hwndFrom,int offsetX,int offsetY)
{
	RECT rect;
	::GetWindowRect(hwndFrom, &rect);
	::MapWindowPoints(HWND_DESKTOP, GetParent(hwndFrom), (LPPOINT)&rect, 2);
	::MoveWindow(hwndFrom, rect.left + offsetX, rect.top + offsetY, rect.right - rect.left, rect.bottom - rect.top, TRUE);
}

void CALLBACK shakeTimerProc(HWND hwnd, UINT message, UINT iTimerID, DWORD dwTime)
{
	::KillTimer(hwnd, SHAKE_TIMER_ID);
	moveWindow(hwnd, -lastOffsetX, -lastOffsetY);
	hasFinishedShake = TRUE;
}

void shake(HWND hwndFrom)
{
	if (!hasFinishedShake)return;
	hasFinishedShake = FALSE;
	switch (::GetTickCount() % 4)
	{
	case 0:
	{
		lastOffsetX = SHAKE_OFFSET;
		lastOffsetY = SHAKE_OFFSET;
		break;
	}
	case 1:
	{
		lastOffsetX = -SHAKE_OFFSET;
		lastOffsetY = SHAKE_OFFSET;
		break;
	}
	case 2:
	{
		lastOffsetX = SHAKE_OFFSET;
		lastOffsetY = -SHAKE_OFFSET;
		break;
	}
	case 3:
	{
		lastOffsetX = -SHAKE_OFFSET;
		lastOffsetY = -SHAKE_OFFSET;
		break;
	}
	default:
		break;
	}
	moveWindow(hwndFrom, lastOffsetX, lastOffsetY);
	::SetTimer(hwndFrom, SHAKE_TIMER_ID, SHAKE_INTERVAL_MS, shakeTimerProc);
}

#define BOOM_TIMER_ID 2
#define BOOM_INTERVAL_MS 20
static double particleRadius;

static int lastUpdateTick;

void drawParticles(HWND hwndFrom)
{
	Particle *temp;
	HDC hdc = GetDC(hwndFrom);
	RECT rect;
	GetClientRect(hwndFrom, &rect);

	SciFnDirect pSciMsg = (SciFnDirect)SendMessage(hwndFrom, SCI_GETDIRECTFUNCTION, 0, 0);
	sptr_t pSciWndData = (sptr_t)SendMessage(hwndFrom, SCI_GETDIRECTPOINTER, 0, 0);

	SelectObject(hdc, GetStockObject(DC_BRUSH));

	HPEN pen = CreatePen(PS_NULL, 0, RGB(0, 0, 0));
	SelectObject(hdc, pen);

	int radius = (int)(particleRadius - 1);
	for (size_t i = 0; i < (sizeof(particles) / sizeof(Particle)); i++)
	{
		temp = &particles[i];
		if (temp->isAlive())
		{
			SetDCBrushColor(hdc, temp->getColor());
			Ellipse(hdc, temp->getX() - radius, temp->getY() - radius, temp->getX() + radius, temp->getY() + radius);
		}
	}

	ReleaseDC(hwndFrom, hdc);
}
void updateParticles()
{
	Particle *temp;
	int now = GetTickCount();
	int elapsed = now - lastUpdateTick;
	particleRadius -= ((double)elapsed) / 1000;
	lastUpdateTick = now;
	isAllDead = TRUE;
	for (size_t i = 0; i < (sizeof(particles) / sizeof(Particle)); i++)
	{
		temp = &particles[i];
		if (temp->isAlive())
		{
			temp->update(elapsed);
		}
		if (temp->isAlive())
		{
			isAllDead = FALSE;
		}
	}
}

COLORREF currentColor(HWND hwnd)
{
  return SendMessage(hwnd, SCI_STYLEGETFORE, SendMessage(hwnd, SCI_GETSTYLEAT, SendMessage(hwnd, SCI_GETENDSTYLED, 0, 0)-1, 0), 0);
}

void CALLBACK boomTimerProc(HWND hwnd, UINT message, UINT iTimerID, DWORD dwTime)
{
	updateParticles();
	if (isAllDead)
	{
		KillTimer(hwnd,BOOM_TIMER_ID);
	}
	UpdateWindow(hwnd);
	RECT rect;
	GetClientRect(hwnd, &rect);
	InvalidateRect(hwnd, &rect, TRUE);
}

void boom(HWND hwndFrom)
{
	Particle *temp;

	GUITHREADINFO info;
	info.cbSize = sizeof(info);
	GetGUIThreadInfo(0, &info);

	particleRadius = 5;

	srand(GetTickCount());
	int count = 0;
	for (size_t i = 0; i < (sizeof(particles) / sizeof(Particle)) && count < PARTICLE_COUNT; i++)
	{
		temp = &particles[i];
		if (!temp->isAlive())
		{
			count += 1;
			temp->reset(info.rcCaret.left, info.rcCaret.top, currentColor(hwndFrom));
		}
	}
	lastUpdateTick = GetTickCount();
	drawParticles(hwndFrom);

	isAllDead = FALSE;
	::SetTimer(hwndFrom, BOOM_TIMER_ID, BOOM_INTERVAL_MS, boomTimerProc);
}

// Here you can process the Npp Messages 
// I will make the messages accessible little by little, according to the need of plugin development.
// Please let me know if you need to access to some messages :
// http://sourceforge.net/forum/forum.php?forum_id=482781
//
extern "C" __declspec(dllexport) LRESULT messageProc(UINT Message, WPARAM wParam, LPARAM lParam)
{
	return TRUE;
}

#ifdef UNICODE
extern "C" __declspec(dllexport) BOOL isUnicode()
{
    return TRUE;
}
#endif //UNICODE
