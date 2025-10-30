import json, subprocess, sys, re
from SCons.Script import Import

Import("env")

hint = env.GetProjectOption("custom_port_hint") or ""
if not hint:
    raise SystemExit("Missing 'custom_port_hint' in platformio.ini")

try:
    out = subprocess.check_output(
        ["platformio", "device", "list", "--json-output"],
        universal_newlines=True
    )
    ports = json.loads(out)
except Exception as e:
    print("Auto-port: failed to list devices:", e)
    sys.exit(1)

target_desc = re.compile(rf"WCH.*CH\s*{re.escape(hint)}", re.IGNORECASE)
chosen = None
for p in ports:
    desc = (p.get("description") or "") + " " + (p.get("hwid") or "")
    if target_desc.search(desc):
        chosen = p.get("port")
        break

if not chosen:
    print(f"Auto-port: no port found for hint '{hint}'. "
          f"Plug the CH348 and refresh.")
    print(f"Continue the build but upload may fail..")
else:
    # Imposta per upload & monitor
    env.Replace(UPLOAD_PORT=chosen)
    env.Replace(MONITOR_PORT=chosen)
    print(f"Auto-port: {hint} -> {chosen}")