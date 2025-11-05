/*
    SPDX-License-Identifier: GPL-3.0-or-later
    Copyright (c) 2025 WoozyMasta
    Source: https://github.com/woozymasta/antifreeze
*/

#ifdef SERVER
/**
    \brief AI antifreeze for ZombieBase: throttle unreachable and far CHASE actors.

    Behavior:
    - Freeze native AI (SetKeepInIdle(true)) when target looks "near but unreachable by height".
    - Throttle far CHASE actors via short-lived tokens; keep actors in the active ring responsive.
    - Cull extra actors in dense local clusters to reduce pathing/collision contention.
    - While frozen, optionally forward rare super() ticks to age native timers/memory.
    - All thresholds/timings are driven by Antifreeze_Config.
*/
modded class ZombieBase
{
	static int s_Antifreeze_SoundCount;
	static float s_Antifreeze_SoundFrame;

	override void OnSoundVoiceEvent(int event_id, string event_user_string)
	{
		float frame = GetGame().GetTickTime();
		if (frame != s_Antifreeze_SoundFrame) {
			s_Antifreeze_SoundFrame = frame;
			s_Antifreeze_SoundCount = 0;
		}
		if (s_Antifreeze_SoundCount >= 32) // max total per frame
			return;

		s_Antifreeze_SoundCount++;
		super.OnSoundVoiceEvent(event_id, event_user_string);
	}

	// --------- Runtime state ---------
	protected float m_Antifreeze_AgeAccum; //!< Accumulator for frozen aging ticks
	protected float m_Antifreeze_UnreachableTime; //!< Persistent "unreachable by height" accumulation
	protected float m_Antifreeze_UnreachableCD; //!< Cooldown until next unfreeze probe
	protected bool m_Antifreeze_AIFrozen; //!< Whether native AI is currently frozen
	protected float m_Antifreeze_RecheckJitter; //!< Per-entity jitter to desync probes
	protected bool m_Antifreeze_ChaseHasToken; //!< Far CHASE token state
	protected float m_Antifreeze_ChaseTokenTTL; //!< Remaining token TTL (seconds)
	protected float m_Antifreeze_StimulusGrace; //!< No-freeze window after hit/contact
	protected float m_Antifreeze_HitWakeCD; //!< Min time between wake triggers
	protected bool m_Antifreeze_OptOut; //!< This zombie runs vanilla if true

	/**
	    \brief Constructor: initialize antifreeze state and seed per-entity jitter.
	*/
	void ZombieBase()
	{
		m_Antifreeze_AgeAccum = 0.0;
		m_Antifreeze_UnreachableTime = 0.0;
		m_Antifreeze_UnreachableCD = 0.0;
		m_Antifreeze_AIFrozen = false;
		m_Antifreeze_ChaseHasToken = false;
		m_Antifreeze_ChaseTokenTTL = 0.0;
		m_Antifreeze_StimulusGrace = 0.0;
		m_Antifreeze_HitWakeCD = 0.0;
		m_Antifreeze_RecheckJitter = Antifreeze_Config.Get().GetFrozenProbeJitterSeconds();
		m_Antifreeze_OptOut = Antifreeze_Config.Get().RollRandomOptOut();
	}

	/**
	    \brief Main command handler wrapper with antifreeze branches.
	    \param pDt Accumulated delta time from engine.
	    \param pCurrentCommandID Current animation/AI command (MOVE/Vault/Attack/...).
	    \param pCurrentCommandFinished Whether previous command has just finished.
	*/
	override void CommandHandler(float pDt, int pCurrentCommandID, bool pCurrentCommandFinished)
	{
		// Global bypass: disabled or critical native paths must not be delayed
		if (!Antifreeze_Config.Get().enableAntifreeze || IsDamageDestroyed() || m_FinisherInProgress) {
			super.CommandHandler(pDt, pCurrentCommandID, pCurrentCommandFinished);
			return;
		}

		// Vanilla roll: skip all antifreeze
		if (m_Antifreeze_OptOut) {
			super.CommandHandler(pDt, pCurrentCommandID, pCurrentCommandFinished);
			return;
		}

		// Wake spam guard tick
		if (m_Antifreeze_HitWakeCD > 0.0)
			m_Antifreeze_HitWakeCD -= pDt;

		// Stimulus grace: keep AI responsive for a short window
		if (m_Antifreeze_StimulusGrace > 0.0) {
			m_Antifreeze_StimulusGrace -= pDt;
			super.CommandHandler(pDt, pCurrentCommandID, pCurrentCommandFinished);
			return;
		}

		// Frozen branch: run rare aging ticks and probe on cooldown
		if (m_Antifreeze_AIFrozen) {
			m_Antifreeze_UnreachableCD -= pDt;

			if (Antifreeze_Config.Get().enableFrozenAgingTicks) {
				m_Antifreeze_AgeAccum += pDt;
				if (m_Antifreeze_AgeAccum >= Antifreeze_Config.Get().frozenAgingTickInterval) {
					float ageDt = m_Antifreeze_AgeAccum;
					if (ageDt > Antifreeze_Config.Get().frozenAgingTickDtCap)
						ageDt = Antifreeze_Config.Get().frozenAgingTickDtCap;

					// Temporarily allow native logic to age its timers/memory
					GetAIAgent().SetKeepInIdle(false);
					super.CommandHandler(ageDt, pCurrentCommandID, pCurrentCommandFinished);
					GetAIAgent().SetKeepInIdle(true);

					m_Antifreeze_AgeAccum = 0.0;
				}
			}

			if (m_Antifreeze_UnreachableCD <= 0.0) {
				if (Antifreeze_ShouldUnfreezeNow()) {
					m_Antifreeze_AIFrozen = false;
					GetAIAgent().SetKeepInIdle(false); // resume native AI
					// fall-through to run super this frame
				} else {
					m_Antifreeze_UnreachableCD = Antifreeze_Config.Get().GetThrottleIntervalSeconds();
					return; // keep frozen
				}
			} else {
				return; // wait until next probe
			}
		}

		// Non-MOVE commands: pass-thru
		if (pCurrentCommandID != DayZInfectedConstants.COMMANDID_MOVE) {
			super.CommandHandler(pDt, pCurrentCommandID, pCurrentCommandFinished);
			return;
		}

		DayZInfectedInputController ic = GetInputController();
		if (!ic) {
			super.CommandHandler(pDt, pCurrentCommandID, pCurrentCommandFinished);
			return;
		}

		// Unreachable-by-height gate
		if (Antifreeze_Config.Get().enableFreezeUnreachableByHeight) {
			if (!Antifreeze_IsTargetReachableCheap(ic)) {
				m_Antifreeze_UnreachableTime += pDt;

				if (m_Antifreeze_UnreachableTime >= Antifreeze_Config.Get().unreachablePersistSeconds) {
					Antifreeze_FreezeFor(Antifreeze_Config.Get().frozenProbeBaseIntervalSeconds + m_Antifreeze_RecheckJitter);
					return; // drop futile geometry/raycast spam
				}
			} else
				m_Antifreeze_UnreachableTime = 0.0;
		}

		// CHASE budget (far actors)
		if (Antifreeze_Config.Get().enableChaseTokenBudget && ic.GetMindState() == DayZInfectedConstants.MINDSTATE_CHASE) {
			EntityAI tgt = ic.GetTargetEntity();
			if (tgt) {
				float dist = vector.Distance(tgt.GetPosition(), GetPosition());

				// Near active ring: keep active to allow smooth transitions into FIGHT
				if (dist > Antifreeze_Config.Get().activeRingRadiusMeters) {

					// Density culling to mitigate congestion walls
					if (Antifreeze_Config.Get().enableLocalDensityCulling && Antifreeze_IsLocallyCrowded()) {
						Antifreeze_FreezeFor(Antifreeze_Config.Get().GetThrottleIntervalSeconds());
						return;
					}

					// Token rotation
					if (m_Antifreeze_ChaseHasToken) {
						m_Antifreeze_ChaseTokenTTL -= pDt;
						if (m_Antifreeze_ChaseTokenTTL <= 0.0) {
							m_Antifreeze_ChaseHasToken = false;
							Antifreeze_FreezeFor(Antifreeze_Config.Get().GetThrottleIntervalSeconds());
							return;
						}

					} else {
						if (Math.RandomFloat01() < Antifreeze_Config.Get().GetChaseKeepProbability(dist)) {
							m_Antifreeze_ChaseHasToken = true;
							m_Antifreeze_ChaseTokenTTL = Antifreeze_Config.Get().GetChaseTokenTTLSeconds();
						} else {
							Antifreeze_FreezeFor(Antifreeze_Config.Get().GetThrottleIntervalSeconds());
							return;
						}
					}

				} else {
					// Inside the active ring: clear token to avoid monopolies
					m_Antifreeze_ChaseHasToken = false;
					m_Antifreeze_ChaseTokenTTL = 0.0;
				}
			}

		} else {
			// Not in CHASE: ensure token cleared
			m_Antifreeze_ChaseHasToken = false;
			m_Antifreeze_ChaseTokenTTL = 0.0;
		}

		// Fallthrough to native logic
		super.CommandHandler(pDt, pCurrentCommandID, pCurrentCommandFinished);
	}

	/**
	    \brief Wake AI when hit to ensure responsiveness.
	*/
	override void EEHitBy(TotalDamageResult damageResult, int damageType, EntityAI source, int component, string dmgZone, string ammo, vector modelPos, float speedCoef)
	{
		super.EEHitBy(damageResult, damageType, source, component, dmgZone, ammo, modelPos, speedCoef);

		if (m_Antifreeze_HitWakeCD > 0.0)
			return;

		Antifreeze_WakeFor(Antifreeze_Config.Get().wakeGraceSeconds);
		m_Antifreeze_HitWakeCD = Antifreeze_Config.Get().wakeCooldownSeconds;
	}

	/**
	    \brief Wake AI on contact (e.g., bump/push) to prevent stale freezes.
	*/
	override protected void EOnContact(IEntity other, Contact extra)
	{
		super.EOnContact(other, extra);

		if (m_Antifreeze_HitWakeCD > 0.0)
			return;

		Antifreeze_WakeFor(Antifreeze_Config.Get().wakeGraceSeconds);
		m_Antifreeze_HitWakeCD = Antifreeze_Config.Get().wakeCooldownSeconds;
	}

	/**
	    \brief Let's make sure that the killed zombie definitely shouldn't be processed.
	*/
	override void EEKilled(Object killer)
	{
		super.EEKilled(killer);

		m_Antifreeze_AIFrozen = true;

		// Hard-freeze AI agent to avoid pathing ticks on corpse
		if (GetAIAgent())
			GetAIAgent().SetKeepInIdle(true);

		if (Antifreeze_Config.Get().enableForceCleanupBodies)
			GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(this.Delete, Antifreeze_Config.Get().cleanupBodiesTTL * 1000, false, true);

		// I'm not sure if this helps, but I'll try.
		GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(this.DisableSimulation, 1500, false, true);
	}

	// --------------------- helpers ---------------------

	/**
	    \brief Freeze native AI for a given window.
	    \param seconds Duration before next unfreeze probe.
	*/
	protected void Antifreeze_FreezeFor(float seconds)
	{
		m_Antifreeze_AIFrozen = true;
		m_Antifreeze_UnreachableCD = seconds;
		GetAIAgent().SetKeepInIdle(true);
	}

	/**
	    \brief Wake native AI and extend stimulus grace if needed.
	    \param seconds No-freeze window to keep after waking.
	*/
	protected void Antifreeze_WakeFor(float seconds)
	{
		// Clear frozen state
		if (m_Antifreeze_AIFrozen) {
			m_Antifreeze_AIFrozen = false;
			GetAIAgent().SetKeepInIdle(false);
		}

		// Reset freeze accumulators
		m_Antifreeze_UnreachableTime = 0.0;
		m_Antifreeze_UnreachableCD = 0.0;

		// Extend grace
		if (seconds > m_Antifreeze_StimulusGrace)
			m_Antifreeze_StimulusGrace = seconds;
	}

	/**
	    \brief Decide whether frozen AI should be reactivated now.
	    \return true if unfreeze should occur this frame.
	*/
	protected bool Antifreeze_ShouldUnfreezeNow()
	{
		DayZInfectedInputController ic = GetInputController();
		if (!ic)
			return true;

		EntityAI target = ic.GetTargetEntity();
		if (!target)
			return true;

		float dist = vector.Distance(target.GetPosition(), GetPosition());

		// Inside active ring: always unfreeze to stay responsive
		if (dist <= Antifreeze_Config.Get().activeRingRadiusMeters)
			return true;

		// Outside: only unfreeze if we can actually attack target position
		return CanAttackToPosition(target.GetPosition());
	}

	/**
	    \brief Cheap reachability heuristic to detect "near-but-above" targets.
	    \param iic Infected input controller.
	    \return true if target seems reachable enough to keep AI active.
	*/
	protected bool Antifreeze_IsTargetReachableCheap(DayZInfectedInputController iic)
	{
		EntityAI target = iic.GetTargetEntity();
		if (!target)
			return true;

		PlayerBase player = PlayerBase.Cast(target);
		if (player && player.IsDamageDestroyed())
			return true;

		vector my = GetPosition();
		vector tp = target.GetPosition();

		float dy = tp[1] - my[1];
		float dx = tp[0] - my[0];
		float dz = tp[2] - my[2];
		float r2 = dx * dx + dz * dz;

		// Close and target is above by threshold → treat as unreachable
		if (r2 <= Antifreeze_Config.Get().nearRadiusSquared && dy > Antifreeze_Config.Get().unreachableHeightDeltaMeters)
			return false;

		// Optional single native probe; cheaper than full fight logic storm
		if (Antifreeze_Config.Get().enableCheapAttackProbe && !CanAttackToPosition(tp))
			return false;

		return true;
	}

	/**
	    \brief Estimate local density using a small box scene query.
	    \return true if number of neighbors >= configured max.
	*/
	protected bool Antifreeze_IsLocallyCrowded()
	{
		float r = Antifreeze_Config.Get().densityWindowRadiusMeters;
		vector pos = GetPosition();
		vector mn = Vector(pos[0] - r, pos[1] - 1.0, pos[2] - r);
		vector mx = Vector(pos[0] + r, pos[1] + 1.0, pos[2] + r);

		array<EntityAI> entities = new array<EntityAI>();
		DayZPlayerUtils.SceneGetEntitiesInBox(mn, mx, entities, QueryFlags.DYNAMIC);

		int cnt = 0;
		foreach (EntityAI eai : entities) {
			ZombieBase zombie;
			if (Class.CastTo(zombie, eai) && zombie != this && !zombie.IsDamageDestroyed())
				cnt++;

			if (cnt >= Antifreeze_Config.Get().densityMaxNeighbors)
				return true;
		}
		return false;
	}
}
#endif
