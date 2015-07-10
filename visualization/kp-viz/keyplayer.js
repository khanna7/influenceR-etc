/* Javascript implementation of Key Players algorithm.
 * Requires matrix.js */

function newKP() {
    
  var nodes = [],
      links = [],
      k = 10,
      D = null,
      tolerance = 0.001,
      logging = true,
      metric = null;

  function keyPlayer(s) {
 
      if (!D)
          D = calculateDistances(nodes, links);

      var n = D.size()[0];
      
      if (!s)
        s = getSample(k, n)
      else {
        k = s.length;
      }

      var fit = metric(s);
      console.log("fit: " + fit + " " + s);
      var i = 0;
      while(true) { // TODO: add d3 tick functionality
          i += 1;
          Dfit = 0;
          bests = [];
          var t = D.rnot(s);
          s.forEach(function(u, ui) {
            t.forEach(function(v, vi) {
       
              var s_ = s.slice(0);
              
              // insert v, remove u
              s_ = switchlist(s_, u, v); /* this is a terrible function */
              if (s_.length != s.length) {
                  console.log(v)
                  console.log(u)
                  console.log("oh no! " + s_ + " " + s)
                  thunk();
              }


              var fit_ = metric(s_);
              var d = fit_ - fit;
              if ((d >= 0) && (d > Dfit)) {
                Dfit = d
                bests = s_
              }
            });
          });

          if (Dfit < tolerance)
              break;
          if (logging)
              console.log("iteration: " + i + " : " + fit + " => " + (fit+Dfit) + " " + s);
          s = bests;
          fit = fit + Dfit;
      }

      return {"nodes": s.map(function(i) { return nodes[i]; }), "fit": fit};
  }

  /* TODO: make this not terrible. (Bin search) */
  function switchlist(sl, x, y) {
    for(var i = 0; i < sl.length; i++) {
      if (sl[i] == x)
          sl.splice(i, 1);
    }
    sl.push(y)
    sl.sort(function(a, b) { return a-b });
    return sl;
  }

  function calculateDistances(nodes, links) {
    var n = nodes.length;
    var m = matrix(n, n, Infinity);

    links.forEach(function(d) {
        m(d.source, d.target, d.value);
        m(d.target, d.source, d.value);
    });
    
    for(var i = 0; i < n; i++) {
       var tmp = matsquare(m);
       if (allequal(tmp, m))
           return m;
       m = tmp;
    }
    return m;

  }

  function matsquare(m0) {
    var m1 = m0.clone(),
        n = m1.size()[0];
    for (var i = 0; i < n; i++)
        for(var j = 0; j < n; j++)
            for(var k = 0; k < n; k++)
                m1(i, j, Math.min(m1(i,j), m0(i,k) + m0(k, j)));
    return m1;
  }

  function allequal(m0, m1) {
    var a0 = m0.data(),
        a1 = m1.data();
    if (a0.length != a1.length)
        return false;
    for (var i = 0; i < a0.length; i++)
        if (a0[i] != a1[i])
            return false
    return true;
  }
  

  // insert into sorted list. TODO: make this efficient (take advantage of sortedness)
  function insertsorted(xl, x) {
    //for (var i = 0; i < xl.length; i++)
    //    if (x > xl[i])

    xl.push(x)
    xl.sort(function(a, b) { return a-b });
  }

  function range(size) {
    var arr = new Array(size);
    for (var i = 0; i < size; i++)
        arr[i] = i;
    return arr;
  }


  // adapted from http://stackoverflow.com/questions/11935175/sampling-a-random-subset-from-an-array
  // get sample of size k from range 1...size
  function getSample(k, size) {
    var copy = range(size), rand = [];
    for (var i = 0; i < k && i < size; i++) {
      var index = Math.floor(Math.random() * copy.length);
      rand.push(copy.splice(index, 1)[0]);
    }
    return rand;
  }

  keyPlayer.distanceMatrix = function() {
     if (!D) {
        D = calculateDistances(nodes, links);
        var n = nodes.length;
        metric = function(s) {
          var H = D.slice(D.rnot(s), s);
          var Hd = H.min().data(), sm = 0;
          for (var i = 0; i < Hd.length; i++) {
              sm += 1/Hd[i];
          }
          return sm/n;
        }
     }

     return D;
  }


  keyPlayer.nodes = function (_) {
    if (!arguments.length)
      return nodes;
    else {
      nodes = _;
      return keyPlayer;
    }
  };

  
  keyPlayer.links = function (_) {
    if (!arguments.length)
      return links;
    else {
      links = _;
      return keyPlayer;
    }
  };


  keyPlayer.k = function (_) {
    if (!arguments.length)
      return k;
    else {
      k = _;
      return keyPlayer;
    }
  };


  keyPlayer.tolerance = function (_) {
    if (!arguments.length)
      return tolerance;
    else {
      tolerance = _;
      return keyPlayer;
    }
  };

  keyPlayer.logging = function (_) {
    if (!arguments.length)
      return logging;
    else {
      logging = _;
      return keyPlayer;
    }
  };


  keyPlayer.metric = function (_) {
    if (!arguments.length)
      return metric;
    else {
      metric = _;
      return keyPlayer;
    }
  };

  return keyPlayer;
}
     
