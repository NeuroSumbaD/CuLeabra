var CurrentNet = null;
var CurrentView = null;

function SwitchToNet(netObj) {
    // Handles tab display and hiding for each Net
    if (netObj == CurrentNet) { // Skip redundant calls
        return;
    }

    // Show the new Net
    // let netDiv = document.getElementById(netName);
    netObj.div.style.display = "block";

    // Select the new Net button
    // let netButton = document.getElementById(`show-${netName}`);
    netObj.tab.classList.add("w3-indigo");

    if (CurrentNet) { // if not null
        // Deselect previous current Net
        CurrentNet.div.style.display = "none";
        CurrentNet.tab.classList.remove("w3-indigo");
    
    }
    // update current Net to netName
    CurrentNet = netObj;

    // ALSO needs to generate the view tab specific to the net
    let viewSelector = document.querySelector("#view-selector");
    for (node of viewSelector.children) { // remove all children
        viewSelector.removeChild(node);
    }
    for (view of CurrentNet.views) { // append views
        viewSelector.appendChild(view.tab);
    }
}

function SwitchToView(viewObj) {
    // Handles tab display and hiding for each view
    if (viewObj == CurrentView) { // Skip redundant calls
        return;
    }

    // Show the new view
    viewObj.div.style.display = "block";

    // Select the view button
    viewObj.tab.classList.add("w3-sand");

    if (CurrentView)  { // if not null
        // Deselect previous current Net
        CurrentView.div.style.display = "none";
        CurrentView.tab.classList.remove("w3-sand");
    }

    CurrentView = viewObj;
}

function OpenMenu (menuSelector) {
    let overlay = document.querySelector(".overlay");
    let menu = document.querySelector(menuSelector);
    return function () {
        overlay.style.display = "flex";
        menu.style.display = "block";
    }
}

function CloseMenu () {
    document.querySelector(".overlay").style.display = "none";
    document.querySelector("#create-net-form").style.display = "none";
    document.querySelector("#create-sim-form").style.display = "none";
    document.querySelector("#create-view-form").style.display = "none";
}


document.addEventListener("DOMContentLoaded", async function () {
    // Make main-container div fill the screen (to hold the view/SVG)
    const topBarHeight = document.querySelector('.top-bar').offsetHeight;
    const mainContainer = document.querySelector('.main-container');
    mainContainer.style.minHeight = `calc(100vh - ${topBarHeight}px)`;
    document.getElementById("view-selector").removeChild(document.getElementById("dummyView"));


    // Initialize default Net with its default view
    let spec = await fetch("/get_net?net=ExampleNet")
        .then(response => response.json())
        .then(netSpec => {
            return netSpec["layers"];
        });
    CurrentNet = new LeabraNet("ExampleNet", spec, true);
      CurrentNet.tab.classList.add("w3-indigo");
    CurrentView = CurrentNet.defaultView;
      CurrentView.tab.classList.add("w3-sand");


    // attach create menu 
    document.querySelector(".overlayBackground").addEventListener("click", CloseMenu);
    document.querySelectorAll("#exit-form").forEach(button => button.addEventListener("click", CloseMenu));
    document.querySelector("#create-net").addEventListener("click", OpenMenu("#create-net-form"));
    document.querySelector("#create-sim").addEventListener("click", OpenMenu("#create-sim-form"));
    document.querySelector("#create-view").addEventListener("click", OpenMenu("#create-view-form"));
});