# Operator Draft - FriendSlop Remaining Work

The TestRoom proof has crossed the first useful line: FriendSlop skeletal Chad imports, has a generated test PhysicsAsset, is used only by the TestRoom override, gets hit by the wipeout arm, ragdolls passively, incapacitates, recovers, and restores control. The capture framing issue is resolved.

Remaining work:

1. Stabilize Physical Animation Component active drive.
   - Hand-tune the FriendSlop PhysicsAsset: bodies, masses, angular limits, damping, collision disables, and likely prune/merge the non-deform armature/root bodies.
   - Create a real physical-animation profile from pelvis/torso outward.
   - Re-enable `t66.TestRoom.WipeoutArmEnablePhysicalAnimation=1` only after it captures without crash/hang.

2. Tune the actual Fall Guys feel.
   - Current accepted path is passive ragdoll infrastructure, not final bouncy active-ragdoll feel.
   - Tune launch vectors, impulse scaling, restitution/friction, wall/ceiling bounce, ground drag, settle threshold, max ragdoll time, blend-out, and recovery.

3. Move incapacitation out of the TestRoom prototype and into hero gameplay state.
   - Model `Ragdolled/Incapacitated/Recovering` as real hero state.
   - Gate attacks/projectile emission, movement input, dodge/roll/jump, and recovery.
   - Make damage/enemy-swarm behavior interact with that state cleanly.

4. Generalize traps from damage-only to ragdoll triggers.
   - Create a reusable trap payload: damage, impulse direction/magnitude, vertical lift, incap duration, max ragdoll duration, cooldown/immunity, and optional physical-animation profile.
   - Convert the wipeout arm from TestRoom proof into the first real trap implementation only after the hero state is ready.

5. Decide production visual integration.
   - The current FriendSlop skeletal Chad is isolated; normal Hero 1 static visual data was intentionally untouched.
   - Production needs a deliberate import/visual-row decision for the rigged FriendSlop male, plus any material/outline/tint rules.

6. Build the physics budget/fallback policy.
   - Hero, bosses, and a few elites may use full active ragdoll.
   - Mobs should use cheap reactions, hit-stun animation, launch arcs, pooled VFX, or pop/firework deaths unless promoted into the active-ragdoll budget.
   - Add hard caps and fallback behavior before scaling this into hordes.

7. Expand proof.
   - Next proof should show wall/ceiling/floor collisions, drag/settle, recovery, and enemies swarming while the hero cannot attack.
   - Later proof should cover bosses/elites and Steam Deck performance.

No user-only decision is required before the next technical pass unless the user wants to choose between PAC-first tuning and hero-state-first integration.
