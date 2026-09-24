const directButtonIds = [
  'btn-direct-select-DANTE_CAVEPC',
  'btn-direct-select-DANTE_CurvedLEDPC',
  'btn-direct-select-Curved_Wall_Bluetooth_Stereo',
  'btn-direct-select-DANTE_Mobile'
];

export function init()
{
  // Inject input controls and disabled buttons (same inputs as Curved_LED_Stereo)
  renderInputControls('input-buttons-container', 'input-group-container', 'Telco_Stereo', 
    ["DANTE_CurvedLEDPC", "DANTE_CurvedLEDPC_Channel_3", "DANTE_Mobile", "Mic_Array", 
      "DANTE_HDMI_Stereo", // to be setup later
    ]);
  enableInputSelectButtons(true);
  enableDirectButtons(true);
  clearActiveInputs();
  GetStatus();

  // Curved PA gain listener
  document.getElementById('sum_bus_CurvedPA-gain').addEventListener('updated', (e) => 
  {
    const vol = document.getElementById('sum_bus_CurvedPA-gain');
    document.getElementById('sum_bus_CurvedPA-volume-slider').value = Math.round(vol.innerText);
    document.getElementById('sum_bus_CurvedPA-volume-number').innerText = Math.round(vol.innerText) + ' dB';
  });	
    
  // Curved PA mute listener
  document.getElementById('btn-sum_bus_CurvedPA-mute').addEventListener('updated', (e) => 
  {
    const btn = document.getElementById('btn-sum_bus_CurvedPA-mute');
    const isActive = btn.classList.contains('active-input');
    const slider = document.getElementById('sum_bus_CurvedPA-volume-slider');
    if (isActive) {
      slider.style.opacity = "1.0";
    } else {
      slider.style.opacity = "0.5";
    }  
  });

  // CAVE PA gain listener
  document.getElementById('sum_bus_CAVEPA-gain').addEventListener('updated', (e) => 
  {
    const vol = document.getElementById('sum_bus_CAVEPA-gain');
    document.getElementById('sum_bus_CAVEPA-volume-slider').value = Math.round(vol.innerText);
    document.getElementById('sum_bus_CAVEPA-volume-number').innerText = Math.round(vol.innerText) + ' dB';
  });

  // CAVE PA mute listener
  document.getElementById('btn-sum_bus_CAVEPA-mute').addEventListener('updated', (e) => 
  {
    const btn = document.getElementById('btn-sum_bus_CAVEPA-mute');
    const isActive = btn.classList.contains('active-input');
    const slider = document.getElementById('sum_bus_CAVEPA-volume-slider');
    if (isActive) {
      slider.style.opacity = "1.0";
    } else {
      slider.style.opacity = "0.5";
    }  
  });
}

export function setActiveInput(buttonId)
{
  directButtonIds.forEach(id => {
    const btn = document.getElementById(id);
    if (btn) {
      if (id === buttonId) {
        btn.classList.add('active-input');
      } else {
        btn.classList.remove('active-input');
      }
    }
  });
}

export function clearActiveInputs()
{
  directButtonIds.forEach(id => {
    const btn = document.getElementById(id);
    if (btn) btn.classList.remove('active-input');
  });
}

function enableDirectButtons(enable = true)
{
  directButtonIds.forEach(id => {
    const btn = document.getElementById(id);
    if (btn) btn.disabled = !enable;
  });
}

export function SetCavePc()
{
  setActiveInput('btn-direct-select-DANTE_CAVEPC');
  sendValue('/matrix/state/settings/easy_routing/400', 256);
  sendValue('/matrix/state/settings/easy_routing/401', 257);
  sendValue('/matrix/state/settings/easy_routing/402', 258);
  sendValue('/matrix/state/settings/easy_routing/403', 259);
  sendValue('/matrix/state/settings/easy_routing/404', 260);
  sendValue('/matrix/state/settings/easy_routing/405', 261);
  sendValue('/matrix/state/settings/easy_routing/406', 262);
  sendValue('/matrix/state/settings/easy_routing/407', 263);
}

