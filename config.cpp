/*
    SPDX-License-Identifier: GPL-3.0-or-later
    Copyright (c) 2025 WoozyMasta
    Source: https://github.com/woozymasta/antifreeze
*/

class CfgMods
{
	class Antifreeze
	{
		type = "mod";
		dir = "antifreeze";
		name = "AntifreeZe";
		version = "0.1.0";
		credits = "WoozyMasta";
		author = "WoozyMasta";
		authorID = "76561198037610867";
		hideName = 1;
		hidePicture = 1;
		defines[] = {"ANTIFREEZE"};
		dependencies[] = {"game", "world", "mission"};

		class defs
		{
			class gameScriptModule
			{
				files[] = {"antifreeze/scripts/3_game"};
			};

			class worldScriptModule
			{
				files[] = { "antifreeze/scripts/4_world" };
			};

			class missionScriptModule
			{
				files[] = { "antifreeze/scripts/5_mission" };
			};
		};
	};
};

class CfgPatches
{
	class Antifreeze
	{
		requiredAddons[] = {
			"DZ_Data",
			"DZ_Scripts",
		};
	};
};
