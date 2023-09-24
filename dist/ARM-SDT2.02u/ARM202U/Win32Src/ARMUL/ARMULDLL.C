/***************************************************************************\
* armuldll.c                                                                *
* Copyright (C) 1995 Advanced RISC Machines Limited.  All rights reserved.  *
\***************************************************************************/

/*
 * RCS $Revision: 1.1.2.1 $
 * Checkin $Date: 1996/02/07 15:50:38 $
 * Revising $Author: jporter $
 */

#include <windows.h>
#include <stdio.h>
#include "armuldll.h"

UINT AbortRead;

BOOL WINAPI DllMain (HANDLE hDLL, DWORD dwReason, LPVOID lpReserved)
{
  switch (dwReason)
  {
    case DLL_PROCESS_ATTACH:
    {
                AbortRead = RegisterWindowMessage("ABORT_READ");
#ifdef _DEBUG
      char buf[BUFSIZE+1];
      //
      // DLL is attaching to the address space of the current process.
      //

      ghMod = hDLL;
      GetModuleFileName (NULL, (LPTSTR) buf, BUFSIZE);
      MessageBox ( GetFocus(),
                  (LPCTSTR) buf,
                  (LPCTSTR) "ARMulator DLL: Process attaching",
                  MB_OK | MB_SYSTEMMODAL);
#endif
      break;
    }

    case DLL_THREAD_ATTACH:
        {
#ifdef _DEBUG

      //
      // A new thread is being created in the current process.
      //

      MessageBox ( GetFocus(),
                  (LPCTSTR) "ARMulator DLL: Thread attaching",
                  (LPCTSTR) "",
                  MB_OK | MB_SYSTEMMODAL);
#endif
      break;
        }
    case DLL_THREAD_DETACH:
        {
#ifdef _DEBUG

      //
      // A thread is exiting cleanly.
      //

      MessageBox ( GetFocus(),
                  (LPCTSTR) "ARMulator DLL: Thread detaching",
                  (LPCTSTR) "",
                  MB_OK | MB_SYSTEMMODAL);
#endif
      break;
        }
    case DLL_PROCESS_DETACH:
        {
#ifdef _DEBUG

      //
      // The calling process is detaching the DLL from its address space.
      //
      MessageBox ( GetFocus(),
                  (LPCTSTR) "ARMulator DLL: Process detaching",
                  (LPCTSTR) "",
                  MB_OK | MB_SYSTEMMODAL );
#endif
      break;
        }
  }

return TRUE;
}


/******************************************************************************\
*
*  FUNCTION: Utils
*
*  RETURNS:  ARMulator DLL Utility functions (not exported)
*
\******************************************************************************/

void YieldControl(int nLoops)
{
        MSG Message;
        int loop = 0;

        if (PeekMessage(&Message, NULL, AbortRead,AbortRead, PM_NOREMOVE)) return;
        if (PeekMessage(&Message, NULL, WM_CLOSE,WM_CLOSE, PM_NOREMOVE)) return;
        while (loop < nLoops)
        {
                while (PeekMessage(&Message, NULL, 0,0, PM_REMOVE))
                {
                        TranslateMessage(&Message);
                        DispatchMessage(&Message);
                }
                loop++;
        }
}

void armsd_hourglass(void)
{
        YieldControl(1); // This could be Selected By Options
}

