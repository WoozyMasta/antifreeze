/*
    SPDX-License-Identifier: GPL-3.0-or-later
    Copyright (c) 2025 WoozyMasta
    Source: https://github.com/woozymasta/antifreeze
*/

#ifdef SERVER
/**
    \brief Runtime configuration for Antifreeze (server-side AI load shedding).
*/
class Antifreeze_Config
{
	// * Const
	protected static const string CONFIG_FILE = "$profile:antifreeze.json";
	protected static const int CONFIG_VERSION = 2;

	// * Singleton
	ref protected static Antifreeze_Config s_Instance;
	protected static bool s_Loaded;

	// * Feature toggles
	bool enableAntifreeze = true; //!< Master switch for the whole module
	bool enableFreezeUnreachableByHeight = true; //!< Freeze when target is near but vertically unreachable
	bool enableChaseTokenBudget = true; //!< Rotate far CHASE actors via short-lived tokens
	bool enableLocalDensityCulling = true; //!< Freeze extra actors if too many neighbors nearby
	bool enableRandomJitter = true; //!< Add jitter to probe/TTL timings to avoid spikes
	bool enableCheapAttackProbe = false; //!< Use single CanAttackToPosition probe as cheap reachability check
	bool enableFrozenAgingTicks = true; //!< While frozen, forward rare super() ticks to age native timers
	bool enableDecayMindState = true; //!< Allow force decay mind states by timers
	bool enableRandomPerZombieOptOut = false; //!< Some zombies run pure vanilla logic
	bool enableForceCleanupBodies = true; //!< Allow delete dead zombie bodies near player
	bool enableHotConfigReload = false; //!< Allow to use afzr command for reload config

	// * Core timing & jitter
	float unreachablePersistSeconds = 0.6; //!< Condition persistence before freezing AI
	float frozenProbeBaseIntervalSeconds = 0.5; //!< Base interval between unfreeze probes
	float frozenProbeJitterSeconds = 0.35; //!< Additional random jitter for unfreeze probes
	float throttleBaseIntervalSeconds = 0.45; //!< Base sleep window for far CHASE throttling
	float throttleJitterSeconds = 0.35; //!< Jitter added to throttleBaseIntervalSeconds

	// * Frozen aging
	float frozenAgingTickInterval = 0.75; //!< Seconds between aging ticks while frozen
	float frozenAgingTickDtCap = 0.06; //!< Max dt forwarded into a single aging tick

	// * Mind state decay
	float decayMindStateFightCooldown = 30.0; //!< Seconds until the Fight mind state drop to Chase
	float decayMindStateChaseCooldown = 120.0; //!< Seconds until the Chase mind state drop to Alerted

	// * Spatial thresholds
	float unreachableHeightDeltaMeters = 1.25; //!< If target is this much higher and near -> treat as unreachable
	float nearRadiusMeters = 6.0; //!< Near radius used by height-unreachable check
	[NonSerialized()]
	float nearRadiusSquared = nearRadiusMeters * nearRadiusMeters; //!< Cached square of nearRadiusMeters
	float activeRingRadiusMeters = 4.5; //!< Inner ring where AI stays active
	float densityWindowRadiusMeters = 2.0; //!< Neighborhood radius for density culling
	int densityMaxNeighbors = 6; //!< Max neighbors before culling triggers

	// * Chase token budget
	float chaseTokenTTLSeconds = 1.2; //!< Token lifetime for far CHASE actor
	float chaseTokenTTLJitterSeconds = 0.5; //!< Jitter added to token TTL
	float chaseKeepBaseProbability = 0.25; //!< Base probability factor to keep far CHASE active
	float chaseKeepMinProbability = 0.04; //!< Lower clamp for the probability
	float chaseKeepMaxProbability = 0.55; //!< Upper clamp for the probability

	// * Stimulus windows
	float wakeGraceSeconds = 3.0; //!< No-freeze window after a wake (hit/contact)
	float wakeCooldownSeconds = 0.30; //!< Min time between consecutive wake triggers

	// * Random opt-out
	float randomOptOutRatio = 0.0; //!< 0.0..0.9: fraction of zombies running vanilla only

	// * Bodies cleanup
	int cleanupBodiesTTL = -1; //!< Seconds before force delete body, value < 0 for use CE settings
	[NonSerialized()]
	int ceMaxTTL;

	// * Version marker
	int version = CONFIG_VERSION;

	/**
	    \brief Get singleton instance (lazy-loads file once).
	*/
	static Antifreeze_Config Get()
	{
		if (!s_Instance)
			s_Instance = new Antifreeze_Config();

		if (!s_Loaded)
			s_Instance.Load();

		return s_Instance;
	}

	/**
	    \brief Drop instance and force reload on next Get().
	*/
	static void Reset()
	{
		if (s_Instance)
			s_Instance = null;

		s_Loaded = false;
		ErrorEx("AntifreeZe configuration reset", ErrorExSeverity.INFO);
	}

	/**
	    \brief Load configuration from disk, normalize values, and write back if new or upgraded.
	*/
	protected void Load()
	{
		ceMaxTTL = GetCEApi().GetCEGlobalInt("CleanupLifetimeDeadInfected");

		string error;

		// Try load existing
		if (FileExist(CONFIG_FILE)) {
			if (!JsonFileLoader<Antifreeze_Config>.LoadFile(CONFIG_FILE, this, error)) {
				ErrorEx(error);
				return;
			}

			Normalize();
			SetLoaded();

			if (version == CONFIG_VERSION)
				return;

			// bump version if config structure changed
			version = CONFIG_VERSION;
		}

		// Write initial or upgraded config
		cleanupBodiesTTL = ceMaxTTL;
		if (!JsonFileLoader<Antifreeze_Config>.SaveFile(CONFIG_FILE, this, error)) {
			ErrorEx(error);
			return;
		}

		ErrorEx("Saved new AntifreeZe config file: " + CONFIG_FILE, ErrorExSeverity.INFO);
		SetLoaded();
	}

