#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ArduinoJson.h>
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
 #include <avr/power.h>
#endif
#define ADD true
#define SUB false
boolean stat1 = true;  
int val = 0; 
float volume=0;
uint8_t a=0;
uint16_t WheelPos=0,times=0;
uint32_t color;
char Direction=0,Flag=0;
//#define NUMPIXELS 16//灯圈 
#define NUMPIXELS 16
//巴法云服务器地址默认即可
#define TCP_SERVER_ADDR "bemfa.com"
//服务器端口//TCP创客云端口8344//TCP设备云端口8340
#define TCP_SERVER_PORT "8344"

//********************需要修改的部分*******************//

//WIFI名称，区分大小写，不要写错
#define DEFAULT_STASSID  "Mi 10"
//WIFI密码
#define DEFAULT_STAPSW   "bisheng1579"
//用户私钥，可在控制台获取,修改为自己的UID
String UID = "22ce0563f226aaac9cf0a7db0ff7cef1";
//主题名字，可在控制台新建
String TOPIC = "LEDA";
String TOPICSTA = "LEDB";
//**************************************************//

//设置上传速率2s（1s<=upDataTime<=60s）
//下面的2代表上传间隔是2秒
#define upDataTime 2*1000
//最大字节数
#define MAX_PACKETSIZE 512
//设置心跳值30s
#define KEEPALIVEATIME 30*1000

struct DEL //JSON上传报告结构体
{
int R = 0;  
int G = 0; 
int B = 0; 
int LED =0;
}DEL;

struct GET //JSON获取控制结构体
{
int R = 0; 
int G = 0;
int B = 0; 
int XX = 0; 
int yy = 0; 
int SKY = 0; 
int TMP = 0; 
int MUSIC = 0; 
}GET;

struct RD //控制单元结构体
{
const int LED_AX = 5; 
const int LED_IN = A0; 
}RD;

//tcp客户端相关初始化，默认即可
WiFiClient TCPclient;
String TcpClient_Buff = "";
unsigned int TcpClient_BuffIndex = 0;
unsigned long TcpClient_preTick = 0;
unsigned long preHeartTick = 0;//心跳
unsigned long preTCPStartTick = 0;//连接
bool preTCPConnected = false;

Adafruit_NeoPixel pixels(NUMPIXELS, RD.LED_AX, NEO_GRB + NEO_KHZ800);

//相关函数初始化
void temps(int x);
void RUNA();
void RUNB();
void RUNC();
//连接WIFI
void doWiFiTick();
void startSTA();
void turnOnLed();
void turnOffLed();

//TCP初始化连接
void doTCPClientTick();
void startTCPClient();
void sendtoTCPServer(String p);

/*
  *发送数据到TCP服务器
 */
void sendtoTCPServer(String p){
  
  if (!TCPclient.connected()) 
  {
    Serial.println("Client is not readly");
    return;
  }
  TCPclient.print(p);
  Serial.println("[Send to TCPServer]:String");
  Serial.println(p);
}


/*
  *初始化和服务器建立连接
*/
void startTCPClient(){
  if(TCPclient.connect(TCP_SERVER_ADDR, atoi(TCP_SERVER_PORT))){
    Serial.print("\nConnected to server:");
    Serial.printf("%s:%d\r\n",TCP_SERVER_ADDR,atoi(TCP_SERVER_PORT));
    String tcpTemp = "";
    tcpTemp = "cmd=1&uid="+UID+"&topic="+TOPIC+"\r\n";

    sendtoTCPServer(tcpTemp);
    tcpTemp = "";
    preTCPConnected = true;
    preHeartTick = millis();
    TCPclient.setNoDelay(true);
  }
  else{
    Serial.print("Failed connected to server:");
    Serial.println(TCP_SERVER_ADDR);
    TCPclient.stop();
    preTCPConnected = false;
  }
  preTCPStartTick = millis();
}


