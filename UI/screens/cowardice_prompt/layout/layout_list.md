# Cowardice Prompt Layout List

Canvas: 1920x1080.

Fixed structure:
- Transparent/dimmed runtime gameplay underlay.
- Centered confirmation modal.
- Large prompt title.
- Empty status/message line area.
- Two modal buttons: affirmative danger action and neutral cancel action.

Runtime-owned regions:
- Prompt title, status/error text, button labels, enabled state, and input routing.

Visual restyle target:
- Use a compact V2 modal surface with dark charcoal fill, crisp purple chrome, restrained danger treatment for the affirmative button, and a neutral cancel button.
- Do not bake gameplay background, HUD values, localized labels, or button state into runtime art.
