# WidgetGames Asset Root Convention

`FT66WidgetGameDescriptor::AssetRoot` declares the intended content root for a widget game. It is metadata only until a widget explicitly reads it.

Future organized asset roots should use:

`/Game/WidgetGames/Casino/<GameName>/{sprites,sounds,data,materials,vfx}`

Examples:

- `/Game/WidgetGames/Casino/CoinFlip/sounds`

Existing widgets can continue using their current hardcoded or subsystem-provided paths until a later content migration updates both assets and loading code together.
