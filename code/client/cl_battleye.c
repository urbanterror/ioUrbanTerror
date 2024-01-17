/*
Copyright (C) 2007 Bastian Suter ("Sable")

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

/************************************************************************/
/* BattlEye Client code                                                 */
/************************************************************************/

#include "../qcommon/q_shared.h"
#include "../qcommon/qcommon.h"
#include "../qcommon/sys_library.h"
#include "client.h"


#ifdef BATTLEYE


static void CL_BE_SendPacket(void *packet, size_t len)
{
	MSG_WriteByte(&cls.be.outMsg, clc_battleye);
	MSG_WriteShort(&cls.be.outMsg, len);
	MSG_WriteData(&cls.be.outMsg, packet, len);
}

static void CL_BE_PrintMessage(char *message)
{
	Cbuf_AddText(va("ut_echo \"" S_COLOR_RED "BattlEye Client" S_COLOR_WHITE ": %s\"\n", message));
}


void CL_BE_Load(void)
{
	if (clbe.module)
		return;

	if ((clbe.module = Sys_Library_Open("BattlEye/BEClient_" ARCH_STRING DLL_EXT)))
	{
		// the BE "Init" export
		byte (*Init)(void (*)(char *), void (*)(void *, size_t), byte (**)(void), void (**)(void *, size_t));
		if ((Init = Sys_Library_ProcAddress(clbe.module, "Init")))
		{
			if (Init(&CL_BE_PrintMessage, &CL_BE_SendPacket, &clbe.Run, &clbe.NewPacket))
				Com_Printf("BattlEye Client loaded\n");
			else
			{
				CL_BE_Unload();
				Com_Error(ERR_DROP, "Failed to initialize BattlEye Client");
			}
		}
		else
		{
			CL_BE_Unload();
			Com_Error(ERR_DROP, "Failed to get BattlEye Client procedure");
		}
	}
	else
		Com_Error(ERR_DROP, "Failed to load BattlEye Client");
}

void CL_BE_Unload(void)
{
	if (!clbe.module)
		return;

	if (Sys_Library_Close(clbe.module))
		clbe.module = NULL;
	else
		Com_Error(ERR_DROP, "Failed to unload BattlEye Client");
}

#endif // BATTLEYE
