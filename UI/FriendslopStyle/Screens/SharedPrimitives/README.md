# FriendslopStyle Shared Primitives

This folder owns the visual iteration records for reusable FriendslopStyle UI primitives that are not full screens.

## Current Pass

Pass 05 is the current accepted standard-modal checkbox variant pass. It adds
an opt-in `Do Not Ask Again` row using generated unchecked and checked checkbox
plates, live Slate label text, and a taller modal layout. Existing standard
modal users remain on the Pass 04 no-checkbox layout unless the checkbox row is
explicitly enabled.

Pass 04 is the current accepted standard-modal button chrome pass. It replaces
the Pass 03 seam-prone 9-slice button rendering with exact-size, textless 300 x
58 generated button plates rendered as Slate images with zero slice margin.

Pass 03 remains the accepted shared modal and tooltip sizing/positioning pass,
except for its button slice-integrity result, which Pass 04 supersedes. Pass 02
is historical failed/superseded evidence because it reused button chrome instead
of generating or approving every visible subcomponent.

The current visual authority is the active Main Menu reference:

`C:\UE\T66\UI\FriendslopStyle\Reference\MainMenu\Current\main_menu_reference_04_foreground_right_panel_icon_filters_coupon_cli.png`

The accepted Pass 02 assets were regenerated as textless shells through separate local Codex CLI workers under:

`C:\UE\T66\Saved\Codex\UI\FriendslopStyle\SharedPrimitives\pass02_workers\`

Runtime text is live Slate text for localization. Do not manually crop, mask, clone, inpaint, or paint text out of these assets.

Pass 05 implementation record:

`C:\UE\T66\UI\FriendslopStyle\Screens\SharedPrimitives\pass05_implementation.md`

Pass 04 implementation record:

`C:\UE\T66\UI\FriendslopStyle\Screens\SharedPrimitives\pass04_implementation.md`

Pass 03 implementation record:

`C:\UE\T66\UI\FriendslopStyle\Screens\SharedPrimitives\pass03_implementation.md`

Pass 02 implementation record:

`C:\UE\T66\UI\FriendslopStyle\Screens\SharedPrimitives\pass02_implementation.md`

Runtime specs:

`C:\UE\T66\UI\FriendslopStyle\Screens\SharedPrimitives\slice_specs.md`
