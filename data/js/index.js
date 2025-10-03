//LEFTBAR
/**
 * initializer of the left bar
 * @param {int} numberOfButton number of button to initialize left bar
 */
function setupLeftBar(numberOfButton) {
    let leftbar = document.getElementsByClassName('left-bar');
    var fragment = document.createDocumentFragment();
    for (let i = 0; i < numberOfButton; i++) {
        var leftbarButton = document.createElement('a');
        leftbarButton.href = "";
        leftbarButton.className = 'btn';
        fragment.appendChild(leftbarButton);
    }
    leftbar[0].appendChild(fragment);
}

/**
 * Function to add a new button to the left bar
 * @param {string} linkTo link to new html page
 * @param {string} iconUrl url to the svg icon to use in the button
 * 
 * @return {HTMLAnchorElement} the created button
 */
function appendNewLeftbarButton(linkTo, iconUrl){
    let leftbar = document.getElementsByClassName('left-bar');
    var leftbarButton = document.createElement('a');
    leftbarButton.href = linkTo;
    leftbarButton.className = 'btn';
    leftbarButton.innerHTML = '<img src="' +iconUrl+ '"></img>';
    leftbar[0].appendChild(leftbarButton);

    return leftbarButton
}


/**
 * General initializer
 */
function init(){
    setupLeftBar(0);
    var btn = appendNewLeftbarButton("", "./assets/gauge-solid-full.svg");
    btn.setAttribute("class", "btn selected");
    appendNewLeftbarButton("", "./assets/flask-vial-solid-full.svg");
}

// SCRIPT START
init()