	/**
	    \brief Mark config as loaded (for logging and control flow).
	*/
	protected void SetLoaded()
	{
		s_Loaded = true;
		ErrorEx("AntifreeZe loaded", ErrorExSeverity.INFO);
	}

	/**
	    \brief Clamp user-editable values to safe ranges and derive cached fields.
	*/
	protected void Normalize()
	{
		// Core timing & jitter
		unreachablePersistSeconds = Math.Clamp(unreachablePersistSeconds, 0.05, 5.0);
		frozenProbeBaseIntervalSeconds = Math.Clamp(frozenProbeBaseIntervalSeconds, 0.05, 10.0);
		frozenProbeJitterSeconds = Math.Clamp(frozenProbeJitterSeconds, 0.00, 5.0);
		throttleBaseIntervalSeconds = Math.Clamp(throttleBaseIntervalSeconds, 0.05, 10.0);
		throttleJitterSeconds = Math.Clamp(throttleJitterSeconds, 0.00, 10.0);

		// Frozen aging
		frozenAgingTickInterval = Math.Clamp(frozenAgingTickInterval, 0.25, 5.0);
		frozenAgingTickDtCap = Math.Clamp(frozenAgingTickDtCap, 0.01, 0.20);

		// Spatial thresholds
		unreachableHeightDeltaMeters = Math.Clamp(unreachableHeightDeltaMeters, 0.10, 12.0);
		nearRadiusMeters = Math.Clamp(nearRadiusMeters, 0.50, 20.0);
		nearRadiusSquared = nearRadiusMeters * nearRadiusMeters; // keep in sync with nearRadiusMeters
		activeRingRadiusMeters = Math.Clamp(activeRingRadiusMeters, 1.0, 16.0);
		densityWindowRadiusMeters = Math.Clamp(densityWindowRadiusMeters, 0.5, 10.0);
		densityMaxNeighbors = Math.Clamp(densityMaxNeighbors, 0, 64);

		// Chase token budget
		chaseTokenTTLSeconds = Math.Clamp(chaseTokenTTLSeconds, 0.05, 10.0);
		chaseTokenTTLJitterSeconds = Math.Clamp(chaseTokenTTLJitterSeconds, 0.00, 5.0);
		chaseKeepBaseProbability = Math.Clamp(chaseKeepBaseProbability, 0.00, 1.0);
		chaseKeepMinProbability = Math.Clamp(chaseKeepMinProbability, 0.00, 1.0);
		chaseKeepMaxProbability = Math.Clamp(chaseKeepMaxProbability, 0.00, 1.0);

		// Stimulus windows
		wakeGraceSeconds = Math.Clamp(wakeGraceSeconds, 0.10, 10.0);
		wakeCooldownSeconds = Math.Clamp(wakeCooldownSeconds, 0.10, 10.0);

		// Random opt-out
		randomOptOutRatio = Math.Clamp(randomOptOutRatio, 0.0, 0.9);

		// Bodies cleanup
		if (cleanupBodiesTTL < 0)
			cleanupBodiesTTL = ceMaxTTL;
		else
			cleanupBodiesTTL = Math.Clamp(cleanupBodiesTTL, 0, ceMaxTTL);
	}

	/**
	    \brief Randomized jitter for frozen probes.
	    \return Extra seconds to add to frozenProbeBaseIntervalSeconds.
	*/
	float GetFrozenProbeJitterSeconds()
	{
		if (enableRandomJitter)
			return Math.RandomFloat(0.0, frozenProbeJitterSeconds);

		return 0.0;
	}

	/**
	    \brief Randomized throttle interval for far CHASE management and frozen probes.
	    \return Interval in seconds.
	*/
	float GetThrottleIntervalSeconds()
	{
		float t = throttleBaseIntervalSeconds;
		if (enableRandomJitter)
			t += Math.RandomFloat(0.0, throttleJitterSeconds);

		return t;
	}

	/**
	    \brief Token lifetime with optional jitter.
	    \return Seconds to keep a far CHASE token.
	*/
	float GetChaseTokenTTLSeconds()
	{
		float ttl = chaseTokenTTLSeconds;
		if (enableRandomJitter)
			ttl += Math.RandomFloat(0.0, chaseTokenTTLJitterSeconds);

		return ttl;
	}

	/**
	    \brief Roll once per zombie to decide whether it should bypass Antifreeze.
	    \return true if the zombie should run vanilla logic for its lifetime.
	*/
	bool RollRandomOptOut()
	{
		if (enableRandomPerZombieOptOut && randomOptOutRatio > 0.0)
			return Math.RandomFloat01() < randomOptOutRatio;

		return false;
	}

	/**
	    \brief Compute far-CHASE keep probability for a given distance.
	    \param distance Current target distance in meters.
	    \return Probability in [0..1] after clamping.
	*/
	float GetChaseKeepProbability(float distance)
	{
		if (distance <= 0.0)
			return chaseKeepMaxProbability;

		float prob = chaseKeepBaseProbability * (nearRadiusMeters / distance);
		return Math.Clamp(prob, chaseKeepMinProbability, chaseKeepMaxProbability);
	}
}
#endif
