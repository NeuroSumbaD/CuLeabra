/* This script contains a set of classes which handle the creation of a new
 */

function HeatmapView(viewName, netObj, layerSpec, display = "none") {
    /* This function defines a view object which will add a view div
        to the .main-container upon creation and give it an id of
        #view-name.
    */
    this.name = viewName;

    let netName = netObj.netName;
    this.netPage = document.querySelector(".main-container").querySelector(`#${netName}`);

    
    // create div for containing view
    this.div = document.createElement("div");
    this.div.id = this.name;
    this.div.classList.add("view");
    this.netPage.appendChild(this.div)
    
    // Generate a tab in the view-selector
    // TODO: MAKE THIS A FUNCTION THAT OTHER VIEW TYPES CAN USE
    this.tab = document.createElement("button");
    this.tab.classList.add("w3-bar-item", "w3-button");
    this.tab.id = `show-${viewName}`;
    let self = this; // reference to current view object for closure?
    this.tab.onclick = function () {SwitchToView(self)};
    this.tab.innerHTML = viewName;
    document.querySelector("#view-selector").appendChild(this.tab);
    
    // create svg canvas for the visualization
    // TODO: MAKE THIS A FUNCTION THAT OTHER VIEW TYPES CAN USE
    let id = "svg-" + this.name;
    this.svgSelection = d3.select(this.div)
      .append("svg")
        .attr("class", "w3-border")
        .attr("width", "100%")
        .attr("height", "100%")
        .attr("id", id);
    this.svg = document.getElementById(id);
    let canvas = this.svgSelection.append("g").attr("id", "canvas");
    this.canvas = document.getElementById("canvas");

    // Attach zoom/pan handler
    const zoom = d3.zoom()
        .scaleExtent([0.25,5])
        .on("zoom", function ({transform}) {
            canvas.attr("transform", transform);
        });
    this.svgSelection.call(zoom);
    
    // Generate maps for each layer
    this.layerMaps = {};
    var leftPosition = 0;
    var spacing = 16; // TODO: Make this a parameter
    var halfHeight = this.svg.parentElement.offsetHeight / 2; 
    for (layer in layerSpec) {
        let shape = layerSpec[layer].shape;
        if (shape.length == 1) shape.push(1);
        let yPosition = halfHeight - (shape[0]*32)/2 - 32; // TODO: change 26 to use padding + label height
        let position = [leftPosition, yPosition]; 
        let heatmap = new Heatmap(layer, self, shape, position); // initialize heatmap
        leftPosition += 32 * shape[1] + spacing; // TODO: use rectWidth/height parameter
        this.layerMaps[layer] = heatmap;
    };
    // TODO: GENERATE arrows between maps

    // TODO: ATTACH the streamToggle listener to each heatmap
    this.isStreaming = false;
    this.streamInterval = 100; // ms
    this.transitionInterval = parseInt(this.streamInterval * 0.95);
    this.ToggleStream = function () {
        if (this.isStreaming) {
            clearInterval(this.interval);
            this.isStreaming = false;
            document.querySelector("#start-stream").innerHTML = "Start Stream";
        }
        else {
            // TODO: Make update function generic
            this.interval = setInterval(() => {
                fetch('/get_activity') // if I want specific data, I can call: fetch('/get_activity?net=netname')
                    .then(response => response.json())
                    .then(data => {
                        for (layer in data) {
                            // TODO: handle error if keys don't match
                            self.layerMaps[layer].updateHeatmap(data[layer]);
                        }
                    });
            }, this.streamDelay);
            this.isStreaming = true;
            document.querySelector("#start-stream").innerHTML = "Stop Stream";
        }
    };
    document.querySelector("#start-stream").addEventListener("click", this.ToggleStream)
    
    this.zoom = 1; // to eventually keep track of zooming
    this.pan = 0; // to eventually keep track of panning
}

function LeabraNet (netName, spec,
                    isDefault = false) {
    /* This function defines a LeabraNet object that creates a div with
        a default view and has methods for adding new views that can be
        called from the create Menu.
     */
    this.netName = netName;
    this.netSpec = spec;

    // Generate a tab in the view-selector
    this.tab = document.createElement("button");
    this.tab.classList.add("w3-bar-item", "w3-button");
    this.tab.id = `show-${netName}`;
    let self = this; // reference to current view object for closure?
    this.tab.onclick = function () {SwitchToNet(self)};
    this.tab.innerHTML = netName;
    document.querySelector("#network-selector").appendChild(this.tab);

    // Generate div
    this.div = document.createElement("div");
    this.div.id = netName;
    this.div.classList.add("netPage", "view");
    document.querySelector("#main-window").appendChild(this.div);
    
    // initialize the view list and default view
    let display = isDefault ? "block" : "none";
    this.defaultView = new HeatmapView("defaultView", self, this.netSpec, display);
    this.div.appendChild(this.defaultView.div);
    this.views = [this.defaultView];
    
    SwitchToNet(self);
    SwitchToView(this.defaultView);
};