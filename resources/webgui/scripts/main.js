
let secWaited = 0; // Global counter for seconds waited after some action

// Prevent going back or reload if a scene has been launched
window.addEventListener('beforeunload', (event) => {
	const launchedScenes = document.querySelector('.launched-scene');
		if (launchedScenes)
		{
			event.preventDefault();
			msg = "A scene has been launched. Are you sure you want to proceed? The state of the current scene will be lost!";
			event.returnValue = msg;
			return msg;
		}
});

// Load the interface of a scene, see index.html
async function loadScene(scene, clickedButton) 
{
	const launchedScenes = document.querySelector('.launched-scene');
	if (launchedScenes)
		if (!confirm("Are you sure you want to proceed with a new scene? The state of the current scene will be lost!")) return; 
	
		// load the activeted scene
	const activeScene = document.getElementById('scene-port');
	try 
	{
		const response = await fetch('scenes/' + scene + '.html');
		if (!response.ok) throw new Error('Page not found');
		const html = await response.text();
		activeScene.innerHTML = html;
	}
	catch (error)
	{
		activeScene.innerHTML = `<h2>Error</h2><p>The scene ${scene} could not be loaded.</p>`;
	}
		// Handle the buttons: highlight the active 
	const allButtons = document.querySelectorAll('.scenes-sidebar button');
	allButtons.forEach(button => { button.classList.remove('active-scene'); });
	clickedButton.classList.add('active-scene');
}

// Launch an app of a scene, consider interface functionality
function launchApp(scene, app)
{
	send('/app/' + app + '/control', 'launch');
	const activeButton = document.getElementById('button-' + scene);
	activeButton.classList.remove('active-scene');	
	activeButton.classList.add('launched-scene');
	secWaited = 0;
}

// Quit an app of a scene, consider interface functionality
function quitApp(scene, app)
{
	send('/app/' + app + '/control', 'quit');
	const launchedButton = document.getElementById('button-' + scene);
	launchedButton.classList.remove('launched-scene');
	launchedButton.classList.add('active-scene');
	secWaited = 0;
}

// Launch an app of a scene (consider interface functionality) and wait until "ready"
function launchAppAndWait(scene, app, initFun, readyFun)
{
	state = document.getElementById(app + "_status");
	state.innerText = "unclear";
	state.style.backgroundColor = "red";
	if (initFun != null) initFun(); 
	launchApp(scene, app);
	setTimeout(() => { checkStateLaunched(scene, app, readyFun); }, 100);
}

function checkStateLaunched(scene, app, readyFun)
{
	secWaited += 100;
	state = document.getElementById(app + "_status");
	console.log("waiting for " + secWaited/1000 + "s, status=" + state.innerText);
	switch (state.innerText)
	{
		case "unclear":
			setTimeout(() => { checkStateLaunched(scene, app, readyFun); }, 100);
			break;
		case "alive":
			state.style.backgroundColor = "yellow";
			setTimeout(() => { checkStateLaunched(scene, app, readyFun); }, 100);
			break;		
		case "ready":
			state.style.backgroundColor = "";
			if (readyFun != null) readyFun(); 
			break;
		case "error":
			state.style.backgroundColor = "red";
			break;
	}
}

function initFly()
{
	document.getElementById("pd-fly_Version").innerText = "---";
	document.getElementById("pd-fly_SampleRate").innerText = "---";
	document.getElementById("pd-fly_CPULoad").innerText = "---";
}

function doneFly()
{
	state = document.getElementById("pd-fly_status");
	state.innerText = "connecting...";
	state.style.backgroundColor = "blue";
	sendResponse('/app/pd-fly/osc/Control/Response');
	sendResponse('/app/pd-fly/osc/Control/Version');
	secWaited = 0;
	setTimeout(() => { checkFlyConnection(); }, 100);
}

function checkFlyConnection()
{
	secWaited += 100;
	state = document.getElementById("pd-fly_Version");
	console.log("waiting for " + secWaited/1000 + "s, version=" + state.innerText);
	switch (state.innerText)
	{
		case "---":
			sendResponse('/app/pd-fly/osc/Control/Response');
			sendResponse('/app/pd-fly/osc/Control/Version');		
			setTimeout(() => { checkFlyConnection(); }, 100);
			break;
		default: 
			state = document.getElementById("pd-fly_status");
			state.innerText = "connected";
			state.style.backgroundColor = "";
			break;
	}
}		
