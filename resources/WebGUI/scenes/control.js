	/* States: 
	  Line (&#8212), black: request started
		Line, red: request in progress
		Open circle (&#9711), black: shutdown
		Closed circle (&#11044), black: switched on
		Closed circle, green: unmuted
		Closed circle, blue: muted
		Closed circle, red: error
		Closed circle, yellow: state unknown
	*/

export function init()
{
	
	const amps_status = [ 
		document.querySelector('#amp1-status'), 
		document.querySelector('#amp2-status'), 
		document.querySelector('#amp3-status'), 
		document.querySelector('#amp4-status'), 
		document.querySelector('#amp5-status'), 
		document.querySelector('#amp6-status'), 
		document.querySelector('#amp7-status'), 
		document.querySelector('#amp8-status'), ];
			
		// Update the status from the amps
	document.querySelector('#control-Update_Status').addEventListener('click', async () => 
	{
		for (var i=1; i<=8; i++)
		{
			amps_status[i-1].innerHTML = "&#8212";
			amps_status[i-1].style.color = "black";
		}
    document.getElementById('control-Update_Status').disabled = true;
	
		var x = await fetchHttpResponse('http://172.16.60.111/api/get/general/model', 300);
		console.log("Model", x);
		if(x.error)
		{
      for (var i=1; i<=8; i++)
      {
        amps_status[i-1].innerHTML = "&#9711";
        amps_status[i-1].style.color = "black";
      }
        //alert ("Error happened: " + x.error.name + "\n" + x.error.message + "\n\nIf the error is something with failed to fetch, try to install the add-on 'Allow CORS' and switch it on."); 
			document.getElementById('control-Update_Status').disabled = false;
      return;
		}
			
		for (var i=1; i<=8; i++)
		{
			x = await fetchHttpResponse('http://172.16.60.11' + i + '/api/get/system/mute', 300);
			console.log("Mute", x);
			if(x.error)
			{
        console.log(x.error);
				amps_status[i-1].innerHTML = "&#11044";
				amps_status[i-1].style.color = "red";
			}
			else
			{
				amps_status[i-1].innerHTML = "&#11044";
				switch (x.response)
				{
					case "1": // muted
						amps_status[i-1].style.color = "blue";
						break;
					case "0": // unmuted
						amps_status[i-1].style.color = "green";
						break; 
					default: // unknown
						amps_status[i-1].style.color = "yellow";
				}
			}
		}
    document.getElementById('control-Update_Status').disabled = false;
	});	

	// Controls the status when running "PDU Start"
	document.getElementById('pdu_start-status').addEventListener('updated', (e) => 
	{
		const status = document.getElementById('pdu_start-status');
		for (var i=1; i<=8; i++)
		{
			switch (status.innerText)
			{
				case "alive": // script starting, feedback to the user
					amps_status[i-1].innerHTML = "&#8212";
					amps_status[i-1].style.color = "black";
					break;
				case "ready": // script running, devices powering up
					amps_status[i-1].innerHTML = "&#8212";
					amps_status[i-1].style.color = "red";
					break; 
				case "success": // script done, devices have power
					amps_status[i-1].innerHTML = "&#11044";
					amps_status[i-1].style.color = "black";
          document.getElementById('system_start-button').disabled = false;
          break;
        default:
          document.getElementById('system_start-button').disabled = false;
			}
		}
	});

	// Controls the status when running "PDU Shutdown"
	document.getElementById('pdu_shutdown-status').addEventListener('updated', (e) => 
	{
		const status = document.getElementById('pdu_shutdown-status');
		for (var i=1; i<=8; i++)
		{
			switch (status.innerText)
			{
				case "alive": // script starting, feedback to the user
					amps_status[i-1].innerHTML = "&#8212";
					amps_status[i-1].style.color = "black";
					break;
				case "ready": // script running, devices powering down
					amps_status[i-1].innerHTML = "&#8212";
					amps_status[i-1].style.color = "blue";
					break; 
				case "success": // script done, devices switched off
					amps_status[i-1].innerHTML = "&#9711";
					amps_status[i-1].style.color = "black";
          document.getElementById('system_shutdown-button').disabled = false;
          break;
        default:
          document.getElementById('system_shutdown-button').disabled = false;
			}
		}
	});

	// Controls the status when running "Amps: Unmute"
	document.getElementById('amps_unmute-status').addEventListener('updated', (e) => 
	{
		const status = document.getElementById('amps_unmute-status');
		for (var i=1; i<=8; i++)
		{
			switch (status.innerText)
			{
				case "alive": // script starting, feedback to the user
					amps_status[i-1].innerHTML = "&#8212";
					amps_status[i-1].style.color = "black";
					break;
				case "ready": // script running, feedback to the user 
					amps_status[i-1].innerHTML = "&#8212";
					amps_status[i-1].style.color = "red";
					break; 
				case "success": // script done, amps unmuted
					amps_status[i-1].innerHTML = "&#11044";
					amps_status[i-1].style.color = "green";
          document.getElementById('amps_unmute-button').disabled = false;
          break;
        default:
          document.getElementById('amps_unmute-button').disabled = false;
			}
		}
	});

	// Controls the status when running "Amps: Mute"
	document.getElementById('amps_mute-status').addEventListener('updated', (e) => 
	{
		const status = document.getElementById('amps_mute-status');
		for (var i=1; i<=8; i++)
		{
			switch (status.innerText)
			{
				case "alive": // script starting, feedback to the user
					amps_status[i-1].innerHTML = "&#8212";
					amps_status[i-1].style.color = "black";
					break;
				case "ready": // script running, feedback to the user
					amps_status[i-1].innerHTML = "&#8212";
					amps_status[i-1].style.color = "red";
					break; 
				case "success": // script done, amps muted
					amps_status[i-1].innerHTML = "&#11044";
					amps_status[i-1].style.color = "blue";
          document.getElementById('amps_mute-button').disabled = false;
          break;
        default:
          document.getElementById('amps_mute-button').disabled = false;
			}
		}
	});

	// Controls the status when running "Amps: Shutdown"
	document.getElementById('amps_shutdown-status').addEventListener('updated', (e) => 
	{
		const status = document.getElementById('amps_shutdown-status');
		for (var i=1; i<=8; i++)
		{
			switch (status.innerText)
			{
				case "alive": // script starting, feedback to the user
					amps_status[i-1].innerHTML = "&#8212";
					amps_status[i-1].style.color = "black";
					break;
				case "ready": // script running, feedback to the user with the old state
					amps_status[i-1].innerHTML = "&#8212";
					amps_status[i-1].style.color = "red";
					break; 
				case "success": // script done, amps shut down
					amps_status[i-1].innerHTML = "&#11044";
					amps_status[i-1].style.color = "black";
          const st = document.getElementById('system-status');
          st.innerHTML = '<span style="color: white;">&#9711;</span>';  // Sytem status to unknown
          document.getElementById('amps_shutdown-button').disabled = false;
          break;
        default:
          document.getElementById('amps_shutdown-button').disabled = false;
			}
		}
	});
}
	
