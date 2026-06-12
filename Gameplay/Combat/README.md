# Combat README

## Collision-First Combat

T66 combat is collision-forward. If an attack is meant to exist in the room, its movement, contact, blocking, bounce, shove, and damage rules are part of the feature contract.

Use these terms consistently:

- `Hurtbox`: enemy, boss, or boss-part target area that can receive combat damage.
- `PlayerHurtbox`: hero body area that can receive hostile combat damage.
- `DamageVolume`: primitive, overlap, sweep, or query shape that is allowed to remove HP.
- `CombatBody`: visible gameplay actor or managed body that moves through the world and can collide, bounce, shove, or be blocked.

Visuals alone are not damage authority. Niagara particles, mesh opacity, and decorative helpers must not decide damage by themselves.

## Active Categories

The active idol/combat category set is:

- `AOE`
- `Bounce`
- `DOT`
- `Summon`

Do not author new data, items, UI, VFX bindings, proof scripts, or generator paths for retired line-category content. Old saves do not need compatibility for that retired category.

## Projectile Mesh Style

Hero and idol projectile meshes should read as inflated physical bodies: glossy balloon, inflatable toy, or plush-like forms with rounded volume and soft-contact readability. They are not generic spell icons, flat elemental symbols, or explosion-only VFX.

The intended read is that the projectile body itself travels into the enemy and knocks or pushes the enemy away. Do not rely on a separate post-impact explosion to explain the hit. Each authored projectile mesh must communicate element/category through its inflated body shape and material. For example, an Electricity idol should be an inflatable lightning plush body, while a Summon idol should be a tiny helpful inflatable spirit/minion body.

The same projectile-body rule applies to hostile projectiles. Bosses and projectile-capable mobs should use the same four active combat categories (`AOE`, `Bounce`, `DOT`, `Summon`) and should eventually point at authored projectile meshes rather than one-off particles or symbolic spell sprites. Until final art is generated, hostile rows may keep temporary visual profiles, but the source data must still declare the intended projectile category and must leave an explicit mesh slot for the generated asset.

When generating Pixal3D source images:

- Use the approved Hero 1 black weapon projectile as the style target.
- Use one clear centered subject per source image unless a sheet will be cropped before Pixal3D.
- Keep shapes chunky, readable, rounded, and game-scale friendly.
- Use glossy inflated material, seam/rib details, and simple color blocks.
- Avoid particle trails, smoke clouds, magical glyphs, flat stickers, text, labels, and explosion-only concepts.

## Summon Idols

Summon idols create visible helper combat bodies. A summon should only be created when a valid enemy is already in range.

Summon behavior contract:

- Choose the locked combat target when it is valid; otherwise choose the closest valid enemy.
- Spawn the summon body around the hero.
- Move or jump the summon toward its target.
- On target contact, apply summon-owned damage and bounce away.
- Continue until lifetime, hit count, or death cleanup ends the summon.
- Attribute damage to the idol/summon source, not the weapon source.

Temporary circle or sphere visuals are acceptable during implementation, but the movement/contact loop still needs a real damage authority path.

## Independent Idol Attacks

The target model is independent idol attacks:

- Weapons fire on their own cadence.
- Each equipped idol has its own cooldown, target selection, hit logic, damage source, and status proc chance.
- Idol attacks do not require a weapon hit point to exist.
- Summon, AOE, Bounce, and DOT should converge on this independent scheduler model.

## Status Effects

Each idol element owns one status-effect family. Every active idol category can roll that element effect when it damages an enemy.

Status effect requirements:

- Data-authored proc chance.
- Data-authored duration and tuning values.
- Clear damage-source attribution.
- Visible enemy marker, material tint, attached VFX, or equivalent read while active.
- Movement-interrupt behavior where appropriate, such as slow, freeze, shock, root, daze, or knockback interruption.

Status visuals are gameplay readability. A proc that changes movement or action state should not be invisible in normal combat.
