# T66 UI Flat Redesign - Icon Manifest

Stage 1 audit source: V3 screen references in `C:\UE\T66\UI\Screen References\` and per-screen specs in `C:\UE\T66\UI\Reference\UI_FLAT_REDESIGN_REFERENCE.md` section 7.2.

Generated flat icons are monochrome transparent PNGs saved under `RuntimeDependencies\T66\UI\Icons\Flat\`. Existing branded/content artwork is preserved where the spec says not to redesign it.

## Top Bar / System

| Icon | Status | Path / Notes |
|---|---|---|
| settings cog | present | `SourceAssets\Archive\UI\Reference\Screens\MainMenu\Ultrakill\Elements\settings_gear_icon.png` |
| globe / language | present | `SourceAssets\Archive\UI\Reference\Screens\MainMenu\Ultrakill\Elements\language_globe_icon.png` |
| profile icon | present | `SourceAssets\Archive\UI\Reference\Screens\MainMenu\Ultrakill\Elements\home_profile_icon.png` |
| ticket / Chad Coupon | present + Hero Selection extracted glyph | Existing reference icon: `SourceAssets\Archive\UI\Reference\Screens\MainMenu\Ultrakill\Elements\coupon_ticket_icon.png`; Hero Selection runtime glyph regenerated from source crop `C:\UE\T66\UI\IconSourceCrops\HeroSelection\ticket_icon_source_crop.png` to `RuntimeDependencies\T66\UI\Icons\Flat\ticket.png`. Prompt recorded for M1: "reproduce this specific icon's visual style and shape exactly. Match line weight, silhouette, fill style, and any decorative detail. Do not creatively interpret - replicate." |
| power button | present | `SourceAssets\Archive\UI\Reference\Screens\MainMenu\Ultrakill\Elements\power_off_icon.png` |
| global leaderboard globe | present | `SourceAssets\Archive\UI\Reference\Screens\MainMenu\Ultrakill\Elements\leaderboard_filter_global_icon.png` |
| friends / people leaderboard | present | `SourceAssets\Archive\UI\Reference\Screens\MainMenu\Ultrakill\Elements\leaderboard_filter_friends_icon.png` |
| broadcast / streamers leaderboard | present | `SourceAssets\Archive\UI\Reference\Screens\MainMenu\Ultrakill\Elements\leaderboard_filter_streamers_icon.png` |
| gear generic | missing -> generated | `RuntimeDependencies\T66\UI\Icons\Flat\gear.png` |
| lock | missing -> generated | `RuntimeDependencies\T66\UI\Icons\Flat\lock.png` |
| people | partial -> generated | Existing leaderboard friends icon is chromed/reference-specific; flat glyph generated at `RuntimeDependencies\T66\UI\Icons\Flat\people.png`. |
| lab flask | missing -> generated | `RuntimeDependencies\T66\UI\Icons\Flat\lab_flask.png` |

## Action / Navigation

| Icon | Status | Path / Notes |
|---|---|---|
| BACK chevron | missing -> generated | `RuntimeDependencies\T66\UI\Icons\Flat\back_chevron.png` |
| forward chevron | missing -> generated | `RuntimeDependencies\T66\UI\Icons\Flat\forward_chevron.png` |
| pagination left | missing -> generated | `RuntimeDependencies\T66\UI\Icons\Flat\pagination_left.png` |
| pagination right | missing -> generated | `RuntimeDependencies\T66\UI\Icons\Flat\pagination_right.png` |
| refresh / circular arrow | missing -> generated | `RuntimeDependencies\T66\UI\Icons\Flat\refresh.png` |
| play / triangle | missing -> generated | `RuntimeDependencies\T66\UI\Icons\Flat\play_triangle.png` |
| home | partial -> generated | Archived reference crop exists under `SourceAssets\Archive\UI\Reference\Shared\TopBar\home_square_chad_crop.png`; flat glyph generated at `RuntimeDependencies\T66\UI\Icons\Flat\home.png`. |
| save / floppy disk | missing -> generated | `RuntimeDependencies\T66\UI\Icons\Flat\save_floppy.png` |
| copy / clipboard | missing -> generated | `RuntimeDependencies\T66\UI\Icons\Flat\copy_clipboard.png` |
| info "i" | missing -> generated | `RuntimeDependencies\T66\UI\Icons\Flat\info.png` |
| warning triangle | missing -> generated | `RuntimeDependencies\T66\UI\Icons\Flat\warning_triangle.png` |
| target / crosshair | missing -> generated | `RuntimeDependencies\T66\UI\Icons\Flat\target_crosshair.png` |
| pencil / edit | missing -> generated | `RuntimeDependencies\T66\UI\Icons\Flat\pencil_edit.png` |
| link / chain | missing -> generated | `RuntimeDependencies\T66\UI\Icons\Flat\link_chain.png` |
| crossed-X | missing -> generated | `RuntimeDependencies\T66\UI\Icons\Flat\crossed_x.png` |
| X-mark | missing -> generated | `RuntimeDependencies\T66\UI\Icons\Flat\x_mark.png` |
| broadcast / antenna | partial -> generated | Existing streamers icon is available for current chrome; flat glyph generated at `RuntimeDependencies\T66\UI\Icons\Flat\broadcast_antenna.png`. |
| trash / delete | missing -> generated | `RuntimeDependencies\T66\UI\Icons\Flat\trash.png` |

## Content / Stat / Role Icons

| Icon | Status | Path / Notes |
|---|---|---|
| trophy / laurel wreath | missing -> generated | `RuntimeDependencies\T66\UI\Icons\Flat\trophy_laurel.png` |
| stopwatch / timer | missing -> generated | `RuntimeDependencies\T66\UI\Icons\Flat\stopwatch.png` |
| helmet | missing -> generated | `RuntimeDependencies\T66\UI\Icons\Flat\helmet.png` |
| shield | partial -> generated | Runtime/gameplay shield VFX assets exist but are not flat UI glyphs; generated at `RuntimeDependencies\T66\UI\Icons\Flat\shield.png`. |
| fist | missing -> generated | `RuntimeDependencies\T66\UI\Icons\Flat\fist.png` |
| book | missing -> generated | `RuntimeDependencies\T66\UI\Icons\Flat\book.png` |
| skull | present | `RuntimeDependencies\T66\UI\MainMenu\mainmenu_cta_skull_imagegen_20260510.png` |
| skull flat | partial -> reference-region regenerated | Existing skull is rendered content art; Hero Selection ENTER glyph regenerated from source crop `C:\UE\T66\UI\IconSourceCrops\HeroSelection\enter_skull_source_crop.png` to `RuntimeDependencies\T66\UI\Icons\Flat\skull.png`. Prompt recorded for M1: "reproduce this specific icon's visual style and shape exactly. Match line weight, silhouette, fill style, and any decorative detail. Do not creatively interpret - replicate." |
| skull variant | missing -> generated | `RuntimeDependencies\T66\UI\Icons\Flat\skull_variant.png` |
| horned skull | missing -> generated | `RuntimeDependencies\T66\UI\Icons\Flat\horned_skull.png` |
| starburst / explosion | missing -> generated | `RuntimeDependencies\T66\UI\Icons\Flat\starburst.png` |
| gauge / speedometer | missing -> generated | `RuntimeDependencies\T66\UI\Icons\Flat\gauge_speedometer.png` |
| dice | missing -> generated | `RuntimeDependencies\T66\UI\Icons\Flat\dice.png` |
| loot bag | present | `RuntimeDependencies\T66\UI\Minimap\Icons\loot_bag.png` |
| loot bag flat | partial -> generated | Minimap icon is colored content art; tintable flat glyph generated at `RuntimeDependencies\T66\UI\Icons\Flat\loot_bag.png`. |
| plus / cross / heal | missing -> generated | `RuntimeDependencies\T66\UI\Icons\Flat\heal_cross.png` |
| chevrons / rank | missing -> generated | `RuntimeDependencies\T66\UI\Icons\Flat\rank_chevrons.png` |
| flame | partial -> generated | Fire skull/world VFX assets exist, but no flat UI flame glyph; generated at `RuntimeDependencies\T66\UI\Icons\Flat\flame.png`. |
| moon | missing -> generated | `RuntimeDependencies\T66\UI\Icons\Flat\moon.png` |
| mountain triangle | missing -> generated | `RuntimeDependencies\T66\UI\Icons\Flat\mountain_triangle.png` |
| wings | missing -> generated | `RuntimeDependencies\T66\UI\Icons\Flat\wings.png` |
| eye | missing -> generated | `RuntimeDependencies\T66\UI\Icons\Flat\eye.png` |
| eye with wings | missing -> generated | `RuntimeDependencies\T66\UI\Icons\Flat\eye_wings.png` |
| lightning bolt | partial -> generated | Item sprites include lightning-like art, but no flat reusable UI glyph; generated at `RuntimeDependencies\T66\UI\Icons\Flat\lightning_bolt.png`. |
| clover | missing -> generated | `RuntimeDependencies\T66\UI\Icons\Flat\clover.png` |
| check shield | missing -> generated | `RuntimeDependencies\T66\UI\Icons\Flat\check_shield.png` |
| chalice / grail | missing -> generated | `RuntimeDependencies\T66\UI\Icons\Flat\chalice_grail.png` |
| diamond / spiral idol shape | missing -> generated | `RuntimeDependencies\T66\UI\Icons\Flat\diamond_spiral_idol.png` |
| cube / box | present | `RuntimeDependencies\T66\UI\Minimap\Icons\crate.png` |
| cube / box flat | partial -> generated | Minimap crate is colored content art; tintable flat glyph generated at `RuntimeDependencies\T66\UI\Icons\Flat\cube_box.png`. |
| bar chart | missing -> generated | `RuntimeDependencies\T66\UI\Icons\Flat\bar_chart.png` |
| log / clipboard | missing -> generated | `RuntimeDependencies\T66\UI\Icons\Flat\log_clipboard.png` |

## Hero Selection Pilot Content Icons

| Icon | Status | Path / Notes |
|---|---|---|
| weapon icon | present | Runtime hero weapon sprites under `Content\Weapons\Sprites\...`; preserve existing per-hero content artwork. |
| ultimate icon | present | `Content\UI\Sprites\Abilities\Hero_1\T_Hero_1_Ultimate.uasset` and fallback `Content\ULTS\KnightULT.uasset`; preserve existing content artwork. |
| rank badge / medal | present | `RuntimeDependencies\T66\UI\HeroSelection\Medals\rank_badge_imagegen_20260427_v1.png` |
| Chad gender icon | reference-region regenerated | `RuntimeDependencies\T66\UI\Icons\Flat\chad_icon.png` regenerated from source crop `C:\UE\T66\UI\IconSourceCrops\HeroSelection\chad_icon_source_crop.png`. Prompt recorded for M1: "reproduce this specific icon's visual style and shape exactly. Match line weight, silhouette, fill style, and any decorative detail. Do not creatively interpret - replicate." Existing companion portrait fallback remains content artwork, not the inline button glyph. |
| Stacy gender icon | reference-region regenerated | `RuntimeDependencies\T66\UI\Icons\Flat\stacy_icon.png` regenerated from source crop `C:\UE\T66\UI\IconSourceCrops\HeroSelection\stacy_icon_source_crop.png`. Prompt recorded for M1: "reproduce this specific icon's visual style and shape exactly. Match line weight, silhouette, fill style, and any decorative detail. Do not creatively interpret - replicate." Existing companion portrait fallback remains content artwork, not the inline button glyph. |
| Steam logo | missing / preserve brand | No standalone Steam logo file was found in the current runtime/reference roots. Generated `RuntimeDependencies\T66\UI\Icons\Flat\steam_placeholder.png` is a placeholder only; Stage 2 should replace with an approved Steam brand asset if used in production UI. |

## Runtime Path

`Config\DefaultGame.ini` stages `RuntimeDependencies/T66/UI/...` as the loose runtime UI root. `T66RuntimeUITextureAccess::BuildLooseTextureCandidatePaths` remaps legacy `SourceAssets/UI/Icons/Flat/...` requests to `RuntimeDependencies/T66/UI/Icons/Flat/...` during migration.

