// Summing bus index for headphones
const hpSumBus = {
  1: 24,
  2: 26,
  3: 28,
  4: 30,
};

export const hpSections = [1, 2, 3, 4];

// Switch states per input: { [inputId]: { 1: boolean, 2: boolean, 3: boolean, 4: boolean } }
export const hpSwitchStates = {};

// Currently selected input ID (null if no input is selected)
export let currentSelectedInput = null;

/**
 * Initializes the Headphones scene
 */
export function init()
{
  // Reset selected input on scene load
  currentSelectedInput = null;

  // Inject input controls and disabled buttons (same inputs as CAVE_Stereo)
  renderInputControls('input-buttons-container', 'input-group-container', 'Headphones', 
    ["DANTE_CurvedLEDPC", "DANTE_CurvedLEDPC_Channel_3", "DANTE_Mobile", "Mic_Array", 
      "DANTE_HDMI_Stereo", // to be setup later
    ]);
  enableInputSelectButtons(true);

  // Hook input section changes
  setupInputSelectionListeners();

  // Initially ensure switch buttons are hidden until an input is selected
  updateSwitchButtonsVisibility(false);

  // Attach event listeners for each headphone section
  hpSections.forEach(hp => {
    const gainSpan = document.getElementById(`sum_bus_HP${hp}-gain`);
    if (gainSpan) {
      gainSpan.addEventListener('updated', (e) => {
        const slider = document.getElementById(`hp-${hp}-volume-slider`);
        const num = document.getElementById(`hp-${hp}-volume-number`);
        if (slider && num) {
          const val = Math.round(Number(gainSpan.innerText));
          slider.value = val;
          num.innerText = val + ' dB';
        }
      });
    }

    const muteBtn = document.getElementById(`btn-sum_bus_HP${hp}-mute`);
    if (muteBtn) {
      muteBtn.addEventListener('updated', (e) => {
        const isActive = muteBtn.classList.contains('active-input');
        const slider = document.getElementById(`hp-${hp}-volume-slider`);
        if (slider) {
          slider.style.opacity = isActive ? "1.0" : "0.5";
        }
      });
    }
  });

  GetStatus();
}

/**
 * Sets up listeners to detect when an input is selected from the input section
 */
function setupInputSelectionListeners()
{
  // 1. Listen for the custom event dispatched when showInputSection runs
  document.addEventListener('inputSectionChanged', onInputSectionChanged);

  // 2. Also attach click listeners to all buttons in the input buttons container
  const btnContainer = document.getElementById('input-buttons-container');
  if (btnContainer) {
    const buttons = btnContainer.querySelectorAll('button');
    buttons.forEach(btn => {
      btn.addEventListener('click', () => {
        const inputId = btn.id.replace('btn-input-select-', '');
        selectInput(inputId);
      });
    });
  }

  // 3. Wrap window.showInputSection to guarantee detection even if invoked elsewhere
  const originalShowInputSection = window.showInputSection;
  window.showInputSection = function(id) {
    if (typeof originalShowInputSection === 'function') {
      originalShowInputSection(id);
    }
    // Only handle if Headphones scene is active
    if (document.getElementById('Headphones')) {
      selectInput(id);
    }
  };
}

function onInputSectionChanged(e)
{
  if (!document.getElementById('Headphones')) return;
  const inputId = e.detail && e.detail.inputId;
  selectInput(inputId);
}

/**
 * Handles selecting an input (or deselecting if 'none')
 * @param {string} id - The input ID
 */
export function selectInput(id)
{
  if (!id || id === 'none') {
    currentSelectedInput = null;
    updateSwitchButtonsVisibility(false);
    return;
  }

  currentSelectedInput = id;

  // Initialize switch states for this input if not already remembered
  if (!hpSwitchStates[id]) {
    hpSwitchStates[id] = { 1: false, 2: false, 3: false, 4: false };
  }

  // Switch buttons become visible when an input is selected
  updateSwitchButtonsVisibility(true);

  // Restore the remembered switch button states for this input
  updateSwitchButtonsDisplay();
}

/**
 * Updates the visibility of the switch buttons on top of each HP section
 * @param {boolean} visible
 */
function updateSwitchButtonsVisibility(visible)
{
  hpSections.forEach(hp => {
    const btn = document.getElementById(`btn-hp-switch-${hp}`);
    if (btn) {
      btn.style.visibility = visible ? 'visible' : 'hidden';
      btn.disabled = !visible;
    }
  });
}

/**
 * Updates the visual state (On / Off, active-input class) of the 4 switch buttons
 * based on the remembered states for the currently selected input
 */
