// pages/index/index.js
Page({

  data: {
    uid: '22ce0563f226aaac9cf0a7db0ff7cef1', //用户密钥，控制台获取
    STAA: "LEDA",//主题名称 
    TOPICSTA:"LEDB",
    A_status: "离线", //默认离线，自定义变量名称，随意命名
    sky:'0',
    music:'1',
  },
    AS() {
    wx.navigateTo({
      url: '/pages/index/A'
    });
  },
  BS() {
    wx.navigateTo({
      url: '/pages/index/B'
    });
  },
  MUSIC: function() {

    //当点击打开按钮，更新开关状态为打开
    var that = this

    //控制接口
    wx.request({
      url: 'https://api.bemfa.com/api/device/v1/data/1/', //api接口，详见接入文档
      method:"POST",
      data: {  //请求字段，详见巴法云接入文档，http接口
        uid: that.data.uid,
        topic: that.data.STAA,
        msg:"{SKY:"+that.data.sky+",TMP:"+'0'+",MUSIC:"+that.data.music+"}",   //发送消息为on的消息
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
})