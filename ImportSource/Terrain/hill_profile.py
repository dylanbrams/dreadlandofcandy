"""Molar Hills uplift in centimetres; roads climb and reserved pads stay level."""
import math


def smooth(a, b, x):
    t = max(0., min(1., (x-a)/(b-a)))
    return t*t*(3-2*t)


def raw_hill(x, y):
    radius = math.hypot(x, y)
    angle = abs((math.degrees(math.atan2(y, x))-120+180)%360-180)
    return 900 * smooth(2600, 5200, radius) * (1-smooth(40, 65, angle))


def elevation(x, y, locations):
    value = raw_hill(x, y)
    for row in locations:
        if row['kind'] == 'Village':
            continue
        radius = row['radius'] * (math.sqrt(2) if row['kind'] == 'FilmSet' else 1)
        d = math.hypot(x-row['x'], y-row['y'])
        blend = 1-smooth(radius+250, radius+900, d)
        if blend:
            value += (raw_hill(row['x'], row['y'])-value)*blend
    return value * smooth(2850, 3450, math.hypot(x, y))


def mesh_elevation(x, y, locations):
    # Same diagonal as Mesh.face([a,a+33,a+34,a+1]); align foliage to the
    # actual triangulated surface rather than the smooth source function.
    gx, gy = math.floor(x/250)*250, math.floor(y/250)*250
    fx, fy = (x-gx)/250, (y-gy)/250
    a, b, c, d = [elevation(px, py, locations) for px, py in
                  [(gx,gy),(gx+250,gy),(gx+250,gy+250),(gx,gy+250)]]
    return a+(b-a)*fx+(c-b)*fy if fx >= fy else a+(c-d)*fx+(d-a)*fy