function updateSwitchButtonsDisplay()
{
  if (!currentSelectedInput) return;
  const states = hpSwitchStates[currentSelectedInput] || { 1: false, 2: false, 3: false, 4: false };

  hpSections.forEach(hp => {
    const btn = document.getElementById(`btn-hp-switch-${hp}`);
    if (btn) {
      const isOn = !!states[hp];
      if (isOn) {
        btn.classList.add('active-input');
        btn.innerText = "On";
      } else {
        btn.classList.remove('active-input');
        btn.innerText = "Off";
      }
    }
  });
}

/**
 * Toggles the switch button for a specific Headphone when pressed in the GUI
 * @param {number} hpIndex - The HP index (1, 2, 3, or 4)
 */
export function toggleHpSwitch(hpIndex)
{
  if (!currentSelectedInput) {
    console.warn("No input selected, cannot toggle HP switch.");
    return;
  }

  const input = currentSelectedInput;
  if (!hpSwitchStates[input]) {
    hpSwitchStates[input] = { 1: false, 2: false, 3: false, 4: false };
  }

  const newState = !hpSwitchStates[input][hpIndex];
  hpSwitchStates[input][hpIndex] = newState;

  // Update button appearance
  const btn = document.getElementById(`btn-hp-switch-${hpIndex}`);
  if (btn) {
    if (newState) {
      btn.classList.add('active-input');
      btn.innerText = "On";
    } else {
      btn.classList.remove('active-input');
      btn.innerText = "Off";
    }
  }

  // Trigger the function with the input and HP index as parameter
  onHpSwitch(input, hpIndex, newState);
  lockScene("Headphones");
}

/**
 * Calculates the 5 32-bit segment values for a headphone based on switched-on inputs.
 * Consider a 160-bit number with the bit number following in the index in inputFlexChannelMap every five bits,
 * i.e., index 0 is bit 0, index 1 is bit 5, index 2 is bit 10, etc.
 * In that number, each switched on input has a "1", each switched off has a "0".
 * When "ear" is 0, considers an input only if "L" is found in inputFlexChannelSide.
 * When "ear" is 1, considers an input only if "R" is found in inputFlexChannelSide.
 * Splits this number into 5 32-bit unsigned integers (segments 0 to 4).
 * @param {number} hpIndex - Headphone index (1, 2, 3, or 4)
 * @param {number} [ear=0] - 0 for Left, 1 for Right
 * @returns {number[]} Array of 5 32-bit integers [seg0, seg1, seg2, seg3, seg4]
 */
export function calculateSumBusSegments(hpIndex, ear = 0)
{
  const channelMap = (typeof inputFlexChannelMap !== 'undefined')
    ? inputFlexChannelMap
    : (window.inputFlexChannelMap || {});

  const channelSide = (typeof inputFlexChannelSide !== 'undefined')
    ? inputFlexChannelSide
    : (window.inputFlexChannelSide || {});

  const targetChar = ear === 1 ? "R" : "L";

  let bitmask = 0n;
  for (let idx = 0; idx < 32; idx++) {
    const side = String(channelSide[idx] || "");
    if (!side.includes(targetChar)) {
      continue;
    }

    const inputName = channelMap[idx];
    if (inputName && hpSwitchStates[inputName] && hpSwitchStates[inputName][hpIndex]) {
      bitmask |= (1n << BigInt(idx * 5));
    }
  }

  const segments = [];
  for (let s = 0; s < 5; s++) {
    const segVal = Number((bitmask >> BigInt(s * 32)) & 0xFFFFFFFFn);
    segments.push(segVal);
  }
  return segments;
}

/**
 * Function triggered when one of the switch buttons is pressed.
 * Receives the input and the HP index as parameter.
 * @param {string} input - The input identifier (e.g. 'DANTE_CurvedLEDPC_Stereo')
 * @param {number} hpIndex - The Headphone index (1, 2, 3, or 4)
 * @param {boolean} [state] - The new state of the switch (true = On, false = Off)
 */
