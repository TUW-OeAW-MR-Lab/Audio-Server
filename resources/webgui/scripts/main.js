
let secWaited = 0; // Global counter for seconds waited after some action
let SceneModule = null; // Module for all Scene-related scripts, will be loaded from scripts/%scene-name%.js

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
	}
	catch (error)
	{
		activeScene.innerHTML = `<h2>Error</h2><p>The scene ${scene} could not be loaded.</p>`;
	}
		// Load Scene-specific scripts, if any
	try
	{
		console.log('Loading script ' + scene + '.js');
		SceneModule = await import('./' + scene + '.js');
		SceneModule.init();	// must have an init
		console.log('Good!');
	}
	catch(error)
	{
		console.log('Error: ' + error);
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

// Quit an app of a scene, consider interface functionality
function quitApp(scene, app, quitFun)
{
	send('/app/' + app + '/control', 'quit');
	const launchedButton = document.getElementById('button-' + scene);
	launchedButton.classList.remove('launched-scene');
	launchedButton.classList.add('active-scene');
	secWaited = 0;
	if (quitFun != null) quitFun();
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

