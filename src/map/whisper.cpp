// Mod By Whispering Rain

#include "whisper.hpp"

#include <iostream>
#include <map>
#include <stdlib.h>

#include "../common/nullpo.hpp"
#include "../common/random.hpp"
#include "../common/showmsg.hpp"
#include "../common/strlib.hpp"
#include "../common/utils.hpp"
#include "../common/utilities.hpp"

#include "battle.hpp"
#include "cashshop.hpp"
#include "clif.hpp"
#include "intif.hpp"
#include "log.hpp"
#include "mob.hpp"
#include "pc.hpp"
#include "status.hpp"
#include "itemdb.hpp"

using namespace rathena;

struct s_donateperks_db donateperks;

/////////////////////////////////////////////////////////////////////////////////
// Rank Title YML control

const std::string RankTitleDatabase::getDefaultLocation() {
	return std::string(db_path) + "/rank_title.yml";
}

/**
 * Reads and parses an entry from the rank_title
 * @param node: YAML node containing the entry.
 * @return count of successfully parsed rows
 */
uint64 RankTitleDatabase::parseBodyNode(const ryml::NodeRef& node) {
	
	if (!this->nodesExist(node, { "Id", "RankName" }))
		return 0;	
	
		uint16 Id;
		
	if (!this->asUInt16(node, "Id", Id))
		return 0;		
	
	std::shared_ptr<s_rank_title> RankTitle = this->find(Id);
	bool exists = RankTitle != nullptr;
	
	if (!exists) {
		if (!this->nodesExist(node, { "Id", "RankName" }))
			return 0;

		RankTitle = std::make_shared<s_rank_title>();
		RankTitle->title_id = Id;
	}	
    
	if (this->nodeExists(node, "RankName")) {
		std::string rank_name;

		if (!this->asString(node, "RankName", rank_name))
			return 0;

		if (rank_name.length() > 50) {
			this->invalidWarning(node["RankName"], "RankName \"%s\" exceeds maximum of %d characters, capping...\n", rank_name.c_str(), 50 - 1);
		}

        RankTitle->title_name = rank_name;
		
	}    
    
    if (this->nodeExists(node, "Script")) {
		std::string script;

		if (!this->asString(node, "Script", script))
			return 0;

		if (exists && RankTitle->script) {
			script_free_code(RankTitle->script);
			RankTitle->script = nullptr;
		}

		RankTitle->script = parse_script(script.c_str(), this->getCurrentFile().c_str(), this->getLineNumber(node["Script"]), SCRIPT_IGNORE_EXTERNAL_BRACKETS);
	} else {
		if (!exists) 
			RankTitle->script = nullptr;
	}   
	
	if( !exists ){
		this->put( RankTitle->title_id , RankTitle );
	}	
	return 1;
}

RankTitleDatabase rank_title_db;
/////////////////////////////////////////////////////////////////////////////////
const std::string DropEffectDatabase::getDefaultLocation() {
	return std::string(db_path) + "/drop_effect.yml";
}

/**
 * Reads and parses an entry from the drop_effect YML.
 * @param node: YAML node containing the entry.
 * @return count of successfully parsed rows
 */
uint64 DropEffectDatabase::parseBodyNode(const ryml::NodeRef& node) {
	
	if (!this->nodesExist(node, { "EquipType", "DropEffect" }))
		return 0;	
	
	std::string TypeName;

	if (!this->asString(node, "EquipType", TypeName))
		return 0;
	
	std::string type_constant = "IT_" + TypeName;
	int64 constant;
	
	if (!script_get_constant(type_constant.c_str(), &constant) || constant < IT_HEALING || constant > IT_MAX) {
		this->invalidWarning(node["EquipType"], "[DropEffect] : EquipType \"%s\" is invalid.\n",type_constant.c_str());
		return 0;
	}
	
	item_types eq_type = static_cast<item_types>(constant);
	
	std::shared_ptr<s_drop_effect> DropEffect = this->find(eq_type);
	bool exists = DropEffect != nullptr;	
	
	if (!exists) {
		if (!this->nodesExist(node, { "DropEffect" }))
			return 0;		
		
		DropEffect = std::make_shared<s_drop_effect>();
		DropEffect->EquipmentType = eq_type;
	}
	
	if (this->nodeExists(node, "DropEffect")) {
		std::string EffectName;

		if (!this->asString(node, "DropEffect", EffectName))
			return 0;

		std::string effect_constant = "DROPEFFECT_" + EffectName;
		int64 effconstant;
		
		if (!script_get_constant(effect_constant.c_str(), &effconstant) || effconstant < DROPEFFECT_CLIENT || effconstant > DROPEFFECT_MAX) {
			this->invalidWarning(node["DropEffect"], "[DropEffect] : Effect \"%s\" is invalid.\n",effect_constant.c_str());
			return 0;
		}	

		e_item_drop_effect enum_dropeffect = static_cast<e_item_drop_effect>(effconstant);
		DropEffect->DropEffect = enum_dropeffect;
	}	

	if( !exists ){
		this->put( DropEffect->EquipmentType , DropEffect );
	}	
	return 1;
}

DropEffectDatabase drop_effect_db;
/////////////////////////////////////////////////////////////////////////////////
const std::string AutotradeTaxDatabase::getDefaultLocation() {
	return std::string(db_path) + "/autotrade_tax.yml";
}

