export function init()
{
	document.getElementById("reaper_flying-status").innerText = "initialized";
	document.getElementById("reaper_flying-status").style.backgroundColor = "";
  document.getElementById("reaper_flying-btn-play").disabled = true;
  document.getElementById("reaper_flying-btn-stop").disabled = true;
  document.getElementById("volume-slider").disabled = true;
  document.getElementById("reaper_flying-btn-rewind").disabled = true;

  document.getElementById('reaper_flying-stop').addEventListener('updated', (e) => 
  {
    document.getElementById("reaper_flying-btn-play").disabled = false;
    document.getElementById("reaper_flying-btn-stop").disabled = false;
    document.getElementById("volume-slider").disabled = false;
    document.getElementById("reaper_flying-btn-rewind").disabled = false;
  });			

  document.getElementById('reaper_flying-track_7_vu').addEventListener('updated', (e) => 
  {
    const vu = document.getElementById('reaper_flying-track_7_vu');
    let vudB = 0;
    if(vu.innerText != "0")
      vudB = vu.innerText*100; // if vu linear: Math.log10(vu.innerText)*20+100; 
    document.getElementById('reaper_flying-Play_VU-bar').style.height = vudB + '%';
    document.getElementById('reaper_flying-Play_VU-number').innerText = (vudB).toFixed(1) + ' dB';
  });			

  document.getElementById('reaper_flying-time').addEventListener('updated', (e) => 
  {
    const dur = document.getElementById('reaper_flying-time').innerText;
    document.getElementById('timecode').innerText = Math.round(dur / 60) + ":" + (dur % 60).toFixed(1);
  });

}
