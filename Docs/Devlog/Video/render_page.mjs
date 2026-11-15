// Render an HTML page to MP4, frame by frame, using headless Edge's virtual time.
// No npm packages: Chrome DevTools Protocol over Node's built-in WebSocket.
//
//   node render_page.mjs <url> <out.mp4> <seconds> [--fps 30] [--w 1920] [--h 1080] [--keys "1.5:Space,4:Space"]
//
// Timers, requestAnimationFrame and CSS animations all advance in virtual time, so the
// output is deterministic and independent of machine speed. Any console.log line that
// starts with "EVT " is recorded with its virtual time into <out>.events.json (used to
// lay keyboard clicks under the cold open).
import { spawn } from 'node:child_process';
import { writeFileSync, mkdtempSync } from 'node:fs';
import { tmpdir } from 'node:os';
import { join } from 'node:path';

const args = process.argv.slice(2);
const opt = (name, def) => { const i = args.indexOf('--' + name); return i >= 0 ? args[i + 1] : def; };
const [url, out, secondsArg] = args;
if (!url || !out || !secondsArg) { console.error('usage: node render_page.mjs <url> <out.mp4> <seconds> [--fps 30] [--w 1920] [--h 1080] [--keys "t:Key,..."]'); process.exit(2); }
const seconds = +secondsArg, fps = +opt('fps', 30), W = +opt('w', 1920), H = +opt('h', 1080);
const keys = (opt('keys', '') || '').split(',').filter(Boolean).map(s => { const [t, k] = s.split(':'); return { t: +t, key: k }; });
const EDGE = process.env.EDGE || 'C:/Program Files (x86)/Microsoft/Edge/Application/msedge.exe';
const port = 9300 + Math.floor(Math.random() * 500);

const profile = mkdtempSync(join(tmpdir(), 'render-edge-'));
const edge = spawn(EDGE, ['--headless=new', '--disable-gpu', '--hide-scrollbars', '--mute-audio', '--no-first-run',
  `--remote-debugging-port=${port}`, `--user-data-dir=${profile}`, `--window-size=${W},${H}`, 'about:blank'], { stdio: 'ignore' });

async function getWs() {
  for (let i = 0; i < 100; i++) {
    try { const r = await fetch(`http://127.0.0.1:${port}/json`); const t = (await r.json()).find(x => x.type === 'page'); if (t) return t.webSocketDebuggerUrl; } catch {}
    await new Promise(r => setTimeout(r, 100));
  }
  throw new Error('Edge did not start');
}
const ws = new WebSocket(await getWs());
await new Promise(r => ws.addEventListener('open', r, { once: true }));
let nextId = 1; const pending = new Map(); const listeners = [];
ws.addEventListener('message', ev => {
  const m = JSON.parse(ev.data);
  if (m.id && pending.has(m.id)) { const p = pending.get(m.id); pending.delete(m.id); m.error ? p.rej(new Error(JSON.stringify(m.error))) : p.res(m.result); }
  else listeners.forEach(fn => fn(m));
});
const send = (method, params = {}) => new Promise((res, rej) => { const id = nextId++; pending.set(id, { res, rej }); ws.send(JSON.stringify({ id, method, params })); });
const once = (method) => new Promise(res => { const fn = m => { if (m.method === method) { listeners.splice(listeners.indexOf(fn), 1); res(m.params); } }; listeners.push(fn); });

let vnow = 0; const events = [];
listeners.push(m => {
  if (m.method === 'Runtime.consoleAPICalled') {
    const text = (m.params.args || []).map(a => a.value).join(' ');
    if (text.startsWith('EVT ')) events.push({ t: +vnow.toFixed(4), evt: text.slice(4) });
  }
});

await send('Page.enable'); await send('Runtime.enable');
await send('Emulation.setDeviceMetricsOverride', { width: W, height: H, deviceScaleFactor: 1, mobile: false });
// Pause virtual time before navigating so the page's clock starts at zero with the first frame.
await send('Emulation.setVirtualTimePolicy', { policy: 'pause' });
const loaded = once('Page.loadEventFired');
await send('Page.navigate', { url });
await send('Emulation.setVirtualTimePolicy', { policy: 'pauseIfNetworkFetchesPending', budget: 50 });
await loaded;

const ff = spawn('ffmpeg', ['-v', 'error', '-y', '-f', 'image2pipe', '-framerate', String(fps), '-i', '-',
  '-c:v', 'libx264', '-pix_fmt', 'yuv420p', '-crf', '18', '-preset', 'medium', '-r', String(fps), out], { stdio: ['pipe', 'inherit', 'inherit'] });

const frames = Math.round(seconds * fps), step = 1000 / fps;
let k = 0;
for (let f = 0; f < frames; f++) {
  vnow = f / fps;
  while (k < keys.length && keys[k].t <= vnow) {
    const key = keys[k++].key, code = key === 'Space' ? 'Space' : key;
    const keyText = key === 'Space' ? ' ' : key;
    await send('Input.dispatchKeyEvent', { type: 'keyDown', key: keyText, code, windowsVirtualKeyCode: key === 'Space' ? 32 : keyText.toUpperCase().charCodeAt(0) });
    await send('Input.dispatchKeyEvent', { type: 'keyUp', key: keyText, code, windowsVirtualKeyCode: key === 'Space' ? 32 : keyText.toUpperCase().charCodeAt(0) });
  }
  const shot = await send('Page.captureScreenshot', { format: 'png', fromSurface: true, captureBeyondViewport: false });
  if (!ff.stdin.write(Buffer.from(shot.data, 'base64'))) await new Promise(r => ff.stdin.once('drain', r));
  const advanced = once('Emulation.virtualTimeBudgetExpired');
  await send('Emulation.setVirtualTimePolicy', { policy: 'advance', budget: step });
  await advanced;
  if (f % (fps * 5) === 0) process.stdout.write(`\r${(f / fps).toFixed(0)}s / ${seconds}s`);
}
ff.stdin.end();
await new Promise(r => ff.on('close', r));
writeFileSync(out.replace(/\.mp4$/i, '') + '.events.json', JSON.stringify(events, null, 1));
console.log(`\nwrote ${out} (${frames} frames, ${events.length} events)`);
ws.close(); edge.kill();
process.exit(0);
