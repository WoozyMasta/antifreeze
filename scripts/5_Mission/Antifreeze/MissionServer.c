/*
    SPDX-License-Identifier: GPL-3.0-or-later
    Copyright (c) 2025 WoozyMasta
    Source: https://github.com/woozymasta/antifreeze
*/

#ifdef SERVER
modded class MissionServer : MissionBase
{
	/**
	    \brief Handle chat command "afz-reload" to reset configuration (server-side).
	*/
	override void OnEvent(EventType eventTypeId, Param params)
	{
		if (eventTypeId == ChatMessageEventTypeID && Antifreeze_Config.Get().enableHotConfigReload) {
			ChatMessageEventParams chatParams = ChatMessageEventParams.Cast(params);
			if (chatParams) {
				string cmd = string.ToString(chatParams.param3, false, false, false);
				if (cmd == "afz-reload")
					Antifreeze_Config.Reset();
			}
		}

		super.OnEvent(eventTypeId, params);
	}
}
#endif
