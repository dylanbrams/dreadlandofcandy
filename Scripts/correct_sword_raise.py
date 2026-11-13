"""Wrist-only native SwordRaise correction. JAM_GRIP_APPLY=1 saves; default inspects.

The fixed (0,0,-90) sword grip maps blade +Z onto hand_r -Y. Correct the
hand track so both turn together. Other bone tracks and timing stay intact.
"""
import json
import math
import os
from pathlib import Path
import shutil
import unreal

ROOT = Path(unreal.Paths.project_dir()).resolve()
OUT = ROOT / 'ImportSource/RigCorrections'
OUT.mkdir(parents=True, exist_ok=True)
DEFAULT_PATH = '/Game/Art/Hero/Anims/A_SK_Hero_Rig_SwordRaise'


def mul(a, b):
    x, y, z, w = a
    X, Y, Z, W = b
    return [w*X+x*W+y*Z-z*Y, w*Y-x*Z+y*W+z*X,
            w*Z+x*Y-y*X+z*W, w*W-x*X-y*Y-z*Z]


def unit(v):
    length = math.sqrt(sum(x*x for x in v))
    return [x/length for x in v]


def rotate(q, v):
    return mul(mul(q, list(v)+[0.0]), [-q[0], -q[1], -q[2], q[3]])[:3]


def slerp(a, b, amount):
    dot = sum(x*y for x, y in zip(a, b))
    if dot < 0:
        b, dot = [-x for x in b], -dot
    dot = min(1.0, dot)
    if dot > 0.9995:
        return unit([x+(y-x)*amount for x, y in zip(a, b)])
    angle = math.acos(dot)
    return [(math.sin((1-amount)*angle)*x+math.sin(amount*angle)*y)/math.sin(angle)
            for x, y in zip(a, b)]


def serialize(transform):
    p, q, s = transform.translation, transform.rotation, transform.scale3d
    return {'p': [p.x, p.y, p.z], 'q': [q.x, q.y, q.z, q.w], 's': [s.x, s.y, s.z]}


def sample(sequence, times, bones):
    options = unreal.AnimPoseEvaluationOptions()
    options.set_editor_property('evaluation_type', unreal.AnimDataEvalType.SOURCE)
    options.set_editor_property('should_retarget', False)
    samples = []
    for time in times:
        pose = unreal.AnimPoseExtensions.get_anim_pose_at_time(sequence, time, options)
        local = {str(b): serialize(unreal.AnimPoseExtensions.get_bone_pose(
                 pose, b, unreal.AnimPoseSpaces.LOCAL)) for b in bones}
        world = {b: serialize(unreal.AnimPoseExtensions.get_bone_pose(
                 pose, b, unreal.AnimPoseSpaces.WORLD)) for b in ('hand_r', 'lowerarm_r')}
        samples.append({'time': time, 'local': local, 'world': world})
    return samples


def correct(asset_path=DEFAULT_PATH, apply=False, refresh_source=False):
    sequence = unreal.load_asset(asset_path)
    if not sequence:
        raise RuntimeError('Missing SwordRaise: '+asset_path)
    model = sequence.get_editor_property('data_model_interface')
    controller = sequence.get_editor_property('controller')
    rate = model.get_frame_rate()
    fps = rate.numerator/rate.denominator
    times = [i/fps for i in range(model.get_number_of_frames()+1)]
    bones = list(model.get_bone_track_names())
    name = asset_path.rsplit('/', 1)[-1]
    backup_path = OUT/(name+'_original_keys.json')
    if backup_path.exists() and not refresh_source:
        original = json.loads(backup_path.read_text())
        assert original['asset'] == asset_path and original['times'] == times
        samples = original['samples']
    else:
        if backup_path.exists():
            shutil.copy2(backup_path, OUT/(name+'_previous_original_keys.json'))
        samples = sample(sequence, times, bones)
        original = {'asset': asset_path, 'fps': fps, 'times': times, 'samples': samples}
        backup_path.write_text(json.dumps(original), encoding='utf-8')
    rotations, before_angles = [], []
    for frame in samples:
        world = frame['world']['hand_r']['q']
        axis = unit(rotate(world, (0, -1, 0)))
        before_angles.append(math.degrees(math.acos(max(-1.0, min(1.0, axis[2])))))
        delta = ([1.0, 0.0, 0.0, 0.0] if axis[2] < -0.999999
                 else unit([axis[1], -axis[0], 0.0, 1.0+axis[2]]))
        parent = frame['world']['lowerarm_r']['q']
        inverse = [-parent[0], -parent[1], -parent[2], parent[3]]
        target = unit(mul(inverse, mul(delta, world)))
        amount = max(0.0, min(1.0, (frame['time']-0.15)/0.5))
        amount = amount*amount*(3-2*amount)
        rotations.append(slerp(frame['local']['hand_r']['q'], target, amount))
    report = {'asset': asset_path, 'fps': fps, 'keys': len(times), 'changed_tracks': ['hand_r'],
              'before_hold_max_tilt_degrees': max(a for a, t in zip(before_angles, times) if t >= 0.65),
              'applied': apply}
    if apply:
        package = ROOT/'Content'/(asset_path[len('/Game/'):]+'.uasset')
        package_backup = OUT/(name+'_original.uasset')
        if not package_backup.exists():
            shutil.copy2(package, package_backup)
        controller.open_bracket('Align sword raise wrist with fixed hand grip', False)
        try:
            positions = [unreal.Vector(*f['local']['hand_r']['p']) for f in samples]
            scales = [unreal.Vector(*f['local']['hand_r']['s']) for f in samples]
            assert controller.set_bone_track_keys('hand_r', positions,
                [unreal.Quat(*q) for q in rotations], scales, False)
        finally:
            controller.close_bracket(False)
        after = sample(sequence, times, bones)
        other_error, tilts = 0.0, []
        for old, new in zip(samples, after):
            for bone in old['local']:
                fields = ('p', 's') if bone == 'hand_r' else ('p', 'q', 's')
                for field in fields:
                    other_error = max(other_error, max(abs(a-b) for a, b in zip(
                                      old['local'][bone][field], new['local'][bone][field])))
            if new['time'] >= 0.65:
                axis = unit(rotate(new['world']['hand_r']['q'], (0, -1, 0)))
                tilts.append(math.degrees(math.acos(max(-1.0, min(1.0, axis[2])))))
        report['unchanged_track_max_error'] = other_error
        report['after_hold_max_tilt_degrees'] = max(tilts)
        assert other_error < 0.0001, report
        assert max(tilts) < 0.1, report
        assert unreal.EditorAssetLibrary.save_loaded_asset(sequence, only_if_is_dirty=False)
    (OUT/(name+'_validation.json')).write_text(json.dumps(report, indent=2))
    unreal.log('SwordGrip: '+json.dumps(report))
    return report


if __name__ == '__main__':
    correct(apply=os.environ.get('JAM_GRIP_APPLY') == '1')
