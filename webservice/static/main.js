
$(function() {
  const canvas = document.getElementById("myCanvas");

  var lat_width = 30;
  var lon_width = 180;

  // latitude from 20 to 200
  // longitude from 55 to 85

  var lat_start = 20;
  var long_start = 55;

  var canvas_width = 1016.665;
  var canvas_height = 495.33;

  var divres = document.getElementById('res');


  var cell_width = canvas_width / lon_width;
  var cell_height = canvas_height / lat_width;

  var stageLegend = new Konva.Stage({
    container : 'legend', 
    width : 550,
    height : 70,
    draggable : false
  });
  var layerLegend = new Konva.Layer();
  stageLegend.add(layerLegend);


  // port label
  layerLegend.add(new Konva.Circle({
    x : 10,
    y : 20,
    radius : 5,
    fill : 'purple',
  }));

  layerLegend.add(new Konva.Text({
    x : 20,
    y : 15,
    text : "- Порт",
    fill : 'black',
    fontSize : 12,
    fontFamily : 'Sans Serif'
  }));

  // vertex label
  layerLegend.add(new Konva.Circle({
    x : 10,
    y : 40,
    radius : 5,
    fill : 'orange',
  }));

  layerLegend.add(new Konva.Text({
    x : 20,
    y : 35,
    text : "- Промежуточная вершина",
    fill : 'black',
    fontSize : 12,
    fontFamily : 'Sans Serif'
  }));


  // # 0 if ice >= 20
  //   # 1 if ice \in [15, 20)
  //   # 2 if ice \in [10, 15)
  //   # 3 if ice < 10 

  // green line >= 20
  layerLegend.add(new Konva.Line({
    points : [160, 10, 180, 10],
    stroke : 'green',
    strokeWidth : 2,
    lineCap : 'round'
  }));

  layerLegend.add(new Konva.Text({
    x : 185,
    y : 5,
    text : "- Ребро с проходимостью >= 20",
    fill : 'black',
    fontSize : 12,
    fontFamily : 'Sans Serif'
  }));

    // blue line [15, 20)
    layerLegend.add(new Konva.Line({
      points : [160, 25, 180, 25],
      stroke : 'blue',
      strokeWidth : 2,
      lineCap : 'round'
    }));
  
    layerLegend.add(new Konva.Text({
      x : 185,
      y : 20,
      text : "- Ребро с проходимостью [15, 20)",
      fill : 'black',
      fontSize : 12,
      fontFamily : 'Sans Serif'
    }));

    // yellow line [10, 15)
    layerLegend.add(new Konva.Line({
      points : [160, 40, 180, 40],
      stroke : 'yellow',
      strokeWidth : 2,
      lineCap : 'round'
    }));
  
    layerLegend.add(new Konva.Text({
      x : 185,
      y : 35,
      text : "- Ребро с проходимостью [10, 15)",
      fill : 'black',
      fontSize : 12,
      fontFamily : 'Sans Serif'
    }));

    // red line < 10
    layerLegend.add(new Konva.Line({
      points : [160, 55, 180, 55],
      stroke : 'red',
      strokeWidth : 2,
      lineCap : 'round'
    }));
  
    layerLegend.add(new Konva.Text({
      x : 185,
      y : 50,
      text : "- Ребро с проходимостью < 10",
      fill : 'black',
      fontSize : 12,
      fontFamily : 'Sans Serif'
    }));

    // icebreaker label
    layerLegend.add(new Konva.Rect({
      x : 370,
      y : 5,
      width: 10, 
      height : 10,
      fill : 'blue'
    }));

    layerLegend.add(new Konva.Text({
      x : 385,
      y : 5,
      text : "- Ледокол",
      fill : 'black',
      fontSize : 12,
      fontFamily : 'Sans Serif'
    }));

    // caravan
    layerLegend.add(new Konva.Text({
      x : 370,
      y : 20,
      text : "{i,j,k} - Караван из i,j,k кораблей",
      fill : 'black',
      fontSize : 12,
      fontFamily : 'Sans Serif'
    }));

    // ship

    layerLegend.add(new Konva.RegularPolygon({
      x : 375,
      y : 45,
      radius : 7,
      fill : "#6779ca",
      sides : 3
    }));

    layerLegend.add(new Konva.Text({
      x : 373,
      y : 39,
      text : "i",
      fontSize: 12,
      fontFamily: 'Sans Serif',
      fill: 'white',
    }));

    layerLegend.add(new Konva.Text({
      x : 380,
      y : 37,
      text : " - Корабль под номером i",
      fill : 'black',
      fontSize : 12,
      fontFamily : 'Sans Serif'
    }));

  var stage = new Konva.Stage({
    container : 'container',
    width : canvas_width,
    height: canvas_height,
    draggable : true
  });

  var scaleBy = 1.1;
  stage.on('wheel', (e) => {
    // stop default scrolling
    e.evt.preventDefault();

    var oldScale = stage.scaleX();
    var pointer = stage.getPointerPosition();

    var mousePointTo = {
      x: (pointer.x - stage.x()) / oldScale,
      y: (pointer.y - stage.y()) / oldScale,
    };

    // how to scale? Zoom in? Or zoom out?
    let direction = e.evt.deltaY > 0 ? -1 : 1;

    // when we zoom on trackpad, e.evt.ctrlKey is true
    // in that case lets revert direction
    if (e.evt.ctrlKey) {
      direction = -direction;
    }

    var newScale = direction > 0 ? oldScale * scaleBy : oldScale / scaleBy;

    stage.scale({ x: newScale, y: newScale });

    var newPos = {
      x: pointer.x - mousePointTo.x * newScale,
      y: pointer.y - mousePointTo.y * newScale,
    };
    stage.position(newPos);
  });

  var backgroundLayer = new Konva.Layer();
  stage.add(backgroundLayer);
  var background = new Konva.Rect({
    x: 0,
    y: 0,
    width : 9150,
    height : 4458,
    scaleX : 0.115,
    scaleY : 0.115
  });

  var backgroundImg = new Image();
  backgroundImg.src = 'static/main.png';

  backgroundImg.onload = function () {
    background.fillPatternImage(backgroundImg);
    backgroundLayer.add(background);
  }

  var layer = new Konva.Layer();
  stage.add(layer);

  var colorDict = {"0" : "cc-light-orange", 
  "1" : "cc-blue", 
  "2" : "cc-light-purple",
  "3" : "cc-light-green",
  "4" : "cc-purple",
  "5" : "cc-grey",
  "6" : "cc-light-green",
  "7" : "cc-orange",
  "8" : "cc-light-blue",
  "9" : "cc-red",
  "10" : "cc-green"};

var colorDictCanvas = {"0" : "#ab7632", 
  "1" : "#00bfff", 
  "2" : "#ac67cf",
  "3" : "#33a26b",
  "4" : "#800d88",
  "5" : "#474048",
  "6" : "#3cae8c",
  "7" : "#d4a004",
  "8" : "#2c83c9",
  "9" : "#a8000e",
  "10" : "#005301"};

  function addHours(date, hours) {
    const hoursToAdd = hours * 60 * 60 * 1000;
    date.setTime(date.getTime() + hoursToAdd);
    return date;
  }

  var months = ['Jan', 'Feb', 'Mar', 'Apr', 'May', 'Jun', 'Jul', 'Aug', 'Sep', 'Oct', 'Nov', 'Dec'];

  var gg = document.getElementById('t');

  document.getElementById('import').onclick = function() {

    var fileIntegr = document.getElementById('integrvelocity').files;
    var fileGraph = document.getElementById('graphdata').files;
    var fileShips = document.getElementById('scheduleships').files;

    if(Object.keys(fileIntegr).length === 0|| Object.keys(fileGraph).length === 0 || Object.keys(fileShips).length === 0) 
      return;
    document.getElementById('import').disabled = true;

    divres.innerHTML = "Программа работает, надо подождать пару минут.";

    var fd = new FormData();
    fd.append('fileIntegr', fileIntegr[0]);
    fd.append('fileGraph', fileGraph[0]);
    fd.append('fileShips', fileShips[0]);
    
    fetch("/putdata?car=" + document.getElementById('numberinput').value.toString(), {
      method: "POST",
      processData: false,
      contentType: false,
      body: fd
    }).then(res => {
      res.json().then(result => {
        var tasks = [];
        var edges = result["edges"];
        var ports = result["vertices"];
        var weeks = Object.keys(edges[0]["type"]);
    
        



        // date from string to Date() with -2 hours because of bug in the dataset
        for (let i = 0; i < weeks.length; i++){
          weeks[i] = weeks[i].slice(0, -1) + '2';
          weeks[i] = new Date(weeks[i]);
        }

        weeks.sort(function(a,b){
          return a - b;
        });
        
        // get start datetime
    
        var start_datetime = weeks[0].getDate().toString().padStart(2, '0') + "-" + months[weeks[0].getMonth()] + "-" + (weeks[0].getFullYear() - 2).toString();
    
        // create array for storing canvas edges
        var konvaLines = Array(edges.length);
    
        // plot edges
        for (let i = 0; i < edges.length; i++){
          edges[i]["len"] = edges[i]["len"] / 1852.0;
          let y1 = ports[edges[i]["start"]]["lat"];
          let x1 = ports[edges[i]["start"]]["lon"];
          let y2 = ports[edges[i]["end"]]["lat"];
          let x2 = ports[edges[i]["end"]]["lon"];
          konvaLines[i] = new Konva.Line({
            points: [(x1 - lat_start) * cell_width - cell_width / 2, (canvas_height) - (y1 - long_start) * cell_height - cell_width / 2,
                     (x2 - lat_start) * cell_width - cell_width / 2, (canvas_height) - (y2 - long_start) * cell_height - cell_width / 2],
            stroke: (edges[i]["type"][start_datetime] == 0 ? 'green' : (edges[i]["type"][start_datetime] == 1 ? 'blue' : (edges[i]["type"][start_datetime] == 2 ? 'yellow' : 'red'))),
            strokeWidth: 1,
            lineCap: 'round',
          });
          layer.add(konvaLines[i]);
        }
    
        for(let i = 0; i < edges.length; i++){
          let y1 = ports[edges[i]["start"]]["lat"];
          let x1 = ports[edges[i]["start"]]["lon"];
          let y2 = ports[edges[i]["end"]]["lat"];
          let x2 = ports[edges[i]["end"]]["lon"];
          let middle_x = x1 + (x2 - x1) / 2;
          let middle_y = y1 + (y2 - y1) / 2;
    
          layer.add(new Konva.Text({
            x: (middle_x - lat_start) * cell_width - cell_width,
            y: (canvas_height) - (middle_y - long_start) * cell_height - cell_width,
            text: i.toString(),
            fontSize: 3,
            fontFamily: 'Sans Serif',
            fill: 'black',
          }));
        }
    
        // plot vertices
    
        for (let i = 0; i < ports.length; i++) {
          let x = ports[i]["lon"];
          let y = ports[i]["lat"];
          layer.add(new Konva.Circle({
            x: (x - lat_start) * cell_width - cell_width / 2,
            y: (canvas_height) - (y - long_start) * cell_height - cell_width / 2,
            radius: 2,
            fill: ports[i]['name'] == '' ? 'orange' : 'purple',
          }));
        }
    
        for(let i = 0; i < ports.length; i++) {
          let x = ports[i]["lon"];
          let y = ports[i]["lat"];
          if(ports[i]["name"] != "")
            layer.add(new Konva.Text({
              x: (x - lat_start) * cell_width ,
              y: (canvas_height) - (y - long_start) * cell_height - cell_width,
              text: ports[i]["name"],
              fontSize: 6,
              fontFamily: 'Sans Serif',
              fill: 'black',
              fontStyle : 'bold'
            }));
        }
    
    
        var sumres = result["sum"];
        
        divres.innerHTML = "Результат: " + sumres.toString() + " суток общее время работы ";
        
        const ticks = new Set();
    
        // icebreakers
        for(let i = 0; i < result["icebreakers"].length; i++){
          for(let j = 0; j < result["icebreakers"][i]["path"].length; j++){
            let start_time= new Date(result["icebreakers"][i]["path"][j]["start_time"]).getTime();
            let end_time = new Date(result["icebreakers"][i]["path"][j]["end_time"]).getTime();
    
            ticks.add(start_time);
            ticks.add(end_time);
            let edge_id;
            for(let k = 0; k < edges.length; k++){
              if((edges[k]["start"] == result["icebreakers"][i]["path"][j]["start"] && 
              edges[k]["end"] == result["icebreakers"][i]["path"][j]["end"]) ||
              (edges[k]["end"] == result["icebreakers"][i]["path"][j]["start"] && 
              edges[k]["start"] == result["icebreakers"][i]["path"][j]["end"])) {
                result["icebreakers"][i]["path"][j]["len"] = edges[k]["len"];
                edge_id = k;
                break;
              }
            }
    
            const time_taken = (end_time - start_time) / 1000.0;
            // sea miles to meters
            const path_length = result["icebreakers"][i]["path"][j]["len"] * 1852;
    
            // узлы
            const vel =  path_length / time_taken * 1.943844492441;
    
            tasks.push({row_id : result["icebreakers"][i]["id"], 
                        start: result["icebreakers"][i]["path"][j]["start_time"],
                        end: result["icebreakers"][i]["path"][j]["end_time"],
                        progress: 0,
                        custom_class: colorDict[(result["icebreakers"][i]["id"] % 10).toString()],
                        name: "{" + result["icebreakers"][i]["path"][j]["caravan"].toString() + "}; " + 
                        "vel: " + (vel.toFixed(1)).toString() + "kn; e_id: " + edge_id.toString()
                      });
    
            for (let k = 0; k < result["icebreakers"][i]["path"][j]["caravan"].length; k++){
              tasks.push({row_id : result["icebreakers"][i]["path"][j]["caravan"][k] + result["icebreakers"].length,
              start : result["icebreakers"][i]["path"][j]["start_time"],
              end: result["icebreakers"][i]["path"][j]["end_time"],
              progress: 0,
              custom_class: colorDict[(result["icebreakers"][i]["id"] % 10).toString()],
              name : "vel: " + (vel.toFixed(1)).toString() + "kn; e_id: " + edge_id.toString()
            });
            }
          }
        }
    
    
        //ships
        if("ships" in result){
        for(let i = 0; i < result["ships"].length; i++){
          for(let j = 0; j < result["ships"][i]["path"].length; j++){
            let start_time = new Date(result["ships"][i]["path"][j]["start_time"]).getTime();
            let end_time = new Date(result["ships"][i]["path"][j]["end_time"]).getTime();
            ticks.add(start_time);
            ticks.add(end_time);
      
            let edge_id;
    
            for(let k = 0; k < edges.length; k++){
              if((edges[k]["start"] == result["ships"][i]["path"][j]["start"] && 
              edges[k]["end"] == result["ships"][i]["path"][j]["end"]) ||
              (edges[k]["end"] == result["ships"][i]["path"][j]["start"] && 
              edges[k]["start"] == result["ships"][i]["path"][j]["end"])) {
                result["ships"][i]["path"][j]["len"] = edges[k]["len"];
                edge_id = k;
                break;
              }
            }
    
            const time_taken = (end_time - start_time) / 1000.0;
            // sea miles to meters
            const path_length = result["ships"][i]["path"][j]["len"] * 1852;
    
            // meters / seconds
            const vel =  path_length / time_taken * 1.943844492441;
            if(time_taken != 0)
            tasks.push({row_id : result["ships"][i]["id"] + result["icebreakers"].length, 
                        start: result["ships"][i]["path"][j]["start_time"],
                        end: result["ships"][i]["path"][j]["end_time"],
                        progress: 0,
                        custom_class: "cc-ships",
                        name: time_taken == 0 ? "wait" : "vel: " + (vel.toFixed(1)).toString() + "kn; e_id: " + edge_id.toString()
                      });
          }
        }
      }
    
    
        var ships_schedule = [];
        for (let i = 0; i < result["ships"].length; i++){
          ships_schedule.push([]);
        }
    
    
        for(let i = 0; i < result["icebreakers"].length; i++){
          for(let j = 0; j < result["icebreakers"][i]["path"].length; j++){
            for (let k = 0; k < result["icebreakers"][i]["path"][j]["caravan"].length; k++){
              ships_schedule[result["icebreakers"][i]["path"][j]["caravan"][k]].push({"start_time" : result["icebreakers"][i]["path"][j]["start_time"],
                                      "end_time" : result["icebreakers"][i]["path"][j]["end_time"],
                                      "start" : result["icebreakers"][i]["path"][j]["start"],
                                      "end" : result["icebreakers"][i]["path"][j]["end"],
                                      "len" : result["icebreakers"][i]["path"][j]["len"],
                                      "caravan" : result["icebreakers"][i]["path"][j]["caravan"]});
            }
          }
        }
    
        if("ships" in result){
        for(let i = 0; i < result["ships"].length; i++){
          for(let j = 0; j < result["ships"][i]["path"].length; j++){
            ships_schedule[result["ships"][i]["id"]].push({"start_time" : result["ships"][i]["path"][j]["start_time"],
            "end_time" : result["ships"][i]["path"][j]["end_time"],
            "start" : result["ships"][i]["path"][j]["start"],
            "end" : result["ships"][i]["path"][j]["end"],
            "len" : result["ships"][i]["path"][j]["len"],
            "caravan" : ""});
          }
        }
      }
    
        for(let i = 0; i < ships_schedule.length; i++){
          ships_schedule[i].sort(function(a, b) {
            return (new Date(a["start_time"]).getTime() - new Date(b["start_time"]).getTime());
        });
        }
    
        // create column of names
        const tbl = document.createElement('table');
        for (let i = 0; i < result["icebreakers"].length; i++){
          const tr = tbl.insertRow();
          const td = tr.insertCell();
          td.appendChild(document.createTextNode(result["icebreakers"][i]["name"] + " [id:" + result["icebreakers"][i]["id"] + "]"));
        }
    
    
    
        for(let i = 0; i < result["ships"].length; i++){
          const tr = tbl.insertRow();
          const td = tr.insertCell();
          td.appendChild(document.createTextNode(result["ships"][i]["name"] + " [id:" + (i).toString() + "]")); 
        }
        
        gg.appendChild(tbl);
    
    
        var gantt = new Gantt("#gantt", tasks, {
          view_mode : "Hour",
          readonly: true,
          today_button: false});
    
        var ticks_arr = Array.from(ticks);
        ticks_arr.sort();
    
        var node = document.getElementById('dateText');
        let initdet = addHours(new Date(ticks_arr[0]), 3);
        node.textContent = initdet.toISOString();
    
        var gann = document.getElementsByClassName("gantt-container")[0];
        gann.style.height = (gg.offsetHeight + 74 + 100).toString() + "px";
    
        $("#slider").slider({
          value: ticks_arr[0],
          ticks: ticks_arr,
          step: 1440
        }, function(){
          $("#slider").css("display","block");
        });
    
        var cur_week_id = 0;
    
        $("#slider").on("change", function(slideEvt) {
          let val = slideEvt.value.newValue;
          var node = document.getElementById('dateText');
          let det = new Date(val);
          det = addHours(det, 3);
          node.textContent = det.toISOString();
          let to_delete = layer.find(".objects");
          for(let i = 0; i < to_delete.length; i++){
            to_delete[i].destroy();
          }
          if( (det < weeks[cur_week_id] && cur_week_id != 0) || (cur_week_id != weeks.length - 1 && det >= weeks[cur_week_id + 1])) {
            for(let i = 0; i < weeks.length; i++){
              if(weeks[i]<= det){
                cur_week_id = i;
              }
            }
    
            // change edges colors in canvas
            let datename = weeks[cur_week_id].getDate().toString().padStart(2, '0') + "-" + months[weeks[cur_week_id].getMonth()] + "-" + (weeks[cur_week_id].getFullYear() - 2).toString();
            console.log(datename);
            for(let j = 0; j < konvaLines.length; j++){
              let color = edges[j]["type"][datename] == 0 ? 'green' : (edges[j]["type"][datename] == 1 ? 'blue' : (edges[j]["type"][datename] == 2 ? 'yellow' : 'red'));
              konvaLines[j].stroke(color);
            }
          }
    
    
          gantt.set_scroll_position(det);
          for(let i = 0; i < result["icebreakers"].length; i++){
    
            if(val < result["icebreakers"][i]["path"][0]["start_time"] || val > result["icebreakers"][i]["path"][result["icebreakers"][i]["path"].length - 1]["end_time"]){
              continue;
            }
    
            for(let j = 0; j < result["icebreakers"][i]["path"].length; j++) {
              let start_time = new Date(result["icebreakers"][i]["path"][j]["start_time"]).getTime();
              let end_time = new Date(result["icebreakers"][i]["path"][j]["end_time"]).getTime();
              if(start_time <= val && end_time >= val){
                const time_taken = end_time - start_time;
                const path_length = result["icebreakers"][i]["path"][j]["len"];
    
                const vel =  path_length / time_taken;
    
                const y = ports[result["icebreakers"][i]["path"][j]["start"]]["lat"] + 
                          (ports[result["icebreakers"][i]["path"][j]["end"]]["lat"] - 
                          ports[result["icebreakers"][i]["path"][j]["start"]]["lat"]) * vel * (val - start_time) / path_length;
    
                const x = ports[result["icebreakers"][i]["path"][j]["start"]]["lon"] + 
                          (ports[result["icebreakers"][i]["path"][j]["end"]]["lon"] - 
                          ports[result["icebreakers"][i]["path"][j]["start"]]["lon"]) * vel * (val - start_time) / path_length;
     
                layer.add(new Konva.Rect({x : (x - lat_start) * cell_width - cell_width, y : (canvas_height) - (y - long_start) * cell_height - cell_width, width: cell_width, height : cell_width, fill : colorDictCanvas[(result["icebreakers"][i]["id"] % 10).toString()], name : "objects"}));           
                
                layer.add(new Konva.Text({
                  x: (x - lat_start) * cell_width - cell_width,
                  y: (canvas_height) - (y - long_start) * cell_height - 2*cell_width - cell_height / 3,
                  text: "{" + result["icebreakers"][i]["path"][j]["caravan"].toString() + "}",
                  fontSize: 8,
                  fontFamily: 'Sans Serif',
                  fill: 'black',
                  name : "objects"
                }));
                break;
              }
              
              if(j+1 != result["icebreakers"][i]["path"].length) {
                let time_left = end_time;
                let time_right = new Date(result["icebreakers"][i]["path"][j+1]["start_time"]).getTime();
                if(val > time_left && val < time_right){
                  const y = ports[result["icebreakers"][i]["path"][j]["end"]]["lat"];
                  const x = ports[result["icebreakers"][i]["path"][j]["end"]]["lon"];
                  layer.add(new Konva.Rect({x : (x - lat_start) * cell_width - cell_width, 
                                            y : (canvas_height) - (y - long_start) * cell_height - cell_width, 
                                            width: cell_width, 
                                            height : cell_width, 
                                            fill : colorDictCanvas[(result["icebreakers"][i]["id"] % 10).toString()], 
                                            name : "objects"}));
                  
                  layer.add(new Konva.Text({
                    x: (x - lat_start) * cell_width - cell_width,
                    y: (canvas_height) - (y - long_start) * cell_height - 2 * cell_width - cell_height / 3,
                    text: "{" + result["icebreakers"][i]["path"][j]["caravan"].toString() + "}",
                    fontSize: 8,
                    fontFamily: 'Sans Serif',
                    fill: 'black',
                    name : "objects"
                  }));
                  break;
                }
              }
            }
          }
    
          for(let i = 0; i < ships_schedule.length; i++){
            if(val < ships_schedule[i][0]["start_time"] || val > ships_schedule[i][ships_schedule[i].length - 1]["end_time"]){
              continue;
            }
    
            for(let j = 0; j < ships_schedule[i].length; j++) {
              let start_time = new Date(ships_schedule[i][j]["start_time"]).getTime();
              let end_time = new Date(ships_schedule[i][j]["end_time"]).getTime();
              if(start_time <= val && end_time >= val && ships_schedule[i][j]["caravan"] == ""){
                const time_taken = end_time - start_time;
                const path_length = time_taken == 0 ? 1 : ships_schedule[i][j]["len"];
                const vel = time_taken == 0 ? 0 : path_length / time_taken;
                const y = ports[ships_schedule[i][j]["start"]]["lat"] + 
                          (ports[ships_schedule[i][j]["end"]]["lat"] - 
                          ports[ships_schedule[i][j]["start"]]["lat"]) * vel * (val - start_time) / path_length;
    
                const x = ports[ships_schedule[i][j]["start"]]["lon"] + 
                          (ports[ships_schedule[i][j]["end"]]["lon"] - 
                          ports[ships_schedule[i][j]["start"]]["lon"]) * vel * (val - start_time) / path_length;
                layer.add(new Konva.RegularPolygon({x : (x - lat_start) * cell_width - cell_width / 2, 
                                                    y : (canvas_height) - (y - long_start) * cell_height - cell_width / 2, 
                                                    radius : cell_width, 
                                                    fill : "#6779ca", 
                                                    sides : 3, 
                                                    name : "objects"}));
                  
                layer.add(new Konva.Text({
                  x: (x - lat_start) * cell_width - cell_width,
                  y: (canvas_height) - (y - long_start) * cell_height - 2*cell_width  + cell_height / 3,
                  text: i.toString(),
                  fontSize: 6,
                  fontFamily: 'Sans Serif',
                  fill: 'white',
                  name : "objects"
                }));
               break;
              }
    
              if(j+1 != ships_schedule[i].length) {
                let time_left = end_time;
                let time_right = new Date(ships_schedule[i][j+1]["start_time"]).getTime();
                if(val > time_left && val < time_right){
                  const y = ports[ships_schedule[i][j]["end"]]["lat"];
                  const x = ports[ships_schedule[i][j]["end"]]["lon"];
                  layer.add(new Konva.RegularPolygon({x : (x - lat_start) * cell_width - cell_width / 2, 
                  y : (canvas_height) - (y - long_start) * cell_height - cell_width / 2, 
                                                      radius :  cell_width, 
                                                      fill : "#6779ca", 
                                                      sides : 3, 
                                                      name : "objects"}));
                  layer.add(new Konva.Text({
                    x: (x - lat_start) * cell_width - cell_width,
                    y: (canvas_height) - (y - long_start) * cell_height - 2*cell_width  + cell_height / 3,
                    text: i.toString(),
                    fontSize: 6,
                    fontFamily: 'Sans Serif',
                    fill: 'white',
                    name : "objects"
                  }));
                  break;
                }
              }
    
            }
          }
        });
    });
    });
  }






});

