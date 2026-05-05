"""
Extract selected Helton Yan Pixel Combat WAVs, import them as SoundWave assets,
and create/reload DT_AudioEvents from Content/Data/AudioEvents.json.

Run from the editor or command line after compiling the C++ row struct:
  UnrealEditor-Cmd.exe T66.uproject -run=pythonscript -script="C:/UE/T66/Scripts/SetupAudioEventsDataTable.py"

The script can also be run with regular Python to only extract WAVs and write
the JSON source file.
"""

from pathlib import Path
from array import array
import io
import json
import re
import sys
import wave
import zipfile

try:
    import unreal
except ModuleNotFoundError:
    unreal = None


DT_PATH = "/Game/Data/DT_AudioEvents"
DESTINATION_PATH = "/Game/Audio/HeltonPixelCombat"
SOURCE_RELATIVE_DIR = Path("SourceAssets") / "Audio" / "HeltonPixelCombat" / "Selected"
JSON_RELATIVE_PATH = Path("Content") / "Data" / "AudioEvents.json"
IMPORT_SAMPLE_RATE = 48000
IMPORT_SAMPLE_WIDTH_BYTES = 2


def log(message):
    if unreal:
        unreal.log(message)
    else:
        print(message)


def log_error(message):
    if unreal:
        unreal.log_error(message)
    else:
        raise RuntimeError(message)


def project_root():
    if unreal:
        return Path(unreal.SystemLibrary.get_project_directory()).resolve()
    return Path(__file__).resolve().parents[1]


def find_source_zip():
    candidates = [
        Path.home() / "Downloads" / "Helton Yan's Pixel Combat - Single Files.zip",
        project_root() / "SourceAssets" / "Audio" / "Helton Yan's Pixel Combat - Single Files.zip",
        project_root() / "Helton Yan's Pixel Combat - Single Files.zip",
    ]
    for candidate in candidates:
        if candidate.exists():
            return candidate
    log_error("Could not find Helton Yan's Pixel Combat - Single Files.zip in Downloads or project SourceAssets/Audio.")
    return None


def sanitize_asset_name(filename):
    stem = Path(filename).stem
    stem = re.sub(r"[^A-Za-z0-9_]+", "_", stem).strip("_")
    stem = re.sub(r"_+", "_", stem)
    if not stem:
        stem = "Audio"
    if stem[0].isdigit():
        stem = f"A_{stem}"
    return f"HYPC_{stem}"[:100]


def decode_pcm_to_int16_samples(frames, sample_width):
    if sample_width == IMPORT_SAMPLE_WIDTH_BYTES:
        samples = array("h")
        samples.frombytes(frames)
        if sys.byteorder != "little":
            samples.byteswap()
        return list(samples)

    samples = []
    if sample_width == 1:
        samples.extend((byte - 128) << 8 for byte in frames)
        return samples

    if sample_width == 3:
        for index in range(0, len(frames), 3):
            value = int.from_bytes(frames[index:index + 3], byteorder="little", signed=False)
            if value & 0x800000:
                value -= 0x1000000
            samples.append(max(-32768, min(32767, value >> 8)))
        return samples

    if sample_width == 4:
        for index in range(0, len(frames), 4):
            value = int.from_bytes(frames[index:index + 4], byteorder="little", signed=True)
            samples.append(max(-32768, min(32767, value >> 16)))
        return samples

    raise ValueError(f"Unsupported WAV sample width: {sample_width}")


def resample_interleaved_int16(samples, channels, source_rate, target_rate):
    if source_rate == target_rate:
        return samples

    input_frames = len(samples) // channels
    if input_frames <= 1:
        return samples

    output_frames = max(1, int(round(input_frames * float(target_rate) / float(source_rate))))
    output = []
    for out_frame in range(output_frames):
        source_pos = out_frame * float(source_rate) / float(target_rate)
        left_index = min(int(source_pos), input_frames - 1)
        right_index = min(left_index + 1, input_frames - 1)
        alpha = source_pos - float(left_index)
        for channel in range(channels):
            left = samples[left_index * channels + channel]
            right = samples[right_index * channels + channel]
            value = int(round(left + (right - left) * alpha))
            output.append(max(-32768, min(32767, value)))
    return output