export function SetCurvedLedPc()
{
  setActiveInput('btn-direct-select-DANTE_CurvedLEDPC');
  sendValue('/matrix/state/settings/easy_routing/400', 128);
  sendValue('/matrix/state/settings/easy_routing/401', 129);
  sendValue('/matrix/state/settings/easy_routing/402', 130);
  sendValue('/matrix/state/settings/easy_routing/403', 131);
  sendValue('/matrix/state/settings/easy_routing/404', 132);
  sendValue('/matrix/state/settings/easy_routing/405', 133);
  sendValue('/matrix/state/settings/easy_routing/406', 134);
  sendValue('/matrix/state/settings/easy_routing/407', 135);
}

export function SetCurvedBluetooth()
{
  setActiveInput('btn-direct-select-Curved_Wall_Bluetooth_Stereo');
  sendValue('/matrix/state/settings/easy_routing/400', 4136);
  sendValue('/matrix/state/settings/easy_routing/401', 4141);
  sendValue('/matrix/state/settings/easy_routing/402', 314); // BT L to FWL
  sendValue('/matrix/state/settings/easy_routing/403', 315); // BT R to FWR
  sendValue('/matrix/state/settings/easy_routing/404', 4156);
  sendValue('/matrix/state/settings/easy_routing/405', 4161);
  sendValue('/matrix/state/settings/easy_routing/406', 312); // BT Aux L to BL
  sendValue('/matrix/state/settings/easy_routing/407', 313); // BT Aux R to BR
}

export function SetDanteMobile()
{
  setActiveInput('btn-direct-select-DANTE_Mobile');
  sendValue('/matrix/state/settings/easy_routing/400', 160);
  sendValue('/matrix/state/settings/easy_routing/401', 161);
  sendValue('/matrix/state/settings/easy_routing/402', 162);
  sendValue('/matrix/state/settings/easy_routing/403', 163);
  sendValue('/matrix/state/settings/easy_routing/404', 164);
  sendValue('/matrix/state/settings/easy_routing/405', 165);
  sendValue('/matrix/state/settings/easy_routing/406', 166);
  sendValue('/matrix/state/settings/easy_routing/407', 167);
}

export function ResetDefaultRouting()
{
  sendNoArgs('/matrix/cmd/recall_snapshot_5');
  clearActiveInputs();
}

export function setVolumeCurvedPA(value)
{
  sendValue('/matrix/state/settings/sum_bus_master/0/gain', value);
  sendValue('/matrix/state/settings/sum_bus_master/1/gain', value);
  document.getElementById('sum_bus_CurvedPA-volume-number').innerText = value + ' dB';
}

export function toggleSumBusCurvedPAMute(state, btn_disable, slider_disable = false)
{
  const btn = document.getElementById('btn-sum_bus_CurvedPA-mute');
  const slider = document.getElementById('sum_bus_CurvedPA-volume-slider');
  if (!btn || !slider) return;

  const isActive = btn.classList.contains('active-input');
  const turnOn = state === undefined ? !isActive : state;
  if (turnOn) {
    btn.classList.add('active-input');
    btn.innerText = "On";
    btn.disabled = btn_disable;
    slider.disabled = slider_disable;
    slider.style.opacity = "1.0";
    sendValue('/matrix/state/settings/sum_bus_master/0/mute', 0);
    sendValue('/matrix/state/settings/sum_bus_master/1/mute', 0);
  } else {
    btn.classList.remove('active-input');
    btn.innerText = "Off";
    btn.disabled = btn_disable;
    slider.disabled = slider_disable;
    slider.style.opacity = "0.5";
    sendValue('/matrix/state/settings/sum_bus_master/0/mute', 1);
    sendValue('/matrix/state/settings/sum_bus_master/1/mute', 1);
  }
}

