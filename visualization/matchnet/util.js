
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
    
    if (d.agecalc && d.agecalc != "") {
      d.source.agecalc = d.agecalc
    }
  
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


