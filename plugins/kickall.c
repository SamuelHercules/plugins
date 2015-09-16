//===== Hercules Plugin ======================================
//= @kickall modification
//===== By: ==================================================
//= Samuel
//===== Current Version: =====================================
//= 1.0
//===== Compatible With: ===================================== 
//= Hercules SVN
//===== Description: =========================================
//= Kick All Players
//= when "-at" is added in the command, it will not kick
//= players on autotrade
//============================================================

#include "common/hercules.h" /* Should always be the first Hercules file included! (if you don't make it first, you won't be able to use interfaces) */
#include "common/malloc.h"
#include "common/mmo.h"
#include "common/socket.h"
#include "common/strlib.h"
#include "map/atcommand.h"
#include "map/clif.h"
#include "map/map.h"
#include "map/pc.h"

#include "common/HPMDataCheck.h" /* should always be the last Hercules file included! (if you don't make it last, it'll intentionally break compile time) */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

HPExport struct hplugin_info pinfo = {
	"kickall",    // Plugin name
	SERVER_TYPE_MAP,// Which server types this plugin works with?
	"0.1",       // Plugin version
	HPM_VERSION, // HPM Version (don't change, macro is automatically updated)
};

/*==========================================
 *
 *------------------------------------------*/
ACMD(kickall)
{
	struct map_session_data* pl_sd;
	struct s_mapiterator* iter;

	if (message && *message) {
		if (!strcmp(message,"-at")) {
			iter = mapit->alloc( MAPIT_NORMAL, BL_PC );
			for ( pl_sd = (TBL_PC*)mapit->first(iter); mapit->exists(iter); pl_sd = (TBL_PC*)mapit->next(iter) )
			{
				if ( pl_sd->group_id < pc_get_group_level(sd) )
				clif->authfail_fd( pl_sd->fd, 1 );
			}
			mapit->free(iter);
			clif->message(sd->fd, "All players except for autotraders have been kicked");
		}
		return true;
	}
	else {
		iter = mapit_getallusers();
			for( pl_sd = (TBL_PC*)mapit->first(iter); mapit->exists(iter); pl_sd = (TBL_PC*)mapit->next(iter) )
			{
				if (pc_get_group_level(sd) >= pc_get_group_level(pl_sd)) { // you can kick only lower or same gm level
				if (sd->status.account_id != pl_sd->status.account_id)
				clif->GM_kick(NULL, pl_sd);
				}
			}

		mapit->free(iter);
		clif->message(fd, msg_fd(fd,195)); // All players have been kicked!
		return true;
	}

}

HPExport void plugin_init (void) {
	addAtcommand("kickall",kickall);
}