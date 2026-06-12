"""
One-shot: repoint BP_T66GameInstance.NPCsDataTable off the deleted DT_HouseNPCs package.

Cook logs warned: "Package /Game/Blueprints/Core/BP_T66GameInstance has a dependency on
package /Game/Data/DT_HouseNPCs which does not exist." The NPC rename pass replaced the
table with /Game/Data/DT_NPCs but the Blueprint default still pointed at the old package.
Delete this script after the fix is verified (task-specific, per Scripts lifecycle rule).

Run headless:
  UnrealEditor-Cmd.exe C:/UE/T66/T66.uproject -run=pythonscript -script="C:/UE/T66/Scripts/FixGameInstanceNPCsTableRef.py"
"""

import unreal

BP_PATH = "/Game/Blueprints/Core/BP_T66GameInstance"
NEW_TABLE_PATH = "/Game/Data/DT_NPCs"


def main():
    bp = unreal.EditorAssetLibrary.load_asset(BP_PATH)
    if not bp:
        unreal.log_error(f"Could not load {BP_PATH}")
        return

    gen_class = bp.generated_class()
    cdo = unreal.get_default_object(gen_class)

    try:
        current = cdo.get_editor_property("NPCsDataTable")
    except Exception as err:
        unreal.log_error(f"NPCsDataTable read failed: {err}")
        return
    unreal.log(f"NPCsDataTable before: {current}")

    new_table = unreal.EditorAssetLibrary.load_asset(NEW_TABLE_PATH)
    if not new_table:
        unreal.log_error(f"Could not load {NEW_TABLE_PATH}")
        return

    cdo.set_editor_property("NPCsDataTable", new_table)
    saved = unreal.EditorAssetLibrary.save_asset(BP_PATH, only_if_is_dirty=False)
    unreal.log(f"NPCsDataTable after: {cdo.get_editor_property('NPCsDataTable')}; saved={saved}")


main()
