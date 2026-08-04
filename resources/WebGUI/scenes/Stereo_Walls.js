export function init()
{
	document.getElementById("Stereo_Walls-status").innerText = "initialized";
	document.getElementById("Stereo_Walls-status").style.backgroundColor = "";	
	document.getElementById("Stereo_Walls-Control_Version").innerText = "---";
    
  // Inject input controls and disabled buttons
  renderInputControls('input-buttons-container', 'input-group-container', 'Stereo_Walls', 
    ["DANTE_CurvedLEDPC", "DANTE_CurvedLEDPC_Channel_3", "DANTE_Mobile", "Mic_Array", 
      "DANTE_HDMI_Stereo", // to be setup later
      ]);
  enableInputSelectButtons(false);

  document.getElementById('Stereo_Walls-Total_VU').addEventListener('updated', (e) => 
  {
    const vu = document.getElementById('Stereo_Walls-Total_VU');
    document.getElementById('Stereo_Walls-Total_VU-bar').style.height = String(Math.round(vu.innerText)+100) + '%';
    document.getElementById('Stereo_Walls-Total_VU-number').innerText = Math.round(vu.innerText) + ' dB';
  });			

  document.getElementById('sum_bus_CurvedPA-gain').addEventListener('updated', (e) => 
  {   // Curved PA Volume
    const vol = document.getElementById('sum_bus_CurvedPA-gain');
    document.getElementById('sum_bus_CurvedPA-volume-slider').value = Math.round(vol.innerText);
    document.getElementById('sum_bus_CurvedPA-volume-number').innerText = Math.round(vol.innerText) + " dB";
  });	

  document.getElementById('sum_bus_CAVEPA-gain').addEventListener('updated', (e) => 
  {   // CAVE PA Volume
    const vol = document.getElementById('sum_bus_CAVEPA-gain');
    document.getElementById('sum_bus_CAVEPA-volume-slider').value = Math.round(vol.innerText);
    document.getElementById('sum_bus_CAVEPA-volume-number').innerText = Math.round(vol.innerText) + " dB";
  });	

  document.getElementById('Stereo_Walls-CAVEDoor_Volume').addEventListener('updated', (e) => 
  {
    const vol = document.getElementById('Stereo_Walls-CAVEDoor_Volume');
    document.getElementById('volume-CAVEDoor-slider').value = Math.round(vol.innerText);
    document.getElementById('volume-CAVEDoor-number').innerText = Math.round(vol.innerText) + " dB";
  });	

  document.getElementById('Stereo_Walls-SA_Volume').addEventListener('updated', (e) => 
  {
    const vol = document.getElementById('Stereo_Walls-SA_Volume');
    document.getElementById('volume-SA-slider').value = Math.round(vol.innerText);
    document.getElementById('volume-SA-number').innerText = Math.round(vol.innerText) + " dB";
  });	

  document.getElementById('Stereo_Walls-CurvedDoor_Volume').addEventListener('updated', (e) => 
  {
    const vol = document.getElementById('Stereo_Walls-CurvedDoor_Volume');
    document.getElementById('volume-CurvedDoor-slider').value = Math.round(vol.innerText);
    document.getElementById('volume-CurvedDoor-number').innerText = Math.round(vol.innerText) + " dB";
  });	
    
}

export function connect()
{
	state = document.getElementById("Stereo_Walls-status");
	state.innerText = "connecting...";
	state.style.backgroundColor = "blue";
	sendResponse('/app/Stereo_Walls/osc/Control/Response', 9336);
	sendNoArgs('/app/Stereo_Walls/osc/Control/Version');
	secWaited = 0;
	setTimeout(() => { checkConnection(); }, 300);
}

