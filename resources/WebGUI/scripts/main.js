
let secWaited = 0; // Global counter for seconds waited after some action
let SceneModule = null; // Module for all Scene-related scripts, will be loaded from scripts/%scene-name%.js

    // Load the version number
fetch('version.txt')
  .then(response => {
    if (!response.ok) throw new Error('File not found');
    return response.text();
  })
  .then(text => {
    // Den Inhalt in das span-Element schreiben (trim entfernt Zeilenumbrüche)
    document.getElementById('git-version').innerText = text.trim();
  })
  .catch(error => {
    document.getElementById('git-version').innerText = 'Version unknown';
  });

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
	{		// Load the GUI page
		const response = await fetch('scenes/' + scene + '.html');
		if (!response.ok) throw new Error('Page not found');
		const html = await response.text();
		activeScene.innerHTML = html;
		console.log('Scene ' + scene + '.html loaded.');
	}
	catch (error)
	{
		activeScene.innerHTML = `<h2>Error</h2><p>The scene "${scene}" could not be loaded.</p>`;
	}
		// Load Scene-specific scripts, if any
	try
	{
		SceneModule = await import('../scenes/' + scene + '.js');
		SceneModule.init();	// must have an init
		console.log('Script ' + scene + '.js loaded.');
	}
	catch(error)
	{
		console.log('Error loading ' + scene + '.js: ' + error);
		SceneModule = null;
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

// Lock a scene, consider interface functionality
function lockScene(scene)
{
	const activeButton = document.getElementById('button-' + scene);
	activeButton.classList.remove('active-scene');	
	activeButton.classList.add('launched-scene');
}

// Unlock a scene, consider interface functionality
// setActive (optional, default: true): if false, 'active-scene' class is not added
function unlockScene(scene, setActive = true)
{
	const activeButton = document.getElementById('button-' + scene);
	activeButton.classList.remove('launched-scene');
	if (setActive) activeButton.classList.add('active-scene');
}

// Quit an app of a scene, consider interface functionality
function quitApp(scene, app, quitFun)
{
	send('/app/' + app + '/control', 'quit');
	const launchedButton = document.getElementById('button-' + scene);
	launchedButton.classList.remove('launched-scene');
	launchedButton.classList.add('active-scene');
	secWaited = 0;
	state = document.getElementById(app + "-status");
	state.style.backgroundColor = "";
	if (quitFun != null) quitFun();
}

// Launch an app of a scene (consider interface functionality) and wait until "ready"
//   timeout in seconds
//   initFun and readyFun: optional functions executed before and after successful launch, respectively
function launchAppAndWait(scene, app, timeout, initFun, readyFun)
{
	state = document.getElementById(app + "-status");
	state.innerText = "unclear";
	state.style.backgroundColor = "red";
	if (initFun != null) initFun();
	launchApp(scene, app);
	setTimeout(() => { checkStateLaunched(scene, app, timeout, readyFun); }, 100);
}

function checkStateLaunched(scene, app, timeout, readyFun)
{
	secWaited += 100;
	state = document.getElementById(app + "-status");
	//console.log("waiting for " + secWaited/1000 + "s, status=" + state.innerText);
	switch (state.innerText)
	{
		case "unclear":
			if (secWaited <= timeout*1000)
				setTimeout(() => { checkStateLaunched(scene, app, timeout, readyFun); }, 100);
			break;
		case "alive":
			state.style.backgroundColor = "yellow";
			if (secWaited <= timeout*1000)
				setTimeout(() => { checkStateLaunched(scene, app, timeout, readyFun); }, 100);
			break;		
		case "ready":
			state.style.backgroundColor = "";
      console.log("App " + app + ": ready");
			if (readyFun != null) readyFun(); 
			break;
		case "error":
      console.log("App " + app + ": error");
			state.style.backgroundColor = "red";
			break;
		case "initialized": // unclear, but the app might have been started, let's act as ready
			state.style.backgroundColor = "";
      console.log("App " + app + ": initialized");
			if (readyFun != null) readyFun(); 
			break;
	}
	if (secWaited > timeout*1000)
	{
		state.style.backgroundColor = "red";
		state.innerText = "time out!";
		console.log("App " + app + ": Time out!"); 
	}
}

/*	Fetches a response from an URL with a timeout. 
		Returns response and error. error is null when no error happened 
			error.name: 
				'TypeError': e.g., if CORS policy blocks access. Install Add-on "Allow CORS"
				'TimeoutError': time out happened, device not reachable
*/
async function fetchHttpResponse (url, timeout)
{
	try 
  {
    const x = await fetch(url, { signal: AbortSignal.timeout(timeout) } );
    if (!x.ok) 
		{
			throw new Error(`Error: ${x.status}`);
    }
    response = await x.text();
		return { response, error: null };
  } 
  catch (error)
  {
		return {response:null, error};
  }
}

function SystemStatusLegend()
{
	const dialog = document.getElementById('infoDialog');
	document.getElementById('infoTitle').innerText = "Legend";
	document.getElementById('infoText').innerHTML = `
    <div>
    This symbol indicates the status of the system. Currently it just watches for a sign of life from the audio matrix.
    <br><br>
    <table style="margin: 0 auto; width:50%;">
    <thead><tr><th>Symbol</th><th>Meaning</th></tr></thead>
    <tbody>
		<tr><td><span style="background-color: #0061a0; color: white;">&#9711;</span></td> <td>Unknown</td></tr>
		<tr><td><span style="background-color: #0061a0; color: green;">&#11044;</span></td> <td>Sign of life received from the audio matrix</td></tr>
    </tbody></table></div>`;
	dialog.showModal();

	document.getElementById('infoOkBtn').onclick = () => 
	{
		dialog.close();
	};
	
}