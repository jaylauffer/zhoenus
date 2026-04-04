import unreal


ASSET_PATH = "/Game/Blueprints/UI/ConvertSpeed"
REPORT_PATH = "/Volumes/Micro-Data-Transfer/Zhoenus/zhoenus/Saved/inspect_convert_button_content.txt"


def emit(lines, message):
    lines.append(message)
    unreal.log_warning(message)


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
widget_asset = unreal.EditorAssetLibrary.load_asset(ASSET_PATH)

for name in (
    "Button_Decrement_Forward",
    "Button_Decrement_Reverse",
    "Button_Increment_Forward",
    "Button_Increment_Reverse",
):
    widget = unreal.EditorUtilityLibrary.find_source_widget_by_name(widget_asset, name)
    emit(lines, f"WIDGET name={name} class={widget.get_class().get_name() if widget else 'None'}")
    if not widget:
        continue
    content = try_call(widget, "get_child_at", 0) or try_call(widget, "get_content")
    emit(lines, f"CONTENT name={name} class={content.get_class().get_name() if content else 'None'} path={content.get_path_name() if content else 'None'}")
    if content:
        for prop_name in ("text", "button_label", "label"):
            value = try_prop(content, prop_name)
            if value is not None:
                emit(lines, f"CONTENT_PROP name={name} prop={prop_name} value={value}")

with open(REPORT_PATH, "w", encoding="utf-8") as report:
    report.write("\n".join(lines) + "\n")