/*
  *检查数据，发送心跳
*/
void doTCPClientTick(){
 //检查是否断开，断开后重连
   if(WiFi.status() != WL_CONNECTED) return;

  if (!TCPclient.connected()) {//断开重连

  if(preTCPConnected == true){

    preTCPConnected = false;
    preTCPStartTick = millis();
    Serial.println();
    Serial.println("TCP Client disconnected.");
    TCPclient.stop();
  }
  else if(millis() - preTCPStartTick > 1*1000)//重新连接
    startTCPClient();
  }
  else
  {
    if (TCPclient.available()) {//收数据
      char c =TCPclient.read();
      TcpClient_Buff +=c;
      TcpClient_BuffIndex++;
      TcpClient_preTick = millis();
      
      if(TcpClient_BuffIndex>=MAX_PACKETSIZE - 1){
        TcpClient_BuffIndex = MAX_PACKETSIZE-2;
        TcpClient_preTick = TcpClient_preTick - 200;
      }
      preHeartTick = millis();
    }
    if(millis() - preHeartTick >= KEEPALIVEATIME){//保持心跳
      preHeartTick = millis();
      Serial.println("--Keep alive:");
      sendtoTCPServer("cmd=0&msg=keep\r\n");
    }
  }
  if(millis() - preHeartTick >= upDataTime){//上传数据
      preHeartTick = millis();
/*****************获取数据*****************/

      /*********************数据上传*******************/
      /*
        数据用#号包裹，以便app分割出来数据，&msg=#23#80#on#\r\n，即#温度#湿度#按钮状态#，app端会根据#号分割字符串进行取值，以便显示
        如果上传的数据不止温湿度，可在#号后面继续添加&msg=#23#80#data1#data2#data3#data4#\r\n,app字符串分割的时候，要根据上传的数据进行分割
      */
      String upstr = "";
      upstr = "cmd=2&uid="+UID+"&topic="+TOPICSTA+"&msg=#"+DEL.R+"#"+DEL.G+"#"+DEL.B+"#"+"\r\n";
      sendtoTCPServer(upstr);
      upstr = "";
    }
  if((TcpClient_Buff.length() >= 1) && (millis() - TcpClient_preTick>=200))
  {//data ready
    TCPclient.flush();
    Serial.println("Buff");
    Serial.println(TcpClient_Buff);
    if((TcpClient_Buff.indexOf("&msg=on") > 0)) {
      turnOnLed();
    }else if((TcpClient_Buff.indexOf("&msg=off") > 0)) {
      turnOffLed();
    }else{

      int msgIndex = TcpClient_Buff.indexOf("msg=");//检测字符串中的msg=位置
      if(msgIndex>=0){
              String myjson = TcpClient_Buff.substring(msgIndex+4);//字符串截取，msg=位置后面第四位开始截取
              
              Serial.print("Get json:");
              Serial.println(myjson);//串口调试
              
              //json数据解析
              StaticJsonDocument<200> doc;
              DeserializationError error = deserializeJson(doc, myjson);
              if (error) {
                Serial.print(F("deserializeJson() failed: "));
                Serial.println(error.c_str());
              }
              JsonVariant LED_R = doc["LEDR"]; //获取解析值
              JsonVariant LED_G = doc["LEDG"];//获取解析值
              JsonVariant LED_B = doc["LEDB"];//获取解析值
              JsonVariant LED_XX = doc["xx"];//获取解析值
              JsonVariant LED_YY = doc["XS"];//获取解析值
              JsonVariant LED_SKY = doc["SKY"];//获取解析值
              JsonVariant LED_TMP = doc["TMP"];//获取解析值
              JsonVariant LED_MUSIC = doc["MUSIC"];//获取解析值

              GET.R = LED_R.as<int>();  //转换成int类型
              GET.G = LED_G.as<int>(); //转换成int类型
              GET.B = LED_B.as<int>(); //转换成int类型
              GET.XX = LED_XX.as<int>(); //转换成int类型
              GET.yy = LED_YY.as<int>(); //转换成int类型
              GET.SKY = LED_SKY.as<int>(); //转换成int类型
              GET.TMP = LED_TMP.as<int>(); //转换成int类型
              GET.MUSIC = LED_MUSIC.as<int>(); //转换成int类型
              
              Serial.print("Get R:"); //串口打印
              Serial.println(GET.R);//串口打印
              Serial.print("Get G:");//串口打印
              Serial.println(GET.G);//串口打印
              Serial.print("Get B:");//串口打印
              Serial.println(GET.B);//串口打印
              Serial.print("Get XX:");//串口打印
              Serial.println(GET.XX);//串口打印
              Serial.print("Get yy:");//串口打印
              Serial.println(GET.yy);//串口打印
              Serial.print("Get SKY:");//串口打印
              Serial.println(GET.SKY);//串口打印
              Serial.print("Get TMP:");//串口打印
              Serial.println(GET.TMP);//串口打印
              Serial.print("Get MUSIC:");//串口打印
              Serial.println(GET.MUSIC);//串口打印
        }
        if(DEL.LED==1){
          if(GET.XX==0){
          RUNB();
          DEL.R=GET.R;
          DEL.G=GET.G;
          DEL.B=GET.B;
          }
          else{
          RUNA();
          DEL.R=GET.R;
          DEL.G=GET.G;
          DEL.B=GET.B;
            }
          }
         else if(GET.SKY==1){
          temps(GET.TMP);
          }
          /*
         else if(GET.MUSIC==1){
             for(int i;i<10000;i++){
      RUNC();
      delay(5);}
            }
            */
      }
   TcpClient_Buff="";
   TcpClient_BuffIndex = 0;
  }
}

