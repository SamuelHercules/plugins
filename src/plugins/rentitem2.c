//===== Hercules Plugin ======================================
//= Rentitem2 atcommand + scriptcommands
//= Getequipexpiretick script command
//===== By: ==================================================
//= Mhalicot/Hercules
//= Edited and Updated by Samuel/Hercules
//===== Current Version: =====================================
//= 1.1
//===== Compatible With: ===================================== 
//= Hercules SVN
//===== Description: =========================================
//= usage: @rentitem <item name/ID> <quantity> <minutes>
//= usage: @rentitem2 <item name/ID> <quantity> <minutes>
//=        <identify_flag> <refine> <attribute> <card1> <card2> 
//=        <card3> <card4> )."
//= Script:
//= renitem2 <item name/ID> <quantity> <minutes>
//=        <identify_flag> <refine> <attribute> <card1> <card2> 
//=        <card3> <card4>
//============================================================
//===== Comments: ============================================
//== v. 1.0 Mhalicot's Release
//== v. 1.1 Updated by Samuel for latest Hercules revision
//==      - Added ability to have <quantity> for rental items
//==        for both atcommand and script commands
//==      - Added some safety checks for the script command
//==        to prevent items that should not be rented
//============================================================

#include "common/hercules.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "../common/HPMi.h"
#include "../common/timer.h"
#include "../map/script.h"
#include "../map/status.h"
#include "../map/pc.h"
#include "../map/clif.h"
#include "../map/pet.h"
#include "../map/script.h"
#include "../map/itemdb.h"

#include "common/HPMDataCheck.h" /* should always be the last file included! (if you don't make it last, it'll intentionally break compile time) */

/**
 * 1.0 - Initial Script [Mhalicot]
 * 2.0 - Added [AtCommand]rentitem
 **/
HPExport struct hplugin_info pinfo = {
	"rentitem2",		// Plugin name
	SERVER_TYPE_MAP,// Which server types this plugin works with?
	"1.0",			// Plugin version
	HPM_VERSION,	// HPM Version (don't change, macro is automatically updated)
};

ACMD(rentitem)
{
	char item_name[100];
	int number = 0, item_id, flag = 0, quantity = 0;
	struct item item_tmp;
	struct item_data *item_data;
	int get_count, get_count2;
	
	memset(item_name, '\0', sizeof(item_name));

	if (!message || !*message || (
		sscanf(message, "\"%99[^\"]\" %d %d", item_name, &quantity, &number) < 3 && 
		sscanf(message, "%99s  %d %d", item_name, &quantity, &number) < 3 )) 
		{
 		clif->message(fd, "Please enter an item name or ID (usage: @rentitem <item name/ID> <quantity> <minutes>).");
		return true;
	}
	
	if (number <= 0)
		number = 1;
	
	if (quantity <= 0)
		quantity = 1;

	if ((item_data = itemdb->search_name(item_name)) == NULL &&
	    (item_data = itemdb->exists(atoi(item_name))) == NULL)
	{
		clif->message(fd, "Invalid item ID or name.");
		return false;
	}

	item_id = item_data->nameid;
	get_count = number;
	get_count2 = quantity;
	//Check if it's stackable.
	if (itemdb->isstackable2(item_data)) {
		clif->message(fd, "Cannot create rented stackable items.");
		return false;
	}
	if(item_data->type == IT_PETEGG || item_data->type == IT_PETARMOR) {
		clif->message(fd, "Cannot create rented pet eggs or pet armors.");
		return false;
	}

		// if not pet egg
		if (!pet->create_egg(sd, item_id)) {
				number = get_count*60;
				memset(&item_tmp, 0, sizeof(item_tmp));
				item_tmp.nameid = item_id;
				item_tmp.identify = 1;
				item_tmp.expire_time = (unsigned int)(time(NULL) + number);
				item_tmp.bound = 0;

			if ((flag = pc->additem(sd, &item_tmp, get_count2, LOG_TYPE_COMMAND)))
				clif->additem(sd, 0, 0, flag);
		}
	
	if (flag == 0)
		clif->message(fd, "Item created.");
	return true;
}

/*==========================================
 * @rentitem2 (revised by Samuel)
 *------------------------------------------*/
