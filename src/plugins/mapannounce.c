//===== Hercules Plugin ======================================
//= Map Announce
//===== By: ==================================================
//= Samuel [Hercules]
//===== Current Version: =====================================
//= 1.0
//===== Compatible With: ===================================== 
//= Hercules
//===== Description: =========================================
//= Shows map name upon entering
//= Uses txt database where in you can edit what will show for
//= a certain map -> see map_desc.txt
//= @reloadmapdesc - reloads map_name_desc.txt
//= With ability to customized color and font size
//===== Credits: =============================================
//= [Cydh] - rAthena
//============================================================

#include "common/hercules.h"
#include "common/memmgr.h"
#include "common/HPMi.h"
#include "common/strlib.h"
#include "common/nullpo.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../map/atcommand.h"
#include "../map/pc.h"
#include "../map/clif.h"
#include "../map/map.h"

#include "plugins/HPMHooking.h"
#include "../common/HPMDataCheck.h" // should always be the last file included! (if you don't make it last, it'll intentionally break compile time)

HPExport struct hplugin_info pinfo = {	
	"mapannounce",	    // Plugin name	
	SERVER_TYPE_MAP,	// Which server types this plugin works with?	
	"0.1",				// Plugin version	
	HPM_VERSION,		// HPM Version (don't change, macro is automatically updated)
};

struct my_map_data {
	char desc[CHAT_SIZE_MAX];
	unsigned long desc_color;
};

int map_announce_color;
int map_announce_fontsize;

void val_bconf(const char *key, const char *val)
{
	if (strcmpi(key,"map_announce_color") == 0) {
		map_announce_color = config_switch(val);
		if (map_announce_color > 0xFFFFFF || map_announce_color < 0x000000) {
			ShowDebug("Wrong Value for map_announce_color: %d\n",config_switch(val));
			map_announce_color = 0xFFFF00;
		}
	} else if (strcmpi(key,"map_announce_fontsize") == 0) {
		map_announce_fontsize = config_switch(val);
		if (map_announce_fontsize > 32 || map_announce_fontsize < 10) {
			ShowDebug("Wrong Value for map_announce_fontsize: %d\n",config_switch(val));
			map_announce_fontsize = 12;
		}
	}
	return;
}

int val_return_bconf(const char *key)
{
	if (strcmpi(key,"map_announce_color") == 0) {
		return map_announce_color;
	} else if (strcmpi(key,"map_announce_fontsize") == 0) {
		return map_announce_fontsize;
	}
	return 0;
}

/*==========================================
 * Announce map name/description to player
 * [Cydh] house.bad@gmail.com
 *------------------------------------------*/
static bool map_parse_row_desc(char* split[], int columns, int current)
{
	struct my_map_data *mymap;
	short m;
	unsigned long map_color;
	char map_name[31];
	char map_desc[CHAT_SIZE_MAX];

	memset(map_name,0,sizeof(map_name));
	memset(map_desc,0,sizeof(map_desc));

	// find mapindex
	if( sscanf(split[0], "%31[^:]", map_name) != 1 )
	{
		ShowInfo("map_parse_row_desc: Invalid map name, skipping... (map: %s)\n", map_name);
		return true;
	}

	m = map->mapname2mapid(map_name);

	if( m < 0 )
	{
		ShowInfo("map_parse_row_desc: Unknown map, skipping... (map: %s)\n", map_name);
		return true;
	}

	if (!( mymap = getFromMAPD(&map->list[m], 0))) {
		CREATE(mymap, struct my_map_data, 1);
		addToMAPD(&map->list[m], mymap, 0, true);
	}
	// set the desc
	if( sscanf(split[1], "%255[^\n]", map_desc) != 1 ) {
		ShowInfo("map_parse_row_desc: Invalid map description, skipping... (map: %s)\n", map_name);
		return true;
	}
	safestrncpy(mymap->desc, map_desc, sizeof(map_desc));
	// set the announce color
	if((map_color = strtoul(split[2],NULL,0)))
	mymap->desc_color = map_color;

	return true;
}

void map_load_name_desc(void);
void map_load_name_desc(void)
{
	sv->readdb(map->db_path, "map_desc.txt", ':', 2, 3, -1, map_parse_row_desc);
}

void clif_parse_LoadEndAck_mappost (int fd, struct map_session_data *sd)
{
	struct my_map_data *mymap;
	nullpo_retv(sd);
	mymap = getFromMAPD(&map->list[sd->bl.m], 0);
	if (mymap->desc != "") {
		clif->broadcast2(&sd->bl, mymap->desc, (int)strlen(mymap->desc) + 1, mymap->desc_color?mymap->desc_color:map_announce_color, 0x190, map_announce_fontsize, 0, 0, SELF);
		hookStop();
		return;
	}
	return;
}

/*==========================================
 * @reloadmapdesc - reloads map_name_desc.txt
 * [Cydh] house.bad@gmail.com
 *------------------------------------------*/
ACMD(reloadmapdesc)
{
	map_load_name_desc();
	clif->messagecolor_self(fd, COLOR_GREEN, "Map descriptions had been reloaded.");
	return true;
}

HPExport void plugin_init (void) {
	addAtcommand("reloadmapdesc",reloadmapdesc);
	addHookPost(clif, pLoadEndAck, clif_parse_LoadEndAck_mappost);

	map_load_name_desc();
}

HPExport void server_preinit (void) {
	addBattleConf("map_announce_color",val_bconf, val_return_bconf);
	addBattleConf("map_announce_fontsize",val_bconf, val_return_bconf);
}