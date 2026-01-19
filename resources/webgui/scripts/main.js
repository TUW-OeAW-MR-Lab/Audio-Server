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
}

// Quit an app of a scene, consider interface functionality
function quitApp(scene, app)
{
	send('/app/' + app + '/control', 'quit');
	const launchedButton = document.getElementById('button-' + scene);
	launchedButton.classList.remove('launched-scene');
	launchedButton.classList.add('active-scene');
}


	