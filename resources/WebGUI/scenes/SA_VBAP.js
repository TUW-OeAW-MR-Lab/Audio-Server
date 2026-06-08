let state_curved;
let state_cave;

export function init()
{
  document.getElementById("btn-SA_VBAP-launch").disabled = false;
  document.getElementById("SA_VBAP-status").innerText = "initialized";
	document.getElementById("SA_VBAP-status").style.backgroundColor = "";	
	document.getElementById("SA_VBAP-Control_Version").innerText = "---";
	document.getElementById("SA_VBAP-Control_SampleRate").innerText = "---"; 
	document.getElementById("SA_VBAP-Control_Door").innerText = "---";
  document.getElementById('SA_VBAP-Total_VU-bar').style.height = '0%';
  document.getElementById('SA_VBAP-Total_VU-number').innerText = '--';
  
  enableInputSelectButtons(false);
  document.getElementById("btn-input-select-DANTE_CurvedLEDPC_Channel_3").classList.remove('active-input');
  document.getElementById("btn-input-select-DANTE_CurvedLEDPC").classList.remove('active-input');
  document.getElementById("btn-input-select-DANTE_Mobile").classList.remove('active-input');

  document.getElementById("SA_VBAP-Door_Curved_Open").disabled = true;
  document.getElementById("SA_VBAP-Door_Curved_Close").disabled = true;
  document.getElementById("SA_VBAP-Door_CAVE_Open").disabled = true;
  document.getElementById("SA_VBAP-Door_CAVE_Close").disabled = true;
  document.getElementById('volume-slider').disabled = true;
  document.getElementById('subwoofer-slider').disabled = true;

  document.getElementById('SA_VBAP-Total_VU').addEventListener('updated', (e) => 
  {
    const vu = document.getElementById('SA_VBAP-Total_VU');
    document.getElementById('SA_VBAP-Total_VU-bar').style.height = String(Math.round(vu.innerText)+100) + '%';
    document.getElementById('SA_VBAP-Total_VU-number').innerText = Math.round(vu.innerText) + ' dB';
  });			

  document.getElementById('SA_VBAP-Total_Volume').addEventListener('updated', (e) => 
  {
    const vol = document.getElementById('SA_VBAP-Total_Volume');
    document.getElementById('volume-slider').value = Math.round(vol.innerText);
    document.getElementById('volume-number').innerText = Math.round(vol.innerText) + " dB";
  });	

  const subLabel = document.getElementById('SA_VBAP-Total_Subwoofer');
  if (subLabel) {
    subLabel.addEventListener('updated', (e) => 
    {
      const sub = document.getElementById('SA_VBAP-Total_Subwoofer');
      document.getElementById('subwoofer-slider').value = Math.round(sub.innerText);
      document.getElementById('subwoofer-number').innerText = Math.round(sub.innerText) + " dB";
    });
  }

  document.getElementById('SA_VBAP-Control_Door').addEventListener('updated', (e) => 
  {
    Door_Status();
  });	
}

export function launch()
{
  document.getElementById("btn-SA_VBAP-launch").disabled = true;
  document.getElementById("SA_VBAP-status").innerText = "initialized";
	document.getElementById("SA_VBAP-status").style.backgroundColor = "";	
	document.getElementById("SA_VBAP-Control_Version").innerText = "---";
	document.getElementById("SA_VBAP-Control_SampleRate").innerText = "---"; 
	document.getElementById("SA_VBAP-Control_Door").innerText = "---";
  enableInputSelectButtons(false);
  document.getElementById('volume-slider').disabled = true;
  document.getElementById('subwoofer-slider').disabled = true;
}

export function connect()
{
	state = document.getElementById("SA_VBAP-status");
	state.innerText = "connecting...";
	state.style.backgroundColor = "blue";
	sendResponse('/app/SA_VBAP/osc/Control/Response', 9336);
	sendNoArgs('/app/SA_VBAP/osc/Control/Version');
	secWaited = 0;
	setTimeout(() => { checkConnection(); }, 300);
}

export function checkConnection()
{
	const timeout = 10000; // timeout in ms
	secWaited += 100;
	state = document.getElementById("SA_VBAP-Control_Version");
	console.log("waiting for " + secWaited/1000 + "s, version=" + state.innerText);
	switch (state.innerText)
	{
		case "---":
			if (secWaited <= timeout)
			{		// try again
				sendResponse('/app/SA_VBAP/osc/Control/Response', 9336);
				sendNoArgs('/app/SA_VBAP/osc/Control/Version');		
				setTimeout(() => { checkConnection(); }, 100);
			}
			break;
		default: // done!
      console.log("SA_VBAP: connected");
      document.getElementById("btn-SA_VBAP-launch").disabled = false;
			state = document.getElementById("SA_VBAP-status");
			state.innerText = "connected";
			state.style.backgroundColor = "";
			sendNoArgs('/app/SA_VBAP/osc/Control/SampleRate');
			sendNoArgs('/app/SA_VBAP/osc/Total/Volume');
      sendNoArgs('/app/SA_VBAP/osc/Subwoofer/Volume');
      sendNoArgs('/app/SA_VBAP/osc/Control/Door');
      enableInputSelectButtons(true);
      document.getElementById('volume-slider').disabled = false;
      document.getElementById('subwoofer-slider').disabled = false;
			break;
	}
	if (secWaited > timeout)
	{ 	// time out -> clean up
		state = document.getElementById("SA_VBAP-status");
		state.innerText = "time out while connecting";
		state.style.backgroundColor = "red";
    document.getElementById("btn-SA_VBAP-launch").disabled = false;
	}
}		

