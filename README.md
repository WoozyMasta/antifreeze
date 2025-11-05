# AntifreeZe

<!-- markdownlint-disable-next-line MD033 MD041 -->
<img src="logo.png" alt="MetricZ" align="right" width="380">

Server-side mod for DayZ.
It cuts CPU load when zombies have little to no chance to reach a player who’s behind obstacles or standing higher.
The project was built as an experiment to understand server FPS drops in scenes where zombies keep slamming into doors/walls and burn AI ticks endlessly.
Background issue: [T195642](https://feedback.bistudio.com/T195642)

In practice, if a server has ~60 players online and ~20 of them are engaging 5–10 zombies each (using elevation or window-peeking), you can hit 100–200 zombies simultaneously running expensive "get to the player" loops.
This mod tries to smooth that by letting distant or obviously-stuck cases "pause their thinking" in short bursts instead of all crunching every frame.
It’s experimental; there is no guarantee it will help every server. If you’re aiming for real hordes that pursue players, it should still be a net positive.

## How it works

Zombies close to a player behave like vanilla.
Farther away, they take short turns in waves to avoid synchronizing heavy AI work.
If a player is nearby horizontally but clearly higher, the AI pauses for short intervals and periodically rechecks whether the target has become reachable.
In tight crowds, some zombies pause to reduce pile-ups and pathfinding spikes.

The logic is server-only. Combat values, damage, loot, and spawn are untouched. Configuration can be hot-reloaded via a chat command.
Integration points are minimal: `ZombieBase` is extended and `MissionServer::OnEvent` handles the reload command.

> [!IMPORTANT]  
> This is not a silver bullet. Your server’s performance depends on many factors, especially if you stack multiple heavy mods.  
> In some scenes zombies may look "dumber" than vanilla; the point here is to trade tiny behavior changes for more stable server ticks.

## Install and configure

Place the mod in the server folder (e.g., `@antifreeze`) and start the server with `-serverMod=@antifreeze`.
On first run, the config is created at `$profile/antifreeze.json`.

Full configuration docs for `antifreeze.json` [live here](CONFIG.md)

For hot config reload, set `enableHotConfigReload` to `1` in `antifreeze.json`, then send `afz-reload` in the server chat (no slash).

## Compatibility

The mod overrides behavior in `class ZombieBase` (primarily `CommandHandler`).
If other mods also patch those methods, mind load order so AntifreeZe runs with the priority you intend.

> [!NOTE]  
> This mod does not change navmesh or map geometry and does not "fix" vanilla bugs. It reduces their CPU impact. Crowd pressure can still happen; the difference is the server won’t burn as many cycles doing redundant work.

## Known Issues

* Zombies can chase the player almost forever.

## Analysis

To analyze the mod's performance, it's best to use metrics collection mechanisms.
For example, a mod like [MetricZ](https://github.com/WoozyMasta/metricz) might be suitable in the Prometheus format.
This way, you can accurately see the correlation between FPS, active players, the number of zombies, and their state of mind on graphs.
You can, of course, use the `-doLogs` server parameter and analyze the log and current FPS records, but this is less informative.

## 👉 [Support Me](https://gist.github.com/WoozyMasta/7b0cabb538236b7307002c1fbc2d94ea)
