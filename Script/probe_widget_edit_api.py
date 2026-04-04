import unreal


ASSET_PATH = "/Game/Blueprints/UI/ConvertSpeed"
REPORT_PATH = "/Volumes/Micro-Data-Transfer/Zhoenus/zhoenus/Saved/probe_widget_edit_api.txt"


def emit(lines, message):
    lines.append(message)
    unreal.log_warning(message)


lines = []
widget_asset = unreal.EditorAssetLibrary.load_asset(ASSET_PATH)
widget = unreal.EditorUtilityLibrary.find_source_widget_by_name(widget_asset, "Button_Decrement_Forward")
parent = widget.get_parent() if widget else None

emit(lines, f"WIDGET_CLASS={widget.get_class().get_name() if widget else 'None'}")
emit(lines, f"WIDGET_NAME={widget.get_name() if widget else 'None'}")
emit(lines, f"PARENT_CLASS={parent.get_class().get_name() if parent else 'None'}")

for obj_name, obj in (("widget", widget), ("parent", parent)):
    if not obj:
        continue
    interesting = []
    for name in dir(obj):
        lname = name.lower()
        if any(token in lname for token in ("rename", "remove", "replace", "insert", "add_child", "clear", "slot", "parent", "visibility")):
            interesting.append(name)
    interesting.sort()
    emit(lines, f"{obj_name.upper()}_METHODS={','.join(interesting)}")

with open(REPORT_PATH, "w", encoding="utf-8") as report:
    report.write("\n".join(lines) + "\n")
