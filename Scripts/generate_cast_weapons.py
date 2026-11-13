"""Resume three textured cast props through Meshy's existing project client."""
import json
from pathlib import Path
import sys
from concurrent.futures import ThreadPoolExecutor

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "Scripts"))
import genlib as g
import meshy

PROPS = {
    "SM_MageStaff": "one complete dark fantasy battle mage staff, long straight charcoal iron shaft, dark leather handgrip at middle, large faceted pale mint crystal securely held by a simple bronze claw at the top, blunt iron bottom cap, full object visible from tip to bottom, vertical shaft along Z, no hands, no character, no stand, no floating parts, no lettering, strong readable silhouette",
    "SM_ArcherBow": "one complete dark fantasy recurved longbow, symmetrical curved blackened wood limbs with dull bronze reinforcement, central dark leather handgrip, one taut straight bowstring connecting both tips, full bow visible, long axis vertical Z, bowstring and limbs lie in XZ plane, no arrow, no quiver, no person, no hand, no stand, no lettering, thick sturdy readable shape",
    "SM_BarbarianRollingPin": "one enormous complete wooden rolling pin wielded as a barbarian weapon, stout straight cylindrical dark warm brown oak barrel, two short thick wooden handles emerging from opposite ends on the same axis, smooth recognisable baking utensil silhouette, simple dull iron collar bands at the ends of the barrel, long axis vertical Z, full object from end to end, no spikes, no blades, no hand, no character, no stand, no lettering",
}


def run(item):
    name, subject = item
    dest = ROOT / f"ImportSource/Meshy/{name}.glb"
    state_path = ROOT / f"ImportSource/_gen/cast_weapons/{name}.json"
    state_path.parent.mkdir(parents=True, exist_ok=True)
    state = json.loads(state_path.read_text()) if state_path.exists() else {}
    def save():
        state_path.write_text(json.dumps(state, indent=2), encoding="utf-8")
    if state.get("done") and dest.exists():
        return
    key = g.env_value("MESHY_API_KEY")
    prompt = g.apply_style(subject, "equipment")
    assert len(prompt) <= 800
    for stage in ("preview", "refine"):
        if not state.get(stage):
            body = {"mode": "preview", "prompt": prompt, "ai_model": "latest",
                    "should_remesh": True, "target_polycount": 6000} if stage == "preview" else {
                        "mode": "refine", "preview_task_id": state["preview"], "enable_pbr": True}
            state[stage] = g.request("POST", meshy.BASE, key, json_body=body)["result"]
            save()
        task = meshy.wait(state[stage], key, 1800)
        state[stage + "_credits"] = task.get("consumed_credits", 0)
        save()
    g.safe_download(task["model_urls"]["glb"], dest, meshy.HOSTS)
    if task.get("thumbnail_url"):
        g.safe_download(task["thumbnail_url"], dest.with_suffix(".png"), meshy.HOSTS)
    g.log_manifest("Meshy", {"file": dest.name, "prompt": prompt, "preview_task": state["preview"],
                           "refine_task": state["refine"], "textured": True,
                           "credits": sum(state.get(s + "_credits", 0) or 0 for s in ("preview", "refine"))})
    state["done"] = True
    save()
    print("Delivered", name, flush=True)


if __name__ == "__main__":
    with ThreadPoolExecutor(max_workers=3) as executor:
        list(executor.map(run, PROPS.items()))
