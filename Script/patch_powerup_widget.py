import unreal


ASSET_PATH = "/Game/Blueprints/UI/PowerUpScreenWidget"
NEW_PARENT_CLASS_PATH = "/Script/Zhoenus.PowerUpScreenUI"
REPORT_PATH = "/Volumes/Micro-Data-Transfer/Zhoenus/zhoenus/Saved/patch_powerup_widget.txt"


lines = []


def emit(message):
    lines.append(message)
    unreal.log_warning(message)


widget_blueprint = unreal.EditorAssetLibrary.load_asset(ASSET_PATH)
if not widget_blueprint:
    raise RuntimeError(f"Failed to load widget blueprint: {ASSET_PATH}")

new_parent_class = unreal.load_class(None, NEW_PARENT_CLASS_PATH)
if not new_parent_class:
    raise RuntimeError(f"Failed to load parent class: {NEW_PARENT_CLASS_PATH}")

unreal.BlueprintEditorLibrary.reparent_blueprint(widget_blueprint, new_parent_class)
unreal.BlueprintEditorLibrary.compile_blueprint(widget_blueprint)
unreal.EditorAssetLibrary.save_loaded_asset(widget_blueprint)
emit(f"REPARENTED={new_parent_class.get_path_name()}")
emit("COMPILED_AND_SAVED=True")

with open(REPORT_PATH, "w", encoding="utf-8") as report:
    report.write("\n".join(lines) + "\n")