void temps(int x){
  if(x<5){
     pixels.clear();
  for(int i=0; i<NUMPIXELS; i++) {
    pixels.setPixelColor(i, pixels.Color(53, 2, 255));
    pixels.show(); 
}
    }
  else if(x<10&&x>=5){
     pixels.clear();
  for(int i=0; i<NUMPIXELS; i++) {
    pixels.setPixelColor(i, pixels.Color(2, 255, 239));
    pixels.show(); 
}
    }
      else if(x<20&&x>=10){
     pixels.clear();
  for(int i=0; i<NUMPIXELS; i++) {
    pixels.setPixelColor(i, pixels.Color(2, 255, 12));
    pixels.show(); 
}
    }
      else if(x<30&&x>=20){
     pixels.clear();
  for(int i=0; i<NUMPIXELS; i++) {
    pixels.setPixelColor(i, pixels.Color(83, 255, 2));
    pixels.show(); 
}
    }
      else if(x<40&&x>=30){
     pixels.clear();
  for(int i=0; i<NUMPIXELS; i++) {
    pixels.setPixelColor(i, pixels.Color(255,123,2));
    pixels.show(); 
}
    }
      else if(x>=40){
     pixels.clear();
  for(int i=0; i<NUMPIXELS; i++) {
    pixels.setPixelColor(i, pixels.Color(255, 2, 2));
    pixels.show(); 
}
    }
  }
void startSTA(){
  WiFi.disconnect();
  WiFi.mode(WIFI_STA);
  WiFi.begin(DEFAULT_STASSID, DEFAULT_STAPSW);
//WiFi.beginSmartConfig();
}


/**************************************************************************
                                 WIFI
***************************************************************************/
/*
  WiFiTick
  检查是否需要初始化WiFi
  检查WiFi是否连接上，若连接成功启动TCP Client
  控制指示灯
*/
void doWiFiTick(){
  static bool startSTAFlag = false;
  static bool taskStarted = false;
  static uint32_t lastWiFiCheckTick = 0;

  if (!startSTAFlag) {
    startSTAFlag = true;
    startSTA();
    Serial.printf("Heap size:%d\r\n", ESP.getFreeHeap());
  }

  //未连接1s重连
  if ( WiFi.status() != WL_CONNECTED ) {
    if (millis() - lastWiFiCheckTick > 1000) {
      lastWiFiCheckTick = millis();
    }
  }
  //连接成功建立
  else {
    if (taskStarted == false) {
      taskStarted = true;
      Serial.print("\r\nGet IP Address: ");
      Serial.println(WiFi.localIP());
      startTCPClient();
    }
  }
}
//打开灯泡
void turnOnLed(){
  Serial.println("Turn ON");
   pixels.clear();
  for(int i=0; i<NUMPIXELS; i++) {
    pixels.setPixelColor(i, pixels.Color(GET.R, GET.G, GET.B));
    pixels.show(); 
}
DEL.R=GET.R;
DEL.G=GET.G;
DEL.B=GET.B;
DEL.LED=1;
}
//关闭灯泡
void turnOffLed(){
  Serial.println("Turn OFF");
      pixels.clear();
  for(int i=0; i<NUMPIXELS; i++) {
    pixels.setPixelColor(i, pixels.Color(0, 0, 0));
    pixels.show(); 
}
DEL.LED=0;
DEL.R=0;
DEL.G=0;
DEL.B=0;
}
  /************************************************************************/

void RUNA(){//依次点亮
   pixels.clear();
  for(int i=0; i<NUMPIXELS; i++) {
    pixels.setPixelColor(i, pixels.Color(GET.R, GET.G, GET.B));
    pixels.show(); 
    delay(GET.yy); 
  }
  }

