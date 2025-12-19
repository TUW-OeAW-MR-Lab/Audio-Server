function showContent(contentId, clickedButton) {
    // ... (Content switching code remains the same) ...
// 1. CONTENT SWITCHING (Unchanged from before)
    const allContent = document.querySelectorAll('.content-section');
    allContent.forEach(section => {
        section.style.display = 'none';
    });
    
    const activeContent = document.getElementById(contentId);
    if (activeContent) {
        activeContent.style.display = 'block';
    }
    // 2. ACTIVE BUTTON HANDLING (Cleaner Code)
    
    // A. Remove the active class from all buttons
    const allButtons = document.querySelectorAll('.left-buttons-sidebar button');
    allButtons.forEach(button => {
        button.classList.remove('active-button');
    });

    // B. Add the active class to the button that was passed into the function
    clickedButton.classList.add('active-button');
}