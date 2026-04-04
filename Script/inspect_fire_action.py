import unreal


SHIP_BLUEPRINT_CLASS_PATH = "/Game/Blueprints/Ships/ZhoenusSpaceShip.ZhoenusSpaceShip_C"


def obj_path(obj):
    return obj.get_path_name() if obj else "None"


def key_label(key):
    if not key:
        return "None"
    try:
        return key.get_display_name().to_string()
    except Exception:
        try:
            return str(key.get_editor_property("key_name"))
        except Exception:
            return str(key)


generated_class = unreal.load_object(None, SHIP_BLUEPRINT_CLASS_PATH)
if not generated_class:
    raise RuntimeError(f"Failed to load generated class: {SHIP_BLUEPRINT_CLASS_PATH}")

default_object = unreal.get_default_object(generated_class)
if not default_object:
    raise RuntimeError("Failed to get default object for ship blueprint")

mapping_context = default_object.get_editor_property("ship_input_mapping_context")
fire_action = default_object.get_editor_property("fire_action")

print(f"SHIP_CDO={obj_path(default_object)}")
print(f"MAPPING_CONTEXT={obj_path(mapping_context)}")
print(f"FIRE_ACTION={obj_path(fire_action)}")

if not mapping_context:
    raise RuntimeError("ShipInputMappingContext is not assigned on the ship CDO")

default_key_mappings = mapping_context.get_editor_property("default_key_mappings")
if not default_key_mappings:
    raise RuntimeError("DefaultKeyMappings is not available on the mapping context")

mappings = list(default_key_mappings.get_editor_property("mappings"))
print(f"MAPPING_COUNT={len(mappings)}")

fire_bindings = []
for mapping in mappings:
    action = mapping.get_editor_property("action")
    key = mapping.get_editor_property("key")
    action_path = obj_path(action)
    key_name = key_label(key)
    print(f"MAPPING action={action_path} key={key_name}")
    if fire_action and action_path == obj_path(fire_action):
        fire_bindings.append(key_name)

if fire_bindings:
    print("FIRE_BINDINGS=" + ",".join(fire_bindings))
else:
    print("FIRE_BINDINGS=<none>")
