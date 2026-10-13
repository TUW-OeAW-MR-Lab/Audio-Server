var oscPort = new osc.WebSocketPort({
		url: "/ws", // WebSocket resource at current URL context.
		metadata: true
});

// Reconnection & Connection State Management
let reconnectTimer = null;
let reconnectDelay = 1000;
const INITIAL_RECONNECT_DELAY = 1000;
const MAX_RECONNECT_DELAY = 5000;
let isConnecting = false;
let isConnected = false;
let lastMessageTime = Date.now();

function updateConnectionUI(status) {
	const el = document.getElementById('matrix_status');
	if (el) {
		switch (status) {
			case 'connected':
				el.innerHTML = '<span style="color: green;" title="Connected to audio server">&#11044;</span>';
				break;
			case 'connecting':
			case 'reconnecting':
				el.innerHTML = '<span style="color: orange;" title="Connecting to audio server...">&#9711;</span>';
				break;
			case 'disconnected':
			default:
				el.innerHTML = '<span style="color: red;" title="Disconnected from audio server">&#9711;</span>';
				break;
		}
	}
	const cms = document.getElementById('control-matrix_status');
	if (cms) {
		cms.innerHTML = (status === 'connected')
			? '<span style="color: green;">&#11044;</span>'
			: '<span style="color: red;">&#9711;</span>';
	}
}

function openOscPort() {
	if (oscPort.socket) {
		if (oscPort.socket.readyState === WebSocket.OPEN) {
			isConnected = true;
			isConnecting = false;
			return;
		}
		if (oscPort.socket.readyState === WebSocket.CONNECTING) {
			return;
		}
		// Clean up dead or closing socket event handlers before reopening
		try {
			oscPort.socket.onopen = null;
			oscPort.socket.onerror = null;
			oscPort.socket.onmessage = null;
			oscPort.socket.onclose = null;
			if (oscPort.socket.readyState !== WebSocket.CLOSED) {
				oscPort.socket.close();
			}
		} catch (e) {
			// ignore cleanup error
		}
		oscPort.socket = null;
	}

	oscPort.open();
}

function tryConnect() {
	if (isConnecting) return;
	isConnecting = true;
	updateConnectionUI('connecting');
	try {
		openOscPort();
	} catch (err) {
		console.warn("Failed to initiate WebSocket connection:", err);
		isConnecting = false;
		scheduleReconnect();
	}
}

let reconnectDialogTimer = null;
let wakeUpTimestamp = 0;

function showReconnectDialog(duration = 2000) {
	const dialog = document.getElementById('reconnectDialog') || document.getElementById('infoDialog');
	if (!dialog) return;

	const title = document.getElementById('reconnectTitle') || document.getElementById('infoTitle');
	const text = document.getElementById('reconnectText') || document.getElementById('infoText');
	const okBtn = document.getElementById('infoOkBtn');

	if (title) title.innerText = "Reconnecting";
	if (text) text.innerText = "Reconnecting to audio server...";
	if (okBtn && dialog.id === 'infoDialog') okBtn.style.display = "none";

	if (reconnectDialogTimer) {
		clearTimeout(reconnectDialogTimer);
		reconnectDialogTimer = null;
	}

	try {
		if (!dialog.open) {
			dialog.showModal();
		}
	} catch (e) {
		console.warn("Could not show reconnect dialog:", e);
	}

	reconnectDialogTimer = setTimeout(() => {
		reconnectDialogTimer = null;
		try {
			if (dialog.open) {
				dialog.close();
			}
		} catch (e) {}
		if (okBtn && dialog.id === 'infoDialog') okBtn.style.display = "";
	}, duration);
}

function scheduleReconnect(immediate = false) {
	isConnected = false;
	if (reconnectTimer) {
		if (!immediate) return;
		clearTimeout(reconnectTimer);
		reconnectTimer = null;
	}

	// Show reconnect dialog if reconnecting right after wake up
	if (Date.now() - wakeUpTimestamp < 3000) {
		showReconnectDialog(2000);
	}

	const delay = immediate ? 0 : reconnectDelay;
	updateConnectionUI('reconnecting');

	reconnectTimer = setTimeout(() => {
		reconnectTimer = null;
		reconnectDelay = Math.min(reconnectDelay * 1.5, MAX_RECONNECT_DELAY);
		tryConnect();
	}, delay);
}

