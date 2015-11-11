//===== Hercules Plugin ======================================
//= @Restock modification
//===== By: ==================================================
//= Dastgir/Hercules
//= Edited by Samuel/Hercules
//===== Current Version: =====================================
//= 1.1
//===== Compatible With: ===================================== 
//= Hercules SVN
//===== Description: =========================================
//= Restock item in your inventory depending in your settings
//= Please Load NPC- Restock.txt too
//============================================================
//===== Comments: ============================================
//== v. 1.0 Dastgir's Release
//== v. 1.1 - Added Config File to enable/disable restock in
//==		pvp,gvg,bg maps
//==		- Added Messages when Success/Fail to restock item
//==		- Added @restock/@restock2 commands
//============================================================

#include "common/hercules.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common/HPMi.h"
#include "common/memmgr.h"
#include "common/mmo.h"
#include "common/socket.h"
#include "common/strlib.h"
#include "common/sql.h"
#include "common/utils.h"
#include "common/nullpo.h"

#include "map/battle.h"
#include "map/mob.h"
#include "map/map.h"
#include "map/clif.h"
#include "map/pc.h"
#include "map/script.h"
#include "map/elemental.h"
#include "map/npc.h"
#include "map/status.h"
#include "map/storage.h"
#include "map/itemdb.h"
#include "map/guild.h"

#include "common/HPMDataCheck.h" /* should always be the last file included! (if you don't make it last, it'll intentionally break compile time) */

HPExport struct hplugin_info pinfo = {
	"Restock System",// Plugin name
	SERVER_TYPE_MAP,// Which server types this plugin works with?
	"1.1",			// Plugin version
	HPM_VERSION,	// HPM Version (don't change, macro is automatically updated)
};

static char atcmd_output[CHAT_SIZE_MAX];

int restock_misc_itemid;
int enable_pvp_map;

int pc_restock_misc_pre(struct map_session_data *sd,int *n,int *amount,int *type, short *reason, e_log_pick_type* log_type){
	int index = *n;
	if (!sd) return 1;
	restock_misc_itemid = 0;
	if(sd->status.inventory[index].nameid > 0){
		restock_misc_itemid = sd->status.inventory[index].nameid;
	}
	return 0;
}

int pc_restock_misc_post(int retVal, struct map_session_data *sd,int *n,int *amount,int *type, short *reason, e_log_pick_type* log_type){
	if (retVal==1) return 1;
	if (restock_misc_itemid && pc->search_inventory(sd,restock_misc_itemid) == -1){
		pc_setglobalreg(sd,script->add_str("restkid"), restock_misc_itemid );
		npc->event(sd, "Restock::OnRestock", 0);
	}
	return 0;
}


/*=========================================
 * Restock System
 *-----------------------------------------*/
ACMD(restock)
{
	if (!sd) return false;

	if( sd->npc_id || sd->vender_id || sd->buyer_id || sd->state.trading || sd->state.storage_flag )
		return false;

	npc->event(sd,"Restock::OnAtcommandEnable",0);
	return true;
}

/*=========================================
 * Restock System
 *-----------------------------------------*/
ACMD(restock2)
{
	if (!sd) return false;

	if( sd->npc_id || sd->vender_id || sd->buyer_id || sd->state.trading || sd->state.storage_flag )
		return false;

	npc->event(sd,"Restock::OnAtcommand",0);
	return true;
}

void enable_pvp_map_setting(const char *val) {
	int value = config_switch(val);
	enable_pvp_map = value;
	ShowDebug("Received 'enable_pvp_map_setting: %d'\n",enable_pvp_map);
}