void RUNB(){//点亮
   pixels.clear();
  for(int i=0; i<NUMPIXELS; i++) {
    pixels.setPixelColor(i, pixels.Color(GET.R, GET.G, GET.B));
    pixels.show(); 
  }
  }

void RUNC(){//音乐
MusicCllect ();                         //调用音乐采集函数;
ColorSelect();                          //调用颜色选择函数;

if(volume<3)                            //判断声音大小;
  {
   times++;
   if(times>100) times=51;
   Silent();                            //调用静默函数;
  }
else  
{
   times=0;
   LitPixel();                         //调用点亮LED灯条函数;
}
}

  void MusicCllect ()                    //音乐采集函数;
{
  volume=analogRead(RD.LED_IN);     //读取音乐传感器的模拟值;
  volume=(volume/10);                 //将读取的值缩小10倍;

  if(volume>66+1)
     volume=volume-66;
  else if(volume<59)
     volume=59-volume;
  else volume=0;
  
  if(volume>=59) volume=59;         //限制音量的最大值;
  else if(volume<=0) volume=0;      //限制音量最小值;
  
  if(NUMPIXELS>=50) volume=volume*1.5;
  if(NUMPIXELS>=100) volume=volume*2;
  if(NUMPIXELS>=200) volume=volume*4;
  
}

void LitPixel()                      //点亮LED灯条函数;
{
    u8 i;                          //灯条个数变量;
    switch(Flag)                     //switch选择判断语句,Flag为标志位用;
    {
      case 1:
        for(i=0;i<NUMPIXELS;i++)           //以下函数所执行的是先发送数据使LED灯全关，防止上一次数据干扰;
        {
          pixels.setPixelColor(i,0);   
        }
        /*以下函数是灯条花样的一种算法,因为如果不用这种算法直接用采集
        的数来点亮LED的个数会很突然，没有一种过渡的感觉，所以用以下
        算法进行一种平稳的过渡，让采集的音量来点亮LED更具有节奏感*/
        if(a<volume)  a++;     
        else if(a>volume) a=a-1.5;
        
        if(a>=NUMPIXELS) a=NUMPIXELS;
        else if(a<=0) a=0;
        
         for(i=0;i<a;i++)            //花样效果哦，有兴趣的好好想想呗。
        {
          pixels.setPixelColor(i,100,100,100);
        }   
        for(i=0;i<a-1;i++)            //将经过处理的数赋值给a，来控制LED灯点亮的个数;
        {
          pixels.setPixelColor(i,color);
        }
        
         pixels.show();               //将点亮的LED显示出来;
        break;  

        /*以下代码是与上面花样效果方向相反，具体是使所有LED点亮，
        然后使总的点亮个数减采集个数的LED灯变灭这样就会出现与上
        面方向相反的效果*/
     case 0:                    
       for(i=0;i<NUMPIXELS;i++)            
       {
         pixels.setPixelColor(i,color);
       }   
       if(a<volume)  a++;
       else if(a>volume) a=a-1.5;
      
       if(a>=NUMPIXELS) a=NUMPIXELS;
       else if(a<=0) a=0;
      
       for(i=0;i<NUMPIXELS-a+1;i++)            
       {
         pixels.setPixelColor(i,100,100,100);
       }   
      
       for(i=0;i<NUMPIXELS-a;i++)
       {
         pixels.setPixelColor(i,0);
       }
        pixels.show();
       break;
         
    case 2:            
      for(i=0;i<NUMPIXELS/2;i++)            
       {
         pixels.setPixelColor(i,color);
       }   
       if(a<volume)  a++;
       else if(a>volume) a=a-1.5;
      
       if(a>=NUMPIXELS) a=NUMPIXELS;
       else if(a<=0) a=0;
      
      for(i=0;i<(NUMPIXELS-a)/2+1;i++)            
       {
         pixels.setPixelColor(i,100,100,100);
       }   
      for(i=0;i<(NUMPIXELS-a)/2;i++)
       {
         pixels.setPixelColor(i,0);
       }
      
      for(i=NUMPIXELS/2;i<NUMPIXELS;i++)           //以下函数所执行的是先发送数据使LED灯全关，防止上一次数据干扰;
        {
          pixels.setPixelColor(i,0);   
        }
    /*    以下函数是灯条花样的一种算法,因为如果不用这种算法直接用采集
        的数来点亮LED的个数会很突然，没有一种过渡的感觉，所以用以下
        算法进行一种平稳的过渡，让采集的音量来点亮LED更具有节奏感*/
        if(a<volume)  a++;     
        else if(a>volume) a=a-1.5;
        
        if(a>=NUMPIXELS) a=NUMPIXELS;
        else if(a<=0) a=0;
        
        for(i=NUMPIXELS/2;i<NUMPIXELS/2+a/2;i++)            //花样效果哦，有兴趣的好好想想呗。
        {
          pixels.setPixelColor(i,100,100,100);
        }   
        for(i=NUMPIXELS/2;i<NUMPIXELS/2+a/2-1;i++)            //将经过处理的数赋值给a，来控制LED灯点亮的个数;
        {
          pixels.setPixelColor(i,color);
        }
        pixels.show();
       break;  
    }
}

