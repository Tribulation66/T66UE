"""
T66 centralized dropdown styling — no asset import.

Dropdown look is driven by FT66Style (same pattern as MakeButton / MakePanel):
- FT66Style::GetDropdownComboButtonStyle() — theme-aware combo style (Dark = black bg + white text, Light = light bg + black text).
- FT66Style::MakeDropdown(FT66DropdownParams(TriggerContent, OnGetMenuContent)) — builds a themed SComboButton.

Usage:
- Settings screen: all dropdown rows use MakeDropdown + Tokens::Panel / Tokens::Text.
- Leaderboard / Lobby / HeroSelection: SComboBox uses Tokens::Text for item text (theme-aware); trigger uses engine default unless wrapped.

Run from editor Python console or:
  UnrealEditor-Cmd.exe T66.uproject -run=pythonscript -script="Scripts/ImportDropdown.py" -unattended -nop4 -nosplash
"""
import unreal


def main():
    unreal.log("[T66 Dropdown] Centralized dropdown style: FT66Style::MakeDropdown and GetDropdownComboButtonStyle(). Dark = black bg + white text; Light = light bg + black text.")
    unreal.log("[T66 Dropdown] Settings uses MakeDropdown; Leaderboard/Lobby/Hero SComboBox use Tokens::Text for items.")


if __name__ == "__main__":
    main()
else:
    main()
