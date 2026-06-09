You are Claude cross-reviewing a Codex draft for the T66 Unreal project.

Rules:
- Include a clear Result: OK or Result: NEEDS_USER line near the top.
- Prefer starting with the result line, but the parser will also accept a clear
  result line or unambiguous OK / needs-user meaning elsewhere in the response.
- Do not edit files.
- Do not run mutating commands.
- Treat Codex as the Operator/final router and you as the Validator.
- Compare the original prompt, Codex draft, and your independent answer when present.
- Look specifically for mistakes, missed constraints, risky assumptions, weak evidence, scope problems, and unclear wording.
- Patch the answer text when the fix is straightforward.
- Return concrete issues when Codex needs to inspect, edit, verify, or ask the user before answering.
- Ask a user question only when the user is the only person who can decide the next path.
- Keep the review concise and practical. Do not create packet-completeness ceremony or hard review-depth categories.

Your result should be one of these two lines:
Result: OK
Result: NEEDS_USER

After that result line, return a concise Markdown review with exactly these headings:
Summary
Suggested Answer Patch
Issues To Fix
Question For User
Evidence Or Verification Gaps
Notes

Result meanings:
- OK: the models can handle the prompt internally. You may still list corrections, evidence gaps, or wording patches for Codex to handle before answering.
- NEEDS_USER: the user's attention is required because only the user can decide, approve, unblock a missing prerequisite, resolve an unavailable required tool, or change the scope.

Do not use NEEDS_USER for ordinary mistakes or missing edits that Codex can fix. List those inside the review body and keep the result OK.

Review scope:
- Original prompt path: C:\UE\T66\Reports\AgentReviews\FireIdolRarityConcepts\original_prompt.md
- Codex draft path: C:\UE\T66\Reports\AgentReviews\FireIdolRarityConcepts\operator_draft.md
- Independent answer path: C:\UE\T66\Reports\AgentReviews\FireIdolRarityConcepts\20260605T091804-IndependentAnswer-pass1\claude_review_pass1.md
- Output scope: targeted cross-review and answer patch only.

<original_prompt>
# Original User Request

The user approved/revised the central idol concepts and now wants Fire only, broken down by category and rarity.

Relevant current request:

> Fire red AOE, that's fine. DOT should be a burn thing: the VFX takes place on the enemy body, not on the floor; hit the enemy and the body catches fire. Pierce is fine. Bounce should be embers snapping and bouncing: it hits and flicks like four embers to four close enemies. Next, go one by one. First, Fire, all four rarities, how they change. Break it down by category and rarity: black, red, yellow, white.

## Task Contract

Working task: Planning-only Fire idol rarity breakdown.
Operator: Codex
Validator: Claude
Scope: Fire AOE, Fire DOT, Fire Pierce, Fire Bounce only. For each, propose Black/Red/Yellow/White rarity visual changes. No imagegen, no Unreal edits, no asset generation.
Stop condition: Return approval-ready rarity concepts for Fire idols.

## Live Roster Evidence

`Content/Data/Idols.csv` contains:

- `Idol_Fire_DOT`
- `Idol_Fire_AOE`
- `Idol_Fire_Pierce`
- `Idol_Fire_Bounce`

## Constraints

- Fire activation VFX should stay red/orange/fire-themed regardless of rarity label.
- Avoid a mechanical 1/3/5/one-big pattern. Rarity should change shape grammar, timing, density, and impact expression while preserving category identity.
- Keep the concepts feasible for simple placeholder shapes first: spheres, rings, capsules, cones, crescent/arc shapes, short line/ribbon trails, and billboard flame/ember sprites.

</original_prompt>

<validator_independent_answer>
Result: OK

## Independent Answer

All four Fire idols are confirmed in the roster (`Idol_Fire_DOT`, `Idol_Fire_AOE`, `Idol_Fire_Pierce`, `Idol_Fire_Bounce`), so this planning-only breakdown is well-grounded. Below is an approval-ready Fire rarity concept set. Treat rarity as an ascending progression Black → Red → Yellow → White, with each tier changing **shape grammar, timing, density, and impact expression** rather than just count. All tiers stay red/orange fire-themed.

### Fire AOE — red ground burst (user: "red AOE, that's fine")
- **Black** — single low red dome-sphere that expands once and fades; dull ember crackle, slow rise. Squat, heavy, "thump."
- **Red** — dome plus a fast outward fire-ring (expanding torus) that races to the edge; brighter core, quicker bloom. Adds a crescent scorch arc on the leading edge.
- **Yellow** — staggered double pulse: inner dome snaps, then a second ring overtakes it; a few upward flame billboards lick off the rim. Punchier timing, more vertical energy.
- **White** — sharp instant flash-core, ring snaps outward with a thin trailing ribbon, and a brief column of upward flame sprites at center. Fastest onset, hottest white-orange core fading to red edges.

### Fire DOT — burn ON the enemy body (user: VFX on enemy, body catches fire, not floor)
- **Black** — small flame billboard parented to the enemy torso; gentle flicker, sparse embers drifting up. Reads as "smoldering."
- **Red** — flames climb to cover torso + head region (capsule-fitted flame sprites); steadier, denser flicker with a faint heat-shimmer feel via opacity pulse.
- **Yellow** — full-body wrap of flame billboards with rhythmic flare-ups (timed intensity pulses) and a steady stream of rising embers. Clearly "engulfed."
- **White** — tight white-orange core flame hugging the body, faster aggressive flicker, embers snapping off in bursts on each damage tick. Most violent, hottest read while still body-locked.

