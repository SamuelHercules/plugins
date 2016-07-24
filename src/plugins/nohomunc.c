//===== Hercules Plugin ======================================
//= Disable homunculus mapflag
//===== By: ==================================================
//= Samuel [Hercules]
//===== Current Version: =====================================
//= 1.0
//===== Compatible With: ===================================== 
//= Hercules/RagEmu
//===== Description: =========================================
//= Disable Homunculus in a certain map
//===== Usage: ===============================================
//= alberta <tab> mapflag <tab> nohomunc
//============================================================


#include "common/hercules.h"
#include "common/utils.h"
#include "common/memmgr.h"
#include "common/nullpo.h"
#include "common/random.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "map/pc.h"
#include "map/npc.h"
#include "map/intif.h"
#include "map/homunculus.h"

#include "plugins/HPMHooking.h"
#include "../common/HPMDataCheck.h" // should always be the last file included! (if you don't make it last, it'll intentionally break compile time)

HPExport struct hplugin_info pinfo = {	
	"nohomunc",	    // Plugin name	
	SERVER_TYPE_MAP,// Which server types this plugin works with?	
	"0.1",			// Plugin version	
	HPM_VERSION,	// HPM Version (don't change, macro is automatically updated)
};

struct mapflag_data {
	unsigned nohomunc : 1;
};

void npc_parse_unknown_mapflag_pre(const char **name, const char **w3, const char **w4, const char **start, const char **buffer, const char **filepath, int **retval)
{
	if (!strcmp(*w3,"nohomunc")) {
		int16 m = map->mapname2mapid(*name);
		struct mapflag_data *mf;
		if (!( mf = getFromMAPD(&map->list[m], 0))) {
			CREATE(mf, struct mapflag_data, 1);
			addToMAPD(&map->list[m], mf, 0, true);
		}
		mf->nohomunc = 1;
		hookStop();
	}
	return;
}
	
void clif_parse_LoadEndAck_post (int fd, struct map_session_data *sd)
{
	struct homun_data *hd;
	struct mapflag_data *mf;
	nullpo_retv(sd);
	hd = sd->hd;
	mf = getFromMAPD(&map->list[sd->bl.m], 0);
	if( homun_alive(sd->hd) ) {
		if (mf && mf->nohomunc) {
			homun->vaporize(sd, HOM_ST_REST);
			clif->messagecolor_self(sd->fd, COLOR_RED, "You can't spawn homunculus here.");
			hookStop();
		}
	}
	return;
}

bool homunculus_call_post(bool retVal___, struct map_session_data *sd)
{
	struct homun_data *hd;
	struct mapflag_data *mf;
	
	hd = sd->hd;
	mf = getFromMAPD(&map->list[sd->bl.m], 0);

	if (retVal___ = true) {
		if (mf && mf->nohomunc && sd->status.hom_id) {
			homun->vaporize(sd, HOM_ST_REST);
			clif->messagecolor_self(sd->fd, COLOR_RED, "You can't spawn homunculus here.");
			hookStop();
		}
	}
	return retVal___;
}

bool homunculus_create_post(bool retVal___, struct map_session_data *sd, const struct s_homunculus *hom)
{
	struct homun_data *hd;
	struct mapflag_data *mf;

	nullpo_retr(false, sd);
	nullpo_retr(false, hom);
	hd = sd->hd;
	mf = getFromMAPD(&map->list[sd->bl.m], 0);
	if (retVal___ = true) {
		if (mf && mf->nohomunc) {
			homun->vaporize(sd, HOM_ST_REST);
			clif->messagecolor_self(sd->fd, COLOR_RED, "You can't spawn homunculus here.");
			retVal___ = false;
			return false;
			hookStop();
		}
	}
	return retVal___;
}

void map_flags_init_pre(void)
{
	int i;
	for (i = 0; i < map->count; i++) {
		struct mapflag_data *mf = getFromMAPD(&map->list[i], 0);
		if (mf)
			removeFromMAPD(&map->list[i], 0);
	}
	return;
}

HPExport void plugin_init (void) {
	addHookPre(npc, parse_unknown_mapflag, npc_parse_unknown_mapflag_pre);
	addHookPost(clif, pLoadEndAck, clif_parse_LoadEndAck_post);
	addHookPost(homun, call, homunculus_call_post);
	addHookPost(homun, create, homunculus_create_post);
	addHookPre(map, flags_init, map_flags_init_pre);
}