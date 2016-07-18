#include "common/hercules.h" /* Should always be the first Hercules file included! (if you don't make it first, you won't be able to use interfaces) */
#include "../common/timer.h"
#include "../map/script.h"
#include "../map/pc.h"
#include "../map/map.h"
#include "../map/unit.h"
#include "../map/atcommand.h"
#include "../map/itemdb.h"

#include "../common/HPMDataCheck.h"/* should always be the last file included! (if you don't make it last, it'll intentionally break compile time) */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define OPTION_AUTOPOTS 0x40000000

/*
3.1 Plugins Release [Mhalicot]
3.2 Update to Latest Revision [Mhalicot]
4.0 Added autohp, autosp command Rev. 137* [Mhalicot]
4.1 Fixed Compilation error. Rev. 145*** [Mhalicot]
-----------------------------------------------------
Documentation
@autopots <hp_rate> <hp_item id> <sp_rate> <sp_item id>
@autohp <hp_rate> <hp_item id>
@autosp <sp_rate> <sp_item id
// Change autopots/hp/sp timer by replacing +500 in milisec
		timer->add(timer->gettick()+500,autoatpots_timer,sd->bl.id,0);
*/
HPExport struct hplugin_info pinfo = {  
	"autopots",		// Plugin name
	SERVER_TYPE_MAP,	// Which server types this plugin works with?
	"4.0",				// Plugin version
	HPM_VERSION,		// HPM Version (don't change, macro is automatically updated)
};
struct autopots {
unsigned int hp_rate, sp_rate, hp_nameid, sp_nameid;
};
struct autopots autopots;
void autoatpots_clean(struct map_session_data *sd)
{
	if( sd )
	{
		sd->sc.option &= ~OPTION_AUTOPOTS;
		autopots.hp_nameid = 0;
		autopots.hp_rate = 0;
		autopots.sp_nameid = 0;
		autopots.sp_rate = 0;
		clif->changeoption(&sd->bl);
	}
	return;
}
void autoathp_clean(struct map_session_data *sd)
{
	if( sd )
	{
		sd->sc.option &= ~OPTION_AUTOPOTS;
		autopots.hp_nameid = 0;
		autopots.hp_rate = 0;
		clif->changeoption(&sd->bl);
	}
	return;
}
void autoatsp_clean(struct map_session_data *sd)
{
	if( sd )
	{
		sd->sc.option &= ~OPTION_AUTOPOTS;
		autopots.sp_nameid = 0;
		autopots.sp_rate = 0;
		clif->changeoption(&sd->bl);
	}
	return;
}
int autoatpots_timer(int tid, int64 tick, int id, intptr_t data)
{
	struct map_session_data *sd=NULL;
	struct item_data* item = NULL;
	int index;
	sd=map->id2sd(id);
	if( sd == NULL )
		return 0;
	if(sd->sc.option & OPTION_AUTOPOTS)
	{
		unsigned int hp_rate = autopots.hp_rate;
		unsigned int sp_rate = autopots.sp_rate;
		unsigned int hp_nameid = autopots.hp_nameid;
		unsigned int sp_nameid = autopots.sp_nameid;
		if( ( !sp_rate && !hp_rate ) || pc_isdead(sd) )
		{
			if( !hp_rate ) 
			{
				clif->message(sd->fd, "Auto-SP : OFF_");
				autoatsp_clean(sd);
				return 0;
			}else
			if( !sp_rate )
			{
				clif->message(sd->fd, "Auto-HP : OFF_");
				autoathp_clean(sd);
				return 0;
			}
		else{
			clif->message(sd->fd, "Auto-pots : OFF_");
			autoatpots_clean(sd);
			return 0;
			}
		}

		if( ( sd->battle_status.hp*100/sd->battle_status.max_hp ) < hp_rate && hp_nameid && hp_rate )
		{
			ARR_FIND(0, MAX_INVENTORY, index, sd->status.inventory[index].nameid == hp_nameid);
			if( sd->status.inventory[index].nameid == hp_nameid )
				pc->useitem(sd,index);
		}
		if( ( sd->battle_status.sp*100/sd->battle_status.max_sp ) < sp_rate && sp_nameid && sp_rate )
		{
			ARR_FIND(0, MAX_INVENTORY, index, sd->status.inventory[index].nameid == sp_nameid);
			if( sd->status.inventory[index].nameid == sp_nameid )
				pc->useitem(sd,index);
		}
		timer->add(timer->gettick()+500,autoatpots_timer,sd->bl.id,0);
	}
	return 0;
}
ACMD(autopots)
{
	int hp_rate=0, hp_nameid=0, sp_rate=0, sp_nameid=0;
	if( !sd ) return 0;

	if ( !message || !*message || (
		sscanf_s(message, "%d %d %d %d ", &hp_rate, &hp_nameid, &sp_rate, &sp_nameid) < 4) ||
		( hp_rate < 0 || hp_rate > 99 ) ||
		( sp_rate < 0 || sp_rate > 99 ) )
	{
		if ( sscanf_s(message, "%d %d %d %d ", &hp_rate, &hp_nameid, &sp_rate, &sp_nameid) < 4 &&
			sscanf_s(message, "%d %d %d %d ", &hp_rate, &hp_nameid, &sp_rate, &sp_nameid) > 0)
		{
			clif->message(fd, "@autopots <hp_rate> <hp_item id> <sp_rate> <sp_item id>");
			return false;
		}
		clif->message(fd, "Auto-pots : OFF");
		autoatpots_clean(sd);
		return true;
	}

	if (sd->sc.option & OPTION_AUTOPOTS)
	{
		autoatpots_clean(sd);
	}
	if( hp_rate == 0 ) hp_nameid = 0;
	if( sp_rate == 0 ) sp_nameid = 0;
	if( hp_nameid == 0 ) hp_rate = 0;
	if( sp_nameid == 0 ) sp_rate = 0;
	if( itemdb->exists(hp_nameid) == NULL && hp_nameid )
	{
		hp_nameid = 0;
		hp_rate = 0;
		clif->message(fd, "Auto-pots : Invalid item for HP");
	}
	if( itemdb->exists(sp_nameid) == NULL && sp_nameid )
	{
		sp_nameid = 0;
		sp_rate = 0;
		clif->message(fd, "Auto-pots : Invalid item for SP");
		return true;
	}
	clif->message(fd, "Auto-pots : ON");
	sd->sc.option |= OPTION_AUTOPOTS;
	autopots.hp_nameid = hp_nameid;
	autopots.hp_rate = hp_rate;
	autopots.sp_nameid = sp_nameid;
	autopots.sp_rate = sp_rate;
	timer->add(timer->gettick()+200,autoatpots_timer,sd->bl.id,0);

	clif->changeoption(&sd->bl);
	return true;
}
ACMD(autohp)
{
	int hp_rate=0, hp_nameid=0;
	if( !sd ) return 0;

	if (!message || !*message || (
		sscanf_s(message, "%d %d ", &hp_rate, &hp_nameid) < 2) ||
		( hp_rate < 0 || hp_rate > 99 ) )
	{
		if ( sscanf_s(message, "%d %d ", &hp_rate, &hp_nameid) < 2 &&
			sscanf_s(message, "%d %d ", &hp_rate, &hp_nameid) > 0) 
		{
			clif->message(fd, "@autohp <hp_rate> <hp_item id>");
			return true;
		}
		clif->message(fd, "Auto-HP : OFF");
		autoathp_clean(sd);
		return true;
	}

	if (sd->sc.option & OPTION_AUTOPOTS)
	{
		autoathp_clean(sd);
	}
	if( hp_rate == 0 ) hp_nameid = 0;
	if( hp_nameid == 0 ) hp_rate = 0;
	if( itemdb->exists(hp_nameid) == NULL && hp_nameid )
	{
		hp_nameid = 0;
		hp_rate = 0;
		clif->message(fd, "Auto-HP : Invalid item for HP");
		return true;
	}
	clif->message(fd, "Auto-HP : ON");
	sd->sc.option |= OPTION_AUTOPOTS;
	autopots.hp_nameid = hp_nameid;
	autopots.hp_rate = hp_rate;
	timer->add(timer->gettick()+200,autoatpots_timer,sd->bl.id,0);

	clif->changeoption(&sd->bl);
	return true;
}
ACMD(autosp)
{
	int sp_rate=0, sp_nameid=0;
	if( !sd ) return 0;

	if (!message || !*message || (
		sscanf_s(message, "%d %d ", &sp_rate, &sp_nameid) < 2) ||
		( sp_rate < 0 || sp_rate > 99 ) )
	{
		if ( sscanf_s(message, "%d %d ", &sp_rate, &sp_nameid) < 2 &&
			sscanf_s(message, "%d %d ", &sp_rate, &sp_nameid) > 0) 
		{
			clif->message(fd, "@autosp <sp_rate> <sp_item id>");
			return true;
		}
		clif->message(fd, "Auto-SP : OFF");
		autoatsp_clean(sd);
		return true;
	}

	if (sd->sc.option & OPTION_AUTOPOTS)
	{
		autoatsp_clean(sd);
	}
	if( sp_rate == 0 ) sp_nameid = 0;
	if( sp_nameid == 0 ) sp_rate = 0;
	if( itemdb->exists(sp_nameid) == NULL && sp_nameid )
	{
		sp_nameid = 0;
		sp_rate = 0;
		clif->message(fd, "Auto-SP : Invalid item for SP");
		return true;
	}
	clif->message(fd, "Auto-SP : ON");
	sd->sc.option |= OPTION_AUTOPOTS;
	autopots.sp_nameid = sp_nameid;
	autopots.sp_rate = sp_rate;
	timer->add(timer->gettick()+200,autoatpots_timer,sd->bl.id,0);

	clif->changeoption(&sd->bl);
	return true;
}
/* Server Startup */
HPExport void plugin_init (void)
{
	addAtcommand("autopots",autopots);//link our '@autopots' command
	addAtcommand("autohp",autohp);//link our '@autohp' command
	addAtcommand("autosp",autosp);//link our '@autosp' command
}