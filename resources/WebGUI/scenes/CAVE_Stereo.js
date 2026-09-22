const sectionChannels = {
  'Front': [8, 9],
  'Left': [10, 12],
  'Right': [11, 13],
  'Back': [14, 15]
};

const sections = ['Front', 'Left', 'Right', 'Back'];

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
  toggleFrontMute(false);
  toggleLeftMute(false);
  toggleRightMute(false);
  toggleBackMute(false);
  showInputSection('none'); // Hide all input sections
}

export function toggleMute(section, state, btn_disable, slider_disable = false)
{
  const btn = document.getElementById(`btn-sum_bus_CAVE_${section}-mute`);
  const slider = document.getElementById(`sum_bus_CAVE_${section}-volume-slider`);
  if (!btn || !slider) return;

  const isActive = btn.classList.contains('active-input');
  const turnOn = state === undefined ? !isActive : state;
  const channels = sectionChannels[section] || [];

  if (turnOn) {
    btn.classList.add('active-input');
    btn.innerText = "On";
    btn.disabled = btn_disable;
    slider.disabled = slider_disable;
    slider.style.opacity = "1.0";
    channels.forEach(ch => {
      sendValue(`/matrix/state/settings/sum_bus_master/${ch}/mute`, 0);
    });
  } else {
    btn.classList.remove('active-input');
    btn.innerText = "Off";
    btn.disabled = btn_disable;
    slider.disabled = slider_disable;
    slider.style.opacity = "0.5";
    channels.forEach(ch => {
      sendValue(`/matrix/state/settings/sum_bus_master/${ch}/mute`, 1);
    });
  }
}

export function toggleFrontMute(state, btn_disable, slider_disable = false) {
  toggleMute('Front', state, btn_disable, slider_disable);
}

export function toggleLeftMute(state, btn_disable, slider_disable = false) {
  toggleMute('Left', state, btn_disable, slider_disable);
}

export function toggleRightMute(state, btn_disable, slider_disable = false) {
  toggleMute('Right', state, btn_disable, slider_disable);
}

export function toggleBackMute(state, btn_disable, slider_disable = false) {
  toggleMute('Back', state, btn_disable, slider_disable);
}

export function setVolume(section, value)
{
  const channels = sectionChannels[section] || [];
  channels.forEach(ch => {
    sendValue(`/matrix/state/settings/sum_bus_master/${ch}/gain`, value);
  });
  const num = document.getElementById(`sum_bus_CAVE_${section}-volume-number`);
  if (num) {
    num.innerText = value + ' dB';
  }
}

export function setVolumeFront(value) {
  setVolume('Front', value);
}

export function setVolumeLeft(value) {
  setVolume('Left', value);
}

export function setVolumeRight(value) {
  setVolume('Right', value);
}

export function setVolumeBack(value) {
  setVolume('Back', value);
}
