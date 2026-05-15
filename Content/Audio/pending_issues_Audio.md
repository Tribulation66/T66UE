# Pending Issues - Audio

## Staged Gameplay Smoke References Missing Audio Assets

- Severity tag: [Minor]
- What's wrong: The Easy mob VAT staged smoke logged missing packages for `/Game/Audio/SC_Music`, `/Game/Audio/SC_SFX`, and several candidate theme paths such as `/Game/Audio/Music/MainTheme` and `/Game/Audio/Theme`.
- Why it's out of scope now: The VAT pass only needed to prove enemy visual animation, material cookability, and standalone boot behavior. It did not change the audio content library or player settings audio routing.
- What fixing it would entail: Decide the canonical music/SFX SoundClass and theme asset paths, create or redirect the missing assets, update the player-settings audio references, cook/stage, and smoke a standalone run with audio enabled.
