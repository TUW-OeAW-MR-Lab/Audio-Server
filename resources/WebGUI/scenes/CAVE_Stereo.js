const sectionChannels = {
  'FL': 8,
  'FR': 9,
  'FWL': 10,
  'FWR': 11,
  'SL': 12,
  'SR': 13,
  'BL': 14,
  'BR': 15
};

const sections = ['BL', 'SL', 'FWL', 'FL', 'FR', 'FWR', 'SR', 'BR'];

export function init()
{
  // Inject input controls and disabled buttons (same inputs as Curved_LED_Stereo)
  renderInputControls('input-buttons-container', 'input-group-container', 'CAVE_Stereo', 
    ["DANTE_CurvedLEDPC", "DANTE_CurvedLEDPC_Channel_3", "DANTE_Mobile", "Mic_Array", 
      "DANTE_HDMI_Stereo", // to be setup later
    ]);
  enableInputSelectButtons(true);
  GetStatus();

  // Attach event listeners for each surround section
  sections.forEach(sec => {
    const gainSpan = document.getElementById(`sum_bus_CAVE_${sec}-gain`);
    if (gainSpan) {
      gainSpan.addEventListener('updated', (e) => {
        const slider = document.getElementById(`sum_bus_CAVE_${sec}-volume-slider`);
        const num = document.getElementById(`sum_bus_CAVE_${sec}-volume-number`);
        if (slider && num) {
          const val = Math.round(Number(gainSpan.innerText));
          slider.value = val;
          num.innerText = val + ' dB';
        }
      });
    }

    const muteBtn = document.getElementById(`btn-sum_bus_CAVE_${sec}-mute`);
    if (muteBtn) {
      muteBtn.addEventListener('updated', (e) => {
        const isActive = muteBtn.classList.contains('active-input');
        const slider = document.getElementById(`sum_bus_CAVE_${sec}-volume-slider`);
        if (slider) {
          slider.style.opacity = isActive ? "1.0" : "0.5";
        }
      });
    }
  });
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
  sections.forEach(sec => {
    toggleMute(sec, false);
  });
  showInputSection('none'); // Hide all input sections
}

export function toggleMute(section, state, btn_disable, slider_disable = false)
{
  const btn = document.getElementById(`btn-sum_bus_CAVE_${section}-mute`);
  const slider = document.getElementById(`sum_bus_CAVE_${section}-volume-slider`);
  if (!btn || !slider) return;

  const isActive = btn.classList.contains('active-input');
  const turnOn = state === undefined ? !isActive : state;
  const ch = sectionChannels[section];

  if (turnOn) {
    btn.classList.add('active-input');
    btn.innerText = "On";
    btn.disabled = btn_disable;
    slider.disabled = slider_disable;
    slider.style.opacity = "1.0";
    if (ch !== undefined) {
      const channels = Array.isArray(ch) ? ch : [ch];
      channels.forEach(c => {
        sendValue(`/matrix/state/settings/sum_bus_master/${c}/mute`, 0);
      });
    }
  } else {
    btn.classList.remove('active-input');
    btn.innerText = "Off";
    btn.disabled = btn_disable;
    slider.disabled = slider_disable;
    slider.style.opacity = "0.5";
    if (ch !== undefined) {
      const channels = Array.isArray(ch) ? ch : [ch];
      channels.forEach(c => {
        sendValue(`/matrix/state/settings/sum_bus_master/${c}/mute`, 1);
      });
    }
  }
}

export function setVolume(section, value)
{
  const ch = sectionChannels[section];
  if (ch !== undefined) {
    const channels = Array.isArray(ch) ? ch : [ch];
    channels.forEach(c => {
      sendValue(`/matrix/state/settings/sum_bus_master/${c}/gain`, value);
    });
  }
  const num = document.getElementById(`sum_bus_CAVE_${section}-volume-number`);
  if (num) {
    num.innerText = value + ' dB';
  }
}

// Section-specific toggle wrappers
export function toggleFLMute(state, btn_disable, slider_disable = false) { toggleMute('FL', state, btn_disable, slider_disable); }
export function toggleFRMute(state, btn_disable, slider_disable = false) { toggleMute('FR', state, btn_disable, slider_disable); }
export function toggleFWLMute(state, btn_disable, slider_disable = false) { toggleMute('FWL', state, btn_disable, slider_disable); }
export function toggleFWRMute(state, btn_disable, slider_disable = false) { toggleMute('FWR', state, btn_disable, slider_disable); }
export function toggleSLMute(state, btn_disable, slider_disable = false) { toggleMute('SL', state, btn_disable, slider_disable); }
export function toggleSRMute(state, btn_disable, slider_disable = false) { toggleMute('SR', state, btn_disable, slider_disable); }
export function toggleBLMute(state, btn_disable, slider_disable = false) { toggleMute('BL', state, btn_disable, slider_disable); }
export function toggleBRMute(state, btn_disable, slider_disable = false) { toggleMute('BR', state, btn_disable, slider_disable); }

// Section-specific volume wrappers
export function setVolumeFL(value) { setVolume('FL', value); }
export function setVolumeFR(value) { setVolume('FR', value); }
export function setVolumeFWL(value) { setVolume('FWL', value); }
export function setVolumeFWR(value) { setVolume('FWR', value); }
export function setVolumeSL(value) { setVolume('SL', value); }
export function setVolumeSR(value) { setVolume('SR', value); }
export function setVolumeBL(value) { setVolume('BL', value); }
export function setVolumeBR(value) { setVolume('BR', value); }
