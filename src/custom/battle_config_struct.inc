// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

/**
 * Custom battle config structure
 * Place the definition of custom battle configuration structure members in this file.
 *
 * Make sure that the name you want is not already taken by looking into battle.hpp
 * Do not forget to add initilization logic to battle_config_init.inc
 *
 * Format:
 *	 <datatype> name;
 **/
	int show_rank;
	int refine_success_annouce;
	int refine_fail_annouce;
	int feature_cellpvp;
	int cellpvp_repawn_delay;
	int cellpvp_hp_rate;
	int cellpvp_sp_rate;
	int cellpvp_autorebuff;
	int cellpvp_party_attackable;
	int cellpvp_guild_attackable;
	int cellpvp_exit_delay;
