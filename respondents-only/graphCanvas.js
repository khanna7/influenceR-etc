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
        
        if (!canvas.getContext) {
          canvas.innerHTML = "Your browser does not support canvas.";
          return
        }
        
        context = canvas.getContext("2d")
    

        if (!force)
            force = d3.layout.force()
                .gravity(.05)
                .distance(100)
                .charge(-100)
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
        //context.beginPath();
        //context.strokeStyle = 'lightgrey';
        var length = 10;

        edges.forEach(function(d) {
            
            context.strokeStyle = d.color || 'lightgrey';
            context.fillStyle = d.color || 'lightgrey';
            
            if (d.arrow)
              context.setLineDash([3])
            else
              context.setLineDash([]);
            
            drawArrow(context, 
              s * d.source.x + t[0], s * d.source.y + t[1], 
              s * d.target.x + t[0], s * d.target.y + t[1], 
              d.arrow ? undefined : function() { }, 
              d.direction,
              undefined,
              10);
            
        });
        //context.stroke()

        
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
                context.fillText(d.label, s * (d.x + 8) + t[0], s * d.y + t[1])
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

/* Taken from http://dbp-consulting.com/scripts/canvasutilities.js */

var drawArrow=function(ctx,x1,y1,x2,y2,style,which,angle,d,dashed)
{
  'use strict';
  // Ceason pointed to a problem when x1 or y1 were a string, and concatenation
  // would happen instead of addition
  if(typeof(x1)=='string') x1=parseInt(x1);
  if(typeof(y1)=='string') y1=parseInt(y1);
  if(typeof(x2)=='string') x2=parseInt(x2);
  if(typeof(y2)=='string') y2=parseInt(y2);
  style=typeof(style)!='undefined'? style:3;
  which=typeof(which)!='undefined'? which:1; // end point gets arrow
  angle=typeof(angle)!='undefined'? angle:Math.PI/8;
  d    =typeof(d)    !='undefined'? d    :10;
  // default to using drawHead to draw the head, but if the style
  // argument is a function, use it instead
  var toDrawHead=typeof(style)!='function'?drawHead:style;

  // For ends with arrow we actually want to stop before we get to the arrow
  // so that wide lines won't put a flat end on the arrow.
  //
  var dist=Math.sqrt((x2-x1)*(x2-x1)+(y2-y1)*(y2-y1));
  var ratio=(dist-d/3)/dist;
  var tox, toy,fromx,fromy;
  if(which&1){
    tox=Math.round(x1+(x2-x1)*ratio);
    toy=Math.round(y1+(y2-y1)*ratio);
  }else{
    tox=x2;
    toy=y2;
  }
  if(which&2){
    fromx=x1+(x2-x1)*(1-ratio);
    fromy=y1+(y2-y1)*(1-ratio);
  }else{
    fromx=x1;
    fromy=y1;
  }
 
 
  
  ctx.beginPath();
  ctx.moveTo(fromx,fromy);
  ctx.lineTo(tox,toy);
  ctx.stroke();

  // calculate the angle of the line
  var lineangle=Math.atan2(y2-y1,x2-x1);
  // h is the line length of a side of the arrow head
  var h=Math.abs(d/Math.cos(angle));

  if(which&1){	// handle far end arrow head
    var angle1=lineangle+Math.PI+angle;
    var topx=x2+Math.cos(angle1)*h;
    var topy=y2+Math.sin(angle1)*h;
    var angle2=lineangle+Math.PI-angle;
    var botx=x2+Math.cos(angle2)*h;
    var boty=y2+Math.sin(angle2)*h;
    toDrawHead(ctx,topx,topy,x2,y2,botx,boty,style);
  }
  if(which&2){ // handle near end arrow head
    var angle1=lineangle+angle;
    var topx=x1+Math.cos(angle1)*h;
    var topy=y1+Math.sin(angle1)*h;
    var angle2=lineangle-angle;
    var botx=x1+Math.cos(angle2)*h;
    var boty=y1+Math.sin(angle2)*h;
    toDrawHead(ctx,topx,topy,x1,y1,botx,boty,style);
  }
}

var drawHead=function(ctx,x0,y0,x1,y1,x2,y2,style)
{
  'use strict';
  if(typeof(x0)=='string') x0=parseInt(x0);
  if(typeof(y0)=='string') y0=parseInt(y0);
  if(typeof(x1)=='string') x1=parseInt(x1);
  if(typeof(y1)=='string') y1=parseInt(y1);
  if(typeof(x2)=='string') x2=parseInt(x2);
  if(typeof(y2)=='string') y2=parseInt(y2);
  var radius=3;
  var twoPI=2*Math.PI;

  // all cases do this.
  ctx.save();
  ctx.beginPath();
  ctx.moveTo(x0,y0);
  ctx.lineTo(x1,y1);
  ctx.lineTo(x2,y2);
  switch(style){
    case 0:
      // curved filled, add the bottom as an arcTo curve and fill
      var backdist=Math.sqrt(((x2-x0)*(x2-x0))+((y2-y0)*(y2-y0)));
      ctx.arcTo(x1,y1,x0,y0,.55*backdist);
      ctx.fill();
      break;
    case 1:
      // straight filled, add the bottom as a line and fill.
      ctx.beginPath();
      ctx.moveTo(x0,y0);
      ctx.lineTo(x1,y1);
      ctx.lineTo(x2,y2);
      ctx.lineTo(x0,y0);
      ctx.fill();
      break;
    case 2:
      // unfilled head, just stroke.
      ctx.stroke();
      break;
    case 3:
      //filled head, add the bottom as a quadraticCurveTo curve and fill
      var cpx=(x0+x1+x2)/3;
      var cpy=(y0+y1+y2)/3;
      ctx.quadraticCurveTo(cpx,cpy,x0,y0);
      ctx.fill();
      break;
    case 4:
      //filled head, add the bottom as a bezierCurveTo curve and fill
      var cp1x, cp1y, cp2x, cp2y,backdist;
      var shiftamt=5;
      if(x2==x0){
	// Avoid a divide by zero if x2==x0
	backdist=y2-y0;
	cp1x=(x1+x0)/2;
	cp2x=(x1+x0)/2;
	cp1y=y1+backdist/shiftamt;
	cp2y=y1-backdist/shiftamt;
      }else{
	backdist=Math.sqrt(((x2-x0)*(x2-x0))+((y2-y0)*(y2-y0)));
	var xback=(x0+x2)/2;
	var yback=(y0+y2)/2;
	var xmid=(xback+x1)/2;
	var ymid=(yback+y1)/2;

	var m=(y2-y0)/(x2-x0);
	var dx=(backdist/(2*Math.sqrt(m*m+1)))/shiftamt;
	var dy=m*dx;
	cp1x=xmid-dx;
	cp1y=ymid-dy;
	cp2x=xmid+dx;
	cp2y=ymid+dy;
      }

      ctx.bezierCurveTo(cp1x,cp1y,cp2x,cp2y,x0,y0);
      ctx.fill();
      break;
  }
  ctx.restore();
};