BUILDIN(restock_item){
	int rid,rqu,fr;
	int i,j,flag;
	TBL_PC *sd;
	struct item item_tmp;
	struct item_data *i_data;
	rid = script_getnum( st, 2 );
	rqu = script_getnum( st, 3 );
	fr = script_getnum( st, 4 );
	sd = script->rid2sd(st);

	if (!sd) return true;
	if(!enable_pvp_map && map_flag_vs(sd->bl.m)) {
		clif->message(sd->fd, "Restocking is not available on pvp/gvg/bg maps");
		return true;
	}
	else if (fr == 2){
		struct guild *g;
		struct guild_storage *gstorage2;
		g = guild->search(sd->status.guild_id);
		if (g == NULL) {
			clif->message(sd->fd, msg_txt(43));
			return true;
		}
		gstorage2 = gstorage->ensure(sd->status.guild_id);
		if (gstorage == NULL) {// Doesn't have opened @gstorage yet, so we skip the deletion since *shouldn't* have any item there.
			return true;
		}
		j = gstorage2->storage_amount;
		gstorage2->lock = 1; 
		for (i = 0; i < j; ++i) {
			if (gstorage2->items[i].nameid == rid && gstorage2->items[i].amount >= rqu){
				memset(&item_tmp, 0, sizeof(item_tmp));
				item_tmp.nameid = gstorage2->items[i].nameid;
				item_tmp.identify = 1;
				gstorage->delitem(sd, gstorage2, i, rqu);
				if ((flag = pc->additem(sd,&item_tmp,rqu,LOG_TYPE_STORAGE))){
					clif->additem(sd, 0, 0, flag);
					pc_setglobalreg(sd,script->add_str("restkid1"), gstorage2->items[i].nameid );
					pc_setglobalreg(sd,script->add_str("restkid2"), rqu );
					script_pushint(st, 1);
					break;
				}
				i_data = itemdb->exists(rid);
				sprintf(atcmd_output, "Successfuly restocked: '%s'/'%s' {%d} from your guild storage.", i_data[i].name, i_data[i].jname, gstorage2->items[i].nameid);
				clif->message(sd->fd, atcmd_output);
			}
			else if (gstorage2->items[i].nameid == rid && gstorage2->items[i].amount < rqu){
				i_data = itemdb->exists(rid);
				sprintf(atcmd_output, "You don't have enough: '%s'/'%s' {%d} from your guild storage to restock.", i_data[i].name, i_data[i].jname, gstorage2->items[i].nameid);
				clif->message(sd->fd, atcmd_output);
				break;
			}
		}
		gstorage->close(sd);
		gstorage2->lock = 0;
	}
	else if (fr == 1){
		struct storage_data* stor = &sd->status.storage;
		if (stor == NULL){
			return true;
		}
		j = stor->storage_amount;
		if (sd->state.storage_flag){
			if (sd->state.storage_flag==1){
				sd->state.storage_flag = 0;
				storage->close(sd);
			}
			else{
				sd->state.storage_flag = 0;
				gstorage->close(sd);
			}
		}
		sd->state.storage_flag = 1;
		for (i = 0; i < j; ++i) {
			if (stor->items[i].nameid == rid && stor->items[i].amount >= rqu){
				i_data = itemdb->exists(rid);
				memset(&item_tmp, 0, sizeof(item_tmp));
				item_tmp.nameid = stor->items[i].nameid;
				item_tmp.identify = 1;
				storage->delitem(sd, i, rqu);
				if ((flag = pc->additem(sd,&item_tmp,rqu,LOG_TYPE_STORAGE))){
					clif->additem(sd, 0, 0, flag);
					pc_setglobalreg(sd,script->add_str("restkid1"), stor->items[i].nameid );
					pc_setglobalreg(sd,script->add_str("restkid2"), rqu );
					script_pushint(st, 1);
					break;
				}
				i_data = itemdb->exists(rid);
				sprintf(atcmd_output, "Successfuly restocked: '%s'/'%s' {%d} from your storage.", i_data[i].name, i_data[i].jname, stor->items[i].nameid);
				clif->message(sd->fd, atcmd_output);
			}
			else if (stor->items[i].nameid == rid && stor->items[i].amount < rqu){
				i_data = itemdb->exists(rid);
				sprintf(atcmd_output, "You don't have enough: '%s'/'%s' {%d} from your storage to restock.", i_data[i].name, i_data[i].jname, stor->items[i].nameid);
				clif->message(sd->fd, atcmd_output);
				break;
			}
		}
		sd->state.storage_flag = 0;
		storage->close(sd);

	}
	else {
		script_pushint(st, 0);
		return true;
	}
	script_pushint(st,0);
	
	return true;
}

HPExport void plugin_init(void) {
	addHookPre("pc->delitem", pc_restock_misc_pre);
	addHookPost("pc->delitem", pc_restock_misc_post);
	addAtcommand("restock",restock);
	addAtcommand("restock2",restock2);
	addScriptCommand("restock_item","iii",restock_item);
}

HPExport void server_preinit (void) {
	addBattleConf("enable_pvp_map",enable_pvp_map_setting);
}

HPExport void server_online (void) {
	ShowInfo ("'%s' Plugin by Dastgir/Hercules. Version '%s'\n",pinfo.name,pinfo.version);
}

