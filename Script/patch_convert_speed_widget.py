import unreal


ASSET_PATH = "/Game/Blueprints/UI/ConvertSpeed"
ZHOENUS_BUTTON_CLASS_PATH = "/Game/Blueprints/UI/ZhoenusButton.ZhoenusButton_C"
NEW_PARENT_CLASS_PATH = "/Script/Zhoenus.ConvertSpeedUI"
REPORT_PATH = "/Volumes/Micro-Data-Transfer/Zhoenus/zhoenus/Saved/patch_convert_speed_widget.txt"

BUTTON_PATCHES = [
    ("Button_Decrement_Forward", "ZhoenusButton_Decrement_Forward", "-"),
    ("Button_Increment_Forward", "ZhoenusButton_Increment_Forward", "+"),
    ("Button_Decrement_Reverse", "ZhoenusButton_Decrement_Reverse", "-"),
    ("Button_Increment_Reverse", "ZhoenusButton_Increment_Reverse", "+"),
]


lines = []


def emit(message):
    lines.append(message)
    unreal.log_warning(message)


def try_prop(obj, name):
    try:
        return obj.get_editor_property(name)
    except Exception:
        return None


def copy_slot_properties(old_widget, new_widget):
    old_slot = try_prop(old_widget, "slot")
    new_slot = try_prop(new_widget, "slot")
    if not old_slot or not new_slot:
        return

    for prop_name in ("padding", "size", "horizontal_alignment", "vertical_alignment"):
        value = try_prop(old_slot, prop_name)
        if value is not None:
            new_slot.set_editor_property(prop_name, value)


widget_blueprint = unreal.EditorAssetLibrary.load_asset(ASSET_PATH)
if not widget_blueprint:
    raise RuntimeError(f"Failed to load widget blueprint: {ASSET_PATH}")

zhoenus_button_class = unreal.load_class(None, ZHOENUS_BUTTON_CLASS_PATH)
if not zhoenus_button_class:
    raise RuntimeError(f"Failed to load widget class: {ZHOENUS_BUTTON_CLASS_PATH}")

new_parent_class = unreal.load_class(None, NEW_PARENT_CLASS_PATH)
if not new_parent_class:
    raise RuntimeError(f"Failed to load parent class: {NEW_PARENT_CLASS_PATH}")

unreal.BlueprintEditorLibrary.reparent_blueprint(widget_blueprint, new_parent_class)
emit(f"REPARENTED={new_parent_class.get_path_name()}")

for old_name, new_name, label in BUTTON_PATCHES:
    old_widget = unreal.EditorUtilityLibrary.find_source_widget_by_name(widget_blueprint, old_name)
    if not old_widget:
        raise RuntimeError(f"Failed to find source widget: {old_name}")

    parent = old_widget.get_parent()
    if not parent:
        raise RuntimeError(f"Widget has no parent: {old_name}")

    new_widget = unreal.EditorUtilityLibrary.find_source_widget_by_name(widget_blueprint, new_name)
    if not new_widget:
        new_widget = unreal.EditorUtilityLibrary.add_source_widget(
            widget_blueprint,
            zhoenus_button_class,
            new_name,
            parent.get_name(),
        )
        emit(f"ADDED={new_name}")
    else:
        emit(f"REUSED={new_name}")

    copy_slot_properties(old_widget, new_widget)
    new_widget.set_editor_property("label", label)
    new_widget.set_visibility(unreal.SlateVisibility.VISIBLE)

    old_widget.set_visibility(unreal.SlateVisibility.COLLAPSED)
    emit(
        f"PATCHED old={old_name} new={new_name} parent={parent.get_name()} "
        f"old_class={old_widget.get_class().get_name()} new_class={new_widget.get_class().get_name()} label={label}"
    )

unreal.BlueprintEditorLibrary.compile_blueprint(widget_blueprint)
unreal.EditorAssetLibrary.save_loaded_asset(widget_blueprint)
emit("COMPILED_AND_SAVED=True")

with open(REPORT_PATH, "w", encoding="utf-8") as report:
    report.write("\n".join(lines) + "\n")
