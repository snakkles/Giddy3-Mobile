// Continuous Web Audio output for the web build (called from webplat.c).
// The game mixes 16-bit stereo at srcRate in blocks of `frames`; this
// resamples linearly to the device rate across block boundaries.
Module.startAudio = function (srcRate, frames) {
    var AC = window.AudioContext || window.webkitAudioContext;
  if (!AC) return 0;
  var ctx = Module.gAudioCtx || (Module.gAudioCtx = new AC());
  
  var step = srcRate / ctx.sampleRate;
  var blk = null, pos = frames; // read position within current block (in frames)
  var prevL = 0, prevR = 0;      // last frame of previous block, for interpolation
  var node = ctx.createScriptProcessor(2048, 0, 2);
  node.onaudioprocess = function (e) {
    var L = e.outputBuffer.getChannelData(0), R = e.outputBuffer.getChannelData(1);
    if (Module.gAudioPaused) { L.fill(0); R.fill(0); return; }
    for (var i = 0; i < L.length; i++) {
      while (pos >= frames) {
        if (blk) { prevL = blk[(frames-1)*2]; prevR = blk[(frames-1)*2+1]; }
        var p = Module._web_audio_pull() >> 1;
        blk = HEAP16.subarray(p, p + frames*2).slice();
        pos -= frames;
      }
      var ip = Math.floor(pos), f = pos - ip, aL, aR;
      if (ip === 0) { aL = prevL; aR = prevR; } else { aL = blk[(ip-1)*2]; aR = blk[(ip-1)*2+1]; }
      // sample at ip is the "next" one; interpolate between ip-1 and ip
      var bL = blk[ip*2], bR = blk[ip*2+1];
      L[i] = (aL + (bL - aL) * f) / 32768;
      R[i] = (aR + (bR - aR) * f) / 32768;
      pos += step;
    }
  };
  node.connect(ctx.destination);
  Module.gAudioNode = node;
  var resume = function () { if (ctx.state !== 'running') ctx.resume(); };
  resume();
  ['touchstart','touchend','mousedown','keydown'].forEach(function (t) {
    document.addEventListener(t, resume, true);
  });
  return 1;
};
