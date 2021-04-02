/*****************GLOBAL VAR*************************/
const server_ip = "192.168.5.18";
const server_port = ":3500";

// defined via PIN number in MCU section PIN = 35
const up_pin = 35;
// defined via PIN number in MCU section PIN = 34
const down_pin = 34;
// custom "pin"
const left_pin = 11;
const right_pin = 12;
const http_post = "/post";
//const send_data = "send_data";


/*****************GLOBAL INIT START**********************/
function OnStart()
{
  app.IsNewVersion();
  app.ExtractAssets();
  var ip = app.GetIPAddress(); 
  if( ip == "0.0.0.0" ) {  
    app.ShowPopup( "Please Enable WiFi" );  
	//app.Exit(); 
  }
  else if( ip == "192.168.5.19"){
      app.ShowPopup("Connected to SyCtrl_Mobile-Device");
  }
  else{
    app.ShowPopup("Connected to: " + ip + "\n" + "*no Device*");
  }  
   
  lay = app.CreateLayout( "Linear", "VCenter,FillXY" );
  lay.SetBackGradient("#ff000fff","#000ff000");
  lay.SetBackAlpha(0.2);

  // ENABLE steering button
  btn_switch = app.CreateToggle( "Enable App Steering", 0.4, 0.1);
  btn_switch.SetOnTouch(steering_enable_handler);
  btn_switch.SetChecked(false);
  btn_switch.SetMargins(0, -0.175, 0, 0 );
  
  // EMERGENCY button
  btnstop = app.CreateButton( "STOP",0.2,0.1 );
  btnstop.SetMargins( 0.35, -0.3, 0, 0);
  btnstop.SetOnTouch(stop_handler);
  btnstop.SetBackColor("#fff00000");
  btnstop.SetBackAlpha(0.5);
  
  // UP button
  btn = app.CreateButton( "UP",0.4,0.25 );
  btn.SetMargins( 0, 0, 0, 0.2); 
  btn.SetOnTouch( up_handler );
  //btn.SetBackColor("#00FFFF");
  //btn.SetBackAlpha(0.5); 
  
  // DOWN button
  btn2 = app.CreateButton( "DOWN",0.4,0.25 );
  btn2.SetMargins( 0, -0.1, 0, 0 ); 
  btn2.SetOnTouch( down_handler );
  
  // LEFT button
  btn3 = app.CreateButton( "LEFT",0.3,0.25 );
  btn3.SetMargins(0, -0.425, 0.35, 0 ); 
  btn3.SetEnabled(false);
  btn3.SetOnTouch( left_handler );
  btn3.SetOnLongTouch(left_handler_long);
  
  
  // RIGHT button
  btn4 = app.CreateButton( "RIGHT",0.3,0.25 );
  btn4.SetMargins(0.35, -0.25, 0, 0 ); 
  btn4.SetEnabled(false);
  btn4.SetOnTouch( right_handler );
  btn4.SetOnLongTouch(right_handler_long);

  
  txt = app.CreateText( "Gear", 0.8, 0.05, "Center,Multiline" ); 
  txt.SetMargins(0,-0.075,0.35,0);
  //txt.SetBackColor( "#ff222222" );  
  txt.SetTextSize( 25 ); 
  
  txt2 = app.CreateText( "", 0.3, 0.05, "Center,Singleline" ); 
  txt2.SetMargins(0,0.475,0.35,-0.525);
  txt2.SetTextSize( 25 ); 

  // add things to layout
  lay.AddChild( txt2 ); 
  lay.AddChild( btn );
  lay.AddChild( btn2 );
  lay.AddChild( btn3 );
  lay.AddChild( btn4 );
  lay.AddChild( btn_switch);
  lay.AddChild( btnstop );
  lay.AddChild( txt );
  
  app.AddLayout( lay );

}
/*****************GLOBAL INIT END**********************/
 
