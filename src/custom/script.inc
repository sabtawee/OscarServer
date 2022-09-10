// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

/**
 * Custom Script Commands
 * Place the body of custom script commands in this file.
 * Format:
 *	BUILDIN_FUNC(command_name)
 *	{
 *		<code>;
 *	}
 **/

//BUILDIN_FUNC(example)
//{
//	ShowInfo("buildin_example: It works!\n");
//	script_pushint(st,1);
//	return 0;
//}
BUILDIN_FUNC(ranktitleupdate)
{
	TBL_PC* sd;

	if( !script_rid2sd(sd) )
		return SCRIPT_CMD_SUCCESS;

    int title_id = script_getnum(st, 2);
	pc_setregistry(sd, reference_uid(add_str("title_id"), 0), title_id);
	sd->status.title_id = title_id;
	status_calc_pc(sd, SCO_FORCE);
	
    clif_name_area(&sd->bl);
    return SCRIPT_CMD_SUCCESS;
}