export function onHpSwitch(input, hpIndex, state)
{
  if (input && state !== undefined) {
    if (!hpSwitchStates[input]) hpSwitchStates[input] = { 1: false, 2: false, 3: false, 4: false };
    hpSwitchStates[input][hpIndex] = !!state;
  }

  const currentState = state !== undefined ? state : (hpSwitchStates[input] && hpSwitchStates[input][hpIndex]);
  
  const x = hpSumBus[hpIndex];
  if (x !== undefined) {
    let segments = calculateSumBusSegments(hpIndex, 0);
    sendValue(`/matrix/state/settings/sum_bus_assign_dsp/${x}/segment/0`, segments[0]);
    sendValue(`/matrix/state/settings/sum_bus_assign_dsp/${x}/segment/1`, segments[1]);
    sendValue(`/matrix/state/settings/sum_bus_assign_dsp/${x}/segment/2`, segments[2]);
    sendValue(`/matrix/state/settings/sum_bus_assign_dsp/${x}/segment/3`, segments[3]);
    sendValue(`/matrix/state/settings/sum_bus_assign_dsp/${x}/segment/4`, segments[4]);
    segments = calculateSumBusSegments(hpIndex, 1);
    sendValue(`/matrix/state/settings/sum_bus_assign_dsp/${x+1}/segment/0`, segments[0]);
    sendValue(`/matrix/state/settings/sum_bus_assign_dsp/${x+1}/segment/1`, segments[1]);
    sendValue(`/matrix/state/settings/sum_bus_assign_dsp/${x+1}/segment/2`, segments[2]);
    sendValue(`/matrix/state/settings/sum_bus_assign_dsp/${x+1}/segment/3`, segments[3]);
    sendValue(`/matrix/state/settings/sum_bus_assign_dsp/${x+1}/segment/4`, segments[4]);
  }
}

// Aliases for callback function compatibility
export function handleHpSwitch(input, hpIndex, state)
{
  return onHpSwitch(input, hpIndex, state);
}

export function switchButtonPressed(input, hpIndex, state)
{
  return onHpSwitch(input, hpIndex, state);
}

/**
 * Programmatically get the switch state for a given input and HP index
 */
export function getHpSwitchState(input, hpIndex)
{
  if (hpSwitchStates[input]) {
    return !!hpSwitchStates[input][hpIndex];
  }
  return false;
}

/**
 * Programmatically set the switch state for a given input and HP index
 */
export function setHpSwitch(input, hpIndex, state)
{
  if (!hpSwitchStates[input]) {
    hpSwitchStates[input] = { 1: false, 2: false, 3: false, 4: false };
  }
  hpSwitchStates[input][hpIndex] = !!state;

  if (currentSelectedInput === input) {
    const btn = document.getElementById(`btn-hp-switch-${hpIndex}`);
    if (btn) {
      if (state) {
        btn.classList.add('active-input');
        btn.innerText = "Linked";
      } else {
        btn.classList.remove('active-input');
        btn.innerText = "Link?";
      }
    }
  }

  onHpSwitch(input, hpIndex, !!state);
}

/**
 * Query current states and gains from the matrix and unassign all headphones 
 */
export function GetStatus()
{
  sendNoArgs('/matrix/state/settings/flex_channel/*/mute'); // get all mute states
  sendNoArgs('/matrix/state/settings/flex_channel/*/gain'); // get all gain values
  sendNoArgs('/matrix/state/settings/sum_bus_master/*/gain'); // get bus master gains
  sendNoArgs('/matrix/state/settings/sum_bus_master/*/mute'); // get bus master mutes
}

/**
 * Mute all inputs
 */
export function MuteAllInputs()
{
  sendValue('/matrix/state/settings/flex_channel/*/mute', 1);
  enableInputSelectButtons(true);
  showInputSection('none'); // Hide all input sections (also hides switch buttons)
}

/**
 * Mute all headphones
 */
export function MuteAllHPs()
{
  hpSections.forEach(hp => {
    toggleMute(hp, false);
  });
}

export function UnlinkAllHPs()
{
  for (const hpIdx in hpSumBus)
  {
    let hp = hpSumBus[hpIdx];
    sendValue(`/matrix/state/settings/sum_bus_assign_dsp/${hp}/segment/0`, 0);
    sendValue(`/matrix/state/settings/sum_bus_assign_dsp/${hp}/segment/1`, 0);
    sendValue(`/matrix/state/settings/sum_bus_assign_dsp/${hp}/segment/2`, 0);
    sendValue(`/matrix/state/settings/sum_bus_assign_dsp/${hp}/segment/3`, 0);
    sendValue(`/matrix/state/settings/sum_bus_assign_dsp/${hp}/segment/4`, 0);
    sendValue(`/matrix/state/settings/sum_bus_assign_dsp/${hp+1}/segment/0`, 0);
    sendValue(`/matrix/state/settings/sum_bus_assign_dsp/${hp+1}/segment/1`, 0);
    sendValue(`/matrix/state/settings/sum_bus_assign_dsp/${hp+1}/segment/2`, 0);
    sendValue(`/matrix/state/settings/sum_bus_assign_dsp/${hp+1}/segment/3`, 0);
    sendValue(`/matrix/state/settings/sum_bus_assign_dsp/${hp+1}/segment/4`, 0);
  }
 // Initialize all hpSwitchStates to false
  for (const input in hpSwitchStates) {
    hpSwitchStates[input] = { 1: false, 2: false, 3: false, 4: false };
  }
  if (typeof inputNames !== 'undefined') {
    inputNames.forEach(input => {
      hpSwitchStates[input] = { 1: false, 2: false, 3: false, 4: false };
    });
  }
  updateSwitchButtonsDisplay(); 
}

