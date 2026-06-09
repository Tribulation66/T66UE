Result: OK

## Independent Answer

Roster confirmed in `Content/Data/Idols.csv`: exactly 20 idols — 5 elements (Fire, Ice, Electricity, Nature, Wind) × 4 mechanics (DOT, AOE, Pierce, Bounce). Planning-only creative task; the models can fully handle it. No user decision is blocking.

**On the Claude review process (the user's direct question):** You and Codex run an Operator/Validator split. Codex (Operator) drafts and routes the final answer; Claude (Validator) independently checks scope, roster facts, and your preserved concept corrections, then returns a Result line (OK / NEEDS_USER). "Tokens spent by Claude" = that validation pass. For this request the validation Result is **OK** — meaning the concept grid can be delivered without needing anything more from you.

**Rarity grammar (applies to every idol, Black→Red→Yellow→White = low→high):**
- **Black (common):** single, small, short, muted. One instance, minimal motion, quick fade.
- **Red (uncommon):** bigger + slightly longer, faint secondary motion (a flicker, a ripple).
- **Yellow (rare):** denser/multi-instance OR layered timing (e.g. a second pulse), brighter core, lingering trail.
- **White (legendary):** largest + longest + most "alive" — repeating/sustained motion, particle density peak, screen-readable impact pop. Never just "one big thing"; it reads as *more refined and energetic*, not merely scaled up.

Element colors held constant per rarity (rarity changes intensity/size/motion, NOT hue): Fire red/orange, Ice ice-blue, Electricity purple, Nature green, Wind grey.

### Fire
- **DOT (body-locked burn, not floor):** Black = thin flame patch on the enemy body, brief. Red = fuller body flame, small rising embers. Yellow = body engulfed, flames flicker faster + faint smoke. White = full-body roaring fire, continuous ember shower, longest burn read.
- **AOE (explosion):** Black = small quick burst. Red = wider burst + short shockwave ring. Yellow = burst with a delayed second flare + lingering scorch flash. Yellow→White = White is a layered double-detonation, dense debris/ember spray, biggest readable boom.
- **Pierce (flame lance):** Black = short thin lance. Red = longer lance, faint trailing flame. Yellow = thicker lance + ember wake. White = long bright lance with sustained ember trail and tip flare.
- **Bounce (embers snapping to nearby enemies):** Black = 1 ember snaps once. Red = ember snaps to 2, slight arc trail. Yellow = several embers snapping in quick succession. White = dense ember web snapping rapidly across many targets.

### Ice (no crystal spikes)
- **DOT (enemy frozen / body ice-blue):** Black = light blue tint on body. Red = fuller ice-blue coat, faint frost shimmer. Yellow = body visibly frozen, slow drifting frost motes. White = fully encased, sustained frost aura + slow shimmer pulse.
- **AOE (frost nova, no spikes):** Black = small frost ring expands once. Red = wider nova + lingering chill flash. Yellow = nova with second expanding ring + ground frost sheen. White = large layered nova, dense frost-mist wash, longest chill.
- **Pierce (icicle spear):** Black = small icicle spear. Red = longer spear + faint frost trail. Yellow = thicker spear + frost mist wake. White = long bright icicle, sustained frost trail and tip burst.
- **Bounce (ice shards in different directions):** Black = a couple shards scatter. Red = more shards, wider spread. Yellow = dense shard fan in several directions. White = shard burst saturating the area in multiple directions.

### Electricity
- **DOT (enemy shocked):** Black = small spark flicker on body. Red = steady arc crackle over body. Yellow = body buzzing with branching arcs. White = full-body electrocution, continuous arcing + jolt twitch read.
- **AOE (lightning strikes from above within radius):** Black = 1 bolt strikes inside radius. Red = 2–3 bolts in quick sequence. Yellow = multiple bolts rapid-fire across radius + ground flash. White = dense bolt barrage hammering the whole radius, sustained flicker.
- **Pierce (forward lightning bolt):** Black = thin short bolt. Red = longer bolt, faint branch. Yellow = thicker bolt + branching forks. White = long bright bolt with persistent forks and tip discharge.
- **Bounce (chain spark):** Black = sparks to 1 target. Red = chains to 2, visible arc links. Yellow = chains rapidly across several. White = dense fast chain lattice across many targets.

### Nature
- **DOT (spore poison):** Black = small spore puff on body. Red = fuller spore cloud, faint drift. Yellow = thick spore cloud + slow falling motes. White = dense sustained spore haze enveloping the enemy.
- **AOE (branch/root pokes in radius):** Black = a few roots poke up briefly. Red = more roots, slightly taller. Yellow = ring of roots/branches in quick sequence. White = dense root/branch eruption filling the radius, sustained sway.
- **Pierce (root travels forward along ground):** Black = short ground root. Red = longer root, faint dirt kick. Yellow = thicker root + side tendrils. White = long fast root with trailing tendrils and tip burst.
- **Bounce (seeds):** Black = 1–2 seeds hop to nearby. Red = several seeds, small arcs. Yellow = many seeds scattering/bouncing. White = dense seed spray bouncing across the area.

### Wind (grey)
- **DOT (dust devil on enemy):** Black = small dust devil on body. Red = taller devil, faint debris. Yellow = swirling devil + visible debris motes. White = sustained tight tornado on the enemy, dense debris swirl.
- **AOE (horizontal tornado, damage radius < movement circle):** Black = small tornado tracing a tight circle. Red = larger tornado, wider movement circle. Yellow = tornado + faint secondary swirl, denser dust. White = large dense tornado sweeping the full circle, sustained debris, with the damage radius clearly smaller than the movement path.
- **Pierce (skinny tornado moving forward):** Black = thin short tornado. Red = longer skinny tornado + faint dust trail. Yellow = denser column + debris wake. White = long fast tornado column with sustained debris trail.
- **Bounce (several little tornadoes bouncing):** Black = 1–2 small tornadoes hop. Red = a few bouncing, short paths. Yellow = several tornadoes bouncing actively. White = a swarm of little tornadoes bouncing densely across targets.

All are buildable first as simple placeholder shapes (circles, lances, puffs, sprites) and scaled up to particles later — rarity differences are expressible purely through count, size, duration, and motion.

## Evidence Checked
- `Content/Data/Idols.csv` — confirmed all 20 idols, 5 elements × {DOT, AOE, Pierce, Bounce}. No 21st idol or extra element.
- Original prompt: rarity order Black→Red→Yellow→White, element color constraints, anti-"1/3/5/one-big" rule, all 20 preserved concept corrections — all incorporated.

## Questions Or Blockers
None requiring the user. This is planning-only and self-contained.

## Caveats
- Roster note for Codex: the CSV reuses sprite families across idols (e.g. Wind shares the Storm sprite set with Electricity AOE; Fire Pierce/Bounce use Steel/Curse; Nature Bounce uses Shadow). These are *icon* assignments, not gameplay-VFX, so they don't affect the in-world concepts above — but if the user later wants per-idol distinct icons, that's a separate asset task.
- Concepts are intentionally engine-agnostic and color-locked per element; rarity reads through intensity/motion only, per the constraint.
- No imagegen, no Unreal edits, no asset generation performed — consistent with scope.
