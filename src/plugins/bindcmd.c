//===== Hercules Plugin ======================================
//= custom commands go to @commands
//===== By: ==================================================
//= Samuel [Hercules]
//===== Current Version: =====================================
//= 1.0
//===== Compatible With: ===================================== 
//= Hercules
//===== Description: =========================================
//= Customizes @command to display custom commands using bind
//===== Credits: =============================================
//= RagEmu
//============================================================


#include "common/hercules.h"
#include "common/memmgr.h"
#include "common/mmo.h"
#include "common/socket.h"
#include "common/strlib.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../map/atcommand.h"
#include "../map/pc.h"
#include "../map/clif.h"
#include "../common/HPMi.h"

#include "plugins/HPMHooking.h"
#include "../common/HPMDataCheck.h" // should always be the last file included! (if you don't make it last, it'll intentionally break compile time)

HPExport struct hplugin_info pinfo = {	
	"bindcmd",	    // Plugin name	
	SERVER_TYPE_MAP,// Which server types this plugin works with?	
	"0.1",			// Plugin version	
	HPM_VERSION,	// HPM Version (don't change, macro is automatically updated)
};

static char atcmd_output2[CHAT_SIZE_MAX];

void atcommand_commands_sub_post(struct map_session_data* sd, const int fd, AtCommandType type)
{
	char line_buff[CHATBOX_SIZE];
	char* cur = line_buff;
	int count = 0;

	if (atcommand->binding_count) {
		int i, count_bind = 0;
		int gm_lvl = pc_get_group_level(sd);
		size_t slen;
		for (i = 0; i < atcommand->binding_count; i++) {
			if (gm_lvl >= ((type == COMMAND_ATCOMMAND) ? atcommand->binding[i]->group_lv : atcommand->binding[i]->group_lv_char)) {
				slen = strlen(atcommand->binding[i]->command);
				if (count_bind == 0) {
					cur = line_buff;
					memset(line_buff, ' ', CHATBOX_SIZE);
					line_buff[CHATBOX_SIZE-1] = 0;
					clif->message(fd, "------------------");
					clif->message(fd, "Custom commands:");
				}
				if (slen + cur - line_buff >= CHATBOX_SIZE) {
					clif->message(fd, line_buff);
					cur = line_buff;
					memset(line_buff,' ',CHATBOX_SIZE);
					line_buff[CHATBOX_SIZE-1] = 0;
				}
				memcpy(cur, atcommand->binding[i]->command, slen);
				cur += slen + (10 - slen % 10);
				count_bind++;
			}
		}
		if (count_bind)
			clif->message(fd, line_buff);	// Last Line
		count += count_bind;
	}
	safesnprintf(atcmd_output2, sizeof(atcmd_output2), msg_fd(fd,274), count); // "%d commands found."
	clif->message(fd, atcmd_output2);
	return;
}

HPExport void plugin_init (void) {
	addHookPost(atcommand, commands_sub, atcommand_commands_sub_post);
}