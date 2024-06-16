
$(function() {
  const canvas = document.getElementById("myCanvas");

  var ctx;
  

  var lat_width = 30;
  var lon_width = 180;

  // latitude from 20 to 200
  // longitude from 55 to 85

  var lat_start = 20;
  var long_start = 55;

  if (canvas.getContext) {
    ctx = canvas.getContext('2d');
    var cell_width = canvas.width / lon_width;
    var cell_height = canvas.height / lat_width;
  };

  document.getElementById('import').onclick = function() {
    var files = document.getElementById('selectFiles').files;
  if (files.length <= 0) {
    return false;
  }

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

  var fr = new FileReader();

  function addHours(date, hours) {
    const hoursToAdd = hours * 60 * 60 * 1000;
    date.setTime(date.getTime() + hoursToAdd);
    return date;
  }

  fr.onload = function(e) { 
  var tasks = [];
    var result = JSON.parse(e.target.result);

    var edges = result["edges"];
    var ports = result["vertices"];
    var sumres = result["sum"];
    var divres = document.getElementById('res');
    divres.innerHTML = "Результат: " + sumres.toString() + " суток общее время работы ";
    
    const ticks = new Set();
    var ships = new Set();

    for (let i = 0; i < edges.length; i++){
      edges[i]["len"] = edges[i]["len"] / 1852.0;
    }
    
    var weeks = Object.keys(edges[0]["type"]);

    for (let i = 0; i < weeks.length; i++){
      weeks[i] = weeks[i].slice(0, -1) + '2';
      weeks[i] = new Date(weeks[i]);
    }

    weeks.sort();

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
        ships.add(result["icebreakers"][i]["path"][j]["caravan"][k]);
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
      ships.add(result["ships"][i]["id"]); 
    }
  }


    var ships_schedule = [];
    for (let i = 0; i <= Math.max.apply(Math, Array.from(ships)); i++){
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
    var gg = document.getElementById('t');
    const tbl = document.createElement('table');
    for (let i = 0; i < result["icebreakers"].length; i++){
      const tr = tbl.insertRow();
      const td = tr.insertCell();
      td.appendChild(document.createTextNode("icb" + result["icebreakers"][i]["id"]));
    }

    for(let i = 0; i <= Math.max.apply(Math, Array.from(ships)); i++){
      const tr = tbl.insertRow();
      const td = tr.insertCell();
      td.appendChild(document.createTextNode("ship" + (i).toString())); 
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
      ctx.clearRect(0, 0, canvas.width, canvas.height);
      if(det < weeks[cur_week_id] || (cur_week_id != weeks.length - 1 && det >= weeks[cur_week_id + 1])) {
        for(let i = 0; i < weeks.length; i++){
          if(weeks[i]<= det){
            cur_week_id = i;
            document.getElementById('myCanvas').style.backgroundImage="url(static/main_" + i.toString() + ".png)";
          }
        }
      }


      gantt.set_scroll_position(det);
      ctx.font = "12px serif";
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

            ctx.fillStyle = colorDictCanvas[(result["icebreakers"][i]["id"] % 10).toString()];
            ctx.fillRect((x - lat_start) * cell_width - cell_width / 2, (canvas.height) - (y - long_start) * cell_height - cell_width / 2, cell_width , cell_width);
            ctx.fillStyle = "black";
            ctx.fillText("{" + result["icebreakers"][i]["path"][j]["caravan"].toString() + "}", (x - lat_start) * cell_width - cell_width / 2, (canvas.height) - (y - long_start) * cell_height - cell_width / 2 - cell_height / 3);
            break;
          }
          
          if(j+1 != result["icebreakers"][i]["path"].length) {
            let time_left = end_time;
            let time_right = new Date(result["icebreakers"][i]["path"][j+1]["start_time"]).getTime();
            if(val > time_left && val < time_right){
              const y = ports[result["icebreakers"][i]["path"][j]["end"]]["lat"];
              const x = ports[result["icebreakers"][i]["path"][j]["end"]]["lon"];

              ctx.fillStyle = colorDictCanvas[(result["icebreakers"][i]["id"] % 10).toString()];
              ctx.fillRect((x - lat_start) * cell_width - cell_width / 2, (canvas.height) - (y - long_start) * cell_height - cell_width / 2, cell_width , cell_width);
              ctx.fillStyle = "black";
              ctx.fillText("{" + result["icebreakers"][i]["path"][j+1]["caravan"].toString() + "}", (x - lat_start) * cell_width - cell_width / 2, (canvas.height) - (y - long_start) * cell_height - cell_width / 2 - cell_height / 3);
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
            
            ctx.fillStyle = "#6779ca";
            ctx.fillRect((x - lat_start) * cell_width - cell_width / 2, (canvas.height) - (y - long_start) * cell_height - cell_width / 2, cell_width , cell_width);
            ctx.fillStyle = "white";
            ctx.fillText(i.toString(), (x - lat_start) * cell_width - cell_width / 2, (canvas.height) - (y - long_start) * cell_height - cell_width / 2 + cell_height / 3);
            break;
          }

          if(j+1 != ships_schedule[i].length) {
            let time_left = end_time;
            let time_right = new Date(ships_schedule[i][j+1]["start_time"]).getTime();
            if(val > time_left && val < time_right){
              const y = ports[ships_schedule[i][j]["end"]]["lat"];
              const x = ports[ships_schedule[i][j]["end"]]["lon"];

              ctx.fillStyle = "#6779ca";
              ctx.fillRect((x - lat_start) * cell_width - cell_width / 2, (canvas.height) - (y - long_start) * cell_height - cell_width / 2, cell_width , cell_width);
              ctx.fillStyle = "white";
              ctx.fillText(i.toString(), (x - lat_start) * cell_width - cell_width / 2, (canvas.height) - (y - long_start) * cell_height - cell_width / 2 + cell_height / 3);
              break;
            }
          }

        }
      }
    });

  
  
    }

  fr.readAsText(files.item(0));
  };

  
});

