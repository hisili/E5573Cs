#CONFIG_EXT_CLK_26M             //是否支持26m时钟晶振
#CONFIG_PHY_EAT_40MHZ           //是否支持40m时钟晶振
#CONFIG_RTL_88E_SUPPORT         //RTL8189芯片需要配置为y
#CONFIG_RTL_92E_SUPPORT         //RTL8192芯片需要配置为y
#WIFI_MODULE_NAME               //wifi 驱动的模块名称

#CONFIG_RTL_FLOW_CTRL           //流控处理
#CFG_XMIT_SKB_QUEUE             //采用xmit采用 skb 队列发送的方式降低对源任务的依赖，提升性能，默认不开启
#CONFIG_TCP_ACK_TXAGG			//性能优化，TX TCP ACK聚合，默认不开启
#CONFIG_TCP_ACK_MERGE           //性能优化，TCP ACK多个合成一个，默认不开启
#CONFIG_AUTOCH_TIMER            //支持动态自动信道选择特性，默认不开启
#CFG_RTL_SDIO30                 //支持SDIO3.0特性
#SDIO_AP_PS                     //支持低功耗模式(即，支持WiFi唤醒BB，带电池产品待机省电功能)
#SOFTAP_PS_DURATION             //为realtek降低wifi低功耗模式下wifi功耗的宏控，目前针对realtek 8192es芯片
#INTEL_BEACON_POWER_NO_INC      //关闭针对intel网卡兼容性远场提升power的处理
#WLAN_PLATFORM_POWER_EXT_LNA    //支持外挂LNA
#CONFIG_SLOT_0_EXT_LNA          //支持外挂LNA的宏，E5770s项目和E5573-156项目此宏打开
#WLAN_SET_RSSI_MIB              //设置realtek mib特性宏

#WLAN_PLATFORM_BALONG_V3        //巴龙V3平台特性宏
#WLAN_PLATFORM_BALONG_V7        //巴龙V7平台特性宏
#WLAN_PLATFORM_QUALCOMM_9x15    //高通9x15平台特性宏
#WLAN_PLATFORM_HUAWEI_COMMON    //华为产品需要开启此特性
#WLAN_PLATFORM_HUAWEI_FACTORY   //烧片版本需要定义此特性

#WLAN_EVENT_WPA                 //修改wpa加密时，sta4次握手连接成功后，再上报sta连接个数事件
#RTL_CHANEL_COPY_FORM_BCM       //适配rtl信道国家码为博通