function checkConnectionAndReconnect(fromWakeup = false) {
	if (!oscPort.socket || oscPort.socket.readyState !== WebSocket.OPEN) {
		if (fromWakeup) {
			showReconnectDialog(2000);
		}
		scheduleReconnect(true);
	} else {
		// Verify socket liveness
		try {
			oscPort.send({ address: "/ping" });
		} catch (e) {
			if (fromWakeup) {
				showReconnectDialog(2000);
			}
			scheduleReconnect(true);
		}
	}
}

// Initial connection
tryConnect();

// Safe send wrapper preventing uncaught exceptions when socket is not open
function safeSend(packet) {
	if (oscPort.socket && oscPort.socket.readyState === WebSocket.OPEN) {
		try {
			oscPort.send(packet);
			return true;
		} catch (err) {
			console.warn("Error sending OSC packet:", err);
			scheduleReconnect(true);
			return false;
		}
	} else {
		console.warn("Cannot send OSC message (port not open):", packet.address);
		scheduleReconnect(true);
		return false;
	}
}

function send(address, value) 
{
	safeSend({ 
		address: address,
		args: [{ type: "s", value: value }]
	});
}

function sendValue(address, value) 
{
	safeSend({
		address: address,
		args: [{ type: "f", value: value }]
	});
}

function sendValues3(address, value1, value2, value3) 
{
	safeSend({
		address: address,
		args: [{ type: "f", value: value1 }, { type: "f", value: value2 }, { type: "f", value: value3 }]
	});
}
			
function sendResponse(address,port) 
{
	safeSend({
		address: address,
		args: [
			{ type: "s", value: "connect" },
			{ type: "s", value: "localhost" },
			{ type: "i", value: port }
		]
	});
}

function sendNoArgs(address) 
{
	safeSend({ address: address });
}

oscPort.on("ready", function () 
{
	console.log("OSC WebSocket connected.");
	isConnecting = false;
	isConnected = true;
	reconnectDelay = INITIAL_RECONNECT_DELAY;
	if (reconnectTimer) {
		clearTimeout(reconnectTimer);
		reconnectTimer = null;
	}
	lastMessageTime = Date.now();
	updateConnectionUI('connected');

	// If reconnect dialog is currently active, update it to connected
	if (reconnectDialogTimer) {
		const title = document.getElementById('reconnectTitle');
		const text = document.getElementById('reconnectText');
		if (title) title.innerText = "Connected";
		if (text) text.innerText = "Connection restored.";
	}

	try {
		oscPort.send({ address: "/ping" });
	} catch (e) {}

	// Re-sync active scene status if available
	if (typeof SceneModule !== 'undefined' && SceneModule && typeof SceneModule.GetStatus === 'function') {
		try {
			SceneModule.GetStatus();
		} catch (e) {
			console.warn("Error re-syncing scene status on reconnect:", e);
		}
	}
});

oscPort.on("close", function (e) 
{
	console.warn("OSC WebSocket closed:", e);
	isConnected = false;
	isConnecting = false;
	updateConnectionUI('disconnected');
	scheduleReconnect();
});

oscPort.on("error", function (err) 
{
	console.warn("OSC WebSocket error:", err);
	isConnected = false;
	isConnecting = false;
	updateConnectionUI('disconnected');
	scheduleReconnect();
});

// Tablet lifecycle & network reconnect triggers
document.addEventListener("visibilitychange", function () {
	if (document.visibilityState === "visible") {
		wakeUpTimestamp = Date.now();
		console.log("Tablet visibility changed to visible, checking connection...");
		checkConnectionAndReconnect(true);
	}
});

window.addEventListener("online", function () {
	console.log("Network online event detected, reconnecting OSC...");
	scheduleReconnect(true);
});

window.addEventListener("pageshow", function () {
	checkConnectionAndReconnect();
});

window.addEventListener("focus", function () {
	checkConnectionAndReconnect();
});

