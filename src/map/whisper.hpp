// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef WHISPER_HPP
#define WHISPER_HPP

#include <map>
#include <vector>

#include "../common/database.hpp"
#include "../common/db.hpp"
#include "../common/malloc.hpp"
#include "../common/mmo.hpp"

#include "script.hpp"
#include "status.hpp"
#include "itemdb.hpp"

/// Define Max donate_perks
#define MAX_DONATE_LEVEL 10

/// Struct of Donate Perks
struct s_donateperks_db {
	int level[MAX_DONATE_LEVEL];
	int exp[MAX_DONATE_LEVEL];
	int drop[MAX_DONATE_LEVEL];
	int regenrate[MAX_DONATE_LEVEL];
};
extern struct s_donateperks_db donateperks;

struct s_rank_title {
	uint16 title_id;
    std::string title_name;
	struct script_code *script;	//Default script for everything.
    
	~s_rank_title() {
		if (this->script){
			script_free_code(this->script);
			this->script = nullptr;
		}
	}    
    
};

class RankTitleDatabase : public TypesafeYamlDatabase<uint16, s_rank_title> {
public:
	RankTitleDatabase() : TypesafeYamlDatabase( "RANK_TITLE_DB", 1 ){

	}

	const std::string getDefaultLocation();
	uint64 parseBodyNode(const ryml::NodeRef& node);
};

extern RankTitleDatabase rank_title_db;

struct s_drop_effect {
	item_types EquipmentType;
	e_item_drop_effect DropEffect;
};

class DropEffectDatabase : public TypesafeYamlDatabase<item_types, s_drop_effect> {
public:
	DropEffectDatabase() : TypesafeYamlDatabase( "DROP_EFFECT_DB", 1 ){

	}

	const std::string getDefaultLocation();
	uint64 parseBodyNode(const ryml::NodeRef& node);
};

extern DropEffectDatabase drop_effect_db;

struct s_autoatrade_tax {
	int16 map;
	uint64 tax;
};

class AutotradeTaxDatabase : public TypesafeYamlDatabase<int16, s_autoatrade_tax> {
public:
	AutotradeTaxDatabase() : TypesafeYamlDatabase( "AUTOTRADE_TAX_DB", 1 ){

	}

	const std::string getDefaultLocation();
	uint64 parseBodyNode(const ryml::NodeRef& node);
};

extern AutotradeTaxDatabase autotrade_tax_db;

void refinedb_reload(void);
void do_final_refinedb(void);
void do_init_refinedb(void);

void whisper_reload(void);
void do_final_whisper(void);
void do_init_whisper(void);

#endif /* WHISPER_HPP */
