Result: OK

## Independent Answer

The roster is confirmed: 5 elements (Fire, Ice, Electricity, Nature, Wind) × 4 deliveries (DOT, AOE, Pierce, Bounce) = 20 idols. Below is one central activation concept per idol — element-true, delivery-true, and buildable from simple placeholder shapes (sphere, ring/torus, cone, line/capsule, particle billboards). No rarity variants.

The organizing principle the user gave (fire AOE = explosion at impact, wind = tornado) is: **delivery dictates the silhouette/motion, element dictates the color + texture + secondary motion.** Apply consistently.

**Fire (red)**
- DOT — a lingering burning patch: a flat circular flame field with small flickering tongues rising. Placeholder: ground ring + upward-stretched billboards.
- AOE — explosion at impact: rapid radial fireball bloom + shockwave ring. Placeholder: scaling sphere + expanding torus.
- Pierce — a lance of fire: an elongated flaming bolt with a trailing ember streak. Placeholder: stretched capsule + ribbon trail.
- Bounce — a fire orb that splatters on each ricochet, leaving a brief scorch flash at each bounce point. Placeholder: sphere + small burst at contact.

**Ice (ice blue)**
- DOT — a frost field / chilling mist hugging the ground that slowly crystallizes. Placeholder: ground ring + low fog billboards.
- AOE — a shattering frost-nova: outward spike of crystals + a frozen ground ring. Placeholder: radial spike fan + torus.
- Pierce — an icicle shard / spear, sharp and clean, with a faint frost trail. Placeholder: stretched cone + ribbon.
- Bounce — a hardened ice ball that "cracks" the air at each bounce, dropping shard flecks. Placeholder: sphere + tiny shard particles at contact.

**Electricity (purple)**
- DOT — a sustained electrified zone: a crackling field with arcs jittering across it. Placeholder: ground ring + flickering line segments.
- AOE — a discharge burst: a bright central flash with forked lightning radiating outward. Placeholder: flash sprite + branching line strokes.
- Pierce — a focused bolt / lightning spear, near-instant, thin and bright. Placeholder: stretched capsule/line + glow.
- Bounce — an arcing spark that "jumps" between bounce points (chain-lightning feel). Placeholder: orb + line drawn to next contact.

**Nature (green)**
- DOT — a poison/spore cloud or creeping growth patch on the ground. Placeholder: ground ring + drifting particle puffs.
- AOE — a sudden burst of growth: vines/spikes erupting outward from impact. Placeholder: radial spike fan + ground ring.
- Pierce — a thorn / wooden spike projectile with a leafy trail. Placeholder: stretched cone + small particle trail.
- Bounce — a seed/spore pod that bursts a small puff at each bounce. Placeholder: sphere + puff at contact.

**Wind (grey)**
- DOT — a lingering gust/dust-devil field swirling in place. Placeholder: ground ring + rotating billboards.
- AOE — a burst gust / pressure shockwave knocking outward (the user's tornado idea fits here as a vertical funnel). Placeholder: expanding torus + vertical swirl.
- Pierce — a slicing air blade / wind cutter, thin crescent moving fast. Placeholder: stretched crescent/capsule + faint trail.
- Bounce — a wind sphere that puffs a gust burst at each ricochet. Placeholder: sphere + puff at contact.

This keeps every idol's element and delivery legible at a glance and avoids the "1/3/5 then one big one" gimmick the user explicitly rejected — differentiation later comes from rarity escalation, not from the central concept.

## Evidence Checked
- `Content/Data/Idols.csv` (read live): 20 rows confirm Fire/Ice/Electricity/Nature/Wind × DOT/AOE/Pierce/Bounce, all Delivery="Traveler".
- Color direction from prompt scope: Fire red, Ice ice-blue, Electricity purple, Nature green, Wind grey.

## Questions Or Blockers
None requiring the user before answering. This is approval-gated by design — the user will green-light concepts before rarity work, which is the natural next step, not a blocker.

## Caveats
- The CSV sprite columns use Black/Red/Yellow/White naming (these appear to be the rarity tiers). I did **not** introduce rarity-specific visuals, per scope — flagging only so Codex doesn't confuse the existing icon naming with the activation VFX being designed here.
- `AoeRadius=300` and `AoeDelay=0.2` exist for AOE rows; the AOE concepts above respect an impact-then-bloom timing that matches that delay. Worth keeping concepts feasible against those existing values, but no tuning is requested now.
- Wind's tornado reference best fits the AOE slot (vertical funnel); confirm with the user whether they want the tornado tied to AOE specifically or treated as the element's overall motif across deliveries.
