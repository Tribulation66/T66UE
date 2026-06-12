# Claude Operator Prompt: Combat VFX Impact-Context Process Doc Recommendation

You are Claude acting as read-only Operator for `C:\UE\T66`.

Goal: recommend concrete process-documentation changes so every future weapon and idol VFX implementation consistently publishes, consumes, and verifies combat impact contexts and damage-source behavior.

User question: "Ok what can we do in our process docs, so that every future, weapon/idol has this correct behavior?"

Read these live files before recommending:

- `AGENTS.md`
- `Gameplay/GAMEPLAY_AGENTS.md`
- `Gameplay/Combat/VFX_PROCESS_INDEX.md`
- `Gameplay/Combat/CombatVFXAuthoringProcedure.md`
- `Gameplay/Combat/CombatVFXDefinitionOfDone.md`
- `Gameplay/Combat/CombatVFXVisualDamageAlignmentContract.md`
- `Gameplay/Combat/CombatVFXIdolOverlayArchitecture.md`
- `Gameplay/Combat/CombatVFXGeneratedAssetPolicy.md`

Known recent diagnostic:

- Water proof added a runtime log line shaped like:
  `CombatIdolImpactDiagnostic SourceID=Idol_Water WaterIdolContextParity=PASS WeaponImpactContexts=1 EligibleWeaponImpactContexts=1 ImpactPresentationIdolSlots=1 ExpectedWaterIdolImpactContexts=1 WaterIdolImpactContexts=1 WaterSkippedNoWeaponContext=0 WaterSkippedInvalidImpactPoint=0 WaterLegacyFallbacks=0`
- The proof also required weapon impact context, idol impact context, Water placeholder spawn, Water damage source, and Earth-neutral absence of Water diagnostics.

Please propose the smallest durable docs/process update set. Focus on:

1. Which existing docs should receive new gates.
2. Exact concepts/rules to add.
3. What every future weapon packet must prove.
4. What every future idol packet must prove.
5. What proof wrappers/validators should require.
6. What wording prevents future agents from treating video-only proof as enough.

Do not edit files. Do not run shell commands. Produce a concise recommendation Codex can validate and explain to the user.
