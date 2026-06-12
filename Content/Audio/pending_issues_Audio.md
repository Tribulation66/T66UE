# Pending Issues - Audio

## Resolved: Staged Gameplay Smoke References Missing Audio Assets

- Severity tag: [Resolved - Minor]
- What's wrong: The Easy mob VAT staged smoke logged missing packages for `/Game/Audio/SC_Music`, `/Game/Audio/SC_SFX`, and several candidate theme paths such as `/Game/Audio/Music/MainTheme` and `/Game/Audio/Theme`.
- Why it's out of scope now: The VAT pass only needed to prove enemy visual animation, material cookability, and standalone boot behavior. It did not change the audio content library or player settings audio routing.
- What fixing it would entail: Decide the canonical music/SFX SoundClass and theme asset paths, create or redirect the missing assets, update the player-settings audio references, cook/stage, and smoke a standalone run with audio enabled.
- Update 2026-06-09 (audio build-out): verified functionally harmless for volume control — `UT66AudioSubsystem::ResolveVolumeMultiplier` applies Master x bus volume per play from `UT66PlayerSettingsSubsystem`, and `UT66MusicSubsystem` applies volume via component multipliers, so the SoundClass route is redundant. The missing-package warnings remain log noise; creating the two SoundClass assets in-editor would silence them.
- Resolution: Resolved 2026-06-09: created `/Game/Audio/SC_Music` + `SC_SFX` as plain default SoundClasses via new `Scripts/SetupAudioSoundClasses.py` (idempotent, documented in-script; assets verified on disk). MainTheme already exists at `/Game/Audio/OSTS/MainTheme` (first candidate path) since the audio build-out. Cook-warning absence check rides the next staged build.

## Resolved: Legacy Alice rabbit hero OST is unreachable [Minor]

- What's wrong: `Content/Audio/OSTS/Heroes/Hero_AliceInWonderlandRabbit/OST.ogg` (+uasset) exists, but `UT66MusicSubsystem` resolves hero themes by `Heroes.csv MapTheme` key (Hero_1..Hero_12 only), so no hero ever resolves that folder.
- Why it's out of scope now: The 2026-06-09 audio build-out added placeholder coverage; re-keying legacy content is data archaeology.
- What fixing it would entail: Rename the folder to the intended hero's MapTheme key (e.g. `Hero_7` for Rabbit Chad) or change that hero's MapTheme in Heroes.csv, then re-import.
- Resolution: Resolved 2026-06-09: moved to `Content/Audio/OSTS/Heroes/Hero_7/MUS_Hero_7_RabbitChad.ogg` (Rabbit Chad = Hero_7, MapTheme=Hero_7), deleted the unreachable old uasset, re-imported via `ImportGeneratedOSTs.py` (BINKA compressed, SoundWave verified on disk).

## No footstep audio [Minor]

- What's wrong: The Helton pack ships footstep sets (`FEETMisc_STEP-Boots on Concrete Dungeon` etc.) but hero movement has no footstep cadence hook (no anim notifies on the rigs), so nothing plays steps.
- Why it's out of scope now: Wiring needs a distance/time-based step trigger inside `UT66HeroMovementComponent`; movement code was deliberately untouched in the audio pass.
- What fixing it would entail: Add a stride-distance accumulator in the movement component that fires a `Hero.Movement.Step` audio event, plus a surface-type lookup if per-ground variants are wanted.

## Casino per-game flavor sounds [Minor]

- What's wrong: `Casino.Bet/Win/Lose` cover the shared resolution funnel only; individual mini-game moments (cup shuffle, joker reveal, coin spin) have no cues.
- Why it's out of scope now: Per-widget flavor is polish beyond the coverage pass.
- What fixing it would entail: Add events in `Source/T66/UI/Gambler/*.cpp` widgets and rows in `Scripts/SetupAudioEventsDataTable.py`.