/*该函数主要是对点亮LED的颜色进行改变严格的来说该LED
支持256颜色变换，以下为其中颜色的几种其主要是通过内部
的一个变量进行计数当满足某个条件时点亮哪种颜色*/
void ColorSelect()
{
  if(WheelPos<200)
    color=pixels.Color(80, 0, 0);
  if(WheelPos>=200&&WheelPos<400)
    color=pixels.Color(80, 80, 0);
  if(WheelPos>=400&&WheelPos<600)
    color=pixels.Color(0, 80, 0);
  if(WheelPos>=600&&WheelPos<800)
    color=pixels.Color(0,80,80);
  if(WheelPos>=800&&WheelPos<1000)
    color=pixels.Color(0,0,80);
  if(WheelPos>=1000&&WheelPos<1200)
    color=pixels.Color(0,0,80);
  WheelPos++;
  if(WheelPos==1200)     //当计数达到1200时执行以下语句;
  {
    WheelPos=0;
    Direction++;
    if(Direction==4) Flag=1;       //Direction为方向选择变量，但计数到4时控制方向标志位置为1;
    else if(Direction==8)  Flag=2;   //Direction为方向选择变量，但计数到8时控制方向标志位置为2;
    else if(Direction==12)         //当Direction计到12是清零，并控制标志位置为0;
    {
      Flag=0;
      Direction=0;
    }
}     
}

/*该函数主要是检测声音的大小，当音乐声音很小或者没有时进入静默状态*/
void Silent()
{
  uint8_t i,b;
  uint16_t SceneVolume;   //场景音量变量;
  SceneVolume=volume;    //将采集音量赋值给的场景音量；
  if(times>50)         //内部进行计数放音量低于3是且times大于200时进入内部语句;
  {
    ClearColor();        //清除灯条颜色，防止干扰;
    if(SceneVolume<3)     //再对场景音量进行判断是否确实小于3；小于执行内部语句;
    {
     color=pixels.Color(50, 0, 0);//color颜色赋值为静默状态时的颜色，该颜色也可执行调节，只需改变strip.Color(50, 0, 0);内部的3个参数值就可以，但参数值不能超过255.  
     b=random(4,NUMPIXELS+1);      //用到了随机函数，其随机的值为4-40之间;
     for(i=0;i<b;i++)          //首先将随机到的LED数值全部赋值颜色;
     {
       pixels.setPixelColor(i,color);   
     }  
     for(i=0;i<b-4;i++)   //再把随机数的最后面4个数，前面的数颜色全都关掉;
     {
       pixels.setPixelColor(i,0);
     }
     pixels.show();        //这样就会出现随机的4个LED灯点亮的效果了;
     if(SceneVolume<3) delay(1500); //该语句主要作用是防止灯闪一下就灭的情况；同时判断场景声音是否小于3;
     }
   }
}

void ClearColor()         //LED灯条清屏操作，防止有其他颜色干扰;
{
    uint8_t i;
     for(i=0;i<NUMPIXELS;i++)
     {
        pixels.setPixelColor(i,0);
     }
     pixels.show();  
}
/**************************************************************************/
void setup() {
      Serial.begin(115200);
#if defined(__AVR_ATtiny85__) && (F_CPU == 16000000)
  clock_prescale_set(clock_div_1);
#endif
  pinMode(RD.LED_IN,INPUT);             //设置端口为输入模式;
  pixels.begin();
  pixels.show();                           //初始化LED灯条输出状态
}

void loop() {
   doWiFiTick();
  doTCPClientTick();
}
