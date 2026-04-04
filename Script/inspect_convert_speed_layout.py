import unreal


ASSET_PATH = "/Game/Blueprints/UI/ConvertSpeed"
REPORT_PATH = "/Volumes/Micro-Data-Transfer/Zhoenus/zhoenus/Saved/inspect_convert_speed_layout.txt"


def try_call(obj, name, *args):
    try:
        return getattr(obj, name)(*args)
    except Exception:
        return None


def try_prop(obj, name):
    try:
        return obj.get_editor_property(name)
    except Exception:
        return None


lines = []


def emit(message):
    lines.append(message)
    unreal.log_warning(message)


widget_asset = unreal.EditorAssetLibrary.load_asset(ASSET_PATH)
if not widget_asset:
    raise RuntimeError(f"Failed to load {ASSET_PATH}")

targets = [
    "Button_Decrement_Forward",
    "Button_Decrement_Reverse",
    "Button_Increment_Forward",
    "Button_Increment_Reverse",
    "SpinBox_MaxForwardSpeed",
    "SpinBox_MaxReverseSpeed",
]

for name in targets:
    widget = unreal.EditorUtilityLibrary.find_source_widget_by_name(widget_asset, name)
    emit(f"WIDGET name={name} class={widget.get_class().get_name() if widget else 'None'} path={widget.get_path_name() if widget else 'None'}")
    if not widget:
        continue
    parent = try_call(widget, "get_parent")
    emit(f"PARENT name={name} parent_class={parent.get_class().get_name() if parent else 'None'} parent_name={parent.get_name() if parent else 'None'}")
    slot = try_prop(widget, "slot")
    emit(f"SLOT name={name} slot_class={slot.get_class().get_name() if slot else 'None'}")
    if slot:
        for prop_name in (
            "padding",
            "size",
            "horizontal_alignment",
            "vertical_alignment",
            "position",
            "size",
            "offsets",
            "anchors",
            "alignment",
            "auto_size",
            "z_order",
        ):
            value = try_prop(slot, prop_name)
            if value is not None:
                emit(f"SLOT_PROP name={name} prop={prop_name} value={value}")

with open(REPORT_PATH, "w", encoding="utf-8") as report:
    report.write("\n".join(lines) + "\n")
