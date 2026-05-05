import os

import unreal


LEVEL_ASSET_PATH = "/Game/Map/TouchConfigPressure"
REPORT_PATH = os.path.join(unreal.Paths.project_saved_dir(), "create_touch_pressure_calibration_level.txt")


def emit(lines, message):
    lines.append(message)
    unreal.log(message)


def main():
    lines = []

    if unreal.EditorAssetLibrary.does_asset_exist(LEVEL_ASSET_PATH):
        emit(lines, f"SKIP existing={LEVEL_ASSET_PATH}")
    else:
        created = unreal.EditorLevelLibrary.new_level(LEVEL_ASSET_PATH)
        emit(lines, f"CREATED level={LEVEL_ASSET_PATH} success={created}")
        if not created:
            raise RuntimeError(f"Failed to create level: {LEVEL_ASSET_PATH}")

    saved = unreal.EditorLevelLibrary.save_current_level()
    emit(lines, f"SAVED current_level={saved}")

    with open(REPORT_PATH, "w", encoding="utf-8") as handle:
        handle.write("\n".join(lines) + "\n")


if __name__ == "__main__":
    main()