export function GetMatrixStatus()
{
	sendNoArgs('/matrix/mgr/state');
	document.getElementById('matrix-Update_Status').disabled = true;
  document.getElementById('control-matrix_status').innerHTML = "&#9711";
}

export function AmpsShutdown()
{
	const dialog = document.getElementById('confirmDialog');
	document.getElementById('confirmTitle').innerText = "Shutdown the Amplifiers?";
	document.getElementById('confirmText').innerText = "This will shutdown the audio amplifiers. If they are unmuted, this will create a loud 'pop'. \n\nIf the amplifiers are unmuted, press Cancel and mute the amplifiers. \n\nTo shutdown the amplifiers, press OK.";
	dialog.showModal();

	document.getElementById('okBtn').onclick = () => 
	{
		dialog.close();
    send('/app/amps_shutdown/control', 'launch');
    document.getElementById('amps_shutdown-button').disabled = true;
	};

	document.getElementById('cancelBtn').onclick = () => 
	{
		dialog.close();
	};
}

export function SystemStart()
{
	send('/app/pdu_start/control', 'launch');
  document.getElementById('system_start-button').disabled = true;
}

export function SystemShutdown()
{
  sendNoArgs('/matrix/cmd/recall_snapshot_5');
  
	const dialog = document.getElementById('confirmDialog');
	document.getElementById('confirmTitle').innerText = "Shutdown the Audio System?";
	document.getElementById('confirmText').innerText = "This will shutdown the audio system. If the amplifiers are unmuted, this will create a loud 'pop'. If the amplifiers have not been shutdown, this might create a damage in their configuration. \n\nIf the amplifiers are not muted or shutdown, press Cancel and mute and shutdown them. \n\nTo shutdown the audio system, press OK.";
	dialog.showModal();

	document.getElementById('okBtn').onclick = () => 
	{
		dialog.close();
    send('/app/pdu_shutdown/control', 'launch');
    document.getElementById('system_shutdown-button').disabled = true;
	};

	document.getElementById('cancelBtn').onclick = () => 
	{
		dialog.close();
	};
	
}

export function CloseAllApps()
{
  const dialog = document.getElementById('confirmDialog');
	document.getElementById('confirmTitle').innerText = "Close all applications?";
	document.getElementById('confirmText').innerText = "This will close all potentially running applications and unlock all scenes. \n\nTo close all applications, press OK.";
	dialog.showModal();

	document.getElementById('okBtn').onclick = () => 
	{  
    dialog.close();
    send('/app/*/control', 'quit');
    document.querySelectorAll('.launched-scene').forEach(btn => unlockScene(btn.id.replace('button-', ''),false));
  };

	document.getElementById('cancelBtn').onclick = () => 
	{
		dialog.close();
	};
}

export function AmpStatusLegend()
{
	const dialog = document.getElementById('infoDialog');
	document.getElementById('infoTitle').innerText = "Amplifier Status Legend";
	document.getElementById('infoText').innerHTML = `
    <div>
    <table style="margin: 0 auto; width:50%;">
    <thead><tr><th>Symbol</th><th>Meaning</th></tr></thead>
    <tbody>
    <tr><td><span style="color: black;">&#8212;</span></td> <td>Request started</td></tr>
		<tr><td><span style="color: red;">&#8212;</span></td> <td>Request in progress</td></tr>
		<tr><td><span style="color: black;">&#9711;</span></td> <td>Shutdown</td></tr>
		<tr><td><span style="color: black;">&#11044;</span></td> <td>Switched on</td></tr>
		<tr><td><span style="color: green;">&#11044;</span></td> <td>Unmuted</td></tr>
		<tr><td><span style="color: blue;">&#11044;</span></td> <td>Muted</td></tr>
		<tr><td><span style="color: red;">&#11044;</span></td> <td>Error</td></tr>
		<tr><td><span style="color: yellow;">&#11044;</span></td> <td>State unknown</td></tr>
    </tbody></table></div>`;  
	dialog.showModal();

	document.getElementById('infoOkBtn').onclick = () => 
	{
		dialog.close();
	};
}