def int16_samples_to_bytes(samples):
    output = array("h", samples)
    if sys.byteorder != "little":
        output.byteswap()
    return output.tobytes()


def write_normalized_wav(wav_bytes, target_file):
    with wave.open(io.BytesIO(wav_bytes), "rb") as reader:
        channels = reader.getnchannels()
        sample_width = reader.getsampwidth()
        frame_rate = reader.getframerate()
        frame_count = reader.getnframes()
        frames = reader.readframes(frame_count)

    samples = decode_pcm_to_int16_samples(frames, sample_width)
    samples = resample_interleaved_int16(samples, channels, frame_rate, IMPORT_SAMPLE_RATE)
    frames = int16_samples_to_bytes(samples)
    sample_width = IMPORT_SAMPLE_WIDTH_BYTES
    frame_rate = IMPORT_SAMPLE_RATE

    with wave.open(str(target_file), "wb") as writer:
        writer.setnchannels(channels)
        writer.setsampwidth(sample_width)
        writer.setframerate(frame_rate)
        writer.writeframes(frames)


class AudioAssetSelector:
    def __init__(self, zip_path, source_dir):
        self.zip_path = Path(zip_path)
        self.source_dir = Path(source_dir)
        self.source_dir.mkdir(parents=True, exist_ok=True)
        with zipfile.ZipFile(self.zip_path) as archive:
            self.wav_names = sorted(info.filename for info in archive.infolist() if info.filename.lower().endswith(".wav"))
        self.selected = {}

    def find(self, pattern, count):
        tokens = [token.lower() for token in pattern]
        matches = [
            name for name in self.wav_names
            if all(token in Path(name).name.lower() for token in tokens)
        ]
        if len(matches) < count:
            log_error(f"Audio pattern {pattern} only matched {len(matches)} files, needed {count}.")
        return matches[:count]

    def select(self, patterns, per_pattern=3):
        selected_paths = []
        for pattern in patterns:
            if isinstance(pattern, str):
                pattern = (pattern,)
            for entry_name in self.find(pattern, per_pattern):
                selected_paths.append(self.extract(entry_name))
        return selected_paths

    def extract(self, entry_name):
        if entry_name in self.selected:
            return self.selected[entry_name]["asset_path"]

        asset_name = sanitize_asset_name(entry_name)
        target_file = self.source_dir / f"{asset_name}.wav"
        with zipfile.ZipFile(self.zip_path) as archive:
            with archive.open(entry_name) as src:
                write_normalized_wav(src.read(), target_file)

        asset_path = f"{DESTINATION_PATH}/{asset_name}.{asset_name}"
        self.selected[entry_name] = {
            "source_file": str(target_file),
            "asset_name": asset_name,
            "asset_path": asset_path,
        }
        return asset_path


def event(event_id, description, patterns, **overrides):
    return {
        "Name": event_id,
        "Description": description,
        "Patterns": patterns,
        "PerPattern": overrides.pop("per_pattern", 3),
        "VolumeBus": overrides.pop("VolumeBus", "SFX"),
        "bEnabled": overrides.pop("bEnabled", True),
        "bPlay2D": overrides.pop("bPlay2D", False),
        "bIsUISound": overrides.pop("bIsUISound", False),
        "bPrimeOnLoad": overrides.pop("bPrimeOnLoad", True),
        "VolumeMultiplier": overrides.pop("VolumeMultiplier", 1.0),
        "PitchMultiplier": overrides.pop("PitchMultiplier", 1.0),
        "PitchRandomRange": overrides.pop("PitchRandomRange", 0.03),
        "StartTime": overrides.pop("StartTime", 0.0),
        "MinReplayIntervalSeconds": overrides.pop("MinReplayIntervalSeconds", 0.03),
        "AttenuationAssetPath": overrides.pop("AttenuationAssetPath", ""),
        "ConcurrencyAssetPath": overrides.pop("ConcurrencyAssetPath", ""),
    }