export function ResetToDefault()
{
  sendNoArgs('/matrix/cmd/recall_snapshot_5');

  enableInputSelectButtons(true);
  hpSections.forEach(hp => {
    toggleMute(hp, false);
  });
  showInputSection('none'); // Hide all input sections (also hides switch buttons)

  // Initialize all hpSwitchStates to false
  for (const input in hpSwitchStates) {
    hpSwitchStates[input] = { 1: false, 2: false, 3: false, 4: false };
  }
  if (typeof inputNames !== 'undefined') {
    inputNames.forEach(input => {
      hpSwitchStates[input] = { 1: false, 2: false, 3: false, 4: false };
    });
  }
  updateSwitchButtonsDisplay();

  unlockScene("Headphones");
}

/**
 * Toggle mute state for a Headphone section
 * @param {number} hpIndex - The HP index (1..4)
 * @param {boolean} [state] - Desired state (true = On/unmuted, false = Off/muted)
 * @param {boolean} [btn_disable]
 * @param {boolean} [slider_disable=false]
 */
export function toggleMute(hpIndex, state, btn_disable, slider_disable = false)
{
  const btn = document.getElementById(`btn-sum_bus_HP${hpIndex}-mute`);
  const slider = document.getElementById(`hp-${hpIndex}-volume-slider`);
  if (!btn || !slider) return;

  const isActive = btn.classList.contains('active-input');
  const turnOn = state === undefined ? !isActive : state;
  const ch = hpSumBus[hpIndex];

  if (turnOn) {
    btn.classList.add('active-input');
    btn.innerText = "On";
    btn.disabled = btn_disable;
    slider.disabled = slider_disable;
    slider.style.opacity = "1.0";
    if (ch !== undefined) {
      const channels = Array.isArray(ch) ? ch : [ch];
      channels.forEach(c => {
        sendValue(`/matrix/state/settings/sum_bus_master/${c}/mute`, 0); // Left channel
        sendValue(`/matrix/state/settings/sum_bus_master/${c+1}/mute`, 0); // Right channel
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
        sendValue(`/matrix/state/settings/sum_bus_master/${c}/mute`, 1); // Left channel
        sendValue(`/matrix/state/settings/sum_bus_master/${c+1}/mute`, 1); // Right channel
      });
    }
  }
}

/**
 * Sets volume for a Headphone section
 * @param {number} hpIndex - The HP index (1..4)
 * @param {number|string} value - Volume in dB
 */
export function setVolume(hpIndex, value)
{
  const ch = hpSumBus[hpIndex];
  if (ch !== undefined) {
    const channels = Array.isArray(ch) ? ch : [ch];
    channels.forEach(c => {
      sendValue(`/matrix/state/settings/sum_bus_master/${c}/gain`, value); // Left channel
      sendValue(`/matrix/state/settings/sum_bus_master/${c+1}/gain`, value); // Right channel
    });
  }
  const num = document.getElementById(`hp-${hpIndex}-volume-number`);
  if (num) {
    num.innerText = value + ' dB';
  }
}

// Section-specific toggle wrappers
export function toggleHP1Mute(state, btn_disable, slider_disable = false) { toggleMute(1, state, btn_disable, slider_disable); }
export function toggleHP2Mute(state, btn_disable, slider_disable = false) { toggleMute(2, state, btn_disable, slider_disable); }
export function toggleHP3Mute(state, btn_disable, slider_disable = false) { toggleMute(3, state, btn_disable, slider_disable); }
export function toggleHP4Mute(state, btn_disable, slider_disable = false) { toggleMute(4, state, btn_disable, slider_disable); }

// Section-specific volume wrappers
export function setVolumeHP1(value) { setVolume(1, value); }
export function setVolumeHP2(value) { setVolume(2, value); }
export function setVolumeHP3(value) { setVolume(3, value); }
export function setVolumeHP4(value) { setVolume(4, value); }
