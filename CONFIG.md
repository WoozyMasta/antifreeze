# AntifreeZe Config

## Parameters

### Switches

* `enableAntifreeze` (bool)
  * **Purpose**: master switch for the whole CPU-saving system.
  * **Effect**: 0 = mod fully off. 1 = mod on.
  * **Gameplay**: 0 = vanilla behavior. 1 = far zombies take short pauses, nearby zombies behave normally.
  * **Performance**: 1 saves CPU, especially with large groups.
  * **Recommended**: 1
  * **Range**: {0,1}

* `enableFreezeUnreachableByHeight` (bool)
  * **Purpose**: if the player is close horizontally but clearly higher (roof, rock, container), zombies stop wasting checks every server frame and take short pauses.
  * **Effect**: watches height difference and distance; if "too high", temporarily freezes that zombie’s logic.
  * **Gameplay**: fewer useless attempts at walls/ledges; zombies calm down until the situation changes.
  * **Performance**: reduces expensive path/geometry checks around vertical obstacles.
  * **Recommended**: 1
  * **Range**: {0,1}

* `enableChaseTokenBudget` (bool)
  * **Purpose**: far-away zombies move in turns instead of all at once; the closest ones remain fully active.
  * **How to think**: there is a near ring around the player. Outside that ring, zombies wake up in small batches, move a bit, then queue again.
  * **Gameplay**: fewer simultaneous long-distance rushes; approaches come in waves; behavior near the player is unchanged.
  * **Performance**: sharply lowers peak load with big hordes.
  * **Recommended**: 1
  * **Range**: {0,1}

* `enableLocalDensityCulling` (bool)
  * **Purpose**: when too many zombies are bunched up, some are temporarily frozen; less shoving and fewer jams.
  * **Gameplay**: fewer zombie "walls"; attacks become more queued.
  * **Performance**: fewer simultaneous collisions and path searches.
  * **Recommended**: 1
  * **Range**: {0,1}

* `enableRandomJitter` (bool)
  * **Purpose**: adds small random delays to timers so zombies do not wake in the same frame.
  * **Gameplay**: invisible for players.
  * **Performance**: smooths micro-stutters.
  * **Recommended**: 1
  * **Range**: {0,1}

* `enableCheapAttackProbe` (bool)
  * **Purpose**: extra cheap check "can we even attack the current player position".
  * **Gameplay**: zombies give up slightly faster in bad spots.
  * **Performance**: fewer useless combat calculations.
  * **Recommended**: 0 (enable only if you know what you are doing)
  * **Range**: {0,1}
  * **Risk**: rare false negatives on broken navmeshes.

* `enableFrozenAgingTicks` (bool)
  * **Purpose**: even while frozen, occasionally allow a short logic tick so internal timers keep aging.
  * **Gameplay**: frozen zombies "cool down" properly and do not get stuck forever.
  * **Performance**: small cost for better stability.
  * **Recommended**: 1
  * **Range**: {0,1}

* `enableRandomPerZombieOptOut` (bool)
  * **Purpose**: a portion of zombies ignore the mod and stay vanilla for their lifetime.
  * **Use case**: A/B testing or mixing behaviors if your tuning makes modded zombies too passive.
  * **Performance**: reduces potential savings; see `randomOptOutRatio`.
  * **Recommended**: 0
  * **Range**: {0,1}

* `enableForceCleanupBodies` (bool)
  * **Purpose**: Forces the removal of dead infected bodies, but corpses can still disappear before the player's eyes.
  * **Purpose**: By default, the game's central economy (`globals.xml`) removes zombie corpses after the `CleanupLifetimeDeadInfected` timer expires. However, if there are no players within the `CleanupAvoidance` radius, enabling this option will ignore `CleanupAvoidance`. You can also override `CleanupLifetimeDeadInfected` with the `cleanupBodiesTTL` option.
  * **Performance**: May slightly improve performance when players hold a single point for a very long time and prevent corpses from despawning.
  * **Recommended**: 1
  * **Range**: {0,1}

* `enableHotConfigReload` (bool)
  * **Purpose**: Enables the `afz-reload` command to hot-reload the configuration file.
  * **Note**: This command is available to any player, and by enabling this option, anyone typing `afz-reload` in chat will trigger a re-read of the config. Nothing bad will happen, but just be aware of this and use it only for the initial setup of the mod.
  * **Features**: Hot-reloading will not affect some parameters that are calculated once when zombies are created. For these parameters, you will need to create new creatures after reloading the configuration. List of affected options:
    * `enableRandomJitter` and `frozenProbeJitterSeconds` parameter
    * `enableRandomPerZombieOptOut` and `randomOptOutRatio` parameter
  * **Recommended**: 0
  * **Range**: {0,1}

### Core timing

* `unreachablePersistSeconds` (float)
  * **Purpose**: how long a zombie must continuously see the player as "too high" before pausing.
  * **Gameplay**: higher = keeps trying longer; lower = gives up sooner.
  * **Performance**: lower saves more CPU.
  * **Default**: 0.6
  * **Range**: 0.05..5.0

