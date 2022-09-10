// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

/**
 * Custom battle config initializations
 * Place the definition of custom battle configuration initializations in this file.
 *
 * Do not forget to add a comma at the end of the line, if you want to add another config
 *
 * Format:
 *	 { "name", &battle_config.<variable name>, <default value>, <minimum value>, <maximum value> },
 **/
	{ "show_rank",					&battle_config.show_rank,					0,		0,		 1,				},
	{ "refine_success_annouce",		&battle_config.refine_success_annouce,		0,		0,		20,				},
	{ "refine_fail_annouce",		&battle_config.refine_fail_annouce,			0,		0,		20,				},
	
	// Addon Cell PVP [Napster]
	{ "feature.cellpvp",			&battle_config.feature_cellpvp,				1,      0,      1,              },
	{ "cellpvp_hp_rate",			&battle_config.cellpvp_hp_rate,			  100,		0,      100,        	},
	{ "cellpvp_sp_rate",			&battle_config.cellpvp_sp_rate,			  100,		0,		100,			},
	{ "cellpvp_autorebuff",			&battle_config.cellpvp_autorebuff,			1,		0,		1,				},
	{ "cellpvp_party_attackable",	&battle_config.cellpvp_party_attackable,	1,		0,      1,              },
	{ "cellpvp_guild_attackable",	&battle_config.cellpvp_guild_attackable,	1,		0,      1,              },
	{ "cellpvp_repawn_delay",		&battle_config.cellpvp_repawn_delay,	 5000,		0,      INT_MAX,		},
	{ "cellpvp_exit_delay",			&battle_config.cellpvp_exit_delay,		 5000,		0,      INT_MAX,		},

 