const char HTML[] PROGMEM = "<!doctype html>\n"
"<html lang=\"en\">\n"
"\n"
"<head>\n"
"  <style media=\"screen\" type=\"text/css\">\n"
"    /* Common stuff */\n"
"\n"
"    .slidecontainer {\n"
"      width: 100%;\n"
"      /* Width of the outside container */\n"
"    }\n"
"\n"
"    /* The slider itself */\n"
"    .slider {\n"
"      -webkit-appearance: none;\n"
"      /* Override default CSS styles */\n"
"      appearance: none;\n"
"      width: 100%;\n"
"      /* Full-width */\n"
"      height: 25px;\n"
"      /* Specified height */\n"
"      background: #d3d3d3;\n"
"      /* Grey background */\n"
"      outline: none;\n"
"      /* Remove outline */\n"
"      opacity: 0.7;\n"
"      /* Set transparency (for mouse-over effects on hover) */\n"
"      -webkit-transition: .2s;\n"
"      /* 0.2 seconds transition on hover */\n"
"      transition: opacity .2s;\n"
"    }\n"
"\n"
"    /* Mouse-over effects */\n"
"    .slider:hover {\n"
"      opacity: 1;\n"
"      /* Fully shown on mouse-over */\n"
"    }\n"
"\n"
"    /* The slider handle (use -webkit- (Chrome, Opera, Safari, Edge) and -moz- (Firefox) to override default look) */\n"
"    .slider::-webkit-slider-thumb {\n"
"      -webkit-appearance: none;\n"
"      /* Override default look */\n"
"      appearance: none;\n"
"      width: 25px;\n"
"      /* Set a specific slider handle width */\n"
"      height: 25px;\n"
"      /* Slider handle height */\n"
"      background: #04AA6D;\n"
"      /* Green background */\n"
"      cursor: pointer;\n"
"      /* Cursor on hover */\n"
"    }\n"
"\n"
"    .slider::-moz-range-thumb {\n"
"      width: 25px;\n"
"      /* Set a specific slider handle width */\n"
"      height: 25px;\n"
"      /* Slider handle height */\n"
"      background: #04AA6D;\n"
"      /* Green background */\n"
"      cursor: pointer;\n"
"      /* Cursor on hover */\n"
"    }\n"
"  </style>\n"
"\n"
"</head>\n"
"\n"
"<body>\n"
"  <br>\n"
"  <strong>*** Front Wall Landscape Lighting Control ***</strong>\n"
"  <br>\n"
"  <br>\n"
"  <strong>ON/OFF</strong>\n"
"  <br>\n"
"  <button id=\"myButton\" class=\"float-left submit-button\" style=\"height:30px;width:80px\" value=\"1\">On</button>\n"
"  <br>\n"
"  <strong>Theme</strong>\n"
"  <br>\n"
"\n"
"  <!-- text select -->\n"
"  <select id=\"mySelectStyle\">\n"
"    <option value=\"1\">Standard Landscape</option>\n"
"    <option value=\"2\">Christmas</option>\n"
"    <option value=\"3\">Autumn</option>\n"
"    <option value=\"4\">Valentines</option>\n"
"    <option value=\"5\">St Patricks</option>\n"
"    <option value=\"6\">Independance Day</option>\n"
"    <option value=\"7\">Rainbow</option>\n"
"    <option value=\"8\">Halloween</option>\n"
"    <option value=\"9\">Custom</option>\n"
"    <option value=\"10\">Special Christmas</option>\n"
"  </select>\n"
"  <br>\n"
"  <br>\n"
"    <strong>Transition type</strong>\n"
"  <br>\n"
"  <!-- text select -->\n"
"  <select id=\"mySelect\">\n"
"    <option value=\"1\">NeoEase::Linear</option>\n"
"    <option value=\"2\">NeoEase::QuadraticInOut</option>\n"
"    <option value=\"3\">NeoEase::CubicInOut</option>\n"
"    <option value=\"4\">NeoEase::QuarticInOut</option>\n"
"    <option value=\"5\">NeoEase::QuinticInOut</option>\n"
"    <option value=\"6\">NeoEase::SinusoidalInOut</option>\n"
"    <option value=\"7\">NeoEase::ExponentialInOut</option>\n"
"    <option value=\"8\">NeoEase::CircularInOut</option>\n"
"  </select>\n"
"  <br>\n"
"  <br>\n"
"  <!-- slider \"myRangeBright\" -->\n"
"    <div class=\"slidecontainer\" style=\"width:300px\">\n"
"        <input type=\"range\" min=\"0\" max=\"100\" value=\"25\" class=\"slider\" id=\"myRangeBright\">\n"
"        </div>\n"
"        Brightness value = <span id=\"rangeValueBright\">25</span>\n"
"  <br>\n"
"  <br>\n"
"  <!-- slider \"myRangeTime\" -->\n"
"  <div class=\"slidecontainer\" style=\"width:300px\">\n"
"    <input type=\"range\" min=\"0\" max=\"300\" value=\"100\" class=\"slider\" id=\"myRangeTime\">\n"
"    </div>\n"
"    Animation time value = <span id=\"rangeValueTime\">100</span>\n"
"  <br>\n"
"  <!-- visible page stuff-->\n"
"\n"
"  <br>\n"
"  <strong>Custom light colors</strong>\n"
"  <br>\n"
"  <!-- slider \"myRangeRed\" -->\n"
"  <div class=\"slidecontainer\" style=\"width:300px\">\n"
"    <input type=\"range\" min=\"0\" max=\"255\" value=\"0\" class=\"slider\" id=\"myRangeRed\">\n"
"  </div>\n"
"  Red value = <span id=\"rangeValueRed\">0</span>\n"
"  <br>\n"
"  <!-- slider \"myRangeGreen\" -->\n"
"  <div class=\"slidecontainer\" style=\"width:300px\">\n"
"    <input type=\"range\" min=\"0\" max=\"255\" value=\"0\" class=\"slider\" id=\"myRangeGreen\">\n"
"  </div>\n"
"  Green value = <span id=\"rangeValueGreen\">0</span>\n"
"  <br>\n"
"  <!-- slider \"myRangeBlue\" -->\n"
"  <div class=\"slidecontainer\" style=\"width:300px\">\n"
"    <input type=\"range\" min=\"0\" max=\"255\" value=\"0\" class=\"slider\" id=\"myRangeBlue\">\n"
"  </div>\n"
"  Blue value = <span id=\"rangeValueBlue\">0</span> <br>\n"
"  <!-- slider \"myRangeWhite\" -->\n"
"  <div class=\"slidecontainer\" style=\"width:300px\">\n"
"    <input type=\"range\" min=\"0\" max=\"255\" value=\"50\" class=\"slider\" id=\"myRangeWhite\">\n"
"  </div>\n"
"  White value = <span id=\"rangeValueWhite\">50</span> <br> <br>\n"
"\n"
"  <strong>Send To Controller</strong>\n"
"  <br>\n"
"  <br>  <button id=\"myPostButton\" class=\"float-left submit-button\" style=\"height:50px;width:100px\">Update</button>\n"
"\n"
"  <!-- javascript stuff-->\n"
"  <script>\n"
"    var http = new XMLHttpRequest();\n"
"    function post_without_reload(path, params, method) {\n"
"      if (http) {\n"
"        http.abort();\n"
"      }\n"
"      http = new XMLHttpRequest();\n"
"      http.open(method, path, true);\n"
"      http.setRequestHeader(\"Content-type\", \"application/x-www-form-urlencoded\");\n"
"      http.send(params);\n"
"    }\n"
"\n"
"    function return_parameter_string() {\n"
"        var val;\n"
"        val = \"onoff=\" + document.getElementById(\"myButton\").value + \n"
"            \"&red=\" + document.getElementById(\"rangeValueRed\").innerHTML + \n"
"            \"&blue=\" + document.getElementById(\"rangeValueBlue\").innerHTML +\n"
"            \"&green=\" + document.getElementById(\"rangeValueGreen\").innerHTML +\n"
"            \"&white=\" + document.getElementById(\"rangeValueWhite\").innerHTML +\n"
"            \"&styleType=\" + document.getElementById(\"mySelectStyle\").value +\n"
"            \"&NeoEase=\" + document.getElementById(\"mySelect\").value + \n"
"            \"&bright=\" + document.getElementById(\"rangeValueBright\").innerHTML +\n"
"            \"&aniTime=\" + document.getElementById(\"rangeValueTime\").innerHTML;\n"
"        return val;\n"
"    }\n"
"  </script>\n"
"\n"
"  <script type=\"text/javascript\">\n"
"    document.getElementById(\"myButton\").onclick = function () {\n"
"      // do your button click stuff here\n"
"      var el = document.getElementById(\"myButton\");\n"
"      var state = el.innerHTML;\n"
"      if (state == \"On\") {\n"
"        el.value = \"0\";  // change the value passed to the next page\n"
"        el.innerHTML = \"Off\";  // change the displayed text on the screen\n"
"      }\n"
"      if (state == \"Off\") {\n"
"        el.value = \"1\";  // change the value passed to the next page\n"
"        el.innerHTML = \"On\";  // change the displayed text on the screen\n"
"      }\n"
"    };\n"
"\n"
"    // Update the current slider value (each time you drag the slider handle)\n"
"    document.getElementById(\"myRangeRed\").oninput = function () {\n"
"      // do your slide change stuff here\n"
"      document.getElementById(\"rangeValueRed\").innerHTML = this.value;\n"
"    };\n"
"\n"
"    document.getElementById(\"myRangeGreen\").oninput = function () {\n"
"      // do your slide change stuff here\n"
"      document.getElementById(\"rangeValueGreen\").innerHTML = this.value;\n"
"    };\n"
"\n"
"    document.getElementById(\"myRangeBlue\").oninput = function () {\n"
"      // do your slide change stuff here\n"
"      document.getElementById(\"rangeValueBlue\").innerHTML = this.value;\n"
"    };\n"
"\n"
"    document.getElementById(\"myRangeWhite\").oninput = function () {\n"
"      // do your slide change stuff here\n"
"      document.getElementById(\"rangeValueWhite\").innerHTML = this.value;\n"
"    };\n"
"\n"
"    document.getElementById(\"myRangeBright\").oninput = function () {\n"
"      // do your slide change stuff here\n"
"      document.getElementById(\"rangeValueBright\").innerHTML = this.value;\n"
"    };\n"
"\n"
"    document.getElementById(\"myRangeTime\").oninput = function () {\n"
"      // do your slide change stuff here\n"
"      document.getElementById(\"rangeValueTime\").innerHTML = this.value;\n"
"    };\n"
"\n"
"    document.getElementById(\"mySelectStyle\").onchange = function () {\n"
"      // do your select change stuff here\n"
"    };\n"
"    \n"
"    document.getElementById(\"mySelect\").onchange = function () {\n"
"      // do your select change stuff here\n"
"    };\n"
"\n"
"    document.getElementById(\"myPostButton\").onclick = function () {\n"
"      // do your button click stuff here\n"
"      var p;\n"
"      p=return_parameter_string();\n"
"      //alert(\"parameters: \" + p)\n"
"      post_without_reload(\"/\", p, \"POST\");\n"
"\n"
"    };\n"
"  </script>\n"
"</body>\n"
"\n"
"</html>";