export function checkConnection()
{
	const timeout = 1000; // timeout in ms
	secWaited += 100;
	state = document.getElementById("Stereo_Walls-Control_Version");
	//console.log("waiting for " + secWaited/1000 + "s, version=" + state.innerText);
	switch (state.innerText)
	{
		case "---":
			if (secWaited <= timeout)
			{		// try again
				sendResponse('/app/Stereo_Walls/osc/Control/Response', 9336);
				sendNoArgs('/app/Stereo_Walls/osc/Control/Version');		
				setTimeout(() => { checkConnection(); }, 100);
			}
			break;
		default: // done!
			state = document.getElementById("Stereo_Walls-status");
			state.innerText = "connected";
			state.style.backgroundColor = "";
	    sendValue('/app/Stereo_Walls/osc/Total/Volume/Set', 0); 
      sendNoArgs('/app/Stereo_Walls/osc/CAVEDoor/Volume');
      sendNoArgs('/app/Stereo_Walls/osc/SA/Volume');
      sendNoArgs('/app/Stereo_Walls/osc/CurvedDoor/Volume');
      sendValue('/matrix/state/settings/sum_bus_master/2/mute', 0); // Unmute Audio PC Left
      sendValue('/matrix/state/settings/sum_bus_master/3/mute', 0); // Unmute Audio PC Right
      sendValue('/matrix/state/settings/sum_bus_master/2/gain', 0); // set Audio PC Left to 0 dB
      sendValue('/matrix/state/settings/sum_bus_master/3/gain', 0); // set Audio PC Right to 0 dB
      enableInputSelectButtons(true);
      toggleWallStateApp('CAVEDoor', false, false, false);
      toggleWallStateApp('SA', false, false, false);
      toggleWallStateApp('CurvedDoor', false, false, false);
      toggleStateCurvedPA(false, false, false);
      toggleStateCAVEPA(false, false, false);
      sendNoArgs('/matrix/state/settings/sum_bus_master/*/gain'); // get all Bus Master volumes
      sendNoArgs('/matrix/state/settings/sum_bus_master/*/mute'); // get all Bus Master mutes
      sendNoArgs('/matrix/state/settings/flex_channel/*/mute'); // get all Input mute states
      sendNoArgs('/matrix/state/settings/flex_channel/*/gain'); // get all Input volumes
			break;
	}
	if (secWaited > timeout)
	{ 	// time out -> clean up
		state = document.getElementById("Stereo_Walls-status");
		state.innerText = "time out while connecting";
		state.style.backgroundColor = "red";
	}
}		

export function quit()
{
	quitApp('Stereo_Walls', 'Stereo_Walls', SceneModule.init);
  enableInputSelectButtons(false);
  showInputSection('none'); // Hide all input sections
  toggleWallStateApp('CAVEDoor', false, true, true);
  toggleWallStateApp('SA', false, true, true);
  toggleWallStateApp('CurvedDoor', false, true, true);
  toggleStateCurvedPA(false, true, true);
  toggleStateCAVEPA(false, true, true);
  sendValue('/matrix/state/settings/flex_channel/*/mute', 1); // Mute all inputs
  sendValue('/matrix/state/settings/sum_bus_master/*/mute', 1); // Mute all sum busses
  document.getElementById('Stereo_Walls-Total_VU-bar').style.height = '0%';
  document.getElementById('Stereo_Walls-Total_VU-number').innerText = '--';
}

export function toggleWallStateApp(id, state, btn_disable, slider_disable = false)
{
	const btn = document.getElementById('btn-wall-' + id);
	const slider = document.getElementById('volume-' + id + '-slider');

	if (!btn || !slider) return;

	const isActive = btn.classList.contains('active-input');
	const turnOn = state === undefined ? !isActive : state;

	if (turnOn) {
		btn.classList.add('active-input');
		btn.innerText = "On";
    btn.disabled = btn_disable;
		slider.disabled = slider_disable;
		slider.style.opacity = "1.0";
    sendValue('/app/Stereo_Walls/osc/' + id + '/Switch', 1);
	} else {
		btn.classList.remove('active-input');
		btn.innerText = "Off";
    btn.disabled = btn_disable;
		slider.disabled = slider_disable;
		slider.style.opacity = "0.5";
    sendValue('/app/Stereo_Walls/osc/' + id + '/Switch', 0);
	}
}

export function toggleStateCurvedPA(state, btn_disable, slider_disable = false)
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
    const vol = document.getElementById('sum_bus_CurvedPA-gain');
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
    const vol = document.getElementById('sum_bus_CAVEPA-gain');
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

export function setVolumeCurvedPA(value)
{
  sendValue('/matrix/state/settings/sum_bus_master/0/gain', value);
  sendValue('/matrix/state/settings/sum_bus_master/1/gain', value);
  document.getElementById('sum_bus_CurvedPA-volume-number').innerText = value + ' dB';
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
