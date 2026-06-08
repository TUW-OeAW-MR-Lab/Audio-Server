var oscPort = new osc.WebSocketPort({
		url: "/ws", // WebSocket resource at current URL context.
		metadata: true
});
oscPort.open();

function send(address, value) 
{
	oscPort.send({ 
		address: address,
		args: [{ type: "s", value: value }]
	});
}

function sendValue(address, value) 
{
	oscPort.send({
		address: address,
		args: [{ type: "f", value: value }]
	});
  //console.log("OSC message sent: ", address, value);
}

function sendValues3(address, value1, value2, value3) 
{
	oscPort.send({
		address: address,
		args: [{ type: "f", value: value1 }, { type: "f", value: value2 }, { type: "f", value: value3 }]
	});
}
			
function sendResponse(address,port) 
{
	oscPort.send({
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
	oscPort.send({ address: address });
  //console.log("OSC message sent: ", address);
}

oscPort.on("ready", function () 
{
	oscPort.send({ address: "/ping" });
});

oscPort.on("message", function (oscMsg) 
{
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
    case 'flex_channel': 
      switch (path[5])
      {
        case 'mute':
          if (inputFlexChannelMap[path[4]]!="")
          {
            console.log("Received mute #"+ path[4] + " -> " + inputFlexChannelMap[path[4]]);
            toggleInputState(inputFlexChannelMap[path[4]], !oscMsg.args[0].value);
          }
          break;

        case 'gain':
          if (inputFlexChannelMap[path[4]]!="")
          {
            const st = document.getElementById('slider-input-' + inputFlexChannelMap[path[4]]);
            st.value = oscMsg.args[0].value;  
            const statusEvent = new CustomEvent('updated', { detail: { time: Date.now() } });
            st.dispatchEvent(statusEvent); // dispatch an event that the status has changed    
          }
          break;
      }
      break;

    case 'sum_bus_master':
      //console.log(oscMsg);
      switch (path[5])
      {
        case 'mute':
          const st = document.getElementById('btn-sum_bus_PA-mute');
          if (!st) {
            console.log("No OSC message handler for: 'btn-sum_bus_PA-mute'");
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
          const statusEvent = new CustomEvent('updated', { detail: { time: Date.now() } });
          st.dispatchEvent(statusEvent); // dispatch an event that the status has changed                      
          break;

        case 'gain':
          //console.log("Received bus master gain #"+ path[4] + ": " + oscMsg.args[0].value);                  
          switch (path[4])
          {
            case "0":
            case "1": { // Output to Curved LED PA
              console.log("Bus master gain 0 or 1");
              const st = document.getElementById('sum_bus_PA-gain');
              st.innerHTML = oscMsg.args[0].value;                
              const statusEvent = new CustomEvent('updated', { detail: { time: Date.now() } });
              st.dispatchEvent(statusEvent); // dispatch an event that the status has changed                                  
              break;
            }
            case "2":
            case "3": { // Output to Audio PC
              console.log("Bus master gain 2 or 3");
              const st = document.getElementById('sum_bus_PC-gain');
              st.innerHTML = oscMsg.args[0].value;                
              const statusEvent = new CustomEvent('updated', { detail: { time: Date.now() } });
              st.dispatchEvent(statusEvent); // dispatch an event that the status has changed                                  
              break;
            }
            case "4":
            case "5": { // Output to CAVE
              console.log("Bus master gain 4 or 5");
              const st = document.getElementById('sum_bus_CAVE-gain');
              st.innerHTML = oscMsg.args[0].value;                
              const statusEvent = new CustomEvent('updated', { detail: { time: Date.now() } });
              st.dispatchEvent(statusEvent); // dispatch an event that the status has changed                                  
              break;
            }
            default:
              console.log("Bus master gain: channel#" + path[3]);
              break;
          }
          break;

        case 'meter':
          switch (path[4])
          {
            case "0":
            case "1": // Meter for Curved LED PA
              //console.log("Bus master meter 0 or 1", oscMsg);
              const st = document.getElementById('sum_bus_PA-meter');
              st.innerHTML = oscMsg.args[0].value;
              const statusEvent = new CustomEvent('updated', { detail: { time: Date.now() } });
              st.dispatchEvent(statusEvent); // dispatch an event that the status has changed
              break;
            default:
              //console.log("Bus master meter: channel#" + path[4]);
              break;
          }
          break;
      }
      break;                   
  }
}