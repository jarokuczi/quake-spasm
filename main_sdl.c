/*
Copyright (C) 1996-2001 Id Software, Inc.
Copyright (C) 2002-2005 John Fitzgibbons and others
Copyright (C) 2007-2008 Kristian Duske
Copyright (C) 2010-2014 QuakeSpasm developers

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

#include "quakedef.h"
#if defined(SDL_FRAMEWORK) || defined(NO_SDL_CONFIG)
#if defined(USE_SDL2)
#include <SDL2/SDL.h>
#else
#include <SDL/SDL.h>
#endif
#else
#include "SDL2/SDL.h"
#endif

#include <stdio.h>

static void Sys_AtExit (void)
{
	SDL_Quit();
}

static void Sys_InitSDL (void)
{
#if defined(USE_SDL2)
	SDL_version v;
	SDL_version *sdl_version = &v;
	SDL_GetVersion(&v);
#else
	const SDL_version *sdl_version = SDL_Linked_Version();
#endif

	Sys_Printf("Found SDL version %i.%i.%i\n",sdl_version->major,sdl_version->minor,sdl_version->patch);

	if (SDL_Init(0) < 0) {
		Sys_Error("Couldn't init SDL: %s", SDL_GetError());
	}
	atexit(Sys_AtExit);
}

#define DEFAULT_MEMORY (256 * 1024 * 1024) // ericw -- was 72MB (64-bit) / 64MB (32-bit)

static quakeparms_t	parms;

// On OS X we call SDL_main from the launcher, but SDL2 doesn't redefine main
// as SDL_main on OS X anymore, so we do it ourselves.
#if defined(USE_SDL2) && defined(__APPLE__)
#define main SDL_main
#endif
#ifdef AMIGAOS
#include "GL/gl.h"
#include "GL/glu.h"
#include "exec/types.h"
#include "interfaces/exec.h"
#include "interfaces/ogles2.h"
#include <proto/exec.h>
#include <interfaces/bsdsocket.h>

extern struct ExecIFace      *IExec;
struct Library		  *Ogles2Base    = NULL;
struct OglesIFace     *IOgles2       = NULL;
struct SocketIFace *ISocket = NULL;
struct Library *SocketBase = NULL;
#endif

#ifdef MORPHOS
#include "proto/exec.h"
#include "ppcinline/exec.h"

struct Library *SocketBase = NULL;
struct Library *MathTransBase = NULL;
#endif

int main(int argc, char *argv[])
{
#ifdef MORPHOS
    SocketBase       = OpenLibrary("bsdsocket.library", 0);
    MathTransBase       = OpenLibrary("mathtrans.library", 0);
#endif
#ifdef AMIGAOS
    IExec = (struct ExecIFace *)(*(struct ExecBase **)4)->MainInterface;
    Ogles2Base = IExec->OpenLibrary("ogles2.library",0);
    IOgles2 = (struct IOgles2 *) IExec->GetInterface(Ogles2Base, "main", 1, NULL);

    if (!Ogles2Base) {
        printf("Can't open ogles2.library");
        exit(123);
    }
    SocketBase       = IExec->OpenLibrary("bsdsocket.library", 0L);
    ISocket          = (struct SocketIFace *)IExec->GetInterface( SocketBase, "main", 1, NULL );
#endif

	int		t;
	double		time, oldtime, newtime;

	host_parms = &parms;
	parms.basedir = ".";

	parms.argc = argc;
	parms.argv = argv;

	parms.errstate = 0;

	COM_InitArgv(parms.argc, parms.argv);

	isDedicated = (COM_CheckParm("-dedicated") != 0);

	Sys_InitSDL ();

	Sys_Init();

	Sys_Printf("Initializing QuakeSpasm v%s\n", QUAKESPASM_VER_STRING);

	parms.memsize = DEFAULT_MEMORY;
	if (COM_CheckParm("-heapsize"))
	{
		t = COM_CheckParm("-heapsize") + 1;
		if (t < com_argc)
			parms.memsize = Q_atoi(com_argv[t]) * 1024;
	}

	parms.membase = malloc (parms.memsize);

	if (!parms.membase)
		Sys_Error ("Not enough memory free; check disk space\n");

	Sys_Printf("Host_Init\n");
	Host_Init();

	oldtime = Sys_DoubleTime();
	if (isDedicated)
	{
		while (1)
		{
			newtime = Sys_DoubleTime ();
			time = newtime - oldtime;

			while (time < sys_ticrate.value )
			{
				SDL_Delay(1);
				newtime = Sys_DoubleTime ();
				time = newtime - oldtime;
			}

			Host_Frame (time);
			oldtime = newtime;
		}
	}
	else
	while (1)
	{
		/* If we have no input focus at all, sleep a bit */
		if (!VID_HasMouseOrInputFocus() || cl.paused)
		{
			SDL_Delay(16);
		}
		/* If we're minimised, sleep a bit more */
		if (VID_IsMinimized())
		{
			scr_skipupdate = 1;
			SDL_Delay(32);
		}
		else
		{
			scr_skipupdate = 0;
		}
		newtime = Sys_DoubleTime ();
		time = newtime - oldtime;

		Host_Frame (time);

		if (time < sys_throttle.value && !cls.timedemo)
			SDL_Delay(1);

		oldtime = newtime;
	}
#ifdef AMIGAOS
    if (IOgles2) {IExec->DropInterface((struct Interface*)IOgles2); IOgles2 = NULL;}
    if (Ogles2Base) {IExec->CloseLibrary(Ogles2Base); Ogles2Base = NULL;}
    if (ISocket)               IExec->DropInterface((struct Interface *)ISocket);
    if (SocketBase)            IExec->CloseLibrary(SocketBase);
#endif

#ifdef MORPHOS
    if (SocketBase)            CloseLibrary(SocketBase);
#endif
	return 0;
}
