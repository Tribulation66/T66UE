# Claude Operator Prompt: Combat VFX Visual/Hitbox Alignment Docs

You are Claude Code acting as read-only Operator for the T66 Unreal repo. Use model `claude-opus-4-8`.

Do not edit files. Do not run shell commands. Inspect the repo with read-only tools only.

## Working Goal

Update the combat VFX instruction and Markdown process docs so future weapon projectile and idol damage effects systematically keep visual meshes/VFX aligned in size, position, and impact location with their authoritative hit boxes, using Claude Opus 4.8 review/proposal where available before applying doc changes.

## User Problem

The Water idol proof exposed a mismatch: the blue visual sphere is smaller and offset relative to the Water idol damage radius/hit box. The Hero 1 AOE weapon visual/hitbox mismatch is mostly a position/origin issue: the weapon VFX spawns around a slash center while the impact context/idol trigger is farther forward on the crescent band. The user wants a systematic solution in the instruction and `.md` docs so future idol and weapon projectile VFX do not repeat this.

## Relevant Files To Inspect

- `AGENTS.md`
- `Gameplay/GAMEPLAY_AGENTS.md`
- `Gameplay/Combat/VFX_PROCESS_INDEX.md`
- `Gameplay/Combat/CombatVFXAuthoringProcedure.md`
- `Gameplay/Combat/CombatVFXDefinitionOfDone.md`
- `Gameplay/Combat/CombatVFXIdolOverlayArchitecture.md`
- `Gameplay/Combat/CombatVFXGeneratedAssetPolicy.md`
- `Gameplay/Combat/MASTER_COMBAT.md`
- `Gameplay/Combat/EffectPacketTemplate.md`
- `Gameplay/Combat/CombatVFXInfrastructureInventory.md`
- `Gameplay/Combat/Hero1AxeAOESlashMechanismPacket.md`
- `Source/T66/Gameplay/T66CombatComponent.cpp`
- `Source/T66/Gameplay/T66CombatVFX.cpp`
- `Source/T66/Data/T66DataTypes.h`
- `Content/Data/CombatVFXBindings.csv`

## Codex First-Pass Diagnosis

The current docs correctly say damage/hitbox authority lives in combat logic, not Niagara collision/material opacity. They do not yet require every effect packet and binding to declare:

- authoritative damage geometry center and extents,
- visual anchor/pivot/origin and any Z or planar offsets,
- visual footprint radius/width/length and how it maps to damage radius/sector/band,
- whether the visual is intentionally smaller/larger than the damage geometry,
- the capture/log evidence proving location and scale agreement,
- explicit user approval when a temporary proof marker intentionally does not match the damage shape.

This creates room for two different failures:

1. Water idol placeholder: damage radius is large but visible mesh is compact; without a declared marker-vs-area policy, it reads like a mismatch or hidden damage.
2. Hero 1 AOE: damage sector/crescent band and visual carrier can use different semantic centers; without a declared visual anchor vs damage origin/impact point rule, future weapons may trigger idols from the wrong point or show VFX in one place while the damage query lives elsewhere.

## Proposed Direction To Review

Add a generic "visual/damage alignment contract" to the combat VFX docs:

- Damage geometry remains authoritative.
- Production weapon/idol VFX must either match the authoritative geometry's location and footprint or declare an intentional mismatch with explicit approval and a visible telegraph/area read.
- Every effect packet must include an alignment section with authoritative center, impact point, shape type, extents, visual anchor, pivot, offsets, footprint mapping, allowed tolerance, and evidence.
- `BaseVisualRadius` and future binding fields should be treated as calibration values, not cosmetic guesses.
- Idol impact VFX must distinguish "impact marker" from "area/damage read"; if the idol has AOE damage, final production visuals need a readable AOE footprint or documented/approved split between marker and area telegraph.
- Captures must include combat debug damage volume overlay plus VFX in the same frame range, and logs should prove source ID, impact point, visual location, radius, base visual radius, and visual scale.
- Future irregular weapon VFX like arcs/slashes must specify whether the visual carrier is center-anchored, impact-anchored, band-anchored, or path-anchored.
- The effect packet template and DoD should fail a VFX as `PARTIAL` when size/position alignment is not evidenced.

## Requested Claude Output

Return a concrete proposal for which Markdown docs should be edited and exactly what rule content should be added. Call out any risk that my proposed doc-only scope is insufficient. Prefer minimal, durable process wording over one-off Water/Hero 1 wording.

Do not provide a formal strict verdict; this is an Operator proposal, not a review greenlight.
