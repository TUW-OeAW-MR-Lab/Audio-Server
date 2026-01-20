      var oscPort = new osc.WebSocketPort({
          url: "/ws", // WebSocket resource at current URL context.
          metadata: true
      });
      oscPort.open();

      function send(address, value) {
          oscPort.send({
              address: address,
	      args: [
		  {
		      type: "s",
		      value: value
		  }
	      ]
          });
      };

      function sendResponse(address) {
          oscPort.send({
              address: address,
	      args: [
		  {
		      type: "s",
		      value: "connect"
		  },
		  {
		      type: "s",
		      value: "localhost"
		  },
		  {
			  type: "i",
			  value: 9336
		  }
		  ]
          });
      };

      function sendNoArgs(address) {
          oscPort.send({
              address: address
          });
      };

      oscPort.on("ready", function () {
          oscPort.send({ address: "/ping" });
      });

    oscPort.on("message", function (oscMsg) {
			console.log("OSC message received: ", oscMsg);

			path = oscMsg.address.substring(1).split("/");

			if (path[0] == "app" && path[2] == "state") {
				// Show status info.
				document.getElementById(path[1] + "_status").innerHTML = oscMsg.args[1].value;
				return;
			};

			if (path[1] == "pd-fly" || path[1] == "pd-jungle")
			{
				if (path[2] == "osc" && path[3] == "Control") {
					// Show response infos.
					document.getElementById(path[1] + "_" + path[4]).innerHTML = oscMsg.args[0].value;
					return;
				}
			}
			else
			{
				console.log("No OSC message handler: ", oscMsg.address);
				return;
			};


			console.log("No OSC message handler: ", oscMsg.address);
    });
