import unreal


REPORT_PATH = "/Volumes/Micro-Data-Transfer/Zhoenus/zhoenus/Saved/editor_utility_widget_methods.txt"


methods = []
for name in dir(unreal.EditorUtilityLibrary):
    if "widget" in name.lower() or "blueprint" in name.lower():
        methods.append(name)

methods.sort()

with open(REPORT_PATH, "w", encoding="utf-8") as report:
    for method in methods:
        unreal.log_warning(method)
        report.write(method + "\n")