export function Door_Change({ state_cave = undefined, state_curved =  undefined })
{
  if(state_curved === undefined)
    state_curved = document.getElementById("SA_VBAP-Door_Curved_Close").disabled;
  if(state_cave === undefined)
    state_cave = document.getElementById("SA_VBAP-Door_CAVE_Close").disabled;

  if(state_cave) state = 'C'; else state = 'O';
  if(state_curved) state = `${state}C`; else state = `${state}O`;

  send('/app/SA_VBAP/osc/Control/Door/Set', state);
  sendNoArgs('/app/SA_VBAP/osc/Control/Door');

  document.getElementById("SA_VBAP-Door_Curved_Open").disabled = true;
  document.getElementById("SA_VBAP-Door_Curved_Close").disabled = true;
  document.getElementById("SA_VBAP-Door_CAVE_Open").disabled = true;
  document.getElementById("SA_VBAP-Door_CAVE_Close").disabled = true;
}

export function Door_Status()
{
  state = document.getElementById("SA_VBAP-Control_Door");
  switch (state.innerText)
  {
    case "OO":
      document.getElementById("SA_VBAP-Door_Curved_Open").disabled = true;
      document.getElementById("SA_VBAP-Door_Curved_Close").disabled = false;
      document.getElementById("SA_VBAP-Door_Curved").innerText = "Door is open";
      document.getElementById("SA_VBAP-Door_CAVE_Open").disabled = true;
      document.getElementById("SA_VBAP-Door_CAVE_Close").disabled = false;
      document.getElementById("SA_VBAP-Door_CAVE").innerText = "Door is open";
      break;
    case "CO":
      document.getElementById("SA_VBAP-Door_Curved_Open").disabled = true;
      document.getElementById("SA_VBAP-Door_Curved_Close").disabled = false;
      document.getElementById("SA_VBAP-Door_Curved").innerText = "Door is open";
      document.getElementById("SA_VBAP-Door_CAVE_Open").disabled = false;
      document.getElementById("SA_VBAP-Door_CAVE_Close").disabled = true;
      break;
    case "OC":
      document.getElementById("SA_VBAP-Door_Curved_Open").disabled = false;
      document.getElementById("SA_VBAP-Door_Curved_Close").disabled = true;
      document.getElementById("SA_VBAP-Door_Curved").innerText = "Door is closed";
      document.getElementById("SA_VBAP-Door_CAVE_Open").disabled = true;
      document.getElementById("SA_VBAP-Door_CAVE_Close").disabled = false;
      document.getElementById("SA_VBAP-Door_CAVE").innerText = "Door is open";
      break;
    case "CC":
      document.getElementById("SA_VBAP-Door_Curved_Open").disabled = false;
      document.getElementById("SA_VBAP-Door_Curved_Close").disabled = true;
      document.getElementById("SA_VBAP-Door_Curved").innerText = "Door is closed";
      document.getElementById("SA_VBAP-Door_CAVE_Open").disabled = false;
      document.getElementById("SA_VBAP-Door_CAVE_Close").disabled = true;
      document.getElementById("SA_VBAP-Door_CAVE").innerText = "Door is closed";
      break;
    default:
      document.getElementById("SA_VBAP-Door_Curved_Open").disabled = true;
      document.getElementById("SA_VBAP-Door_Curved_Close").disabled = true;
      document.getElementById("SA_VBAP-Door_Curved").innerText = "Door is unknown";
      document.getElementById("SA_VBAP-Door_CAVE_Open").disabled = true;
      document.getElementById("SA_VBAP-Door_CAVE_Close").disabled = true;
      document.getElementById("SA_VBAP-Door_CAVE").innerText = "Door is unknown";
      break;
  }
}

export function SetDanteLedPcChannel3()
{
  if (setInputButtonExclusively('DANTE_CurvedLEDPC_Channel_3', 'SA_VBAP'))
  {
    sendValue('/app/SA_VBAP/osc/VirtualSource/3/Switch', 1);
    sendValue('/app/SA_VBAP/osc/VirtualSource/3/Volume/Set', 0);
    sendValues3('/app/SA_VBAP/osc/VirtualSource/3/Position/Set', 0, 0, 3.2);
  }
  else
  { 
    send('/app/SA_VBAP/osc/VirtualSource/3/Switch', 0);
  }
}

export function SetDanteLedPc()
{
  setInputButtonExclusively('DANTE_CurvedLEDPC', 'SA_VBAP');
  sendValue('/app/SA_VBAP/osc/VirtualSource/3/Switch', 0);
  sendValue('/matrix/state/settings/sum_bus_master/0/mute', 1); // Mute Curved PA Left
  sendValue('/matrix/state/settings/sum_bus_master/1/mute', 1); // Mute Curved PA Right
  sendValue('/matrix/state/settings/sum_bus_master/2/mute', 0); // Unmute Audio PC Left
  sendValue('/matrix/state/settings/sum_bus_master/3/mute', 0); // Unmute Audio PC Right
  sendValue('/matrix/state/settings/sum_bus_master/2/gain', 0); // set Audio PC Left to 0 dB
  sendValue('/matrix/state/settings/sum_bus_master/3/gain', 0); // set Audio PC Right to 0 dB
}

export function SetDanteMobile()
{
  setInputButtonExclusively('DANTE_Mobile', 'SA_VBAP');
  sendValue('/app/SA_VBAP/osc/VirtualSource/3/Switch', 0);
  sendValue('/matrix/state/settings/sum_bus_master/*/mute', 1); // Mute all Sum Busses
}

export function quit()
{
  quitApp('SA_VBAP', 'SA_VBAP', SceneModule.init);
  sendValue('/matrix/state/settings/sum_bus_master/*/mute', 1); // Mute all Sum Busses
}