// Periodic heartbeat & zombie connection detection
setInterval(function () {
	if (oscPort.socket && oscPort.socket.readyState === WebSocket.OPEN) {
		const now = Date.now();
		if (now - lastMessageTime > 30000) {
			console.warn("OSC connection silent for > 30s, reconnecting...");
			try {
				oscPort.socket.close();
			} catch (e) {}
			scheduleReconnect(true);
			return;
		}
		try {
			oscPort.send({ address: "/ping" });
		} catch (e) {
			scheduleReconnect(true);
		}
	} else if (!isConnecting && !reconnectTimer) {
		scheduleReconnect();
	}
}, 10000);

oscPort.on("message", function (oscMsg) 
{
	lastMessageTime = Date.now();
	isConnected = true;
	pathstr= oscMsg.address.substring(1);
	path = pathstr.split("/");
		// Format of the message: device/scenename/osc/command1/command2/.../commandN
	switch (path[0])
	{
		case 'app': // Messages related to an app
    //console.log("OSC message related to an app received: ", oscMsg);
			switch (path[2])
			{
				case 'state': // Launch status received 
					const st = document.getElementById(path[1] + "-status");
					st.innerHTML = oscMsg.args[1].value;
					const statusEvent = new CustomEvent('updated', { detail: { time: Date.now() } });
					st.dispatchEvent(statusEvent); // dispatch an event that the status has changed
					break;
	
				case 'osc': // OSC message from the app received
					commands = (pathstr.substring(path[0].length+path[1].length+path[2].length+3));
						// Format of the variable as scenename-command1_command2_..._commandN
					const el = document.getElementById(path[1] + "-" + commands.replaceAll("/", "_"));
					el.innerHTML = oscMsg.args[0].value;
					const elementEvent = new CustomEvent('updated', { detail: { time: Date.now() } });
					el.dispatchEvent(elementEvent); // dispatch an event that the variable has changed
					break;
			};
			break;

		case 'matrix': // Messages related to the matrix
			//console.log("Matrix message received", oscMsg);
      switch (path[1])
			{
        case 'status': // Status received, path[1] == 'status'
          document.getElementById('matrix_status').innerHTML = '<span style="color: green;">&#11044;</span>';  // update sign of life on any message from the matrix
          switch(path[2])
          {
            case 'ears_status':
              break;
            default:
              console.log("Matrix unknown status message received: " + path[2]);
              break;
          }
          break;

        case 'state':
          document.getElementById('matrix_status').innerHTML = '<span style="color: green;">&#11044;</span>';  // update sign of life on any message from the matrix
          switch(path[2])
          {
            case 'settings':
              sendResponseMatrixStateSettings(path, oscMsg);
              break;

            case 'fan': // Fan status received, path[1] == 'fan'
              //console.log("Fan message received", oscMsg);
              const st = document.getElementById('fan-' + path[3]);
              if (st != null)
              {
                st.innerHTML = Math.round(oscMsg.args[0].value*100)/100;
                //console.log("Fan status received for " + path[2], );
              }
              else
              {
                console.log("No OSC message handler for: 'fan-" + path[3] + "'");
              }
              break;
          }
          break;

        case 'mgr':
          //console.log("Matrix mgr message received:", oscMsg);
          switch(path[2])
          {
            case 'state':
              if (oscMsg.args[0].value == 5)
              { // connected
                document.getElementById('matrix_status').innerHTML = '<span style="color: green;">&#11044;</span>';
                document.getElementById('control-matrix_status').innerHTML = '<span style="color: green;">&#11044;</span>';
                document.getElementById('matrix-Update_Status').disabled = false;
              }
              else
              { // init, disconnected, waitingToReconnect, disconnecting, connecting
                document.getElementById('matrix_status').innerHTML = '<span style="color: white;">&#9711;</span>';
                document.getElementById('control-matrix_status').innerHTML = '<span style="color: black;">&#9711;</span>';
                document.getElementById('matrix-Update_Status').disabled = false;
              }
              break;
            default:
              console.log("Unknown matrix mgr message received: " + path[2]);
              break;
          }
          break;
        default: 
          console.log("Unknown matrix message received:", oscMsg);
          break;

      }
      break; 

    case 'pong':
      console.log("Pong message received");
      break;

		default:
			console.log("No OSC message handler: ", oscMsg);
			break;
	}
});


