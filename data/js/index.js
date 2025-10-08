import testRender from "./test.js"

let diagnostic = `
<!-- LOGO -->
<div class="full-width-row">
    <img class="logo" src="/assets/logo.svg"></img>
</div>
<!-- DIAGNOSTIC -->
<div class="full-width-row">
    <div class="card">
        <div class="title">System Status</div>
        <div id="cpu-data" class="data">
            <div class="cnt-status-data">
                <div class="value" >100%</div>
                <div class="val-title">CPU</div>
            </div>
            <div class="cnt-status-data">
                <div class="value">100%</div>
                <div class="val-title">RAM</div>
            </div>
        </div>
    </div>
    <div class="card">
        <div class="title">Disk Usage</div>
        <div class="data">
            <div class="cnt-status-data">
                <div class="value">100%</div>
                <div class="val-title">Root</div>
            </div>
        </div>
    </div>
    <div class="card">
        <div class="title">Connected Devices</div>
        <div class="data">
            <div class="cnt-status-data">
                <div class="value">2</div>
                <div class="val-title">Current</div>
            </div>
            <div class="cnt-status-data">
                <div class="value">0</div>
                <div class="val-title">Previous</div>
            </div>
        </div>
    </div>
    <div class="card">
        <div class="title">SSIDs Connected</div>
        <div class="data">
            <div class="cnt-status-data">
                <div class="value">0</div>
                <div class="val-title">Sessions</div>
            </div>
            <div class="cnt-status-data">
                <div class="value">18</div>
                <div class="val-title">Total</div>
            </div>
        </div>
    </div>
</div>
`

let secondPage = `<div>HELLO</div>`


function renderNewPage(idPageToRender)
{
    let mainCtn = document.getElementsByClassName("main-ctn");
    switch (idPageToRender) {
        case 0:
            mainCtn[0].innerHTML = diagnostic;
            break;
        case 1:
            mainCtn[0].innerHTML = secondPage;
            break;
        default:
            break;
    }
}



//LEFTBAR
/**
 * initializer of the left bar
 * @param {Array} buttons number of button to initialize left bar
 */
function setupLeftBar(buttons) {
    let leftbar = document.getElementsByClassName('left-bar');
    var fragment = document.createDocumentFragment();
    for (let i = 0; i < buttons.length; i++) {
        var leftbarButton = document.createElement('button');
        leftbarButton.innerHTML = '<img src="' +buttons[i]+ '"></img>';
        leftbarButton.addEventListener('click', function(){ renderNewPage(i)})
        leftbarButton.className = 'btn';
        fragment.appendChild(leftbarButton);
    }
    leftbar[0].appendChild(fragment);
}

/**
 * 
 * Function to add a new button to the left bar
 * @param {string} linkTo link to new html page
 * @param {string} iconUrl url to the svg icon to use in the button
 * 
 * @return {HTMLAnchorElement} the created button
 */
function appendNewLeftbarButton(iconUrl){
    let leftbar = document.getElementsByClassName('left-bar');
    var leftbarButton = document.createElement('button');
    leftbarButton.className = 'btn';
    leftbarButton.innerHTML = '<img src="' +iconUrl+ '"></img>';
    leftbar[0].appendChild(leftbarButton);
    return leftbarButton
}


/**
 * General initializer
 */
function init(){
    setupLeftBar(["/assets/gauge.svg", "/assets/flask-viall.svg"]);
    renderNewPage(0);
    testRender();
}

// SCRIPT START
init()