* `frozenProbeBaseIntervalSeconds` (float)
  * **Purpose**: base delay between "can I move again?" probes while frozen.
  * **Gameplay**: lower = wakes faster; higher = sleeps longer.
  * **Performance**: higher saves more CPU.
  * **Default**: 0.5
  * **Range**: 0.05..10.0

* `frozenProbeJitterSeconds` (float)
  * **Purpose**: random addon to the previous delay so many zombies do not wake together.
  * **Default**: 0.35
  * **Range**: 0.0..5.0

* `throttleBaseIntervalSeconds` (float)
  * **Purpose**: base pause for far zombies when it is not their turn to move.
  * **Mental model**: "step — wait — step".
  * **Gameplay**: higher = far zombies approach slower, in clearer waves.
  * **Performance**: higher saves more CPU.
  * **Default**: 0.45
  * **Range**: 0.05..10.0

* `throttleJitterSeconds` (float)
  * **Purpose**: random addon to the previous `throttleBaseIntervalSeconds` pause to avoid synchronized waves.
  * **Default**: 0.35
  * **Range**: 0.0..10.0

### Unfreeze support

* `frozenAgingTickInterval` (float)
  * **Purpose**: how often to give a short logic tick to a frozen zombie.
  * **Gameplay**: higher = longer sleeps without updates.
  * **Performance**: higher saves more CPU.
  * **Default**: 0.75
  * **Range**: 0.25..5.0

* `frozenAgingTickDtCap` (float)
  * **Purpose**: max delta-time allowed into one such `frozenAgingTickInterval` tick to keep it cheap.
  * **Default**: 0.06
  * **Range**: 0.01..0.20

### Spatial thresholds

* `unreachableHeightDeltaMeters` (float)
  * **Purpose**: how many meters above the zombie the player must be to count as "too high".
  * **Gameplay**: lower = gives up sooner on low obstacles; higher = tries longer.
  * **Performance**: lower saves more CPU.
  * **Default**: 1.25
  * **Range**: 0.10..12.0

* `nearRadiusMeters` (float)
  * **Purpose**: horizontal "near" radius where the height `unreachableHeightDeltaMeters` check is relevant.
  * **Gameplay**: higher = more cases where "player is too high" will be detected.
  * **Performance**: higher can reduce wasted checks near obstacles.
  * **Default**: 6.0
  * **Range**: 0.50..20.0

* `activeRingRadiusMeters` (float)
  * **Purpose**: near ring around the player. Inside this ring, zombies do not queue and behave normally.
  * **Gameplay**: higher = more consistent responsiveness near players; lower = more savings but possible short "bursts" right next to the player.
  * **Performance**: lower saves more CPU.
  * **Default**: 4.5
  * **Range**: 1.0..16.0

* `densityWindowRadiusMeters` (float)
  * **Purpose**: radius used for measuring "how many zombies are packed together".
  * **Gameplay**: higher = crowded groups are throttled more often.
  * **Performance**: higher can cut collision/path spikes.
  * **Default**: 2.0
  * **Range**: 0.5..10.0

* `densityMaxNeighbors` (int)
  * **Purpose**: if at least this many living zombies are within `densityWindowRadiusMeters`, some will pause.
  * **Gameplay**: lower = fewer "walls", but too low breaks group pressure.
  * **Performance**: lower saves more CPU.
  * **Default**: 6
  * **Range**: 0..64

### Pursuit

* `chaseTokenTTLSeconds` (float)
  * **Purpose**: how long a far zombie is active once it is "his turn".
  * **Mental model**: the length of its current "move window".
  * **Gameplay**: higher = longer waves and denser approaches; lower = shorter steps.
  * **Performance**: higher costs more CPU.
  * **Default**: 1.2
  * **Range**: 0.05..10.0

* `chaseTokenTTLJitterSeconds` (float)
  * **Purpose**: random addon to `chaseTokenTTLSeconds` to avoid synchronized waves.
  * **Default**: 0.5
  * **Range**: 0.0..5.0

* `chaseKeepBaseProbability` (float)
  * **Purpose**: base chance to give a "move window" to a far zombie. Final chance = `chaseKeepBaseProbability * (nearRadiusMeters / distance)`, clamped between min and max.
  * **Gameplay**: higher = more far zombies move at the same time.
  * **Performance**: lower saves more CPU.
  * **Default**: 0.25
  * **Range**: 0.0..1.0

* `chaseKeepMinProbability` (float)
  * **Purpose**: lower clamp for `chaseKeepBaseProbability` chance; ensures even very far zombies occasionally get a step.
  * **Gameplay**: higher = guaranteed rare steps from far away.
  * **Performance**: higher costs more CPU.
  * **Default**: 0.04
  * **Range**: 0.0..1.0

* `chaseKeepMaxProbability` (float)
  * **Purpose**: upper clamp for `chaseKeepBaseProbability` chance; prevents "everyone at once" just outside the near ring.
  * **Gameplay**: lower = smaller mass bursts.
  * **Performance**: lower saves more CPU.
  * **Default**: 0.55
  * **Range**: 0.0..1.0

