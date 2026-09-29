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
  document.getElementById('output_gain_CurvedPA-gain').addEventListener('updated', (e) => 
  {
    const vol = document.getElementById('output_gain_CurvedPA-gain');
    document.getElementById('output_gain_CurvedPA-volume-slider').value = Math.round(vol.innerText);
    document.getElementById('output_gain_CurvedPA-volume-number').innerText = Math.round(vol.innerText) + ' dB';
  });	
    
  // CAVE PA gain listener
  document.getElementById('output_gain_CAVEPA-gain').addEventListener('updated', (e) => 
  {
    const vol = document.getElementById('output_gain_CAVEPA-gain');
    document.getElementById('output_gain_CAVEPA-volume-slider').value = Math.round(vol.innerText);
    document.getElementById('output_gain_CAVEPA-volume-number').innerText = Math.round(vol.innerText) + ' dB';
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

function updateInputButtonsAvailability(disabledInputId)
{
  const container = document.getElementById('input-buttons-container');
  if (!container) return;
  const buttons = container.querySelectorAll('button');
  buttons.forEach(btn => {
    btn.disabled = false;
  });

  if (disabledInputId) {
    const targetBtn = document.getElementById('btn-input-select-' + disabledInputId);
    if (targetBtn) {
      targetBtn.disabled = true;
      // If this input was active, mute it
      if (targetBtn.classList.contains('active-input')) {
        toggleInputState(disabledInputId, false);
      }
      // If this input's section was visible, hide it
      const section = document.getElementById('input-section-' + disabledInputId);
      if (section && section.style.display !== 'none') {
        section.style.display = 'none';
      }
    }
  }
}

function confirmRoutingReset(onConfirm)
{
  const dialog = document.getElementById('confirmDialog');
  if (!dialog) {
    onConfirm();
    return;
  }
  document.getElementById('confirmTitle').innerText = "Warning!";
  document.getElementById('confirmText').innerText = "This will reset the current routing! Confirm to continue...";
  dialog.showModal();

  document.getElementById('okBtn').onclick = () => {
    dialog.close();
    onConfirm();
  };

  document.getElementById('cancelBtn').onclick = () => {
    dialog.close();
  };
}

export function SetCurvedLedPc()
{
  confirmRoutingReset(async () => {
    await ResetDefaultRouting();
    setActiveInput('btn-direct-select-DANTE_CurvedLEDPC');
    updateInputButtonsAvailability('DANTE_CurvedLEDPC_Stereo');
    sendValue('/matrix/state/settings/output_gain/318', -100); // Mute Curved PA L
    sendValue('/matrix/state/settings/output_gain/319', -100); // Mute Curved PA R
    sendValue('/matrix/state/settings/output_gain/400', -100); // Mute CAVE PA FWL
    sendValue('/matrix/state/settings/output_gain/401', -100); // Mute CAVE PA FWR
    sendValue('/matrix/state/settings/easy_routing/318', 128); // Curved PA L from Curved PC L
    sendValue('/matrix/state/settings/easy_routing/319', 129); // Curved PA R from Curved PC R
    sendValue('/matrix/state/settings/easy_routing/400', 128); // CAVE PA FWL from Curved PC L
    sendValue('/matrix/state/settings/easy_routing/401', 129); // CAVE PA FWR from Curved PC R
    GetStatus();
    sendValue('/matrix/state/settings/sum_bus_master/0/mute', 0); // Unmute Summing Bus Curved PA L 
    sendValue('/matrix/state/settings/sum_bus_master/1/mute', 0); // Unmute Summing Bus Curved PA R*/
    lockScene('Telco_Stereo');
  });
}

export async function SetCavePc()
{
  confirmRoutingReset(async () => {
    await ResetDefaultRouting();
    setActiveInput('btn-direct-select-DANTE_CAVEPC');
    updateInputButtonsAvailability('DANTE_CAVEPC_Stereo');
    sendValue('/matrix/state/settings/output_gain/318', -100); // Mute Curved PA L
    sendValue('/matrix/state/settings/output_gain/319', -100); // Mute Curved PA R
    sendValue('/matrix/state/settings/output_gain/400', -100); // Mute CAVE PA FWL
    sendValue('/matrix/state/settings/output_gain/401', -100); // Mute CAVE PA FWR
    sendValue('/matrix/state/settings/easy_routing/318', 256); // Curved PA L from CAVE PC L
    sendValue('/matrix/state/settings/easy_routing/319', 257); // Curved PA R from CAVE PC R
    sendValue('/matrix/state/settings/easy_routing/400', 256); // CAVE PA FWL from CAVE PC L
    sendValue('/matrix/state/settings/easy_routing/401', 257); // CAVE PA FWR from CAVE PC R
    GetStatus();
    sendValue('/matrix/state/settings/sum_bus_master/8/mute', 0); // Unmute Summing Bus CAVE PA FWL 
    sendValue('/matrix/state/settings/sum_bus_master/9/mute', 0); // Unmute Summing Bus CAVE PA FWR
    lockScene('Telco_Stereo');
  });
}

export function SetCurvedBluetooth()
{
  confirmRoutingReset(async () => {
    await ResetDefaultRouting();
    setActiveInput('btn-direct-select-Curved_Wall_Bluetooth_Stereo');
    updateInputButtonsAvailability('Curved_Wall_Bluetooth_Stereo');
    sendValue('/matrix/state/settings/output_gain/318', -100); // Mute Curved PA L
    sendValue('/matrix/state/settings/output_gain/319', -100); // Mute Curved PA R
    sendValue('/matrix/state/settings/output_gain/400', -100); // Mute CAVE PA FWL
    sendValue('/matrix/state/settings/output_gain/401', -100); // Mute CAVE PA FWR
    sendValue('/matrix/state/settings/easy_routing/318', 314); // Curved PA L from CurvedWall BT L
    sendValue('/matrix/state/settings/easy_routing/319', 315); // Curved PA R from CurvedWall BT R
    sendValue('/matrix/state/settings/easy_routing/400', 314); // CAVE PA FWL from CurvedWall BT L
    sendValue('/matrix/state/settings/easy_routing/401', 315); // CAVE PA FWR from CurvedWall BT R
    GetStatus();
    sendValue('/matrix/state/settings/sum_bus_master/4/mute', 0); // Unmute Summing Bus CurvedWall BT L 
    sendValue('/matrix/state/settings/sum_bus_master/5/mute', 0); // Unmute Summing Bus CurvedWall BT R
    lockScene('Telco_Stereo');
  });
}

export function SetDanteMobile()
{
  confirmRoutingReset(async () => {
    await ResetDefaultRouting();
    setActiveInput('btn-direct-select-DANTE_Mobile');
    updateInputButtonsAvailability('DANTE_Mobile_Stereo');
    sendValue('/matrix/state/settings/output_gain/318', -100); // Mute Curved PA L
    sendValue('/matrix/state/settings/output_gain/319', -100); // Mute Curved PA R
    sendValue('/matrix/state/settings/output_gain/400', -100); // Mute CAVE PA FWL
    sendValue('/matrix/state/settings/output_gain/401', -100); // Mute CAVE PA FWR
    sendValue('/matrix/state/settings/easy_routing/318', 160); // Curved PA L from Mobile Dante L
    sendValue('/matrix/state/settings/easy_routing/319', 161); // Curved PA R from Mobile Dante R
    sendValue('/matrix/state/settings/easy_routing/400', 160); // CAVE PA FWL from Mobile Dante L
    sendValue('/matrix/state/settings/easy_routing/401', 161); // CAVE PA FWR from Mobile Dante R
    GetStatus();
    sendValue('/matrix/state/settings/sum_bus_master/6/mute', 0); // Unmute Summing Bus Mobile Dante L 
    sendValue('/matrix/state/settings/sum_bus_master/7/mute', 0); // Unmute Summing Bus Mobile Dante R
    lockScene('Telco_Stereo');
  });
}

export async function ResetDefaultRouting(eventOrButton)
{
  const btn = (eventOrButton instanceof HTMLElement) ? eventOrButton
    : (eventOrButton?.currentTarget instanceof HTMLElement) ? eventOrButton.currentTarget
    : document.getElementById('btn-reset-default-routing')
    || document.querySelector("button[onclick*='ResetDefaultRouting' i]")
    || Array.from(document.querySelectorAll('button')).find(b => b.textContent.trim() === 'Reset to Default');

  if (btn) btn.disabled = true;

  try {
    sendNoArgs('/matrix/cmd/recall_snapshot_5');
    await new Promise(resolve => setTimeout(resolve, 500));
    clearActiveInputs();
    updateInputButtonsAvailability(null);
  } finally {
    if (btn) btn.disabled = false;
  }
}

export function setVolumeCurvedPA(value)
{
  sendValue('/matrix/state/settings/output_gain/318', value);
  sendValue('/matrix/state/settings/output_gain/319', value);
  document.getElementById('output_gain_CurvedPA-volume-number').innerText = value + ' dB';
}

export function setVolumeCavePA(value)
{
  sendValue('/matrix/state/settings/output_gain/400', value);
  sendValue('/matrix/state/settings/output_gain/401', value);
  document.getElementById('output_gain_CAVEPA-volume-number').innerText = value + ' dB';
}

export function GetStatus()
{
  sendNoArgs('/matrix/state/settings/flex_channel/*/mute'); // get all mute states
  sendNoArgs('/matrix/state/settings/flex_channel/*/gain'); // get all gain values
  sendNoArgs('/matrix/state/settings/output_gain/*'); // get output gain values
}

export function MuteAll()
{
  sendValue('/matrix/state/settings/flex_channel/*/mute', 1);
  enableInputSelectButtons(true);
  showInputSection('none'); // Hide all input sections
}
