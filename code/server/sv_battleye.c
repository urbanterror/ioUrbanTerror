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
/* BattlEye Server code                                                 */
/************************************************************************/

#include "../qcommon/q_shared.h"
#include "../qcommon/qcommon.h"
#include "../qcommon/sys_library.h"
#include "server.h"


#ifdef BATTLEYE


static void SV_BE_SendPacket(int pid, void *packet, size_t len)
{
	MSG_WriteByte(&svs.clients[pid].be.outMsg, svc_battleye);
	MSG_WriteShort(&svs.clients[pid].be.outMsg, len);
	MSG_WriteData(&svs.clients[pid].be.outMsg, packet, len);
}

static void SV_BE_KickPlayer(int pid, char *reason)
{
	SV_SendServerCommand(&svs.clients[pid], "disconnect \"%You have been kicked by BattlEye: %s\"", reason);
	SV_DropClient(&svs.clients[pid], va("has been kicked by BattlEye: " S_COLOR_RED "%s", reason));
}

static void SV_BE_PrintMessage(char *message)
{
	Com_Printf("BattlEye Server: %s\n", message);
}


void SV_BE_Load(void)
{
	if (svbe.module)
		return;

	if ((svbe.module = Sys_Library_Open("BattlEye/BEServer_" ARCH_STRING DLL_EXT)))
	{
		// the BE "Init" export
		byte (*Init)(char *, int, void (*)(char *), void (*)(int, void *, size_t), void (*)(int, char *), byte (**)(void), void (**)(int, char *), void (**)(int), void (**)(int, void *, size_t));
		if ((Init = Sys_Library_ProcAddress(svbe.module, "Init")))
		{
			if (Init(Cvar_VariableString("g_modversion"), sv_maxclients->integer, &SV_BE_PrintMessage, &SV_BE_SendPacket, &SV_BE_KickPlayer, &svbe.Run, &svbe.AddPlayer, &svbe.RemovePlayer, &svbe.NewPacket))
			{
				int i;
				Com_Printf("BattlEye Server loaded\n");
				for (i = 0; i < sv_maxclients->integer; i++)
				{
					// hack so activationTime is never set to 0 here (0 stands for already
					// added in conjunction with CS_ACTIVE)
					if (svs.clients[i].state == CS_ACTIVE && !(svs.clients[i].gentity && (svs.clients[i].gentity->r.svFlags & SVF_BOT)))
						svs.clients[i].be.activationTime = svs.time != 0 ? svs.time : 1;
				}
			}
			else
			{
				SV_BE_Unload();
				Com_Error(ERR_DROP, "Failed to initialize BattlEye Server");
			}
		}
		else
		{
			SV_BE_Unload();
			Com_Error(ERR_DROP, "Failed to get BattlEye Server procedure");
		}
	}
	else
		Com_Error(ERR_DROP, "Failed to load BattlEye Server");
}

void SV_BE_Unload(void)
{
	if (!svbe.module)
		return;

	if (Sys_Library_Close(svbe.module))
		svbe.module = NULL;
	else
		Com_Error(ERR_DROP, "Failed to unload BattlEye Server");
}

#endif // BATTLEYE