def build_event_specs():
    ui = {"VolumeBus": "UI", "bPlay2D": True, "bIsUISound": True, "VolumeMultiplier": 0.82, "PitchRandomRange": 0.015, "MinReplayIntervalSeconds": 0.04}
    two_d = {"bPlay2D": True, "VolumeMultiplier": 0.9, "PitchRandomRange": 0.025, "MinReplayIntervalSeconds": 0.04}
    hit = {"per_pattern": 1, "VolumeMultiplier": 0.78, "PitchRandomRange": 0.045, "MinReplayIntervalSeconds": 0.045}
    trap = {"VolumeMultiplier": 0.92, "PitchRandomRange": 0.035, "MinReplayIntervalSeconds": 0.05}

    specs = [
        event("UI.Click", "Default Slate button click.", ["positive click"], **ui),
        event("UI.Confirm", "Positive confirmation click.", ["strong click 1"], **ui),
        event("UI.Back", "Back/navigation click.", ["rattling click"], **ui),
        event("UI.Deny", "Denied/unavailable UI action.", ["denied"], **ui),
        event("UI.Toggle", "Toggle or tab selection.", ["zap select"], **ui),

        event("Hero.Attack.Generic", "Fallback hero attack.", ["laser shot"], VolumeMultiplier=0.86, PitchRandomRange=0.04),
        event("Hero.Attack.Pierce", "Fallback piercing hero attack.", ["pew laser"], VolumeMultiplier=0.88, PitchRandomRange=0.04),
        event("Hero.Attack.AOE", "Fallback area hero attack.", ["mana bomb"], VolumeMultiplier=0.92, PitchRandomRange=0.04),
        event("Hero.Attack.Bounce", "Fallback bounce hero attack.", ["ricochet 1"], VolumeMultiplier=0.88, PitchRandomRange=0.04),
        event("Hero.Attack.DOT", "Fallback damage-over-time hero attack.", ["enemy debuff"], VolumeMultiplier=0.82, PitchRandomRange=0.04),

        event("Hero.Attack.Hero_1", "Arthur unique primary attack.", ["sword slash"], VolumeMultiplier=0.9, PitchRandomRange=0.035),
        event("Hero.Attack.Hero_2", "Lu Bu unique primary attack.", ["mana bomb"], VolumeMultiplier=0.94, PitchRandomRange=0.04),
        event("Hero.Attack.Hero_3", "Mike unique primary attack.", ["flare nova"], VolumeMultiplier=0.93, PitchRandomRange=0.04),
        event("Hero.Attack.Hero_4", "George unique primary attack.", ["pew laser"], VolumeMultiplier=0.86, PitchRandomRange=0.035),
        event("Hero.Attack.Hero_5", "Robo unique primary attack.", ["ricochet 2"], VolumeMultiplier=0.88, PitchRandomRange=0.04),
        event("Hero.Attack.Hero_6", "Billy unique primary attack.", ["laser shot"], VolumeMultiplier=0.82, PitchRandomRange=0.035),
        event("Hero.Attack.Hero_7", "Rabbit unique primary attack.", ["bamboo whip"], VolumeMultiplier=0.86, PitchRandomRange=0.04),
        event("Hero.Attack.Hero_8", "Shroud unique primary attack.", ["sharp summon"], VolumeMultiplier=0.84, PitchRandomRange=0.035),
        event("Hero.Attack.Hero_9", "xQc unique primary attack.", ["comical shot"], VolumeMultiplier=0.84, PitchRandomRange=0.04),
        event("Hero.Attack.Hero_10", "Moist unique primary attack.", ["enemy debuff"], VolumeMultiplier=0.82, PitchRandomRange=0.04),
        event("Hero.Attack.Hero_11", "North unique primary attack.", ["weirdness"], VolumeMultiplier=0.82, PitchRandomRange=0.04),
        event("Hero.Attack.Hero_12", "Asmon unique primary attack.", ["critical strike"], VolumeMultiplier=0.9, PitchRandomRange=0.04),

        event("Combat.Hit.Enemy", "Enemy takes non-lethal damage.", ["hit noise", "fleeting hit", "noisy hit"], **hit),
        event("Combat.Hit.Boss", "Boss takes non-lethal damage.", ["mecha core damage"], VolumeMultiplier=0.88, PitchRandomRange=0.04, MinReplayIntervalSeconds=0.07),
        event("Combat.Enemy.Death", "Enemy death burst.", ["crunchy burst"], VolumeMultiplier=0.9, PitchRandomRange=0.045, MinReplayIntervalSeconds=0.04),
        event("Combat.Boss.Death", "Boss death burst.", ["mecha core damage"], VolumeMultiplier=1.05, PitchRandomRange=0.03, MinReplayIntervalSeconds=0.25),
        event("Combat.Ultimate.Cast", "Hero ultimate/large skill cast.", ["strong energy", "high powering up", "skill ready"], per_pattern=1, VolumeMultiplier=0.92, PitchRandomRange=0.035, MinReplayIntervalSeconds=0.08),

        event("Combat.Hit.Enemy.Melee", "Melee enemy takes non-lethal damage.", ["gore pierce"], per_pattern=1, VolumeMultiplier=0.78, PitchRandomRange=0.045, MinReplayIntervalSeconds=0.045),
        event("Combat.Hit.Enemy.Flying", "Flying enemy takes non-lethal damage.", ["fleeting hit"], per_pattern=1, VolumeMultiplier=0.72, PitchRandomRange=0.05, MinReplayIntervalSeconds=0.045),
        event("Combat.Hit.Enemy.Ranged", "Ranged enemy takes non-lethal damage.", ["laser impact"], per_pattern=1, VolumeMultiplier=0.76, PitchRandomRange=0.045, MinReplayIntervalSeconds=0.045),
        event("Combat.Hit.Enemy.Rush", "Rush enemy takes non-lethal damage.", ["hit noise"], per_pattern=1, VolumeMultiplier=0.78, PitchRandomRange=0.05, MinReplayIntervalSeconds=0.045),
        event("Combat.Hit.Enemy.Special", "Special enemy takes non-lethal damage.", ["electric hit"], per_pattern=1, VolumeMultiplier=0.8, PitchRandomRange=0.045, MinReplayIntervalSeconds=0.045),
        event("Combat.Enemy.Death.Melee", "Melee enemy death.", ["crunchy burst"], VolumeMultiplier=0.9, PitchRandomRange=0.045, MinReplayIntervalSeconds=0.04),
        event("Combat.Enemy.Death.Flying", "Flying enemy death.", ["noise decay"], VolumeMultiplier=0.84, PitchRandomRange=0.045, MinReplayIntervalSeconds=0.04),
        event("Combat.Enemy.Death.Ranged", "Ranged enemy death.", ["laser impact"], VolumeMultiplier=0.88, PitchRandomRange=0.045, MinReplayIntervalSeconds=0.04),
        event("Combat.Enemy.Death.Rush", "Rush enemy death.", ["sand impact"], VolumeMultiplier=0.9, PitchRandomRange=0.045, MinReplayIntervalSeconds=0.04),
        event("Combat.Enemy.Death.Special", "Special enemy death.", ["weirdness"], VolumeMultiplier=0.88, PitchRandomRange=0.045, MinReplayIntervalSeconds=0.04),

        event("Hero.Ultimate.SpearStorm", "Spear Storm ultimate cast.", ["sword slash", "strong energy"], per_pattern=1, VolumeMultiplier=1.0, PitchRandomRange=0.025, MinReplayIntervalSeconds=0.10),
        event("Hero.Ultimate.MeteorStrike", "Meteor Strike ultimate cast.", ["mana bomb", "sand impact"], per_pattern=1, VolumeMultiplier=1.0, PitchRandomRange=0.025, MinReplayIntervalSeconds=0.10),
        event("Hero.Ultimate.ChainLightning", "Chain Lightning ultimate cast.", ["voltaic blast", "electric hit"], per_pattern=1, VolumeMultiplier=0.98, PitchRandomRange=0.025, MinReplayIntervalSeconds=0.10),
        event("Hero.Ultimate.PlagueCloud", "Plague Cloud ultimate cast.", ["enemy debuff", "aura up"], per_pattern=1, VolumeMultiplier=0.94, PitchRandomRange=0.03, MinReplayIntervalSeconds=0.10),
        event("Hero.Ultimate.PrecisionStrike", "Precision Strike ultimate cast.", ["critical strike", "sharp summon"], per_pattern=1, VolumeMultiplier=0.96, PitchRandomRange=0.025, MinReplayIntervalSeconds=0.10),
        event("Hero.Ultimate.FanTheHammer", "Fan the Hammer ultimate cast.", ["comical shot", "laser shot"], per_pattern=1, VolumeMultiplier=0.94, PitchRandomRange=0.035, MinReplayIntervalSeconds=0.10),
        event("Hero.Ultimate.Deadeye", "Deadeye ultimate cast.", ["skill ready", "critical strike"], per_pattern=1, VolumeMultiplier=0.94, PitchRandomRange=0.025, MinReplayIntervalSeconds=0.10),
        event("Hero.Ultimate.Discharge", "Discharge ultimate cast.", ["voltaic blast"], VolumeMultiplier=0.98, PitchRandomRange=0.025, MinReplayIntervalSeconds=0.10),
        event("Hero.Ultimate.Juiced", "Juiced ultimate cast.", ["weirdness", "zap select"], per_pattern=1, VolumeMultiplier=0.9, PitchRandomRange=0.04, MinReplayIntervalSeconds=0.10),
        event("Hero.Ultimate.DeathSpiral", "Death Spiral ultimate cast.", ["hollow tornado"], VolumeMultiplier=0.98, PitchRandomRange=0.03, MinReplayIntervalSeconds=0.10),
        event("Hero.Ultimate.Shockwave", "Shockwave ultimate cast.", ["sand impact", "flare nova"], per_pattern=1, VolumeMultiplier=1.0, PitchRandomRange=0.025, MinReplayIntervalSeconds=0.10),
        event("Hero.Ultimate.TidalWave", "Tidal Wave ultimate cast.", ["water bolt"], VolumeMultiplier=0.96, PitchRandomRange=0.03, MinReplayIntervalSeconds=0.10),
        event("Hero.Ultimate.GoldRush", "Gold Rush ultimate cast.", ["coin toss", "critical strike"], per_pattern=1, VolumeMultiplier=0.95, PitchRandomRange=0.035, MinReplayIntervalSeconds=0.10),
        event("Hero.Ultimate.MiasmaBomb", "Miasma Bomb ultimate cast.", ["enemy debuff", "mana bomb"], per_pattern=1, VolumeMultiplier=0.96, PitchRandomRange=0.03, MinReplayIntervalSeconds=0.10),
        event("Hero.Ultimate.RabidFrenzy", "Rabid Frenzy ultimate cast.", ["pyro burst", "gore pierce"], per_pattern=1, VolumeMultiplier=0.98, PitchRandomRange=0.035, MinReplayIntervalSeconds=0.10),
        event("Hero.Ultimate.Blizzard", "Blizzard ultimate cast.", ["water bolt", "noise decay"], per_pattern=1, VolumeMultiplier=0.94, PitchRandomRange=0.03, MinReplayIntervalSeconds=0.10),
        event("Hero.Ultimate.ScopedSniper", "Scoped Sniper ultimate ready.", ["skill ready", "pew laser"], per_pattern=1, VolumeMultiplier=0.92, PitchRandomRange=0.025, MinReplayIntervalSeconds=0.10),
        event("Hero.Ultimate.ScopedSniper.Fire", "Scoped Sniper shot fire.", ["pew laser", "critical strike"], per_pattern=1, VolumeMultiplier=0.94, PitchRandomRange=0.025, MinReplayIntervalSeconds=0.08),

        event("Boss.Projectile.Fire", "Boss projectile fire.", ["laser shot"], VolumeMultiplier=0.82, PitchRandomRange=0.04, MinReplayIntervalSeconds=0.055),
        event("Boss.Projectile.Impact", "Boss projectile impact.", ["laser impact", "electric hit"], per_pattern=2, VolumeMultiplier=0.9, PitchRandomRange=0.04, MinReplayIntervalSeconds=0.05),
        event("Boss.AOE.Warning", "Boss ground AOE warning.", ["energy riser"], VolumeMultiplier=0.72, PitchRandomRange=0.03, MinReplayIntervalSeconds=0.10),
        event("Boss.AOE.Impact", "Boss ground AOE impact.", ["voltaic blast"], VolumeMultiplier=1.0, PitchRandomRange=0.035, MinReplayIntervalSeconds=0.08),
        event("Boss.Projectile.Fire.Balanced", "Balanced boss projectile fire.", ["laser shot"], VolumeMultiplier=0.82, PitchRandomRange=0.04, MinReplayIntervalSeconds=0.055),
        event("Boss.Projectile.Fire.Sharpshooter", "Sharpshooter boss projectile fire.", ["pew laser"], VolumeMultiplier=0.82, PitchRandomRange=0.035, MinReplayIntervalSeconds=0.055),
        event("Boss.Projectile.Fire.Juggernaut", "Juggernaut boss projectile fire.", ["mecha core damage"], per_pattern=1, VolumeMultiplier=0.86, PitchRandomRange=0.025, MinReplayIntervalSeconds=0.07),
        event("Boss.Projectile.Fire.Duelist", "Duelist boss projectile fire.", ["ricochet 2"], VolumeMultiplier=0.82, PitchRandomRange=0.04, MinReplayIntervalSeconds=0.055),
        event("Boss.Projectile.Fire.Vendor", "Vendor boss projectile fire.", ["coin toss"], VolumeMultiplier=0.78, PitchRandomRange=0.035, MinReplayIntervalSeconds=0.055),
        event("Boss.Projectile.Fire.Gambler", "Gambler boss projectile fire.", ["rattling click", "weirdness"], per_pattern=1, VolumeMultiplier=0.8, PitchRandomRange=0.04, MinReplayIntervalSeconds=0.055),
        event("Boss.AOE.Warning.Balanced", "Balanced boss AOE warning.", ["energy riser"], VolumeMultiplier=0.72, PitchRandomRange=0.03, MinReplayIntervalSeconds=0.10),
        event("Boss.AOE.Warning.Sharpshooter", "Sharpshooter boss AOE warning.", ["sharp summon"], VolumeMultiplier=0.72, PitchRandomRange=0.03, MinReplayIntervalSeconds=0.10),
        event("Boss.AOE.Warning.Juggernaut", "Juggernaut boss AOE warning.", ["high powering up"], per_pattern=1, VolumeMultiplier=0.76, PitchRandomRange=0.025, MinReplayIntervalSeconds=0.10),
        event("Boss.AOE.Warning.Duelist", "Duelist boss AOE warning.", ["phase sweeps"], VolumeMultiplier=0.72, PitchRandomRange=0.03, MinReplayIntervalSeconds=0.10),
        event("Boss.AOE.Warning.Vendor", "Vendor boss AOE warning.", ["zap select"], VolumeMultiplier=0.7, PitchRandomRange=0.03, MinReplayIntervalSeconds=0.10),
        event("Boss.AOE.Warning.Gambler", "Gambler boss AOE warning.", ["weirdness"], VolumeMultiplier=0.72, PitchRandomRange=0.035, MinReplayIntervalSeconds=0.10),

        event("Trap.Arrow.Windup", "Wall arrow trap windup.", ["wind sweep swish"], **trap),
        event("Trap.Arrow.Fire", "Wall arrow trap projectile release.", ["simple whoosh"], **trap),
        event("Trap.Arrow.Impact", "Trap arrow projectile hit.", ["gore pierce"], **trap),
        event("Trap.Flame.Warning", "Floor flame warning.", ["flare ignite"], **trap),
        event("Trap.Flame.Activate", "Floor flame burst.", ["fire hit"], **trap),
        event("Trap.Spike.Warning", "Spike patch warning.", ["noise decay"], **trap),
        event("Trap.Spike.Rise", "Spike patch rise.", ["sand impact"], **trap),
        event("Trap.PressurePlate.Trigger", "Pressure plate trigger.", ["metallic click"], **trap),

        event("Pickup.LootBag", "Loot bag accepted.", ["buff pickup"], **two_d),
        event("Pickup.Item", "World item pickup.", ["magic item"], **two_d),
        event("Pickup.Gold", "Gold pickup.", ["coin toss"], **two_d),
        event("Pickup.Discard", "Loot bag discarded.", ["rattling click"], **two_d),
        event("Interact.Generic", "Generic world interaction.", ["generic use"], **two_d),
        event("Interact.Chest.Open", "Chest opened.", ["generic item"], **two_d),
        event("Interact.Crate.Open", "Crate opened.", ["generic consume"], **two_d),
        event("Interact.Totem.Activate", "Totem or magical world interaction.", ["effect success", "aura up"], per_pattern=2, **two_d),

        event("Hero.Movement.Jump", "Hero jump.", ["retro jump"], **two_d),
        event("Hero.Movement.Dash", "Hero dash.", ["phase sweeps"], **two_d),
        event("Hero.Damage", "Hero takes damage.", ["denied"], VolumeBus="SFX", bPlay2D=True, VolumeMultiplier=0.9, PitchRandomRange=0.035, MinReplayIntervalSeconds=0.12),
    ]
    return specs