function sendResponseMatrixStateSettings(path, oscMsg)
{
  switch (path[3])
  {
    case 'easy_routing':
      // console.log("Received easy routing #"+ path[4] + " to " + oscMsg.args[0].value);
      break;
    case 'flex_channel': 
      switch (path[5])
      {
        case 'mute':
          if (inputFlexChannelMap[path[4]]!="")
          {
            //console.log("Received mute #"+ path[4] + " -> " + inputFlexChannelMap[path[4]]);
            toggleInputState(inputFlexChannelMap[path[4]], !oscMsg.args[0].value);
          }
          break;

        case 'gain':
          if (inputFlexChannelMap[path[4]]!="")
          {
            var st = document.getElementById('slider-input-' + inputFlexChannelMap[path[4]]);
            st.value = oscMsg.args[0].value;  
            var statusEvent = new CustomEvent('updated', { detail: { time: Date.now() } });
            st.dispatchEvent(statusEvent); // dispatch an event that the status has changed    
          }
          break;
      }
      break;

    case 'sum_bus_assign_dsp':
      //console.log("Received sum bus assign dsp #" + path[4] + " and segment #" + path[6] + " set to: " + oscMsg.args[0].value);
      break;

    case 'sum_bus_master':
      //console.log(oscMsg);
      switch (path[5])
      {
        case 'mute':
          switch (path[4])
          {
            case "0":
            case "1":
              var st = document.getElementById('btn-sum_bus_CurvedPA-mute');
              break;
            case "2":
            case "3":
              var st = document.getElementById('btn-sum_bus_AudioPC-mute');
              break;
            case "8":
              var st = document.getElementById('btn-sum_bus_CAVE_FL-mute') || document.getElementById('btn-sum_bus_CAVEPA-mute');
              break;
            case "9":
              var st = document.getElementById('btn-sum_bus_CAVE_FR-mute') || document.getElementById('btn-sum_bus_CAVEPA-mute');
              break;
            case "10":
              var st = document.getElementById('btn-sum_bus_CAVE_FWL-mute') || document.getElementById('btn-sum_bus_CAVEPA-mute');
              break;
            case "11":
              var st = document.getElementById('btn-sum_bus_CAVE_FWR-mute') || document.getElementById('btn-sum_bus_CAVEPA-mute');
              break;
            case "12":
              var st = document.getElementById('btn-sum_bus_CAVE_SL-mute') || document.getElementById('btn-sum_bus_CAVEPA-mute');
              break;
            case "13":
              var st = document.getElementById('btn-sum_bus_CAVE_SR-mute') || document.getElementById('btn-sum_bus_CAVEPA-mute');
              break;
            case "14":
              var st = document.getElementById('btn-sum_bus_CAVE_BL-mute') || document.getElementById('btn-sum_bus_CAVEPA-mute');
              break;
            case "15":
              var st = document.getElementById('btn-sum_bus_CAVE_BR-mute') || document.getElementById('btn-sum_bus_CAVEPA-mute');
              break;
            case "24":
            case "25":
              var st = document.getElementById('btn-sum_bus_HP1-mute');
              break;
            case "26":
            case "27":
              var st = document.getElementById('btn-sum_bus_HP2-mute');
              break;
            case "28":
            case "29":
              var st = document.getElementById('btn-sum_bus_HP3-mute');
              break;
            case "30":
            case "31":
              var st = document.getElementById('btn-sum_bus_HP4-mute');
              break;
          }
          if (!st) {
            //console.log("No OSC message handler for bus_master " + path[4] + " mute");
            return;
          }
          if (oscMsg.args[0].value == 1)
          {
            st.classList.remove('active-input');
            st.innerText = "Off";
          }
          else
          {
            st.classList.add('active-input');
            st.innerText = "On";
          }
          var statusEvent = new CustomEvent('updated', { detail: { time: Date.now() } });
          st.dispatchEvent(statusEvent); // dispatch an event that the status has changed                      
          break;

        case 'gain':
          //console.log("Received bus master gain #"+ path[4] + ": " + oscMsg.args[0].value);                  
          switch (path[4])
          {
            case "0":
            case "1": { // Output to Curved LED PA
              //console.log("Bus master gain 0 or 1");
              var st = document.getElementById('sum_bus_CurvedPA-gain');                            
              break;
            }
            case "2":
            case "3": { // Output to Audio PC
              //console.log("Bus master gain 2 or 3");
              var st = document.getElementById('sum_bus_AudioPC-gain');                                             
              break;
            }
            case "8": { // Output to CAVE FL
              var st = document.getElementById('sum_bus_CAVE_FL-gain') || document.getElementById('sum_bus_CAVEPA-gain');
              break;
            }
            case "9": { // Output to CAVE FR
              var st = document.getElementById('sum_bus_CAVE_FR-gain') || document.getElementById('sum_bus_CAVEPA-gain');
              break;
            }
            case "10": { // Output to CAVE FWL
              var st = document.getElementById('sum_bus_CAVE_FWL-gain') || document.getElementById('sum_bus_CAVEPA-gain');
              break;
            }
            case "11": { // Output to CAVE FWR
              var st = document.getElementById('sum_bus_CAVE_FWR-gain') || document.getElementById('sum_bus_CAVEPA-gain');
              break;
            }
            case "12": { // Output to CAVE SL
              var st = document.getElementById('sum_bus_CAVE_SL-gain') || document.getElementById('sum_bus_CAVEPA-gain');
              break;
            }
            case "13": { // Output to CAVE SR
              var st = document.getElementById('sum_bus_CAVE_SR-gain') || document.getElementById('sum_bus_CAVEPA-gain');
              break;
            }
            case "14": { // Output to CAVE BL
              var st = document.getElementById('sum_bus_CAVE_BL-gain') || document.getElementById('sum_bus_CAVEPA-gain');
              break;
            }
            case "15": { // Output to CAVE BR
              var st = document.getElementById('sum_bus_CAVE_BR-gain') || document.getElementById('sum_bus_CAVEPA-gain');
              break;
            }
            case "24":
            case "25": { // Output to HP1
              var st = document.getElementById('sum_bus_HP1-gain');
              break;
            }
            case "26":
            case "27": { // Output to HP2
              var st = document.getElementById('sum_bus_HP2-gain');
              break;
            }
            case "28":
            case "29": { // Output to HP3
              var st = document.getElementById('sum_bus_HP3-gain');
              break;
            }
            case "30":
            case "31": { // Output to HP4
              var st = document.getElementById('sum_bus_HP4-gain');
              break;
            }
            default:
              //console.log("Bus master gain: channel#" + path[3]);
              break;
          }
          if (!st) {
            //console.log("No OSC message handler for bus_master " + path[4] + " gain");
            return;
          }
          st.innerHTML = oscMsg.args[0].value;                
          var statusEvent = new CustomEvent('updated', { detail: { time: Date.now() } });
          st.dispatchEvent(statusEvent); // dispatch an event that the status has changed   
          break;

        case 'meter':
          switch (path[4])
          {
            case "0":
            case "1": // Meter for Curved LED PA
              //console.log("Bus master meter 0 or 1", oscMsg);
              var st = document.getElementById('sum_bus_PA-meter');
              st.innerHTML = oscMsg.args[0].value;
              var statusEvent = new CustomEvent('updated', { detail: { time: Date.now() } });
              st.dispatchEvent(statusEvent); // dispatch an event that the status has changed
              break;
            default:
              //console.log("Bus master meter: channel#" + path[4]);
              break;
          }
          break;
      }
      break;

    case 'output_gain':
      var ch = path[4];
      if (ch === 'gain' && path[5]) ch = path[5];
      var st = null;
      switch (ch)
      {
        case "318":
        case "319": // Output to Curved LED PA
          st = document.getElementById('output_gain_CurvedPA-gain');
          break;
        case "400":
        case "401": // Output to CAVE PA FWL/FWR
          st = document.getElementById('output_gain_CAVEPA-gain');
          break;
        default:
          break;
      }
      if (!st) {
        return;
      }
      st.innerHTML = oscMsg.args[0].value;
      var statusEvent = new CustomEvent('updated', { detail: { time: Date.now() } });
      st.dispatchEvent(statusEvent); // dispatch an event that the status has changed
      break;
  }
}