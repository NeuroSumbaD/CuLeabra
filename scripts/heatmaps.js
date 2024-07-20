var layerData = -1;

function zoomed(event) {
    const { transform } = event;
    heatmapGroups.attr("transform", transform);
}

function dragstarted(event, d) {
    // ... drag behavior ...
    let map = d3.select(this);
    map.raise().classed("active", true); // Bring the dragged element to the front
    const existingTransform = map.attr("transform");
    const match = existingTransform.match(/translate\((-?\d+\.?\d*),\s*(-?\d+\.?\d*)\)/);
    if (match) {
        this.initialX = +match[1] - event.x;
        this.initialY = +match[2] - event.y;
    }
}

function dragged(event, d) {
    // ... drag behavior ...
    let map = d3.select(this);
    map.attr("transform", `translate(${this.initialX + event.x}, ${this.initialY + event.y})`);
}

function dragended(event, d) {
    // ... drag behavior ...
    d3.select(this).classed("active", false);
}

function createHeatmap(canvasNode, shape, layerName, position = [0,0],
                       rectWidth = 30, rectHeight=30,
                       space = 2, // TODO: add distance between rects
                       ) {
    let canvas = d3.select(canvasNode);
    let zeros = Array(shape[0]).fill().map(() => Array(shape[1]).fill(0));
    let height = (rectHeight + space) * shape[0]; // TODO: Add space
    let width = (rectWidth + space) * shape[1];

    // Create a color scale
    const colorScale = d3.scaleSequential(d3.interpolateViridis)
        .domain([0, 1]);

    const xScale = d3.scaleLinear()
        .domain([0, zeros[0].length])
        .range([0, width]);
    
    const yScale = d3.scaleLinear()
        .domain([0, zeros.length])
        .range([0, height]);

    const group = canvas.append("g");

    // add the map to the group
    group.attr("id", layerName)
    .attr("transform", `translate(${position[0]}, ${position[1]})`)
    .selectAll()
        .data(zeros.flat())
        .enter()
        .append("rect")
        .attr("x", (d, i) => xScale(i % zeros[0].length))
        .attr("y", (d, i) => yScale(Math.floor(i / zeros[0].length)))
        .attr("width", width/zeros[0].length - space )
        .attr("height", height/zeros.length - space )
        .style("fill", function(d) {return colorScale(d)} );

    group.append("text")
        .attr("id", "label-"+layerName)
        .attr("x", width/2)
        .attr("y", height + 16)
        .attr("text-anchor", "middle")
        .text(layerName)

    // Apply the drag handler
    const drag = d3.drag()
        .on("start", dragstarted)
        .on("drag", dragged)
        .on("end", dragended);
    group.call(drag);

    return [group, colorScale, xScale, yScale];
}

function updateHeatmap(group, data) {
    const rect = group.selectAll("rect")
        .data(data.flat())
        .join("rect")
        .transition().duration(80)
        .style("fill", function(d) {return colorScale(d)} );
}

function resizeHeatmap(parentSelector, group, data) {
    let svgDOM = document.querySelector(parentSelector);
    let svgParent = svgDOM.parentElement;
    let padding = parseInt(window.getComputedStyle(svgParent, null).padding);
    const width = svgParent.offsetWidth - 2*padding;
    const height = svgParent.offsetHeight - 2*padding;
    
    // Create a color scale
    const colorScale = d3.scaleSequential(d3.interpolateViridis)
    .domain([0, 1]);
    
    const xScale = d3.scaleLinear()
    .domain([0, data[0].length])
    .range([0, width]);
    
    const yScale = d3.scaleLinear()
    .domain([0, data.length])
    .range([0, height]);
    
    const rect = group.selectAll("rect")
        .data(data.flat())
        .join("rect")
        .transition().duration(10)
        .attr("x", (d, i) => xScale(i % data[0].length))
        .attr("y", (d, i) => yScale(Math.floor(i / data[0].length)))
        .attr("width", width/data[0].length )
        .attr("height", height/data.length )
}

function ToggleStream (groupSelection) {
    let isStreaming = false;
    let group = groupSelection;
    let interval;
    return function () {
        if (isStreaming) {
            clearInterval(interval);
            isStreaming = false;
            document.querySelector("#start-stream").innerHTML = "Start Stream";
        }
        else {
            // TODO: Make update function generic
            interval = setInterval(() => {
                fetch('/get_activity') // if I want specific data, I can call: fetch('/get_activity?net=netname')
                    .then(response => response.json())
                    .then(data => {
                        layerData = data;
                        updateHeatmap("#netView1", group, data["layer1"])
                    });
            }, 100);
            isStreaming = true;
            document.querySelector("#start-stream").innerHTML = "Stop Stream";
        }
    }
}
    
function Heatmap (layerName, viewObj, shape, position = [0,0]) { //TODO: add width, height, and spacing parameters for each map
    this.layerName = layerName;
    this.view = viewObj; // View object from View.js

    // shape = shape.length > 1 ? shape : [shape,1];
    let [group, colorScale, xScale, yScale] = createHeatmap(viewObj.canvas, shape, layerName, position);
    this.mapGroup = group;
    this.colorScale = colorScale;
    this.xScale = xScale;
    this.yScale = yScale;

    let self = this;
    this.updateHeatmap = function (data) {
        const rect = this.mapGroup.selectAll("rect")
            .data(data.flat())
            .join("rect")
            .transition().duration(80)
            .style("fill", function(d) {return colorScale(d)} );
    };

    
    

}