def materialize_rows(selector):
    rows = []
    for spec in build_event_specs():
        sound_paths = selector.select(spec.pop("Patterns"), spec.pop("PerPattern"))
        row = dict(spec)
        row["SoundAssetPaths"] = sound_paths
        rows.append(row)
    return rows


def write_json(rows, json_path):
    json_path.parent.mkdir(parents=True, exist_ok=True)
    with open(json_path, "w", encoding="utf-8") as handle:
        json.dump(rows, handle, indent=2)
        handle.write("\n")
    log(f"Wrote {len(rows)} audio event rows to {json_path}")


def import_selected_assets(selector):
    if not unreal:
        log("Unreal Python is unavailable; skipping SoundWave import/DataTable reload.")
        return

    unreal.EditorAssetLibrary.make_directory(DESTINATION_PATH)
    tasks = []
    for selected in selector.selected.values():
        task = unreal.AssetImportTask()
        task.set_editor_property("filename", selected["source_file"])
        task.set_editor_property("destination_path", DESTINATION_PATH)
        task.set_editor_property("destination_name", selected["asset_name"])
        task.set_editor_property("automated", True)
        task.set_editor_property("replace_existing", True)
        try:
            task.set_editor_property("replace_existing_settings", True)
        except Exception:
            pass
        task.set_editor_property("save", True)
        tasks.append(task)

    if tasks:
        unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks(tasks)
        log(f"Imported/reimported {len(tasks)} selected WAV assets into {DESTINATION_PATH}")
    else:
        log("No selected WAV assets to import.")


