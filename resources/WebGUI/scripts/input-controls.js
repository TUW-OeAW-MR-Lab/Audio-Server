
const inputNames = [
    "DANTE_CurvedLEDPC",
    "DANTE_CurvedLEDPC_Stereo",
    "DANTE_CurvedLEDPC_Channel_3",
    "DANTE_Mobile", 
    "Mic_Wireless_1", 
    "Mic_Wireless_2", 
    "Mic_Array",
    "Curved_Wall_Bluetooth_Stereo", 
    "Curved_Wall_Aux_Stereo", 
    "DANTE_Mobile_Stereo", 
    "Beam_Curved_Focus", 
    "Beam_CAVE_Focus", 
    "Analog_Mono_1", "Analog_Mono_2", "Analog_Mono_3", "Analog_Mono_4", "Analog_Mono_5", "Analog_Mono_6", "Analog_Mono_7", "Analog_Mono_8",
    "Beam_Curved_Ambient",
    "Beam_CAVE_Ambient", 
];

const inputLabels = {
    "DANTE_CurvedLEDPC": "Curved LED PC",
    "DANTE_CurvedLEDPC_Stereo": "Curved LED PC Stereo",
    "DANTE_CurvedLEDPC_Channel_3": "Curved LED PC Test VBAP on Channel #3",
    "Curved_Wall_Bluetooth_Stereo": "Curved Bluetooth",
    "Curved_Wall_Aux_Stereo": "Curved Aux Stereo",
    "DANTE_Mobile": "Mobile Dante", 
    "DANTE_Mobile_Stereo": "Mobile Dante: Stereo", 
    "Beam_Curved_Focus": "Curved Beam Focus", 
    "Beam_Curved_Ambient": "Curved Beam Ambient",
    "Beam_CAVE_Focus": "CAVE Beam Focus", 
    "Beam_CAVE_Ambient": "CAVE Beam Ambient", 
    "Mic_Wireless_1": "Wireless Mic #1", 
    "Mic_Wireless_2": "Wireless Mic #2", 
    "Mic_Array": "Ceiling Mic Array",
    "Analog_Mono_1": "Analog: Mono #1", "Analog_Mono_2": "Analog: Mono #2",
    "Analog_Mono_3": "Analog: Mono #3", "Analog_Mono_4": "Analog: Mono #4",
    "Analog_Mono_5": "Analog: Mono #5", "Analog_Mono_6": "Analog: Mono #6",
    "Analog_Mono_7": "Analog: Mono #7", "Analog_Mono_8": "Analog: Mono #8",
};

const inputFlexChannelMap = {
    0: "Mic_Wireless_1",
    1: "Mic_Wireless_2",
    2: "Beam_Curved_Focus",
    3: "Beam_Curved_Ambient",
    4: "Beam_CAVE_Focus",
    5: "Beam_CAVE_Ambient",
    6: "DANTE_CurvedLEDPC_Stereo",
    7: "DANTE_CurvedLEDPC_Stereo",
    8: "Analog_Mono_1",
    9: "Analog_Mono_2",
    10: "Analog_Mono_3",
    11: "Analog_Mono_4",
    12: "Analog_Mono_5",
    13: "Analog_Mono_6",
    14: "Analog_Mono_7",
    15: "Analog_Mono_8",
    16: "Curved_Wall_Bluetooth_Stereo",
    17: "Curved_Wall_Bluetooth_Stereo",
    18: "Curved_Wall_Aux_Stereo",
    19: "Curved_Wall_Aux_Stereo",
    20: "DANTE_Mobile_Stereo",
    21: "DANTE_Mobile_Stereo",
    22: "",
    23: "",
    24: "",
    25: "",
    26: "",
    27: "",
    28: "",
    29: "",
    30: "",
    31: "",
};

function renderInputControls(buttonContainerId, groupContainerId, sceneName, hiddenInputs = []) {
    const btnContainer = document.getElementById(buttonContainerId);
    if (btnContainer) {
        let html = '';
        html += '<div style="max-height: 350px; overflow-y: auto; display: flex; flex-direction: column; align-items: center; gap: 10px;">';
        inputNames.forEach(id => {
             if (!hiddenInputs.includes(id)) {
                 html += `<button disabled id="btn-input-select-${id}" onClick="showInputSection('${id}')" class="big-button">${inputLabels[id]}</button> `;
             }
        });
        html += '</div>';
        btnContainer.innerHTML = html;
    }

    const groupContainer = document.getElementById(groupContainerId);
    if (groupContainer) {
        let html = '';
        inputNames.forEach(id => {
            if (!hiddenInputs.includes(id)) {
                let matchingKeys = Object.keys(inputFlexChannelMap).filter(key => inputFlexChannelMap[key] === id);
                let oscCmd = '';
                if (matchingKeys.length > 0) {
                    matchingKeys.forEach(ch => {
                        oscCmd += `sendValue('/matrix/state/settings/flex_channel/${ch}/gain', this.value); `;
                    });
                } else {
                    oscCmd = ` `;
                }
                
                html += `
                <div id="input-section-${id}" class="input-section" style="display:none; flex-direction: column; align-items: center; text-align: center;">
                    <span style="text-align: center; font-weight: bold; gap: 0px;">${inputLabels[id]}</span>
                    <button id="btn-input-mute-${id}" style="margin-top: 10px;" class="small-button" 
                      onclick="toggleInputState('${id}')">Input: Off
                    </button>
                    <div id="slider-container-${id}">
                      <input class="volume-slider" type="range" id="slider-input-${id}" disabled min="-60" max="18" value="-60" 
                        oninput="${oscCmd}document.getElementById('volume-number-${id}').innerText = this.value + ' dB';" 
                        style="opacity: 0.5; max-height: 220px;">
                      <div class="level-box" id="volume-number-${id}">--</div>
                    </div>
                </div>`;
            }
        });
        groupContainer.innerHTML = html;

        // Add event listeners for the 'updated' event to each slider
        inputNames.forEach(id => {
            if (!hiddenInputs.includes(id)) {
                const slider = document.getElementById(`slider-input-${id}`);
                const numberDisplay = document.getElementById(`volume-number-${id}`);
                if (slider && numberDisplay) {
                    slider.addEventListener('updated', (e) => {
                        slider.value = Math.round(slider.value); // Ensure it's rounded if needed, though usually OSC sends formatted values
                        numberDisplay.innerText = slider.value + ' dB';
                    });
                }
            }
        });
    }
}

