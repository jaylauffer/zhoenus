import unreal


ASSET_PATH = "/Game/Blueprints/UI/ConvertSpeed"
ZHOENUS_BUTTON_PATH = "/Game/Blueprints/UI/ZhoenusButton.ZhoenusButton_C"
REPORT_PATH = "/Volumes/Micro-Data-Transfer/Zhoenus/zhoenus/Saved/inspect_convert_speed_widget.txt"


LINES = []


def emit(message):
    LINES.append(message)
    unreal.log_warning(message)


def obj_path(obj):
    return obj.get_path_name() if obj else "None"


def try_prop(obj, name):
    try:
        return obj.get_editor_property(name)
    except Exception:
        return None


def print_widget_tree(widget_tree):
    if not widget_tree:
        emit("WIDGET_TREE=None")
        return

    emit(f"WIDGET_TREE_CLASS={widget_tree.get_class().get_name()}")
    try:
        widgets = list(widget_tree.get_all_widgets())
    except Exception as exc:
        emit(f"WIDGET_TREE_ERROR={exc}")
        return

    emit(f"WIDGET_COUNT={len(widgets)}")
    for widget in widgets:
        widget_name = widget.get_name()
        class_name = widget.get_class().get_name()
        slot = try_prop(widget, "slot")
        emit(f"WIDGET name={widget_name} class={class_name} path={obj_path(widget)} slot={slot.get_class().get_name() if slot else 'None'}")


widget_asset = unreal.EditorAssetLibrary.load_asset(ASSET_PATH)
if not widget_asset:
    raise RuntimeError(f"Failed to load widget asset: {ASSET_PATH}")

emit(f"ASSET_CLASS={widget_asset.get_class().get_name()}")
emit(f"ASSET_PATH={obj_path(widget_asset)}")

generated_class = unreal.load_object(None, ASSET_PATH + ".ConvertSpeed_C")
emit(f"GENERATED_CLASS={obj_path(generated_class)}")

widget_tree = try_prop(widget_asset, "widget_tree")
print_widget_tree(widget_tree)

emit(f"HAS_EDITOR_UTILITY_LIBRARY={hasattr(unreal, 'EditorUtilityLibrary')}")
editor_utility_library = getattr(unreal, "EditorUtilityLibrary", None)
if editor_utility_library:
    emit(f"HAS_FIND_SOURCE_WIDGET={hasattr(editor_utility_library, 'find_source_widget_by_name')}")
    emit(f"HAS_CAST_TO_WIDGET_BLUEPRINT={hasattr(editor_utility_library, 'cast_to_widget_blueprint')}")

named_widgets = [
    "Button_Decrement_Forward",
    "Button_Decrement_Reverse",
    "Button_Increment_Forward",
    "Button_Increment_Reverse",
    "SpinBox_MaxForwardSpeed",
    "SpinBox_MaxReverseSpeed",
    "CommonNumericTextBlock_Forward",
    "CommonNumericTextBlock_Reverse",
    "CommonNumericTextBlock_Total",
]

for name in named_widgets:
    widget = None
    if widget_tree:
        try:
            widget = widget_tree.find_widget(name)
        except Exception:
            widget = None
    emit(f"NAMED_WIDGET name={name} class={widget.get_class().get_name() if widget else 'None'} path={obj_path(widget)}")

    source_widget = None
    if editor_utility_library and hasattr(editor_utility_library, "find_source_widget_by_name"):
        try:
            source_widget = editor_utility_library.find_source_widget_by_name(widget_asset, name)
        except Exception as exc:
            emit(f"SOURCE_WIDGET_ERROR name={name} error={exc}")
    emit(f"SOURCE_WIDGET name={name} class={source_widget.get_class().get_name() if source_widget else 'None'} path={obj_path(source_widget)}")
    if source_widget:
        for prop_name in ("value", "current_value", "current_numeric_value", "max_value", "max_slider_value", "min_value", "min_slider_value"):
            value = try_prop(source_widget, prop_name)
            if value is not None:
                emit(f"SOURCE_WIDGET_PROP name={name} prop={prop_name} value={value}")

zhoenus_button_class = unreal.load_object(None, ZHOENUS_BUTTON_PATH)
emit(f"ZHOENUS_BUTTON_CLASS={obj_path(zhoenus_button_class)}")

if generated_class:
    cdo = unreal.get_default_object(generated_class)
    emit(f"CDO={obj_path(cdo)}")
    for name in named_widgets:
        prop_name = name.lower()
        widget = try_prop(cdo, prop_name)
        if not widget:
            widget = try_prop(cdo, name)
        emit(f"CDO_WIDGET name={name} class={widget.get_class().get_name() if widget else 'None'} path={obj_path(widget)}")

with open(REPORT_PATH, "w", encoding="utf-8") as report:
    report.write("\n".join(LINES) + "\n")
