
function nodesFromLinks(links) {
  /* specific to this */
  for (d of links) {
    d.source = d["UniqueID1"]
    d.target = d["ParentUniqueID1"] // these guys are egos
  }
  
  var nodesById = d3.map()
  
  function makeNode(s) {
    var d = nodesById.get(s)
    if (!d) {
      d = {"name": s, sourceCount: 0, targetCount: 0, degree: 0}
      nodesById.set(s, d)
    }
    return d;
  }
  for (d of links) {
    d.source = makeNode(d.source)
    d.target = makeNode(d.target)
    
    /* TODO: edge attributes ? */
    if (d.agecalc && d.agecalc != "") {
      d.source.agecalc = d.agecalc
    }
    if (d.hivresult && d.hivresult != "")
      d.source.hivresult = d.hivresult
  
    d.source.sourceCount = d.source.sourceCount + 1
    d.target.targetCount = d.target.targetCount + 1
    d.source.degree += 1
    d.target.degree += 1
  
  }
  
  return nodesById.values()
}

function getComponents(nodes, links)
{
  for (d of nodes) {
    d.neighbors = []
  }
  for (d of links) {
    d.target.neighbors.push(d.source)
    d.source.neighbors.push(d.target)
  }
  
  var components = []
  
  for (u of nodes) {
    if (!u.visited) {
      var c = get_component_dfs(u)
      if (c.length > 0) {
        var cs = d3.map(c, function(d) { return d.name })
        var cl = links.filter(function(d) { return cs.has(d.source.name)})
        components.push({nodes: c, links: cl})
      }
    }
  }
  return components
}

function get_component_dfs(u) {
  
  var component = d3.map()
  function dfs(u) {
    u.visited = true;
    component.set(u.name, u)
    for (v of u.neighbors) {
      if (!v.visited) {
        dfs(v)
      }
    }
  }
  dfs(u);
  return component.values()
}

function getComponentGraph(nodesl, links) {
  var nodes = d3.map(nodesl, function(d) { return d.name })
  var links = links.filter(function(d) { return nodes.has(d.source.name)})
  return {nodes:nodesl, links:links}
}

/* Adapted from respondents-only */
function legend(array, attr, colorBounds, domain) {
  var lw = 200, lh=20, padding=20;
  
  var color;
  
  var create = function(home) {
    if (!domain)
      domain = d3.extent(array, function(d) { return d[attr]})

    color = d3.scale.linear()
      .domain(domain)
      .range(colorBounds)

    var legend = home
      .append("svg")
      .attr("height", lh+padding)
      //.attr("width", lw+2*padding)
  
    legend.append("linearGradient")
        .attr({"id":"grad_"+attr, "x1": "0%", "y1": "0%", "x2":"100%", "y2":"0%"})
        .selectAll("stop")
      .data(domain).enter()
        .append("stop")
        .attr("offset", function(_, i) { return (i*100).toString()+"%" })
        .style("stop-opacity", "1")
        .style("stop-color", color)


    legend.append("rect")
      .attr("width", lw)
      .attr("height", lh)
      .attr("x", padding)
      .style("fill", "url(#grad_"+attr+")")

    var lscale = d3.scale.linear()
      .domain(domain)
      .range([0, lw]);

    var format = d3.format("g")
    var axis = d3.svg.axis()
      .scale(lscale)
      .ticks(4)
      .tickFormat(format);

    legend.append("g")
      .attr("class", "axis")
        .attr("transform", "translate(" + padding + "," + lh +")")
        .call(axis);
      
    return create;
  }
  
  
  create.color = function(_) {
    if (!_)
      return color
    else
      color = _
  }
  
  create.colorX = function(d) {
    return (d[attr] != undefined) ? color(+d[attr]) : "white";
  }

  return create;
  
}

