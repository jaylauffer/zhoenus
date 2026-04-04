import unreal


ASSET_PATH = "/Game/Blueprints/UI/ZhoenusButton"
REPORT_PATH = "/Volumes/Micro-Data-Transfer/Zhoenus/zhoenus/Saved/inspect_zhoenus_button.txt"


def emit(lines, message):
    lines.append(message)
    unreal.log_warning(message)


def try_prop(obj, name):
    try:
        return obj.get_editor_property(name)
    except Exception:
        return None


lines = []
widget_asset = unreal.EditorAssetLibrary.load_asset(ASSET_PATH)
emit(lines, f"ASSET_CLASS={widget_asset.get_class().get_name() if widget_asset else 'None'}")

widget_tree = try_prop(widget_asset, "widget_tree") if widget_asset else None
widgets = list(widget_tree.get_all_widgets()) if widget_tree else []
emit(lines, f"WIDGET_COUNT={len(widgets)}")
for widget in widgets:
    emit(lines, f"WIDGET name={widget.get_name()} class={widget.get_class().get_name()}")
    for prop_name in ("text", "label", "button_label", "display_label"):
        value = try_prop(widget, prop_name)
        if value is not None:
            emit(lines, f"WIDGET_PROP name={widget.get_name()} prop={prop_name} value={value}")

source_widget = unreal.EditorUtilityLibrary.find_source_widget_by_name(widget_asset, "ZhoenusButton")
emit(lines, f"SOURCE_WIDGET={source_widget.get_class().get_name() if source_widget else 'None'}")

generated_class = unreal.load_object(None, ASSET_PATH + ".ZhoenusButton_C")
emit(lines, f"GENERATED_CLASS={generated_class.get_path_name() if generated_class else 'None'}")
cdo = unreal.get_default_object(generated_class) if generated_class else None
emit(lines, f"CDO_CLASS={cdo.get_class().get_name() if cdo else 'None'}")

if cdo:
    interesting = []
    for name in dir(cdo):
        lname = name.lower()
        if any(token in lname for token in ("label", "text", "set")):
            interesting.append(name)
    interesting.sort()
    emit(lines, f"CDO_METHODS={','.join(interesting)}")
    for prop_name in ("button_label", "display_label", "label", "text"):
        value = try_prop(cdo, prop_name)
        if value is not None:
            emit(lines, f"CDO_PROP prop={prop_name} value={value}")

with open(REPORT_PATH, "w", encoding="utf-8") as report:
    report.write("\n".join(lines) + "\n")