def resolve_row_struct():
    if not unreal:
        return None

    struct_type = getattr(unreal, "T66AudioEventRow", None)
    if struct_type is None:
        unreal.log_error("Could not resolve T66AudioEventRow in Python. Compile the C++ project first.")
        return None
    if hasattr(struct_type, "static_struct"):
        return struct_type.static_struct()
    return struct_type


def load_or_create_datatable(row_struct):
    if unreal.EditorAssetLibrary.does_asset_exist(DT_PATH):
        data_table = unreal.EditorAssetLibrary.load_asset(DT_PATH)
        if data_table:
            return data_table

    factory = unreal.DataTableFactory()
    factory.set_editor_property("struct", row_struct)

    asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
    package_path, asset_name = DT_PATH.rsplit("/", 1)
    data_table = asset_tools.create_asset(asset_name, package_path, unreal.DataTable, factory)
    if not data_table:
        unreal.log_error(f"Failed to create DataTable asset at {DT_PATH}")
        return None

    unreal.log(f"Created DataTable asset at {DT_PATH}")
    return data_table


def reload_datatable(json_path):
    if not unreal:
        return

    row_struct = resolve_row_struct()
    if row_struct is None:
        return

    data_table = load_or_create_datatable(row_struct)
    if not data_table:
        return

    success = unreal.DataTableFunctionLibrary.fill_data_table_from_json_file(data_table, str(json_path), row_struct)
    if not success:
        unreal.log_error("Failed to fill DT_AudioEvents from JSON.")
        return

    if not unreal.EditorAssetLibrary.save_asset(DT_PATH):
        unreal.log_error(f"Failed to save {DT_PATH}")
        return

    log(f"DT_AudioEvents reloaded from {json_path}")


def main():
    log("=== SetupAudioEventsDataTable ===")
    root = project_root()
    zip_path = find_source_zip()
    source_dir = root / SOURCE_RELATIVE_DIR
    json_path = root / JSON_RELATIVE_PATH

    selector = AudioAssetSelector(zip_path, source_dir)
    rows = materialize_rows(selector)
    write_json(rows, json_path)
    import_selected_assets(selector)
    reload_datatable(json_path)
    log(f"Selected {len(selector.selected)} unique WAVs from {zip_path}")
    log("=== SetupAudioEventsDataTable DONE ===")


if __name__ == "__main__":
    main()