### Fire Pierce — lance through enemies (user: "Pierce is fine")
- **Black** — short red line/ribbon trail behind the projectile; small spark on each pass-through.
- **Red** — longer, brighter ribbon with a thin leading flame cone; pass-through pops a small ember crescent.
- **Yellow** — twin-ribbon trail with a hotter cone tip; each pierce leaves a brief lingering ember streak at the contact point. Faster, sharper.
- **White** — concentrated white-hot lance with a crisp cone tip and a snapping spark burst at every enemy it passes through; trail is thinner but more intense (a "cutting beam" feel rather than a fat trail).

### Fire Bounce — snapping embers (user: hits and flicks ~4 embers to 4 close enemies)
- **Black** — impact flicks 2 slow embers to the 2 nearest enemies; soft snap, short ember arcs.
- **Red** — flicks ~4 embers in quick succession to nearby enemies, each a small bouncing spark with a short ribbon tail; crisper snap timing.
- **Yellow** — the 4 embers snap out near-simultaneously, each leaving a brief crescent arc and a small flame pop on landing. More energetic, tighter spacing.
- **White** — a fast radial snap of 4 hot embers that chain-flick with a sharp white-orange flash on each landing and a quick secondary spark; reads as electric-fast ember scatter while staying fire-colored.

Note the rarity escalation deliberately varies *expression* (dome→ring→pulse→flash-column for AOE; smolder→engulf for DOT; trail thickness/heat for Pierce; ember count→sync→snap-speed for Bounce) instead of a flat 1/3/5/one-big count ramp.

## Evidence Checked
- `Content/Data/Idols.csv` lines 2–5: all four Fire rows present; sprite refs map to Black/Red/Yellow/White columns, confirming the four-rarity structure.
- Original prompt directives (AOE red ok, DOT burn-on-body, Pierce ok, Bounce 4-ember flick) and constraints (stay fire-themed, avoid mechanical count pattern, placeholder-shape feasible).

## Questions Or Blockers
None requiring the user. One thing for Codex to confirm in its draft: the **rarity ordering direction** (I assumed Black=lowest → White=highest). The user only listed the label order "black, red, yellow, white" and didn't state which is rarest. If the intended hierarchy differs, the escalation should be re-mapped, but the per-tier concepts themselves still hold.

## Caveats
- Concepts are intentionally placeholder-shape friendly (spheres, rings, capsules, cones, crescents, ribbons, ember billboards) per the constraint; no asset/VFX implementation implied.
- DOT explicitly avoids any floor decal — all flame is body/capsule-parented as the user required.
- Bounce ember count is held at the user's "~4 to 4 close enemies"; I scaled lower tiers down to 2 to give rarity headroom — flag if the user wants 4 fixed across all tiers with only style changing.

</validator_independent_answer>

<codex_draft>
# Operator Draft: Fire Idol Rarity Concepts

Assumption: Black -> Red -> Yellow -> White escalates in intensity, with White the strongest. Fire stays red/orange across all rarities; rarity changes silhouette, timing, density, and impact expression, not element color.

## Fire AOE

Central identity: impact explosion.

- Black: compact circular burst. A low red fire dome expands from the impact point, with one simple ring fading out at the edge.
- Red: pressure-ring explosion. The center blooms, then a fast outward fire ring races across the radius, making the blast read wider and sharper.
- Yellow: staggered detonation. A central bloom hits first, then a second delayed fire wave erupts from the rim with a few upward flame tongues.
- White: inferno bloom. The impact flashes white-orange at the center, a clean circular flame wall rolls outward, and the center leaves a brief rising flame column.

## Fire DOT

Central identity: burn on the enemy body, not the floor.

- Black: smoldering body burn. Small torso-anchored flames flicker on the enemy with a few embers drifting upward.
- Red: climbing burn. Flames wrap more clearly around torso and head, with a steady body-locked flicker.
- Yellow: engulfed burn. Flame bands wrap the full enemy silhouette, pulsing on damage ticks with a stronger ember stream.
- White: white-hot body blaze. A tight white-orange core hugs the enemy body, with aggressive flicker and snap-off embers each tick.

## Fire Pierce

Central identity: flame lance.

- Black: simple fire spear. A narrow red bolt/capsule shoots forward with a short ember trail.
- Red: cone-tipped lance. The forward tip becomes sharper and brighter, with a longer trailing ribbon.
- Yellow: drilling lance. Two small flame ribbons spiral or flank the main spear, and each pierced enemy gets a brief ember streak.
- White: white-hot cutting lance. Thin, fast, and very bright, with crisp pass-through spark bursts instead of a fat trail.

## Fire Bounce

Central identity: snapping embers that flick from the first hit toward nearby enemies.

- Black: small ember flick. The hit spits quick red cinders in short arcing paths, with a soft pop on landing.
- Red: four clear snapping embers. The embers leave readable curved tails and land with small flame pops.
- Yellow: simultaneous ember scatter. The embers snap out almost at once, with sharper crescent trails and hotter landing flares.
- White: molten cinder volley. The embers launch with a white-orange flash, rebound in very fast arcs, and each landing creates a tiny secondary spark burst.

## Implementation Note

When we wire this later, the visible ember count should match the real gameplay target count for that rarity. The concept is not "more rarity means mechanically 1/3/5"; the concept is that Fire Bounce always reads as snapping embers, while the tier controls speed, clarity, heat, and landing behavior.

</codex_draft>
