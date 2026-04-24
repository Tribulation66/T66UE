# Core Frontend Asset/State Checklists

Source basis: existing UI reconstruction docs, HeroSelection and Settings screen intakes, current frontend screen code, and `T66FrontendScreen` command-line resolver behavior. Runtime art must stay text-free unless explicitly called out as baked title art.

Required control states use `normal`, `hover`, `pressed`, `disabled`, and `selected` where applicable. Stretchable shells need valid nine-slice margins and stable state anchors.

## HeroSelection

- Scene/background needs: UI-free selection-stage scene plate; open center preview aperture for the live 3D hero/companion stage; no baked carousel, stats, party, buff, difficulty, or CTA content.
- Panel/shell families: inherited/top strip, left skins/stats panel, center preview frame, right info/records panel, bottom party/run-control band, temporary buff socket frames, decor.
- Button/control families: hero carousel arrows and hero grid button; companion arrows/grid/choose controls; body type toggles; skin preview/equip/buy/equipped rows; lore/info target toggles; difficulty/ready/enter/challenges/back controls; buff slot buttons. States: all visible buttons need `normal/hover/pressed/disabled`; toggles and selected body/difficulty/info targets also need `selected`.
- Runtime-owned live text/data/image regions: hero and companion names, lore, stats, records, difficulty/party/ready labels, AC balance, skin ownership/prices, temp buff icons/counts, party portraits/state, hero/companion carousel portraits, 3D preview, preview media.
- Automated capture key: known, `-T66FrontendScreen=HeroSelection`; existing intake command writes `output/hero_selection_workflow_anchor_fixed.png`.

## CompanionSelection

- Scene/background needs: UI-free companion-selection scene plate or transparent in-world preview treatment; center preview stays open for live companion stage.
- Panel/shell families: compact top strip, left skins/economy panel, center companion preview frame and confirm footer, right info/lore panel, bottom/back affordance, focus-mask shells for side panels when using the reconstructed theme.
- Button/control families: previous/next companion arrows, companion grid, no-companion, skins/lore switch, skin preview/equip/buy/equipped rows, confirm, back. States: buttons `normal/hover/pressed/disabled`; selected companion/no-companion, lore/info view, equipped skin, and locked companion states need `selected` or disabled visuals.
- Runtime-owned live text/data/image regions: companion carousel portraits, companion 3D preview, companion names, lore, friendliness/union progress, healing type, skin names, AC balance, ownership/prices, locked/unlocked state.
- Automated capture key: not known; screen class exists as `ET66ScreenType::CompanionSelection`, but current `T66FrontendScreen` resolver does not map `CompanionSelection`.

## SaveSlots

- Scene/background needs: UI-free frontend background behind save-grid surface; no baked save cards, party portraits, hero portraits, page/filter labels, timestamps, or status copy.
- Panel/shell families: full-page save surface, party-size dropdown shell/menu, save slot card family, party avatar socket frames, hero portrait socket frames, pagination/footer shell, optional empty-state panel.
- Button/control families: party-size dropdown options, preview, load, previous page, next page, back. States: all buttons `normal/hover/pressed/disabled`; active party-size filter needs `selected`; load is disabled when party size/host checks fail.
- Runtime-owned live text/data/image regions: visible slot count, slot number, stage, date/time, difficulty, party size, requires-party messages, page count, filter hint, empty-state copy, Steam avatars, hero portraits, saved player initials, host/party state.
- Automated capture key: not known; screen class exists as `ET66ScreenType::SaveSlots`, but current `T66FrontendScreen` resolver does not map `SaveSlots`.

## Settings

- Scene/background needs: UI-free frontend/settings background; no baked tabs, row labels, values, dropdown choices, toggles, sliders, keybinds, or confirm timers.
- Panel/shell families: inherited frontend top bar, settings tab strip, primary content shell, scroll viewport, row/card shell family, dropdown menu shell, toggle pair shell, slider track/fill shell, graphics confirm modal shell, rebinding state shell, decor.
- Button/control families: tab buttons, dropdowns/options, toggles, sliders, reset/apply/back buttons, keybind/rebind buttons, confirm/revert modal buttons. States: buttons `normal/hover/pressed/disabled`; active tab, selected dropdown option, on/off toggle, focused rebind row, and modal/default selections need `selected`.
- Runtime-owned live text/data/image regions: tab labels, row labels, selected values, slider percentages/fills, keybind labels, monitor/display modes, audio device names, HUD/media/fog/retro-fx values, confirm countdowns.
- Automated capture key: known, `-T66FrontendScreen=Settings`; existing intake command writes `output/settings_workflow_anchor_fixed.png`.

## LanguageSelect

- Scene/background needs: UI-free frontend background or modal scrim variant; no baked language names, title, confirm, or back text.
- Panel/shell families: full-page language panel, centered modal shell variant, language list scroll shell, language row family, footer button row.
- Button/control families: language row buttons, confirm, back. States: row buttons `normal/hover/pressed/selected`; confirm/back `normal/hover/pressed/disabled`.
- Runtime-owned live text/data/image regions: localized screen title, available language list, language display names, previewed/current language selection, confirm/back labels.
- Automated capture key: not known; screen class exists as `ET66ScreenType::LanguageSelect`, but current `T66FrontendScreen` resolver does not map `LanguageSelect`.

