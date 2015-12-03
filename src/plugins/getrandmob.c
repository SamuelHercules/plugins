//===== Hercules Plugin ======================================
//= getrandmob [script command]
//===== By: ==================================================
//= Samuel
//===== Current Version: =====================================
//= 1.2
//===== Compatible With: ===================================== 
//= Hercules SVN
//===== Description: =========================================
// Grabs a random monster from one of the branch databases. 
// Useful for hunter quests, disguise events, and anything
// else you can think of.
// Uses database stored in memory, so it's faster and more 
// efficient than running SQL queries and doesn't require you
// to use SQL dbs for items and mobs.
//===== Credits: =============================================
// Converted by: Samuel
// Original Made by: Akinari
//============================================================
//============================================================
//===== Comments: ============================================
//== v. 1.1 Samuel's Release
//== v. 1.2 Updated by Samuel for latest Hercules revision
//============================================================


#include "common/hercules.h" /* Should always be the first Hercules file included! (if you don't make it first, you won't be able to use interfaces) */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../common/HPMi.h"
#include "../common/mmo.h"
#include "../map/script.h"
#include "../map/mob.h"

#include "common/HPMDataCheck.h" /* should always be the last Hercules file included! (if you don't make it last, it'll intentionally break compile time) */

HPExport struct hplugin_info pinfo = {
	"getrandmob",		// Plugin name
	SERVER_TYPE_MAP,	// Which server types this plugin works with?
	"1.1",				// Plugin version
	HPM_VERSION,		// HPM Version (don't change, macro is automatically updated)
};

/*==========================================
 * Fetches a random mob_id
 * getrandmob(num,type);
 *
 * type: Where to fetch from:
 * 0: dead branch list
 * 1: poring list
 * 2: bloody branch list
 * num: Mob level to check against (0 = all)
 *------------------------------------------*/
BUILDIN(getrandmob)
{
	int num, id, type;
	num = script_getnum(st,2);
	type = script_getnum(st,3);
	
	if(type > 2 || type < 0) {
		ShowError("script:getrandmob: unknown random mob type %d\n", type);
		return false;
	}
	if(num > MAX_LEVEL || num < 0) {
		ShowError("script:getrandmob: out of range level %d\n", type);
		return false;
	}

	id = mob->get_random_id(type, 1, (num==0?MAX_LEVEL:num));
	script_pushint(st,id);

	return 1;
}

/* run when server starts */
HPExport void plugin_init (void) {
	addScriptCommand("getrandmob","ii",getrandmob);
}