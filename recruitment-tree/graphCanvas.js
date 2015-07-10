function graphCanvas() {
    

    var nodes = undefined,
        edges = undefined,
        context = undefined,
        canvas = undefined,
        selectedNodes = [],
        force = undefined,
        labels = false;
   
    var width = 0, height = 0; // these change in draw()

    var zoom = d3.behavior.zoom();

    var colors = (function() {
        var i = 0;
        var col = ["red", "orange", "yellow", "green", "brown", "purple"]
        function nextColor() {
            var c = col[i]
            i = (i + 1) % col.length
            return c;
        }
        return nextColor
    })()


    var graph = function(view) {
        width = parseInt(view.style("width")), height = parseInt(view.style("height"));
        
        canvas = view
            .append("canvas")
            .attr("width", width)
            .attr("height", height)
            .attr("pointer-events", "all")
            .call(zoom)
            .node()

        context =  canvas.getContext("2d")

        if (!force)
            force = d3.layout.force()
                .gravity(0.07)
                .distance(25)
                .charge(-280)
                .size([width, height]);

        return graph;
    }

    graph.start = function() {
        force
            .nodes(nodes)
            .links(edges)
            .on("end", function() { zoom.on("zoom", redraw) })
            .on("start", function() { zoom.on("zoom", undefined) })
            .start()
                window.requestAnimationFrame(redraw)
        return graph;
    }

    graph.stop = function() {
        force.alpha(0);
        return graph;
    };

    function redraw() {
        redraw.started = true; /* other things can redraw now */

        context.clearRect(0,0,canvas.width,canvas.height)
        
        var s = zoom.scale(),
            t = zoom.translate()

        /* Batch path drawing */
        context.beginPath();
        context.strokeStyle = 'lightgrey';
        edges.forEach(function(d) {
            context.moveTo(s * d.source.x + t[0], s * d.source.y + t[1])
            context.lineTo(s * d.target.x + t[0], s * d.target.y + t[1])
        });
        context.stroke()

        nodes.forEach(function(d) {
            context.beginPath();
            context.arc(s * d.x + t[0], s * d.y + t[1], s * (d.size || 8), 0, 2 * Math.PI);
            context.fillStyle = d.color;
            context.fill();
        });

        if (labels) {
            context.font = (s * 10).toString() + "pt Arial"
            context.fillStyle = "black";
            nodes.forEach(function(d) {
                context.fillText(d.name, s * (d.x + 8) + t[0], s * d.y + t[1])
            });
        } 
        
        if (force.alpha())
            window.requestAnimationFrame(redraw)
    }

    graph.center = function() {
        zoom.scale(0.25);
        zoom.translate([width/4,height/4])
        redraw()
    }


    /* NOTE: Why don't we color gen0 with C0, gen1 with C1=lighter(C0), gen2 with C2=lighter(C1), and so forth?
     * 1) What if a node is in gen1 and gen2 ?
     * 2) Colors decay to white too quickly. Need a better function than "make 1.2x brighter" 
     *    (which I guess makes colors exponentially brighter each generation...)
     */

    graph.selectNode = function(node, ngen, color) {
    
        function colorGenerations(n, ngen, color) {
            if (ngen == 0)
                return;
            if (!n.neighbors)
                n.neighbors = findNeighbors(nodes, edges, n);
            n.neighbors.forEach(function(d) {
                if (d != node) {
                    d.color = color;
                    colorGenerations(d, ngen-1, color);
                }
            });
        }

        selectedNodes.push(node)
        node.color = d3.hsl(color || colors());
        node.selectedGens = ngen

        colorGenerations(node, ngen, node.color.brighter(1.2));

        zoom.scale(1)
        zoom.translate([width/2 - node.x, height/2 - node.y])
        
        if (! force.alpha())
            redraw();
    }

    graph.unSelectNode = function(node) {
        function uncolorGenerations(node, ngen) {
            node.color = node.oldcolor;
            if (ngen > 0)
                node.neighbors.forEach(function(d) {
                    uncolorGenerations(d, ngen-1);
                });
        }

        uncolorGenerations(node, node.selectedGens);
       
        var i = selectedNodes.findIndex(function(x) { return x == node });
        if (i >= 0)
            selectedNodes.splice(i, 1);

        if (!force.alpha())
            redraw();
    }

    graph.nodes = function(_) {
        if (!arguments.length)
            return nodes;
        else
            nodes = _;
        nodes.forEach(function(d) {
            if (!d.color)
                d.color = "black";
            d.oldcolor = d.color;
        });
        return graph;
    }

    graph.edges = function(_) {
        if (!arguments.length)
            return edges;
        else
            edges = _;
        return graph;
    }


    graph.selectedNodes = function(_) {
        if (!arguments.length)
            return selectedNodes;
        else
            selectedNodes = _;
        return graph;
    }

    graph.labels = function(_) {
        if (!arguments.length)
            return labels;
        else
            labels = _;
        if (redraw.started)
            redraw()
        return graph;
    }

    graph.redraw = function() {
        if (!force)
            return
        if (force.alpha())
            return
        redraw()
    }

    graph.force = function(_) {
        if (!arguments.length)
            return force;
        else
            force = _;
        return graph;
    }


    
    function cleanName(name) {
        if (+name > 0)
            return "_" + name;
        else
            return name.replace("'", "_apos_");
    }

    function selectNode(name) {
        return d3.select("#" + cleanName(name));
    }



    return graph;

}

function findNeighbors(nodes, edges, node_obj) {
    //var node_obj = nodes.find(function (n) { return n.label == name });
    var nb_l = edges.filter(function(e) { return e.source == node_obj; })
            .map(function (n) { return n.target; });
    var nb_r = edges.filter(function(e) { return e.target == node_obj; })
            .map(function (n) { return n.source; });
    var neighbors = nb_l;
    for (i in nb_r) {
        if (! nb_r[i] in neighbors)
           neighbors.push(nb_r[i]) 
    }
    return neighbors;

}

