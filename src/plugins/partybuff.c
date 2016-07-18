//===== Hercules Plugin ======================================
//= Show Party Buffs
//===== By: ==================================================
//= Samuel [Hercules]
//===== Current Version: =====================================
//= 1.0
//===== Compatible With: ===================================== 
//= Hercules/RagEmu
//===== Description: =========================================
//= With this plugin, it will show certain buffs that are
//= present with you and your party members in the party
//= window.
//= B - Blessing
//= A - Agility Up
//= F - Full Chemical Protection
//= S - Soul Link
//= + - Devotion
//===== Credits: =============================================
//= AnnieRuru - originally made the plugin
//= Functor - bug fix
//============================================================

#include "common/hercules.h"
#include "common/nullpo.h"
#include "common/socket.h"
#include "common/memmgr.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "map/pc.h"
#include "map/clif.h"
#include "map/party.h"

#include "plugins/HPMHooking.h"
#include "common/HPMDataCheck.h" // should always be the last file included! (if you don't make it last, it'll intentionally break compile time)

HPExport struct hplugin_info pinfo = {
    "partybuff",    // Plugin name
    SERVER_TYPE_MAP,// Which server types this plugin works with?
    "0.0",            // Plugin version
    HPM_VERSION,    // HPM Version (don't change, macro is automatically updated)
};

struct player_data {
    int buff;
};

int status_change_start_post( int retVal___, struct block_list *src, struct block_list *bl, enum sc_type type, int rate, int val1, int val2, int val3, int val4, int tick, int flag)
{
    if ( bl->type == BL_PC && retVal___ > 0 ) {
        TBL_PC *sd = BL_CAST(BL_PC, bl);
        struct party_data *p;
        if ( p = party->search(sd->status.party_id )) {
			struct player_data *ssd = getFromMSD( sd, 0 );
			int before_buff = ssd->buff;
			if ( type == SC_BLESSING )
				ssd->buff |= 0x1;
			if ( type == SC_INC_AGI )
				ssd->buff |= 0x2;
			if ( type == SC_PROTECTWEAPON || type == SC_PROTECTSHIELD || type == SC_PROTECTARMOR || type == SC_PROTECTHELM )
				if ( sd->sc.data[SC_PROTECTWEAPON] && sd->sc.data[SC_PROTECTSHIELD] && sd->sc.data[SC_PROTECTARMOR] && sd->sc.data[SC_PROTECTHELM] )
					ssd->buff |= 0x4;
			if ( type == SC_SOULLINK )
				ssd->buff |= 0x8;
			if ( type == SC_DEVOTION )
				ssd->buff |= 0x10;
			if ( before_buff != ssd->buff ) // only send the packet if update the status is newly apply, no need to resend if just renew the status
				clif->party_info( p, NULL );
		}
	}
    return retVal___;
}

int status_change_end_post( int retVal___, struct block_list *bl, enum sc_type type, int tid, const char *file, int line )
{
    if ( bl->type == BL_PC && retVal___ > 0 ) {
        TBL_PC *sd = BL_CAST(BL_PC, bl);
        struct party_data *p;
        if ( sd->state.active == 1 ) { // fix map-server crash when player logout
        if (( p = party->search(sd->status.party_id ))) {
           struct player_data *ssd = getFromMSD( sd, 0 );
			int before_buff = ssd->buff;
			if ( type == SC_BLESSING )
				ssd->buff &= ~0x1;
			if ( type == SC_INC_AGI )
				ssd->buff &= ~0x2;
			if ( type == SC_PROTECTWEAPON || type == SC_PROTECTSHIELD || type == SC_PROTECTARMOR || type == SC_PROTECTHELM )
				if ( sd->sc.data[SC_PROTECTWEAPON] && sd->sc.data[SC_PROTECTSHIELD] && sd->sc.data[SC_PROTECTARMOR] && sd->sc.data[SC_PROTECTHELM] )
					ssd->buff &= ~0x4;
			if ( type == SC_SOULLINK )
				ssd->buff &= ~0x8;
			if ( type == SC_DEVOTION )
				ssd->buff &= ~0x10;
			if ( before_buff != ssd->buff ) // only send the packet if update the status is newly apply, no need to resend if just renew the status
				clif->party_info( p, NULL );
			}
		}
	}
    return retVal___;
}

void clif_party_info_overload( struct party_data* p, struct map_session_data *sd ) {
    unsigned char buf[2+2+NAME_LENGTH+(4+NAME_LENGTH+MAP_NAME_LENGTH_EXT+1+1)*MAX_PARTY];
    struct map_session_data* party_sd = NULL;
    int i, c;
    nullpo_retv(p);
    WBUFW(buf,0) = 0xfb;
    memcpy( WBUFP(buf,4), p->party.name, NAME_LENGTH );
    for ( i = 0, c = 0; i < MAX_PARTY; i++ ) {
        struct party_member* m = &p->party.member[i];
        if(!m->account_id) continue;
        if(party_sd == NULL) party_sd = p->data[i].sd;
        WBUFL(buf,28+c*46) = m->account_id;
        if ( m->online && p->data[i].sd != NULL ) {
            struct player_data *ssd = getFromMSD( p->data[i].sd, 0 );
            char temp[NAME_LENGTH];
            safesnprintf( temp, NAME_LENGTH, "[%s%s%s%s%s]%s",
                                                            ( ssd->buff & 0x1 )? "B" : "_",
                                                            ( ssd->buff & 0x2 )? "A" : "_",
                                                            ( ssd->buff & 0x4 )? "F" : "_",
                                                            ( ssd->buff & 0x8 )? "S" : "_",
                                                            ( ssd->buff & 0x10 )? "+" : "_",
                                                            m->name );
            memcpy(WBUFP(buf,28+c*46+4), temp, NAME_LENGTH);
        } else
            memcpy(WBUFP(buf,28+c*46+4), m->name, NAME_LENGTH);
        mapindex->getmapname_ext(mapindex_id2name(m->map), (char*)WBUFP(buf,28+c*46+28));
        WBUFB(buf,28+c*46+44) = (m->leader) ? 0 : 1;
        WBUFB(buf,28+c*46+45) = (m->online) ? 0 : 1;
        c++;
    }
    WBUFW(buf,2) = 28+c*46;
    if (sd)
        clif->send(buf, WBUFW(buf,2), &sd->bl, SELF);
    else if (party_sd)
        clif->send(buf, WBUFW(buf,2), &party_sd->bl, PARTY);
    return;
}

bool pc_authok_pre( struct map_session_data **sd, int *login_id2, time_t *expiration_time, int *group_id, const struct mmo_charstatus **st, bool *changing_mapservers ) {
    struct player_data *ssd;
    CREATE( ssd, struct player_data, true );
    ssd->buff = 0;
    addToMSD( *sd, ssd, 0, true );
    return true;
}

int map_quit_post( int retVal___, struct map_session_data *sd ) {
    struct player_data *ssd = getFromMSD( sd, 0 );
    removeFromMSD( sd, 0 );
    return retVal___;
}

void clif_party_member_info_overload(struct party_data* p, struct map_session_data* sd)
{
	return;
}

HPExport void plugin_init (void) {
	clif->party_member_info = &clif_party_member_info_overload;
	addHookPre( pc, authok, pc_authok_pre );
    addHookPost( map, quit, map_quit_post );
	clif->party_info = &clif_party_info_overload;
    addHookPost( status, change_start, status_change_start_post );
    addHookPost( status, change_end_, status_change_end_post );
}