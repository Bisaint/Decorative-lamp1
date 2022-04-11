//index.js
//获取应用实例
var app = getApp()
var day = ["今天", "明天", "后天"];
Page({
  data: {
    uid: '22ce0563f226aaac9cf0a7db0ff7cef1', //用户密钥，控制台获取
    STAA: "LEDA",//主题名称 
    TOPICSTA:"LEDB",
    A_status: "离线", //默认离线，自定义变量名称，随意命名
    //初始化数据
    hideNotice: false,
    day: day,
    tmp:'0',
    sky:'1',
    MUSIC:'0',
  },

  onLoad: function () {
    var that = this
    that.getLocation();
    wx.request({
      url: 'https://api.bemfa.com/api/device/v1/data/1/', //api接口，详见接入文档
      method:"POST",
      data: {  //请求字段，详见巴法云接入文档，http接口
        uid: that.data.uid,
        topic: that.data.STAA,
        msg:"{SKY:"+that.data.sky+",TMP:"+that.data.tmp+",MUSIC:"+that.data.MUSIC+"}",   //发送消息为on的消息
      },
      header: {
        'content-type': "application/x-www-form-urlencoded"
      },
      success (res) {
        console.log(res.data)
        wx.showToast({
          title:'上传成功',
          icon:'success',
          duration:1000
        })
      }
    })
  },

  //获取经纬度方法
  getLocation: function () {
    var that = this
    wx.getLocation({
      type: 'wgs84',
      success: function (res) {
        var latitude = res.latitude
        var longitude = res.longitude
        that.getCity(latitude, longitude);
      }
    })
  },
  //获取城市信息
  getCity: function (latitude, longitude) {
    var that = this
    var url = "https://api.map.baidu.com/reverse_geocoding/v3/";
    var params = {
      ak: "1ziTYEE2PZaGZwPhQGd11dMdAcM03cs1",

      output: "json",
      location: latitude + "," + longitude
    }
    wx.request({
      url: url,
      data: params,
      success: function (res) {
        var city = res.data.result.addressComponent.city;
        var district = res.data.result.addressComponent.district;
        var street = res.data.result.addressComponent.street;

        that.setData({
          city: city,
          district: district,
          street: street,
        })

        var descCity = city.substring(0, city.length - 1);
        that.getWeahter(descCity);
      },
      fail: function (res) { },
      complete: function (res) { },
    })
  },

  //获取常规天气信息
  getWeahter: function (city) {
    var that = this
    var url = "https://free-api.heweather.net/s6/weather"
    var params = {
      location: city,
      key: "5112f7792a1b4e4d9da940c49f643d70"
    }
    wx.request({
      url: url,
      data: params,
      success: (res) => { 
        var tmp = res.data.HeWeather6[0].now.tmp;
        var txt = res.data.HeWeather6[0].now.cond_txt;
        var code = res.data.HeWeather6[0].now.cond_code;
        var vis = res.data.HeWeather6[0].now.vis;
        var dir = res.data.HeWeather6[0].now.wind_dir;
        var sc = res.data.HeWeather6[0].now.wind_sc;
        var hum = res.data.HeWeather6[0].now.hum;
        var fl = res.data.HeWeather6[0].now.fl;
        var notice = res.data.HeWeather6[0].lifestyle[2].txt;
        var daily_forecast = res.data.HeWeather6[0].daily_forecast;
        this.setData({
          tmp: tmp,
          txt: txt,
          code: code,
          vis: vis,
          dir: dir,
          sc: sc,
          hum: hum,
          fl: fl,
          daily_forecast: daily_forecast,
          notice: notice
        })
        that.getWeahterAir(city);
      },
      fail: function (res) { },
      complete: function (res) { },
    })
  },
  //获取空气质量
  getWeahterAir: function (city) {
    var that = this
    var url = "https://free-api.heweather.net/s6/air"
    var params = {
      location: city,
      key: "5112f7792a1b4e4d9da940c49f643d70"
    }
    wx.request({
      url: url,
      data: params,
      success: function (res) {
        var qlty = res.data.HeWeather6[0].air_now_city.qlty;
        that.setData({
          qlty: qlty,
        })
      },
      fail: function (res) { },
      complete: function (res) { },
    })
  },
  //下拉刷新
  onPullDownRefresh: function () {
    var that = this
    that.getLocation();
    //下拉刷新后回弹
    wx.stopPullDownRefresh();
    wx.request({
      url: 'https://api.bemfa.com/api/device/v1/data/1/', //api接口，详见接入文档
      method:"POST",
      data: {  //请求字段，详见巴法云接入文档，http接口
        uid: that.data.uid,
        topic: that.data.STAA,
        msg:"{SKY:"+that.data.sky+",TMP:"+that.data.tmp+",MUSIC:"+that.data.MUSIC+"}",   //发送消息为on的消息
      },
      header: {
        'content-type': "application/x-www-form-urlencoded"
      },
      success (res) {
        console.log(res.data)
        wx.showToast({
          title:'上传成功',
          icon:'success',
          duration:1000
        })
      }
    })
  },

  // 点击关闭公告
  /*switchNotice: function () {
    this.setData({
      hideNotice: true
    })
  },*/
}
)