ACMD(rentitem2)
{
	struct item item_tmp;
	struct item_data *item_data;
	char item_name[100];
	int item_id, number = 0, quantity = 0;
	int identify = 0, refine = 0, attr = 0;
	int c1 = 0, c2 = 0, c3 = 0, c4 = 0;

	memset(item_name, '\0', sizeof(item_name));

	if (!*message
	         || ( sscanf(message, "\"%99[^\"]\" %12d %12d %12d %12d %12d %12d %12d %12d %12d", item_name, &quantity, &number, &identify, &refine, &attr, &c1, &c2, &c3, &c4) < 10
	           && sscanf(message, "%99s %12d %12d %12d %12d %12d %12d %12d %12d %12d", item_name, &quantity, &number, &identify, &refine, &attr, &c1, &c2, &c3, &c4) < 10
	)) {
		clif->message(fd, "Please enter an item name or ID (usage: @rentitem2 <item name/ID> <quantity> <minutes> ");
		clif->message(fd, "    <identify_flag> <refine> <attribute> <card1> <card2> <card3> <card4> ).");
		return false;
	}

	if (number <= 0)
		number = 1;

	if (quantity <= 0)
		quantity = 1;

	item_id = 0;
	if ((item_data = itemdb->search_name(item_name)) != NULL ||
	    (item_data = itemdb->exists(atoi(item_name))) != NULL)
		item_id = item_data->nameid;

	if (item_id > 500) {
		int flag = 0;
		int loop, get_count, get_count2, i;
		loop = 1;
		get_count = quantity;
		get_count2 = number;
		//Check if it's stackable.
		if (itemdb->isstackable2(item_data)) {
			clif->message(fd, "Cannot create rented stackable items.");
			return false;
		}
		if(item_data->type == IT_PETEGG || item_data->type == IT_PETARMOR) {
		clif->message(fd, "Cannot create rented pet eggs or pet armors.");
		return false;
		}
		if( !itemdb->isstackable2(item_data) ) {
			loop = quantity;
			get_count = 1;
		}
		for (i = 0; i < loop; i++) {
			number = get_count2*60;
			memset(&item_tmp, 0, sizeof(item_tmp));
			item_tmp.nameid = item_id;
			item_tmp.expire_time = (unsigned int)(time(NULL) + number);
			item_tmp.identify = identify;
			item_tmp.refine = refine;
			item_tmp.attribute = attr;
			item_tmp.card[0] = c1;
			item_tmp.card[1] = c2;
			item_tmp.card[2] = c3;
			item_tmp.card[3] = c4;

			if ((flag = pc->additem(sd, &item_tmp, get_count, LOG_TYPE_COMMAND)))
				clif->additem(sd, 0, 0, flag);
		}

		if (flag == 0)
			clif->message(fd, msg_fd(fd,18)); // Item created.
	} else {
		clif->message(fd, msg_fd(fd,19)); // Invalid item ID or name.
		return false;
	}

	return true;
}

BUILDIN(getequipexpiretick) {
	int i, num;
	TBL_PC* sd;

	sd = script->rid2sd(st);
	if( sd == NULL )
		return true;

	num = script_getnum(st,2) - 1;
	if( num < 0 || num >= ARRAYLENGTH(script->equip) )
	{
		script_pushint(st,-1);
		return true;
	}

	// get inventory position of item
	i = pc->checkequip(sd,script->equip[num]);
	if( i < 0 )
	{
		script_pushint(st,-1);
		return true;
	}
	
	if( sd->inventory_data[i] != 0 && sd->status.inventory[i].expire_time)
		script_pushint(st, (unsigned int)(sd->status.inventory[i].expire_time - time(NULL)) );
	else
		script_pushint(st,0);

	return true;
}

BUILDIN(rentitem2) {
	struct map_session_data *sd;
	struct script_data *data;
	struct item it;
	struct item_data *item_data;
	int seconds, quantity;
	int nameid = 0, flag;
	int iden,ref,attr,c1,c2,c3,c4;
	int loop, get_count, get_count2, i;
	data = script_getdata(st,2);
	script->get_val(st,data);

	if( (sd = script->rid2sd(st)) == NULL )
		return 0;

	if( data_isstring(data) )
	{
		const char *name = script->conv_str(st,data);
		item_data = itemdb->search_name(name);
		if( item_data == NULL )
		{
			ShowError("buildin_rentitem: Nonexistant item %s requested.\n", name);
			return 1;
		}
		nameid = item_data->nameid;
	}
	else if( data_isint(data) )
	{
		nameid = script->conv_num(st,data);
		item_data = itemdb->exists(nameid);
		if( nameid <= 0 || !item_data)
		{
			ShowError("buildin_rentitem: Nonexistant item %d requested.\n", nameid);
			return 1;
		}
	}
	else
	{
		ShowError("buildin_rentitem: invalid data type for argument #1 (%d).\n", data->type);
		return 1;
	}

	quantity = script_getnum(st,3);
	seconds = script_getnum(st,4);
	iden=script_getnum(st,5);
	ref=script_getnum(st,6);
	attr=script_getnum(st,7);
	c1=(short)script_getnum(st,8);
	c2=(short)script_getnum(st,9);
	c3=(short)script_getnum(st,10);
	c4=(short)script_getnum(st,11);

	loop = 1;
	get_count = quantity;
	get_count2 = seconds;
	//Check if it's stackable.
	if (itemdb->isstackable2(item_data)) {
		ShowError("buildin_rentitem: Cannot create rented stackable items.");
		return false;
	}
	if(item_data->type == IT_PETEGG || item_data->type == IT_PETARMOR) {
		ShowError("buildin_rentitem: Cannot create rented pet eggs or pet armors.");
		return false;
	}
	if( !itemdb->isstackable2(item_data) ) {
		quantity = script_getnum(st,3);
		loop = quantity;
		get_count = 1;
	}
	for (i = 0; i < loop; i++) {
		memset(&it, 0, sizeof(it));
		it.nameid = nameid;
		it.identify = iden;
		it.refine=ref;
		it.attribute=attr;
		it.card[0]=(short)c1;
		it.card[1]=(short)c2;
		it.card[2]=(short)c3;
		it.card[3]=(short)c4;
		it.expire_time = (unsigned int)(time(NULL) + (seconds * 60) );

		if ((flag = pc->additem(sd, &it, get_count, LOG_TYPE_COMMAND)))
			clif->additem(sd, 0, 0, flag);
	}

	return true;
}

/* run when server starts */
HPExport void plugin_init (void) {

	addAtcommand("rentitem",rentitem);
	addAtcommand("rentitem2",rentitem2);
	
	addScriptCommand("rentitem2","viiiiiiiii",rentitem2);
	addScriptCommand("getequipexpiretick","i",getequipexpiretick);
}