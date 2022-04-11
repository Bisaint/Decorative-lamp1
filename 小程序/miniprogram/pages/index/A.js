const app = getApp()
//消息用#号做了包裹，方便字符串分割，不过自己要知道哪个数是表示什么意思就行，可以用其他符号隔开，这里用了#号，不局限，自己设定就行
//假设控制台有消息：#26#99#47.5#27#17#on#   //获取的是控制台消息，故控制台要有传感器数据
Page({
  data: {
    uid: '22ce0563f226aaac9cf0a7db0ff7cef1', //用户密钥，控制台获取
    STAA: "LEDA",//主题名称 
    TOPICSTA:"LEDB",
    A_status: "离线", //默认离线，自定义变量名称，随意命名
    rgb: 'rgb(0,0,0)',
    XXA: 'rgb(100,100,100)',
    XS:'',
    RGBR: '0',
    RGBG: '0',
    RGBB: '0',
    rgbx:'0',
    pick: false,
    itemsX: [
      { name: '依次', value: '1' },
      { name: '长亮', value: '0', checked: 'true' },
    ],
    radioStrX: '0',
  },
  AS: function(res){
    console.log("输入的值为："+res.detail.value);//打印输入的值
    this.setData({
      XS: res.detail.value//赋值给name_value
    })
  },
  radioChangeX: function (e) {
    var str = null;
    for (var value of this.data.itemsX) {
      if (value.value === e.detail.value) {
        str = value.value;
        break;
      }
    }
    this.setData({ radioStrX: str });
  },
  toPick: function () {
    this.setData({
      pick: true
    })
  },
  pickColor(e) {
    this.setData({
      rgb:e.detail.color,
      RGBR:e.detail.color.split(',')[0].split('(')[1],
      RGBG:e.detail.color.split(',')[1],
      RGBB:e.detail.color.split(',')[2].split(')')[0],
    })
  },
  OPEN: function() {

    //当点击打开按钮，更新开关状态为打开
    var that = this

    //控制接口
    wx.request({
      url: 'https://api.bemfa.com/api/device/v1/data/1/', //api接口，详见接入文档
      method:"POST",
      data: {  //请求字段，详见巴法云接入文档，http接口
        uid: that.data.uid,
        topic: that.data.STAA,
        msg:"on",   //发送消息为on的消息
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
  OFF: function() {

    //当点击打开按钮，更新开关状态为打开
    var that = this

    //控制接口
    wx.request({
      url: 'https://api.bemfa.com/api/device/v1/data/1/', //api接口，详见接入文档
      method:"POST",
      data: {  //请求字段，详见巴法云接入文档，http接口
        uid: that.data.uid,
        topic: that.data.STAA,
        msg:"off",   //发送消息为on的消息
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
  UPDAA: function() {

    //当点击打开按钮，更新开关状态为打开
  var that = this

      //控制接口
      wx.request({
        url: 'https://api.bemfa.com/api/device/v1/data/1/', //api接口，详见接入文档
        method:"POST",
        data: {  //请求字段，详见巴法云接入文档，http接口
          uid: that.data.uid,
          topic: that.data.STAA,
          msg:"{LEDR:"+that.data.RGBR+",LEDG:"+that.data.RGBG+",LEDB:"+that.data.RGBB+",xx:"+that.data.radioStrX+",XS:"+that.data.XS+"}",   //发送消息为on的消息
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
  handlerGobackClick() {
    wx.showModal({
      title: '你点击了返回',
      content: '是否确认放回',
      success: e => {
        if (e.confirm) {
          const pages = getCurrentPages();
          if (pages.length >= 2) {
            wx.navigateBack({
              delta: 1
            });
          } else {
            wx.navigateTo({
              url: '/pages/index/index'
            });
          }
        }
      }
    });
  },
  handlerGohomeClick() {
    wx.reLaunch({
      url: '/pages/index/index'
    });
  },
  onLoad: function () {
    var that = this
    //请求设备状态
    //设备断开不会立即显示离线，由于网络情况的复杂性，离线1分钟左右才判断真离线
    wx.request({
      url: 'https://api.bemfa.com/api/device/v1/status/', //状态api接口，详见巴法云接入文档
      data: {
        uid: that.data.uid,
        topic: that.data.STAA,
      },
      header: {
        'content-type': "application/x-www-form-urlencoded"
      },
      success (res) {
        console.log(res.data)
        if(res.data.status === "online"){
          that.setData({
            A_status:"在线",
          })
        }else{
          that.setData({
            A_status:"离线",
          })
        }
        console.log(that.data.A_status)
      }
    })
    wx.request({
      url: 'https://api.bemfa.com/api/device/v1/data/1/', //get接口，详见巴法云接入文档
      data: {
        uid: that.data.uid,
        topic: that.data.TOPICSTA,
      },
      header: {
        'content-type': "application/x-www-form-urlencoded"
      },
      success (res) {
          console.log(res.data)
          if(res.data.msg.indexOf("#") != -1){//如果数据里包含#号，表示获取的是传感器值，因为单片机上传数据的时候用#号进行了包裹
            //如果有#号就进行字符串分割
            var all_data_arr = res.data.msg.split("#"); //分割数据，并把分割后的数据放到数组里。
            console.log(all_data_arr)//打印数组
            that.setData({ //数据赋值给变量
              XXA:'rgb('+all_data_arr[1]+','+all_data_arr[2]+','+all_data_arr[3]+')',//rgb
            })
          }

      }
    })
    setInterval(function () {
      console.log("定时请求设备状态,默认1秒");
      wx.request({
        url: 'https://api.bemfa.com/api/device/v1/status/',  //get 设备状态接口，详见巴法云接入文档
        data: {
          uid: that.data.uid,
          topic: that.data.STAA,
        },
        header: {
          'content-type': "application/x-www-form-urlencoded"
        },
        success (res) {
          console.log(res.data)
          if(res.data.status === "online"){
            that.setData({
              A_status:"在线"
            })
          }else{
            that.setData({
              A_status:"离线"
            })
          }
          console.log(that.data.A_status)
        }
      })
      wx.request({
        url: 'https://api.bemfa.com/api/device/v1/data/1/', //get接口，详见巴法云接入文档
        data: {
          uid: that.data.uid,
          topic: that.data.TOPICSTA,
        },
        header: {
          'content-type': "application/x-www-form-urlencoded"
        },
        success (res) {
            console.log(res.data)
            if(res.data.msg.indexOf("#") != -1){//如果数据里包含#号，表示获取的是传感器值，因为单片机上传数据的时候用#号进行了包裹
              //如果有#号就进行字符串分割
              var all_data_arr = res.data.msg.split("#"); //分割数据，并把分割后的数据放到数组里。
              console.log(all_data_arr)//打印数组
              that.setData({ //数据赋值给变量
                XXA:'rgb('+all_data_arr[1]+','+all_data_arr[2]+','+all_data_arr[3]+')',//rgb
              })
            }
  
        }
      })
    }, 1000)
  }


})