"""
Create / refresh the retro chromatic aberration post-process material.

Run with:
  UnrealEditor.exe T66.uproject -ExecutePythonScript="C:/UE/T66/Scripts/CreateRetroChromaticAberrationMaterial.py"
"""

import unreal


def main():
    subsystem = unreal.get_editor_subsystem(unreal.T66UISetupSubsystem)
    if not subsystem:
        raise RuntimeError("T66UISetupSubsystem was not available")

    if not subsystem.create_retro_chromatic_aberration_material():
        raise RuntimeError("CreateRetroChromaticAberrationMaterial returned false")

    unreal.log("[CreateRetroChromaticAberrationMaterial] Success")


if __name__ == "__main__":
    main()
