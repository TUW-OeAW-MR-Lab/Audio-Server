async function loadScene(scene, clickedButton) 
{
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