/****************ENABLE HANDLER START******************/ 
function steering_enable_handler(){ 
    txt2.SetText("\n\n");
    if (btn_switch.GetChecked()){
        btn4.SetEnabled(true);
        btn3.SetEnabled(true);
        server_post(http_post, "move",  "command", 1);
    } 
    else {
        btn3.SetEnabled(false);
        btn4.SetEnabled(false);
        txt2.SetText("\n\n");
        server_post(http_post, "move",  "command", 0);
    }
    
}
 
/****************ENABLE HANDLER END********************/  
 

/****************BUTTON HANDLER START******************/
function stop_handler(){
    app.Vibrate ("0,100,30,100,50,300");
    server_post(http_post, "move",  "command", -1);
    btn_switch.SetChecked(false);
    btn3.SetEnabled(false);
    btn4.SetEnabled(false);
    txt2.SetText("Reset" + "\n\n");
    
}

function up_handler()
{   
    server_post(http_post, "move", "throttle_gear", up_pin);
}

function down_handler()
{   
    server_post(http_post, "move", "throttle_gear", down_pin);
}

function left_handler()
{   
    server_post(http_post, "move", "steering", left_pin);    
}

function left_handler_long()
{   
    for(i = 0; i < 5; i++){
        server_post(http_post, "move", "steering", left_pin);
        app.Wait(0.1);
    }    
}

function right_handler()
{    
    server_post(http_post, "move", "steering", right_pin);
}

function right_handler_long()
{   
    for(i = 0; i < 5; i++){
        server_post(http_post, "move", "steering", right_pin); 
        app.Wait(0.1);
    }    
}
/*******************BUTTON HANDLER END*******************/

/*****************SERVER CONNECT START*********************/
function server_get(uri){
    
    var http_req = new XMLHttpRequest(); 
    http_req.onreadystatechange = function() { http_reply_handler(http_req); };  
    http_req.open("GET", "http://"+server_ip+server_port+uri, true); 
    http_req.send(null);
}

function server_post(uri, operation, type, data){
    
    var http_post = new XMLHttpRequest(); 
    http_post.onreadystatechange = function() {http_reply_handler(http_post) ; };  
    http_post.open("POST", "http://"+server_ip+server_port+uri, true);
    http_post.setRequestHeader("Content-type","application/json"); 

    var json_obj = {};
    json_obj[operation] = {};
    json_obj[operation][type] = {};
    json_obj[operation][type]["send_data"]  = data;
    http_post.send(JSON.stringify(json_obj));
    http_post.responseType = 'json';
}

function http_reply_handler(http_req){ 
    
    if( http_req.readyState==4 ) { 
        //If we got a valid response. 
        if( http_req.status==200 ){ 
            if (http_req.responseType === 'json') {
                var obj = http_req.response;
                if (obj.move.throttle_gear){
                    txt.SetText(obj.move.throttle_gear.send_data + "\n\n");
                }
                else if (obj.move.steering){
                    if(obj.move.steering.send_dir === 1){
                        txt2.SetText(obj.move.steering.send_data + "% L" + "\n\n");
                    }
                    else if (obj.move.steering.send_dir === 0){
                        txt2.SetText(obj.move.steering.send_data + "% R" + "\n\n");
                    }
                }
                else if (obj.move.command.send_data===-1){
                    txt.SetText(0 + "\n\n");
                }
                
               // else if(obj.move.command){
               //     if ((obj.move.command.send_data)===0){
               //         txt.SetText("App steering disbaled" + "\n\n");
               //     }
               //     else if ((obj.move.command.send_data)===1){
               //         txt.SetText("App steering enabled" + "\n\n");
               //     }
               // }
                else{
                    // no operation
                    txt.SetText("__NOP" + "\n\n");
                }
            }   
            else{
                txt.SetText( "No Response" + "\n\n" );  
            } 
        } 
        //An error occurred 
        else{
         // txt.SetText( "Error: Invalid Response");
        }
    } 
    app.HideProgress(); 
} 
/******************SERVER CONNECT END*********************/