export function toggleSumBusPAMute(state, btn_disable, slider_disable = false)
{
  toggleSumBusCurvedPAMute(state, btn_disable, slider_disable);
}

export function setVolumeCavePA(value)
{
  sendValue('/matrix/state/settings/sum_bus_master/8/gain', value);
  sendValue('/matrix/state/settings/sum_bus_master/9/gain', value);
  sendValue('/matrix/state/settings/sum_bus_master/10/gain', value);
  sendValue('/matrix/state/settings/sum_bus_master/11/gain', value);
  sendValue('/matrix/state/settings/sum_bus_master/12/gain', value);
  sendValue('/matrix/state/settings/sum_bus_master/13/gain', value);
  sendValue('/matrix/state/settings/sum_bus_master/14/gain', value);
  sendValue('/matrix/state/settings/sum_bus_master/15/gain', value);
  document.getElementById('sum_bus_CAVEPA-volume-number').innerText = value + ' dB';
}

export function toggleStateCAVEPA(state, btn_disable, slider_disable = false)
{
  const btn = document.getElementById('btn-sum_bus_CAVEPA-mute');
  const slider = document.getElementById('sum_bus_CAVEPA-volume-slider');
  if (!btn || !slider) return;

  const isActive = btn.classList.contains('active-input');
  const turnOn = state === undefined ? !isActive : state;
  if (turnOn) {
    btn.classList.add('active-input');
    btn.innerText = "On";
    btn.disabled = btn_disable;
    slider.disabled = slider_disable;
    slider.style.opacity = "1.0";
    sendValue('/matrix/state/settings/sum_bus_master/8/mute', 0);
    sendValue('/matrix/state/settings/sum_bus_master/9/mute', 0);
    sendValue('/matrix/state/settings/sum_bus_master/10/mute', 0);
    sendValue('/matrix/state/settings/sum_bus_master/11/mute', 0);
    sendValue('/matrix/state/settings/sum_bus_master/12/mute', 0);
    sendValue('/matrix/state/settings/sum_bus_master/13/mute', 0);
    sendValue('/matrix/state/settings/sum_bus_master/14/mute', 0);
    sendValue('/matrix/state/settings/sum_bus_master/15/mute', 0);
  } else {
    btn.classList.remove('active-input');
    btn.innerText = "Off";
    btn.disabled = btn_disable;
    slider.disabled = slider_disable;
    slider.style.opacity = "0.5";
    sendValue('/matrix/state/settings/sum_bus_master/8/mute', 1);
    sendValue('/matrix/state/settings/sum_bus_master/9/mute', 1);
    sendValue('/matrix/state/settings/sum_bus_master/10/mute', 1);
    sendValue('/matrix/state/settings/sum_bus_master/11/mute', 1);
    sendValue('/matrix/state/settings/sum_bus_master/12/mute', 1);
    sendValue('/matrix/state/settings/sum_bus_master/13/mute', 1);
    sendValue('/matrix/state/settings/sum_bus_master/14/mute', 1);
    sendValue('/matrix/state/settings/sum_bus_master/15/mute', 1);
  }
}

export function GetStatus()
{
  sendNoArgs('/matrix/state/settings/flex_channel/*/mute'); // get all mute states
  sendNoArgs('/matrix/state/settings/flex_channel/*/gain'); // get all gain values
  sendNoArgs('/matrix/state/settings/sum_bus_master/*/gain'); // get bus master gains
  sendNoArgs('/matrix/state/settings/sum_bus_master/*/mute'); // get bus master mutes
}

export function MuteAll()
{
  sendValue('/matrix/state/settings/flex_channel/*/mute', 1);
  enableInputSelectButtons(true);
  toggleSumBusCurvedPAMute(false);
  toggleStateCAVEPA(false);
  showInputSection('none'); // Hide all input sections
}

export function EasyRouting()
{
  sendNoArgs('/matrix/state/settings/easy_routing/*');
}