/**
 * Reads and parses an entry from the autotrade_tax YML.
 * @param node: YAML node containing the entry.
 * @return count of successfully parsed rows
 */
uint64 AutotradeTaxDatabase::parseBodyNode(const ryml::NodeRef& node) {
	
	if (!this->nodesExist(node, { "Map", "Tax" }))
		return 0;	
	
	std::string map;
	int16 map_id;

	if (!this->asString(node, "Map", map))
		return 0;
		
	util::tolower( map );	
	
	if( (map_id = map_mapname2mapid(map.c_str())) < 0){
		this->invalidWarning(node["Map"], "[AutotradeTax] : Map \"%s\" do not exists or invalid name.\n",map);
		return 0;
	}
	
	std::shared_ptr<s_autoatrade_tax> AutotradeTax = this->find(map_id);
	bool exists = AutotradeTax != nullptr;	
	
	if (!exists) {
		if (!this->nodesExist(node, { "Tax" }))
			return 0;		
		
		AutotradeTax = std::make_shared<s_autoatrade_tax>();
		AutotradeTax->map = map_id;
	}
	
	if (this->nodeExists(node, "Tax")) {
		uint64 tax;

		if (!this->asUInt64(node, "Tax", tax))
			return 0;	
			
		AutotradeTax->tax = tax;
	}	

	if( !exists ){
		this->put( AutotradeTax->map , AutotradeTax );
	}	
	return 1;
}

AutotradeTaxDatabase autotrade_tax_db;
/////////////////////////////////////////////////////////////////////////////////

/**
* Read donate_perks.txt
**/
static bool itemdb_read_donateperks(char* fields[], int columns, int current)
{
	int level,exp,drop,regenrate;

	level = atoi(fields[0]);
	exp = atoi(fields[1]);
	drop = atoi(fields[2]);
	regenrate = atoi(fields[3]);

	donateperks.level[current] = level;
	donateperks.exp[current] = exp;
	donateperks.drop[current] = drop;
	donateperks.regenrate[current] = regenrate;

	return true;
}

////////////////////////////////////////////////////////////////////////////////////////

/**
* Read all refine DB
*/
static void refinedb_read(void) {
	refine_db.load();
	random_option_db.load();
	random_option_group.load();
	enchantgrade_db.load();
	refine_randomopt_db.load();
}

/**
* Reload refine DB
*/
void refinedb_reload(void) {
	do_final_refinedb();
	refinedb_read();
}

/**
* Finalizing refine DB
*/
void do_final_refinedb(void) {
	refine_db.clear();	
	random_option_db.clear();
	random_option_group.clear();
	enchantgrade_db.clear();
	refine_randomopt_db.clear();
}

/**
* Initializing refine DB
*/
void do_init_refinedb(void) {
	refinedb_read();
}

////////////////////////////////////////////////////////////////////////////////////////

/**
* Read all Whisper custom DB
*/
static void whisper_read(void) {
	int i;
	const char* dbsubpath[] = {
		"",
		"/" DBIMPORT,
	};
	
	for(i=0; i<ARRAYLENGTH(dbsubpath); i++){
		uint8 n1 = (uint8)(strlen(db_path)+strlen(dbsubpath[i])+1);
		uint8 n2 = (uint8)(strlen(db_path)+strlen(DBPATH)+strlen(dbsubpath[i])+1);
		char* dbsubpath1 = (char*)aMalloc(n1+1);
		char* dbsubpath2 = (char*)aMalloc(n2+1);

		if(i==0) {
			safesnprintf(dbsubpath1,n1,"%s%s",db_path,dbsubpath[i]);
			safesnprintf(dbsubpath2,n2,"%s/%s%s",db_path,DBPATH,dbsubpath[i]);
		}
		else {
			safesnprintf(dbsubpath1,n1,"%s%s",db_path,dbsubpath[i]);
			safesnprintf(dbsubpath2,n1,"%s%s",db_path,dbsubpath[i]);
		}

		sv_readdb(dbsubpath1, "donate_perks.txt",		',', 4, 4, -1, &itemdb_read_donateperks, i > 0);
		aFree(dbsubpath1);
		aFree(dbsubpath2);
	}	
	
	rank_title_db.load();
	drop_effect_db.load();
	autotrade_tax_db.load();
}

/**
* Reload All Whisper
*/
void whisper_reload(void) {
	struct s_mapiterator* iter;
	struct map_session_data* sd;
	
	do_final_whisper();
	whisper_read();

	// readjust itemdb pointer cache for each player
	iter = mapit_geteachpc();
	for( sd = (struct map_session_data*)mapit_first(iter); mapit_exists(iter); sd = (struct map_session_data*)mapit_next(iter) ) {
		status_calc_pc(sd, SCO_FORCE);
		sd->state.donate_buff = false;
		donate_level_buff(sd);
	}
	mapit_free(iter);	
	
}

/**
* Finalizing Whisper
*/
void do_final_whisper(void) {
	rank_title_db.clear();
	drop_effect_db.clear();
	autotrade_tax_db.clear();
}

/**
* Initializing Whisper Custom
*/
void do_init_whisper(void) {
	whisper_read();
}
