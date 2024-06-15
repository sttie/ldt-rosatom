
$(function() {
  const canvas = document.getElementById("myCanvas");

  var ctx;
  const edges = [{"start" : 44, "end":	15, "len":	270.0166416},
  {"start" : 10, "end":	11, "len":	277.1898363},
  {"start" : 18, "end":	39, "len":	58.39035132},
  {"start" : 13, "end":	16, "len":	238.8589885},
  {"start" : 10, "end":	13, "len":	86.93004916},
  {"start" : 21, "end":	5, "len":	655.3557374},
  {"start" : 4, "end":	5, "len":	410.5701116},
  {"start" : 45, "end":	37, "len":	119.5634492},
  {"start" : 13, "end":	7, "len":	206.8821279},
  {"start" : 9, "end":	24, "len":	191.1526107},
  {"start" : 27, "end":	18, "len":	178.645412},
  {"start" : 28, "end":	33, "len":	205.7748943},
  {"start" : 40, "end":	22, "len":	134.3049847},
  {"start" : 42, "end":	30, "len":	174.3916815},
  {"start" : 31, "end":	3, "len":	81.8159608},
  {"start" : 0, "end":	43, "len":	123.6831421},
  {"start" : 12, "end":	24, "len":	246.2458649},
  {"start" : 12, "end":	42, "len":	339.778226},
  {"start" : 15, "end":	2, "len":	270.5921169},
  {"start" : 10, "end":	35, "len":	127.6546236},
  {"start" : 9, "end":	32, "len":	140.7360068},
  {"start" : 2, "end":	3, "len":	278.1227556},
  {"start" : 0, "end":	1, "len":	251.8626597},
  {"start" : 33, "end":	18, "len":	225.0238977},
  {"start" : 40, "end":	42, "len":	152.2019466},
  {"start" : 4, "end":	41, "len":	288.1919175},
  {"start" : 8, "end":	12, "len":	121.8205679},
  {"start" : 2, "end":	29, "len":	315.3047628},
  {"start" : 15, "end":	3, "len":	69.16861273},
  {"start" : 12, "end":	40, "len":	234.8941148},
  {"start" : 19, "end":	8, "len":	47.63018916},
  {"start" : 28, "end":	30, "len":	308.4151996},
  {"start" : 13, "end":	0, "len":	77.9639623},
  {"start" : 21, "end":	2, "len":	445.9281382},
  {"start" : 30, "end":	18, "len":	436.2716913},
  {"start" : 40, "end":	9, "len":	229.4577372},
  {"start" : 21, "end":	4, "len":	297.1508085},
  {"start" : 16, "end":	34, "len":	296.3206745},
  {"start" : 45, "end":	17, "len":	373.6308274},
  {"start" : 17, "end":	18, "len":	281.0870454},
  {"start" : 18, "end":	26, "len":	98.81552937},
  {"start" : 38, "end":	46, "len":	460.2848327},
  {"start" : 30, "end":	33, "len":	260.5701136},
  {"start" : 24, "end":	22, "len":	179.7422785},
  {"start" : 10, "end":	16, "len":	263.6393465},
  {"start" : 27, "end":	45, "len":	355.6231614},
  {"start" : 7, "end":	36, "len":	157.6035239},
  {"start" : 7, "end":	20, "len":	192.4522229},
  {"start" : 34, "end":	13, "len":	215.0532376},
  {"start" : 43, "end":	36, "len":	223.5714878},
  {"start" : 6, "end":	7, "len":	352.936821},
  {"start" : 10, "end":	25, "len":	108.6630553},
  {"start" : 37, "end":	38, "len":	414.6912251},
  {"start" : 13, "end":	15, "len":	410.4549185},
  {"start" : 7, "end":	16, "len":	260.1771091},
  {"start" : 6, "end":	16, "len":	171.4172136},
  {"start" : 36, "end":	19, "len":	116.7439955},
  {"start" : 33, "end":	23, "len":	261.600589},
  {"start" : 20, "end":	16, "len":	409.3111575},
  {"start" : 10, "end":	15, "len":	326.4328417},
  {"start" : 34, "end":	15, "len":	195.7328443},
  {"start" : 44, "end":	2, "len":	156.0644819},
  {"start" : 14, "end":	0, "len":	45.6421557},
  {"start" : 8, "end":	9, "len":	206.5979139},
  {"start" : 2, "end":	5, "len":	456.2676239},
  {"start" : 16, "end":	21, "len":	145.1826364},
  {"start" : 27, "end":	28, "len":	275.6604549},
  {"start" : 13, "end":	43, "len":	130.3081116},
  {"start" : 19, "end":	20, "len":	58.95750201},
  {"start" : 6, "end":	15, "len":	280.7806599},
  {"start" : 2, "end":	41, "len":	227.2269304},
  {"start" : 22, "end":	23, "len":	124.9092209},
  {"start" : 17, "end":	28, "len":	285.0795135}];


  // lat long name
  const ports = [{"id":0 ,"lat": 73.1, "lon": 80, "name": "Бухта Север и Диксон"},
  {"id":1 ,"lat": 69.4, "lon": 86.15	, "name": "Дудинка"},
  {"id":2 ,"lat": 69.9, "lon": 44.6	, "name": "кромка льда на Западе"},
  {"id":3 ,"lat": 69.15,"lon": 57.68	, "name": "Варандей-Приразломное"},
  {"id":4 ,"lat": 73,   "lon": 44	, "name": "Штокман"},
  {"id":5 ,"lat": 71.5, "lon": 22	, "name": "Окно в Европу"},
  {"id":6 ,"lat": 74.6, "lon": 63.9	, "name": "Победа месторождение"},
  {"id":7 ,"lat": 76.4, "lon": 86.4	, "name": "Карское - 3 (центр)"},
  {"id":8 ,"lat": 77.6, "lon": 107.7	, "name": "пролив Вилькицкого - 3"},
  {"id":9 ,"lat": 74.9, "lon": 116.7	, "name": "Лаптевых - 4 (юг)"},
  {"id":10 ,"lat": 73.1, "lon": 72.7	, "name": "Вход в Обскую губу"},
  {"id":11 ,"lat": 68.5, "lon": 73.7	, "name": "Новый порт"},
  {"id":12 ,"lat": 76.75,"lon": 116	, "name": "Лаптевых - 1 (центр)"},
  {"id":13 ,"lat": 74,   "lon": 76.7	, "name": "Карское - 1 (сбор каравана)"},
  {"id":14 ,"lat": 72.35,"lon": 79.6	, "name": "Лескинское м-е"},
  {"id":15 ,"lat": 70.3, "lon": 57.8	, "name": "Карские ворота"},
  {"id":16 ,"lat": 77.3, "lon": 67.7	, "name": "Мыс Желания"},
  {"id":17 ,"lat": 71.74,"lon": 184.7, "name": "остров Врангеля"},
  {"id":18 ,"lat": 70.7, "lon": 170.5, "name": "Восточно-Сибирское - 1 (восток)"},
  {"id":19 ,"lat": 77.8, "lon": 104.1, "name": "пролив Вилькицкого - восток"},
  {"id":20 ,"lat": 77.7, "lon": 99.5	, "name": "пролив Вилькицкого - запад"},
  {"id":21 ,"lat": 76.2, "lon": 58.3	, "name": "около Новой Земли"},
  {"id":22 ,"lat": 74.4, "lon": 139	, "name": "Пролив Санникова - 1"},
  {"id":23 ,"lat": 74.3, "lon": 146.7, "name": "Пролив Санникова - 2"},
  {"id":24 ,"lat": 74,   "lon": 128.1, "name": "устье Лены"},
  {"id":25 ,"lat": 71.3, "lon": 72.15, "name": "Сабетта"},
  {"id":26 ,"lat": 69.1, "lon": 169.4, "name": "мыс.Наглёйнын"},
  {"id":27 ,"lat": 69.9, "lon": 179	, "name": "пролив Лонга"},
  {"id":28 ,"lat": 73.5, "lon": 169.9, "name": "Восточно-Сибирское - 3 (север)"},
  {"id":29 ,"lat": 64.95,"lon": 40.05, "name": "Архангельск"},
  {"id":30 ,"lat": 75.9, "lon": 152.6, "name": "Лаптевых - 3 (восток)"},
  {"id":31 ,"lat": 68.37,"lon": 54.6	, "name": "МОТ Печора"},
  {"id":32 ,"lat": 73.7, "lon": 109.26, "name": "Хатангский залив"},
  {"id":33 ,"lat": 72,   "lon": 159.5, "name": "Восточно-Сибирское - 2 (запад)"},
  {"id":34 ,"lat": 72.4, "lon": 65.6	, "name": "Ленинградское-Русановское"},
  {"id":35 ,"lat": 71,   "lon": 73.73, "name": "терминал Утренний"},
  {"id":36 ,"lat": 76.5, "lon": 97.6	, "name": "Таймырский залив"},
  {"id":37 ,"lat": 64.2, "lon": 188.2, "name": "Берингово"},
  {"id":38 ,"lat": 60.7, "lon": 175.3, "name": "кромка льда на Востоке"},
  {"id":39 ,"lat": 69.75,"lon": 169.9, "name": "Рейд Певек"},
  {"id":40 ,"lat": 75.5, "lon": 131.5, "name": "Лаптевых - 2 (центр)"},
  {"id":41 ,"lat": 69.5, "lon": 33.75, "name": "Рейд Мурманска"},
  {"id":42 ,"lat": 76.7, "lon": 140.8, "name": "остров Котельный"},
  {"id":43 ,"lat": 74.8, "lon": 84.2	, "name": "Карское - 2 (прибрежный)"},
  {"id":44 ,"lat": 67.58,"lon": 47.82, "name": "Индига"},
  {"id":45 ,"lat": 65.9, "lon": 190.65, "name": "Берингов пролив"},
  {"id":46 ,"lat": 55.7, "lon": 164.25, "name": "Окно в Азию"}];


  var lat_width = 30;
  var lon_width = 180;

  // latitude from 20 to 200
  // longitude from 55 to 85

  var lat_start = 20;
  var long_start = 55;

function drawPoint(x, y){
  ctx.fillRect((x - lat_start) * cell_width - cell_width / 2, (canvas.height) - (y - long_start) * cell_height - cell_width / 2, cell_width , cell_width)
}

  if (canvas.getContext) {
    ctx = canvas.getContext('2d');
    var img1 = new Image();

    var cell_width = canvas.width / lon_width;
    var cell_height = canvas.height / lat_width;
    console.log(cell_width, cell_height)

  //   img1.onload = function () {
  //     //draw background image
  //     ctx.drawImage(img1, 0, 0, 1800, 350);

  //     for (let i = 0; i < ports.length; i++) {
  //       //drawPoint(ports[i].lon, ports[i].lat);
  //     }
  //   };

  // img1.src = 'static/main.png';
  };

  document.getElementById('import').onclick = function() {
    var files = document.getElementById('selectFiles').files;
  console.log(files);
  if (files.length <= 0) {
    return false;
  }

  var fr = new FileReader();

  fr.onload = function(e) { 
  console.log(e);
    var result = JSON.parse(e.target.result);
    //var formatted = JSON.stringify(result, null, 2);
    
    const ticks = new Set();

    for(let i = 0; i < result["icebreakers"].length; i++){
      for(let j = 0; j < result["icebreakers"][i]["path"].length; j++){
        ticks.add(result["icebreakers"][i]["path"][j]["start_time"]);
        ticks.add(result["icebreakers"][i]["path"][j]["end_time"]);

        for(let k = 0; k < edges.length; k++){
          if((edges[k]["start"] == result["icebreakers"][i]["path"][j]["start"] && 
          edges[k]["end"] == result["icebreakers"][i]["path"][j]["end"]) ||
          (edges[k]["end"] == result["icebreakers"][i]["path"][j]["start"] && 
          edges[k]["start"] == result["icebreakers"][i]["path"][j]["end"])) {
            result["icebreakers"][i]["path"][j]["len"] = edges[k]["len"];
            break;
          }
        }

      }
    }

    const ticks_arr = Array.from(ticks).sort();
    console.log(ticks_arr);

    $("#slider").slider({
      value: 0,
      ticks: Array.from(ticks),
      step: 0.1
    }, function(){
      $("#slider").css("display","block");
    });

    $("#slider").on("slide", function(slideEvt) {
      console.log(canvas.width)
      ctx.clearRect(0, 0, canvas.width, canvas.height);

      for(let i = 0; i < result["icebreakers"].length; i++){

        if(slideEvt.value < result["icebreakers"][i]["path"][0]["start_time"] || slideEvt.value > result["icebreakers"][i]["path"][result["icebreakers"][i]["path"].length - 1]["end_time"]){
          continue;
        }

        for(let j = 0; j < result["icebreakers"][i]["path"].length; j++) {
          if(result["icebreakers"][i]["path"][j]["start_time"] <= slideEvt.value && result["icebreakers"][i]["path"][j]["end_time"] >= slideEvt.value){
            const time_taken = result["icebreakers"][i]["path"][j]["end_time"] - result["icebreakers"][i]["path"][j]["start_time"];
            const path_length = result["icebreakers"][i]["path"][j]["len"];

            const vel =  path_length / time_taken;

            const y = ports[result["icebreakers"][i]["path"][j]["start"]]["lat"] + 
                      (ports[result["icebreakers"][i]["path"][j]["end"]]["lat"] - 
                      ports[result["icebreakers"][i]["path"][j]["start"]]["lat"]) * vel * (slideEvt.value - result["icebreakers"][i]["path"][j]["start_time"]) / path_length;

            const x = ports[result["icebreakers"][i]["path"][j]["start"]]["lon"] + 
                      (ports[result["icebreakers"][i]["path"][j]["end"]]["lon"] - 
                      ports[result["icebreakers"][i]["path"][j]["start"]]["lon"]) * vel * (slideEvt.value - result["icebreakers"][i]["path"][j]["start_time"]) / path_length;

            drawPoint(x,y);
            break;
          }
        }
      }

 
    });

  
  
    }

  fr.readAsText(files.item(0));
  };

  

  
});