function showInputSection(id)
{
	// hide all input sections
	const allSections = document.querySelectorAll('.input-section');
	allSections.forEach(section => { section.style.display = 'none'; });

	// show the selected input section
	const section = document.getElementById('input-section-' + id);
	if (section) section.style.display = 'flex';
}

    // id: input name, see inputNames
    // mute: false: unmuted, input active; true: muted, input inactive
function toggleInputState(id, mute)
{	
  //console.log("toggleInputState", id, state);
	const smallBtn = document.getElementById('btn-input-mute-' + id);
	const bigBtn = document.getElementById('btn-input-select-' + id);
	const slider = document.getElementById('slider-input-' + id);

	if (!smallBtn || !bigBtn || !slider) return;

	const isActive = smallBtn.classList.contains('active-input');
	const turnOn = mute === undefined ? !isActive : mute; // Unmute if mute is true or undefined and the button was not active

	if (turnOn) // Unmute this input
	{
		smallBtn.classList.add('active-input');
		smallBtn.innerText = "Input: On";
		bigBtn.classList.add('active-input');
		slider.disabled = false;
		slider.style.opacity = "1.0";
    if (mute===undefined) 
    {   // mute undefined: The user is toggling the input state, send OSC to unmute
      let matchingKeys = Object.keys(inputFlexChannelMap).filter(key => inputFlexChannelMap[key] === id);
      if (matchingKeys.length > 0) {
          matchingKeys.forEach(ch => {
              sendValue("/matrix/state/settings/flex_channel/" + ch + "/mute", 0);
          });
      }
    }
    else 
    { // mute defined: The GUI is updating the input state, trigger update of the volume slider
      let matchingKeys = Object.keys(inputFlexChannelMap).filter(key => inputFlexChannelMap[key] === id);
      if (matchingKeys.length > 0) {
          matchingKeys.forEach(ch => {
              send("/matrix/state/settings/flex_channel/" + ch + "/gain");
          });
      }
    }
	}
	else // Mute this input
	{
		smallBtn.classList.remove('active-input');
		smallBtn.innerText = "Input: Off";
		bigBtn.classList.remove('active-input');
		slider.disabled = false;
		slider.style.opacity = "0.5";
    if (mute===undefined) // Only send OSC if the user is toggling the input state
    {  
      let matchingKeys = Object.keys(inputFlexChannelMap).filter(key => inputFlexChannelMap[key] === id);
      if (matchingKeys.length > 0) {
          matchingKeys.forEach(ch => {
              sendValue("/matrix/state/settings/flex_channel/" + ch + "/mute", 1);
          });
      }
    }
	}
}

function renderInputButtons(buttonContainerId, sceneName, hiddenInputs = []) {
    const btnContainer = document.getElementById(buttonContainerId);
    if (btnContainer) {
        let html = '';
        html += '<div style="max-height: 350px; overflow-y: auto; display: flex; flex-direction: column; align-items: center; gap: 10px;">';
        inputNames.forEach(id => {
             if (!hiddenInputs.includes(id)) {
                 html += `<button id="btn-input-select-${id}" onClick="setInputButtonExclusively('${id}', '${sceneName}')" class="big-button">${inputLabels[id]}</button> `;
             }
        });
        html += '</div>';
        btnContainer.innerHTML = html;
    }
}

function setInputButtonExclusively(id, sceneName)
{	
	const bigBtn = document.getElementById('btn-input-select-' + id);
	if (!bigBtn) return;

	// Check if currently active by class
	const isActive = bigBtn.classList.contains('active-input');

	if (!isActive) // User wants to turn it ON
	{
		// Find any currently active simple buttons
		inputNames.forEach(otherId => {
			if (otherId === id) return;
			const otherBtn = document.getElementById('btn-input-select-' + otherId);
			if (otherBtn && otherBtn.classList.contains('active-input')) {
				otherBtn.classList.remove('active-input');
			}
		});
		bigBtn.classList.add('active-input');
    return true;
	}
	else // Turn OFF
	{
		bigBtn.classList.remove('active-input');
    return false;
	}
}

function enableInputSelectButtons(enable = true)
{
	inputNames.forEach(id => {
		const btn = document.getElementById('btn-input-select-' + id);
		if (btn) 
    {
      btn.disabled = !enable;
      toggleInputState(id, false);
    }
	});
}