### Forced awakening

* `wakeGraceSeconds` (float)
  * **Purpose**: after being hit or touching something (e.g., vehicle), the zombie will not be paused for a short time; fights stay responsive.
  * **Gameplay**: higher = fewer odd pauses during combat.
  * **Performance**: higher = slightly more CPU during fights.
  * **Default**: 3.0
  * **Range**: 0.10..10.0

* `wakeCooldownSeconds` (float)
  * **Purpose**: anti-spam for wake triggers; delays back-to-back wakeups.
  * **Gameplay**: too high = slightly slower re-engagement when pinged very often.
  * **Performance**: higher reduces event spam.
  * **Default**: 0.30
  * **Range**: 0.10..10.0

### Misc

* `randomOptOutRatio` (float)
  * **Purpose**: share of zombies that ignore the mod (vanilla) for their whole life.
  * **Gameplay**: mixed behavior for comparison.
  * **Performance**: the higher this is, the less CPU you save.
  * **Default**: 0.0
  * **Range**: 0.0..0.9

* `cleanupBodiesTTL` (int)
  * **Purpose**: Overrides the global `CleanupLifetimeDeadInfected` option.
  * **Default**: equal to `CleanupLifetimeDeadInfected`
  * **Range**: 5..`CleanupLifetimeDeadInfected`

* `version` (int, do not edit)
  * **Purpose**: config schema version; managed by the mod.

## How it works in 2 lines

* Zombies inside the near ring are always fully active.
* Zombies outside the near ring move in short turns, in waves. If the player is too high above, they pause and stop burning CPU.

## Tuning recipes by symptom

### FPS drops when a player drags a big horde from far away

* Raise `throttleBaseIntervalSeconds` to 0.6–0.9
* Lower `chaseKeepBaseProbability` to 0.18–0.22
* Lower `chaseKeepMaxProbability` to 0.45–0.50
* Keep `activeRingRadiusMeters` at 4.5–5.0

### Zombies approach too slowly

* Raise `chaseKeepBaseProbability` to 0.30–0.35
* Raise `chaseTokenTTLSeconds` to 1.5–1.8
* Lower `throttleBaseIntervalSeconds` to 0.30–0.40
* Slightly raise `activeRingRadiusMeters` to 5.0–5.5

### Zombie walls at doors/chokepoints

* Lower `densityMaxNeighbors` to 4–5
* Raise `densityWindowRadiusMeters` to 2.5–3.0

### Players camp at height (roofs/rocks/dumpsters), zombies pile underneath

* Raise `unreachablePersistSeconds` to 0.8–1.0 (pause faster)
* Lower `unreachableHeightDeltaMeters` to 1.0–1.1 (easier to mark "too high")

### Micro-stutters when many wake together

* Ensure `enableRandomJitter` = 1
* Raise `frozenProbeJitterSeconds` and/or `throttleJitterSeconds` by +0.1..0.2

## Parameter relationships

* Higher `chaseTokenTTLSeconds` pairs well with a slightly higher `throttleBaseIntervalSeconds` to avoid overlapping waves.
* Higher `activeRingRadiusMeters` allows reducing `chaseKeepBaseProbability` because nearby zombies are unrestricted anyway.
* `nearRadiusMeters` is part of the keep-probability formula: larger radius increases the chance at the same distance.
* Larger `densityWindowRadiusMeters` with the same `densityMaxNeighbors` reduces jams and "walls".
* `unreachableHeightDeltaMeters` together with `unreachablePersistSeconds` controls how quickly zombies give up under vertical obstacles.

## Starter profiles

### Balanced (default)

* `throttleBaseIntervalSeconds`=0.45
* `throttleJitterSeconds`=0.35
* `chaseKeepBaseProbability`=0.25
* `chaseTokenTTLSeconds`=1.2
* `activeRingRadiusMeters`=4.5
* `densityMaxNeighbors`=6

### More savings (low-end server)

* `throttleBaseIntervalSeconds`=0.8
* `throttleJitterSeconds`=0.5
* `chaseKeepBaseProbability`=0.18
* `chaseKeepMaxProbability`=0.48
* `chaseTokenTTLSeconds`=1.0
* `activeRingRadiusMeters`=4.5
* `densityMaxNeighbors`=4

### More pressure (fewer pauses)

* `throttleBaseIntervalSeconds`=0.3
* `throttleJitterSeconds`=0.2
* `chaseKeepBaseProbability`=0.35
* `chaseTokenTTLSeconds`=1.8
* `activeRingRadiusMeters`=5.5
* `densityMaxNeighbors`=7

## File format and maintenance

* Location: `$profile/antifreeze.json`
* Booleans: use 0/1
* Distances and times: use numbers with a dot.
* Do not edit: `version` (the mod will overwrite them).
* Hot-reload without restart: send "afzr" in the server chat (exactly like that, no slash). The mod reloads and logs.
* Example [antifreeze.json](antifreeze.json) (balanced)