## Achievements

- Scene/background needs: UI-free frontend background; no baked achievement rows, names, descriptions, progress, reward labels, favorite state, or claim text.
- Panel/shell families: inherited frontend top bar, achievement shell, tab strip, completion/progress header shell, progress bar track/fill, scroll list shell, achievement row family, reward/action socket, favorite socket.
- Button/control families: Achievements/Secret tabs, claim/unlock, favorite toggle, back. States: buttons `normal/hover/pressed/disabled`; active tab and favorited state need `selected`; claim disabled/hidden state must be represented.
- Runtime-owned live text/data/image regions: achievement names/descriptions, category/secret masking, completion counts/percent, progress values, rewards, claim/claimed state, favorite glyph/state.
- Automated capture key: not known; screen class exists as `ET66ScreenType::Achievements`, but current `T66FrontendScreen` resolver does not map `Achievements`.

## Unlocks/Minigames

- Scene/background needs: UI-free frontend background; no baked minigame names, status tags, descriptions, availability text, or future slot labels.
- Panel/shell families: inherited frontend top bar, centered minigames container, slice panel family for available and unavailable entries, tag pill/socket, back affordance.
- Button/control families: clickable minigame slice panels, disabled coming-soon slice panels, back. States: clickable slices/buttons `normal/hover/pressed/disabled`; active/available slices may use `selected` or highlighted treatment; unavailable slots need disabled state.
- Runtime-owned live text/data/image regions: minigame titles, descriptions, availability tags, launch targets, future slot copy, localized back label.
- Automated capture key: known, `-T66FrontendScreen=Unlocks` or alias `-T66FrontendScreen=Minigames`.

## Shop/PowerUp

- Scene/background needs: UI-free frontend shop background; no baked stat names, prices, owned counts, progress, coupon balance, buff icons, or statue-fill values.
- Panel/shell families: inherited frontend top bar, shop shell, permanent/single-use tab strip, permanent stat card family, statue art socket/inset, single-use row/card family, secondary buff icon sockets, scroll shell, back/footer region.
- Button/control families: Permanent/Single Use tabs, permanent unlock buttons, single-use buy buttons, back. States: buttons `normal/hover/pressed/disabled`; active tab needs `selected`; maxed stat and insufficient-balance states need disabled or max-state visuals.
- Runtime-owned live text/data/image regions: tab labels, stat labels, Forbidden Chad part labels, statue art/fill fraction, progress counts, costs, coupon balance, secondary buff icons/names, owned counts, buy/max text.
- Automated capture key: not known; screen class exists as legacy enum `ET66ScreenType::PowerUp` with display name `Shop`, but current `T66FrontendScreen` resolver does not map `PowerUp` or `Shop`.

## DailyClimb

- Scene/background needs: layered UI-free animated main-menu-derived background (`sky_bg`, `fire_moon`, `pyramid_chad`) plus dark overlay; no baked rules, leaderboard rows, challenge values, or CTA text.
- Panel/shell families: left rules/challenge shell, info-card family, rule row family, center CTA shell, right leaderboard shell/chrome, title lockup region, back control.
- Button/control families: back, start challenge, continue challenge, leaderboard filters/controls inherited from leaderboard panel. States: buttons `normal/hover/pressed/disabled`; start/continue availability and in-flight/attempt-used states need disabled variants; leaderboard tabs/toggles need `selected`.
- Runtime-owned live text/data/image regions: daily challenge status, hero, difficulty, seed quality, reward, rule labels/descriptions, attempt state, continue availability, leaderboard rows/avatars/scores/ranks, title/subtitle text.
- Automated capture key: known, `-T66FrontendScreen=DailyClimb`.

## AccountStatus

- Scene/background needs: UI-free frontend background or modal scrim variant; no baked account status, recent runs, progress values, filters, appeal text, hero portraits, or run history.
- Panel/shell families: inherited frontend top bar, full-width account frame, modal frame variant, tab strip, overview sections, standing/suspension header shell, progress row family, personal-best block, filter/dropdown shells, recent-run row family, appeal editor shell, text-entry field shell.
- Button/control families: Suspension/Overview/History tabs, standing info toggle, view reviewed run, personal-best view and party-size dropdowns, history filters, run-summary row buttons, appeal/open/submit/cancel, back. States: buttons `normal/hover/pressed/disabled`; active tab/filter/dropdown choice and info popup state need `selected`; submit disabled/error/success states need clear variants.
- Runtime-owned live text/data/image regions: restriction kind/reason/appeal status, eligibility text, progress counts/bars, achievement/power/hero/companion/stage/challenge counts, personal bests, global rank cache state, recent run rows, hero portraits, filters, editable appeal message, submit status.
- Automated capture key: not known; screen class exists as `ET66ScreenType::AccountStatus`, but current `T66FrontendScreen` resolver does not map `AccountStatus`.
