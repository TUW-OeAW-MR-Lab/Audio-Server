export function init()
{
    // Inject input controls and disabled buttons
   renderInputControls('input-buttons-container', 'input-group-container', 'Curved_LED_Stereo', 
      ["DANTE_CurvedLEDPC", "DANTE_CurvedLEDPC_Channel_3", "DANTE_Mobile", "Mic_Array", 
        "DANTE_Mobile_Stereo", // to be setup later
        "DANTE_HDMI_Stereo", // to be setup later
        "DANTE_Bluetooth", // to be setup later
        ]);
  enableInputSelectButtons(true);
  GetStatus();

  document.getElementById('sum_bus_master-gain').addEventListener('updated', (e) => 
  {
    const vol = document.getElementById('sum_bus_master-gain');
    //console.log(vol.innerText);
    document.getElementById('sum_bus_master-volume-slider').value = Math.round(vol.innerText);
    document.getElementById('sum_bus_master-volume-number').innerText = Math.round(vol.innerText) + ' dB';
  });	
    
}

export function GetStatus()
{
  //lockScene('Curved_LED_Stereo');
  sendNoArgs('/matrix/state/settings/flex_channel/*/mute'); // get all mute states
  sendNoArgs('/matrix/state/settings/flex_channel/*/gain'); // get all gain values
  sendNoArgs('/matrix/state/settings/sum_bus_master/0/gain'); // get master gain
  sendNoArgs('/matrix/state/settings/sum_bus_master/1/gain'); // get master gain
  sendNoArgs('/matrix/state/settings/sum_bus_master/0/mute'); // get master mute
  sendNoArgs('/matrix/state/settings/sum_bus_master/1/mute'); // get master mute
}

function triggerMeter() // currently unused because the VU meter does not work in the matrix this way
{
  sendNoArgs('/matrix/state/settings/sum_bus_master/0/meter');
  if (!document.getElementById("sum_bus_master-volume-slider").disabled)
    setTimeout(() => { triggerMeter(); }, 100);
}

export function MuteAll()
{
  sendValue('/matrix/state/settings/flex_channel/*/mute', 1);
  enableInputSelectButtons(true);
  toggleSumBusMasterMute(false);
  showInputSection('none'); // Hide all input sections
}

export function toggleSumBusMasterMute(state, btn_disable, slider_disable = false)
{
	const btn = document.getElementById('btn-sum_bus_master-mute');
	const slider = document.getElementById('sum_bus_master-volume-slider');
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
