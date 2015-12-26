//===== Hercules Plugin ======================================
//= @cashreload
//===== By: ==================================================
//= Samuel [Hercules]
//===== Current Version: =====================================
//= 1.0
//===== Compatible With: ===================================== 
//= Hercules
//===== Description: =========================================
//= Reloads cash shop but requires players to relog
//= to see changes made in the cash shop
//===== Special Thanks: ======================================
//= AnnieRuru [Hercules]
//= For helping me code the countdown and for 
//= the maintenance plugin's code in kicking players
//============================================================

#include "common/hercules.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "map/atcommand.h"
#include "map/pc.h"
#include "map/clif.h"
#include "map/map.h"
#include "map/intif.h"
#include "common/timer.h"
#include "common/strlib.h"
#include "common/memmgr.h"
#include "common/nullpo.h"
#include "common/HPMDataCheck.h"

HPExport struct hplugin_info pinfo = {
	"cashreload",		// Plugin name
	SERVER_TYPE_MAP,// Which server types this plugin works with?
	"1.0",			// Plugin version
	HPM_VERSION,	// HPM Version (don't change, macro is automatically updated)
};

int cashreload_countid = INVALID_TIMER;
int countdown = 0;
int color = 0x00FFFF;

int cashreload_countdown ( int tid, int64 tick, int id, intptr data ) {
	char output[CHAT_SIZE_MAX];
	if ( --countdown ) {
		 safesnprintf ( output, CHAT_SIZE_MAX, "CashShop reloaded, players will be kicked in %d seconds", countdown);
		 intif->broadcast2( output, strlen(output) +1, color, 0x190, 12, 0, 0);
		 cashreload_countid = timer->add( timer->gettick() + 1000, cashreload_countdown, 0, 0 );
	 }
	 else {
		struct s_mapiterator* iter = mapit->alloc( MAPIT_NORMAL, BL_PC );
		TBL_PC *sd;
		safesnprintf( output, CHAT_SIZE_MAX, "CashShop reloaded. Every player will be kick out." );
		intif->broadcast2( output, strlen(output) +1, color, 0x190, 12, 0, 0);
		for ( sd = (TBL_PC*)mapit->first(iter); mapit->exists(iter); sd = (TBL_PC*)mapit->next(iter) )
				clif->authfail_fd( sd->fd, 1 );
		mapit->free(iter);
		countdown = 0;
		timer->delete( cashreload_countid, cashreload_countdown );
		cashreload_countid = INVALID_TIMER;
	 }
	return 0;
}

ACMD(cashreload) {//@sample command - 5 params: const int fd, struct map_session_data* sd, const char* command, const char* message, struct AtCommandInfo *info
	if ( countdown )
		return false;
	clif->cashshop_load();
	cashreload_countid = timer->add( timer->gettick() + 1000, cashreload_countdown, 0, 0 );
	clif->message(fd, "Cash Shop Reloaded");
	countdown = 11;
	return true;
}

HPExport void plugin_init (void) {
	addAtcommand("cashreload",cashreload);//link our '@sample' command
}