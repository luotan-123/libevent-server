-- ----------------------------
-- Records of web_admin_member
-- ----------------------------
INSERT INTO `web_admin_member` VALUES ('1', 'adminser', 'boss', 'e10adc3949ba59abbe56e057f20f883e', '15180092940', '0', '0', '443', '1533004228', '192.168.1.73', '0','1');

-- ----------------------------
-- Records of web_admin_menu
-- ----------------------------
INSERT INTO `web_admin_menu` VALUES ('1', '系统管理', '', '0', '0', '1', 'fa fa-cogs');
INSERT INTO `web_admin_menu` VALUES ('2', '用户分组', 'Adminsystem/user_group', '0', '1', '1', '');
INSERT INTO `web_admin_menu` VALUES ('3', '菜单管理', 'Adminsystem/menu_config', '0', '1', '0', '');
INSERT INTO `web_admin_menu` VALUES ('9', '玩家管理', '', '0', '0', '4', 'fa fa-user');
INSERT INTO `web_admin_menu` VALUES ('11', '机器人管理', 'User/vbot_list', '1', '9', '1', '');
INSERT INTO `web_admin_menu` VALUES ('12', '银行操作记录', 'Hall/bank', '0', '129', '1', '');
INSERT INTO `web_admin_menu` VALUES ('13', '用户限制列表', 'User/limit_ip', '0', '9', '3', '');
INSERT INTO `web_admin_menu` VALUES ('14', '在线玩家列表', 'User/online_list', '1', '9', '4', '');
INSERT INTO `web_admin_menu` VALUES ('15', '代理管理', '', '0', '0', '9', 'fa fa-user-secret');
INSERT INTO `web_admin_menu` VALUES ('16', '代理列表', 'Agent/member_list', '0', '15', '1', '');
INSERT INTO `web_admin_menu` VALUES ('17', '商城管理', '', '0', '0', '5', 'fa fa-shopping-cart');
INSERT INTO `web_admin_menu` VALUES ('18', '大厅功能管理', '', '0', '0', '7', 'fa fa-desktop');
INSERT INTO `web_admin_menu` VALUES ('19', '开发人员选项', '', '0', '0', '2', 'fa fa-gamepad');
INSERT INTO `web_admin_menu` VALUES ('20', '游戏管理', '', '0', '0', '6', 'fa fa-rocket');
INSERT INTO `web_admin_menu` VALUES ('21', '网站管理', '', '0', '0', '2', 'fa fa-internet-explorer');
INSERT INTO `web_admin_menu` VALUES ('22', '代理权限控制', 'Agent/member_rules', '1', '15', '1', '');
INSERT INTO `web_admin_menu` VALUES ('23', '充值账单明细', 'Agent/bill_detail', '1', '17', '2', '');
INSERT INTO `web_admin_menu` VALUES ('24', '代理信息统计', 'Agent/info_count', '0', '15', '3', '');
INSERT INTO `web_admin_menu` VALUES ('25', '代理申请提现记录', 'Agent/apply_pos', '0', '15', '3', '');
INSERT INTO `web_admin_menu` VALUES ('26', '抽奖', 'Hall/turntable', '0', '18', '5', '');
INSERT INTO `web_admin_menu` VALUES ('27', '签到', 'Hall/sign', '0', '18', '1', '');
INSERT INTO `web_admin_menu` VALUES ('28', '邮件管理', 'Hall/email', '0', '127', '2', '');
INSERT INTO `web_admin_menu` VALUES ('29', '公告管理', 'Hall/notice', '0', '127', '3', '');
INSERT INTO `web_admin_menu` VALUES ('30', '反馈管理', 'Hall/feedback', '0', '18', '7', '');
INSERT INTO `web_admin_menu` VALUES ('31', '救济金', 'Hall/alms', '0', '18', '3', '');
INSERT INTO `web_admin_menu` VALUES ('32', '转赠', 'Hall/given', '0', '18', '4', '');
INSERT INTO `web_admin_menu` VALUES ('34', '实物兑换管理', 'Hall/convert', '1', '18', '8', '');
INSERT INTO `web_admin_menu` VALUES ('35', '世界广播', 'Hall/radio', '0', '18', '6', '');
INSERT INTO `web_admin_menu` VALUES ('36', '跳转和游戏参数管理', 'Hall/web_page', '0', '19', '4', '');
INSERT INTO `web_admin_menu` VALUES ('37', '魔法表情管理', 'Game/magic', '1', '19', '0', '');
INSERT INTO `web_admin_menu` VALUES ('38', '房间列表配置', 'Game/room_list_config', '0', '20', '3', '');
INSERT INTO `web_admin_menu` VALUES ('39', '游戏列表配置', 'Game/game_list_config', '0', '20', '2', '');
INSERT INTO `web_admin_menu` VALUES ('40', '版本控制', 'Game/version', '0', '19', '2', '');
INSERT INTO `web_admin_menu` VALUES ('41', '充值分润列表', 'Mall/recharge_record', '0', '17', '4', '');
INSERT INTO `web_admin_menu` VALUES ('42', '充值统计', 'Mall/data_count', '0', '17', '3', '');
INSERT INTO `web_admin_menu` VALUES ('43', '商城商品管理', 'Mall/goods', '0', '17', '1', '');
INSERT INTO `web_admin_menu` VALUES ('44', '平台数据记录', 'Operate/plat_data_record', '1', '124', '0', '');
INSERT INTO `web_admin_menu` VALUES ('45', '实物兑换统计', 'Operate/convert_record', '1', '124', '1', '');
INSERT INTO `web_admin_menu` VALUES ('46', '注册下载量统计', 'Operate/register_download_record', '0', '124', '1', '');
INSERT INTO `web_admin_menu` VALUES ('47', '用户充值统计', 'Operate/user_recharge_count', '1', '124', '3', '');
INSERT INTO `web_admin_menu` VALUES ('48', '在线用户统计', 'Operate/online_count', '0', '124', '2', '');
INSERT INTO `web_admin_menu` VALUES ('49', '活跃玩家和流失玩家', 'Operate/active_loss_user', '0', '124', '3', '');
INSERT INTO `web_admin_menu` VALUES ('51', '签到统计', 'Operate/sign_count', '1', '124', '7', '');
INSERT INTO `web_admin_menu` VALUES ('52', '领取救济金统计', 'Operate/alms_count', '1', '124', '8', '');
INSERT INTO `web_admin_menu` VALUES ('53', '抽奖统计', 'Operate/turntable_count', '1', '124', '9', '');
INSERT INTO `web_admin_menu` VALUES ('54', '反馈统计', 'Operate/feedback_count', '1', '124', '10', '');
INSERT INTO `web_admin_menu` VALUES ('55', '转赠统计', 'Operate/given_count', '1', '124', '11', '');
INSERT INTO `web_admin_menu` VALUES ('56', '世界广播统计', 'Operate/radio_count', '1', '124', '12', '');
INSERT INTO `web_admin_menu` VALUES ('57', '分享统计', 'Operate/share_count', '1', '124', '13', '');
INSERT INTO `web_admin_menu` VALUES ('58', '游戏输赢统计', 'Operate/game_record_info', '1', '124', '14', '');
INSERT INTO `web_admin_menu` VALUES ('59', '网站参数配置', 'Home/config', '0', '21', '0', '');
INSERT INTO `web_admin_menu` VALUES ('60', '图片位管理', 'Home/img', '0', '21', '1', '');
INSERT INTO `web_admin_menu` VALUES ('61', '产品中心', 'Home/product', '0', '21', '2', '');
INSERT INTO `web_admin_menu` VALUES ('62', '新闻管理', 'Home/news', '0', '21', '3', '');
INSERT INTO `web_admin_menu` VALUES ('63', '留言记录', 'Home/feedback', '1', '21', '4', '');
INSERT INTO `web_admin_menu` VALUES ('64', '游戏底包上传', 'Game/upload_view', '1', '19', '3', '');
INSERT INTO `web_admin_menu` VALUES ('65', '游戏常用参数配置', 'Hall/game_config', '0', '19', '5', '');
INSERT INTO `web_admin_menu` VALUES ('66', '游戏分享', 'Hall/share', '0', '18', '2', '');
INSERT INTO `web_admin_menu` VALUES ('67', '管理用户', 'Adminsystem/member_list', '0', '1', '2', '');
INSERT INTO `web_admin_menu` VALUES ('68', '操作日志', 'Adminsystem/operation', '0', '1', '3', '');
INSERT INTO `web_admin_menu` VALUES ('69', 'VIP房管理', 'Game/vipRoom', '1', '19', '0', '');
INSERT INTO `web_admin_menu` VALUES ('70', '金币变化日志', 'Operate/money_change_record', '0', '123', '14', '');
INSERT INTO `web_admin_menu` VALUES ('71', '代理分成比例配置', 'Agent/agent_config', '0', '15', '4', '');
INSERT INTO `web_admin_menu` VALUES ('72', '平台状态管理', 'Adminsystem/plat_status', '0', '19', '1', '');
INSERT INTO `web_admin_menu` VALUES ('73', '钻石变化日志', 'Operate/jewels_change_record', '0', '123', '15', '');
INSERT INTO `web_admin_menu` VALUES ('74', '银行操作记录', 'Hall/bank', '1', '18', '12', '');
INSERT INTO `web_admin_menu` VALUES ('75', '登录管理', 'Hall/login', '1', '18', '0', '');
INSERT INTO `web_admin_menu` VALUES ('76', '代理后台', 'Agent/Public/login', '0', '15', '6', '');
INSERT INTO `web_admin_menu` VALUES ('77', '游戏控制', 'Hall/rewardsPool', '0', '20', '1', '');
INSERT INTO `web_admin_menu` VALUES ('78', '充值', 'User/user_recharge', '0', '10', '0', '');
INSERT INTO `web_admin_menu` VALUES ('79', '提取', 'User/user_pos', '0', '10', '0', '');
INSERT INTO `web_admin_menu` VALUES ('80', '限制登录', 'User/limit_login', '0', '10', '0', '');
INSERT INTO `web_admin_menu` VALUES ('81', '添加白名单', 'User/white', '0', '10', '0', '');
INSERT INTO `web_admin_menu` VALUES ('82', '添加超端', 'User/set_supper_user', '0', '10', '0', '');
INSERT INTO `web_admin_menu` VALUES ('84', '发送个人邮件', 'User/personal_send_email', '0', '10', '0', '');
INSERT INTO `web_admin_menu` VALUES ('85', '绑定邀请码', 'User/bind_code', '0', '10', '0', '');
INSERT INTO `web_admin_menu` VALUES ('86', '离开房间', 'User/clearRoom', '0', '10', '0', '');
INSERT INTO `web_admin_menu` VALUES ('87', '充值记录', 'User/personal_recharge_record', '0', '10', '0', '');
INSERT INTO `web_admin_menu` VALUES ('88', '金币变化记录', 'User/personal_money_change', '0', '10', '0', '');
INSERT INTO `web_admin_menu` VALUES ('89', '钻石变化记录', 'User/personal_jewels_change', '1', '9', '0', '');
INSERT INTO `web_admin_menu` VALUES ('90', '个人对局游戏记录', 'User/personal_game_record', '0', '10', '0', '');
INSERT INTO `web_admin_menu` VALUES ('91', '个人房卡游戏记录', 'User/personal_game_jewsels_record', '1', '9', '0', '');
INSERT INTO `web_admin_menu` VALUES ('92', '签到记录', 'User/personal_sign_record', '0', '10', '0', '');
INSERT INTO `web_admin_menu` VALUES ('93', '抽奖记录', 'User/personal_turntable_record', '0', '10', '0', '');
INSERT INTO `web_admin_menu` VALUES ('94', '分享记录', 'User/personal_share_record', '0', '10', '0', '');
INSERT INTO `web_admin_menu` VALUES ('95', '实物兑换记录 ', 'User/personal_convert_record', '0', '10', '0', '');
INSERT INTO `web_admin_menu` VALUES ('96', '转赠记录', 'User/personal_given_record', '0', '10', '0', '');
INSERT INTO `web_admin_menu` VALUES ('97', '魔法表情记录', 'User/personal_magic_record', '0', '10', '0', '');
INSERT INTO `web_admin_menu` VALUES ('98', '世界广播记录', 'User/personal_radio_record', '0', '10', '0', '');
INSERT INTO `web_admin_menu` VALUES ('99', '登录记录', 'Hall/login', '0', '10', '0', '');
INSERT INTO `web_admin_menu` VALUES ('100', '代理信息', 'User/agentinfo', '0', '10', '0', '');
INSERT INTO `web_admin_menu` VALUES ('101', '订单管理', 'Mall/orders', '0', '17', '2', '');
INSERT INTO `web_admin_menu` VALUES ('102', '后台充值提取记录', 'User/admin_action_record', '1', '17', '6', '');
INSERT INTO `web_admin_menu` VALUES ('103', '代理申请列表', 'Agent/examine', '0', '15', '2', '');
INSERT INTO `web_admin_menu` VALUES ('104', ' 菜单管理', 'Wechat/index', '0', '112', '0', '');
INSERT INTO `web_admin_menu` VALUES ('105', '关键词管理', 'wechat/keywords', '0', '112', '0', '');
INSERT INTO `web_admin_menu` VALUES ('106', '关注默认回复', 'wechat/subscribe', '0', '112', '0', '');
INSERT INTO `web_admin_menu` VALUES ('107', '用户管理', 'wechat/user', '0', '112', '0', '');
INSERT INTO `web_admin_menu` VALUES ('108', '素材管理', 'wechat/material', '0', '112', '0', '');
INSERT INTO `web_admin_menu` VALUES ('109', '图文管理', 'wechat/news', '1', '112', '0', '');
INSERT INTO `web_admin_menu` VALUES ('110', '客服管理', 'wechat/kf', '0', '112', '0', '');
INSERT INTO `web_admin_menu` VALUES ('111', '微信代理申请', 'Agent/apply', '1', '15', '0', '');
INSERT INTO `web_admin_menu` VALUES ('112', '微信管理', 'Wechat/Index', '0', '0', '0', 'fa fa-qq');
INSERT INTO `web_admin_menu` VALUES ('113', '代理收入明细', 'User/bill_detail', '0', '10', '0', '');
INSERT INTO `web_admin_menu` VALUES ('114', '设置用户状态', 'User/set_user_status', '0', '10', '0', '');
INSERT INTO `web_admin_menu` VALUES ('115', '好友打赏记录', 'Hall/reward', '1', '18', '0', '');
INSERT INTO `web_admin_menu` VALUES ('116', '俱乐部列表', 'Club/index', '0', '118', '0', '');
INSERT INTO `web_admin_menu` VALUES ('117', '俱乐部金币变化日志', 'Club/money_change_record', '0', '118', '0', '');
INSERT INTO `web_admin_menu` VALUES ('118', '俱乐部管理', '', '0', '0', '8', 'fa fa-group');
INSERT INTO `web_admin_menu` VALUES ('119', '游戏输赢设置', 'Game/jackpotconfig', '1', '18', '0', '');
INSERT INTO `web_admin_menu` VALUES ('120', '意见反馈', 'wechat/feedback', '1', '112', '0', '');
INSERT INTO `web_admin_menu` VALUES ('122', '俱乐部火币变化日志', 'Club/firCoinChange', '0', '118', '0', '');
INSERT INTO `web_admin_menu` VALUES ('123', '货币日志变化', '', '0', '0', '10', 'fa fa-calendar');
INSERT INTO `web_admin_menu` VALUES ('124', '平台数据统计', '', '0', '0', '12', 'fa fa-line-chart');
INSERT INTO `web_admin_menu` VALUES ('126', '统计图表', 'Hall/statistical_chart', '0', '18', '8', '');
INSERT INTO `web_admin_menu` VALUES ('127', '邮件与公告', '', '0', '0', '3', 'fa fa-bell');
INSERT INTO `web_admin_menu` VALUES ('129', '银行管理', '', '0', '0', '11', 'fa fa-bank');
INSERT INTO `web_admin_menu` VALUES ('130', '银行统计', 'Hall/bank_count', '0', '129', '0', '');
INSERT INTO `web_admin_menu` VALUES ('131', '代理申请页面', 'agent/register', '0', '15', '7', '');
INSERT INTO `web_admin_menu` VALUES ('132', '大厅登入记录', 'User/logon_record', '0', '9', '2', '');
INSERT INTO `web_admin_menu` VALUES ('133', '玩家列表', 'User/user_list', '0', '9', '1', '');
INSERT INTO `web_admin_menu` VALUES ('135', '代理反馈问题', 'Agent/feedback', '0', '15', '8', '');
INSERT INTO `web_admin_menu` VALUES ('136', '支付配置', 'Mall/payConfig', '0', '19', '6', '');
INSERT INTO `web_admin_menu` VALUES ('137', '游戏登入记录', 'User/game_login', '0', '9', '2', '');
INSERT INTO `web_admin_menu` VALUES ('138', '活动', 'Hall/broadcast', '1', '18', '8', '');
INSERT INTO `web_admin_menu` VALUES ('139', '桌子购买配置', 'Game/desk_buy_config', '0', '20', '4', '');


-- ----------------------------
-- Records of web_admin_group
-- ----------------------------
INSERT INTO `web_admin_group` VALUES ('1', '管理员', '1,2,3,9,12,13,14,15,16,18,19,20,21,24,25,26,27,28,29,30,31,32,35,36,38,39,40,46,48,49,59,60,61,62,64,65,66,67,68,70,71,72,73,76,77,103,116,117,118,122,123,124,126,127,129,130,131,132,133,135,136,137', '管理员', '0');
INSERT INTO `web_admin_group` VALUES ('2', '商务', '9,12,13,14,15,16,17,18,20,24,25,26,27,28,29,30,31,32,35,38,39,41,42,43,46,48,49,66,70,71,73,76,77,101,103,116,117,118,122,123,124,126,127,129,130,131,132,133,135,137', '商务', '0');


-- ----------------------------
-- Records of web_agent_config
-- ----------------------------
INSERT INTO `web_agent_config` VALUES ('1', 'recharge_commission_level', '0', '充值返佣级别 0为无限级');
INSERT INTO `web_agent_config` VALUES ('2', 'agent_level_ratio_1', '60', '一级代理获得分成比例 百分比');
INSERT INTO `web_agent_config` VALUES ('3', 'agent_level_ratio_2', '50', '二级代理获得分成比例 百分比');
INSERT INTO `web_agent_config` VALUES ('4', 'agent_level_ratio_3', '40', '三级代理获得分成比例 百分比');
INSERT INTO `web_agent_config` VALUES ('5', 'exchange_proportion', '100', '兑换比例 1:value');
INSERT INTO `web_agent_config` VALUES ('6', 'put_count', '3', '每日提现次数');
INSERT INTO `web_agent_config` VALUES ('7', 'put_min_money', '80', '每次最低提现额度');
INSERT INTO `web_agent_config` VALUES ('8', 'agent_notice', '皓天棋牌', '最新公告');

-- ----------------------------
-- Records of web_agent_group
-- ----------------------------
INSERT INTO `web_agent_group` VALUES ('1', '1级代理', '1,2,3,4,5,6', '1级代理权限控制', '0');
INSERT INTO `web_agent_group` VALUES ('2', '2级代理', '1,2,3,4,5,6', '2级代理权限控制', '0');
INSERT INTO `web_agent_group` VALUES ('3', '3级代理', '1,2,3,4,5,6', '3级代理权限控制', '0');

-- ----------------------------
-- Records of web_agent_recharge_config
-- ----------------------------
INSERT INTO `web_agent_recharge_config` VALUES ('1', '微信支付', 'wx123', '1', 'http://w.php.com/Public/newadmin/img/1529477603753.jpg');
INSERT INTO `web_agent_recharge_config` VALUES ('2', '支付支付', 'zfb123', '2', 'http://w.php.com/Public/newadmin/img/1529477605553.jpg');
INSERT INTO `web_agent_recharge_config` VALUES ('3', '银行卡', 'ic123', '3', '49641579441467');

-- ----------------------------
-- Records of web_game_config
-- ----------------------------
INSERT INTO `web_game_config` VALUES ('1', 'kefu_phone_1', '18025305658', '客服1电话');
INSERT INTO `web_game_config` VALUES ('2', 'bind_agentid_send_jewels', '10', '绑定代理号送的房卡数');
INSERT INTO `web_game_config` VALUES ('3', 'bind_agentid_send_money', '5000', '绑定代理号送的金币');
INSERT INTO `web_game_config` VALUES ('6', 'android_packet_address', 'http://haoyue.szhuomei.com/haotianqipai-125.apk', '游戏底包安卓包地址');
INSERT INTO `web_game_config` VALUES ('7', 'apple_packet_address', 'https://itunes.apple.com/us/app/皓天棋牌/id1274971104?mt=8', '游戏底包苹果包下载地址');
INSERT INTO `web_game_config` VALUES ('8', 'share_begin_time', '1509638400', '分享活动开始时间');
INSERT INTO `web_game_config` VALUES ('9', 'share_end_time', '4099737600', '分享活动结束时间');
INSERT INTO `web_game_config` VALUES ('10', 'share_interval', '4', '分享奖励间隔时间');
INSERT INTO `web_game_config` VALUES ('11', 'share_img1', 'http://ht.szhuomei.com/Uploads/home_img/5a570b6b7d8f9.png', '分享图片');
INSERT INTO `web_game_config` VALUES ('12', 'share_url', 'dwadadwadwaddwad', '分享链接');
INSERT INTO `web_game_config` VALUES ('13', 'share_address', '2', '分享到哪个平台才有奖励');
INSERT INTO `web_game_config` VALUES ('14', 'share_send_money', '2000', '分享送金币数');
INSERT INTO `web_game_config` VALUES ('15', 'share_send_jewels', '1', '分享赠送房卡数');
INSERT INTO `web_game_config` VALUES ('19', 'bindAgent_sendUser_money', '5000', '代理邀请玩家 绑定的玩家赠送的奖励 金币');
INSERT INTO `web_game_config` VALUES ('20', 'bindAgent_sendUser_diamonds', '10', '代理邀请玩家 绑定的玩家赠送的奖励 钻石');
INSERT INTO `web_game_config` VALUES ('21', 'user_invitationUser_SendMoney', '1000', '玩家邀请玩家 绑定的玩家赠送的奖励 金币');
INSERT INTO `web_game_config` VALUES ('22', 'user_invitationUser_SendDiamonds', '5', '玩家邀请玩家 绑定的玩家赠送的奖励 钻石');
INSERT INTO `web_game_config` VALUES ('23', 'userBind_sendMoney', '500', '玩家邀请玩家 邀请的玩家赠送的的奖励 金币');
INSERT INTO `web_game_config` VALUES ('24', 'userBind_sendDiamonds', '2', '玩家邀请玩家 邀请的玩家赠送的的奖励 钻石');
INSERT INTO `web_game_config` VALUES ('25', 'share_img2', 'http://ht.szhuomei.com/Uploads/home_img/5a571bd12adc5.png', '分享图片2');
INSERT INTO `web_game_config` VALUES ('26', 'share_img3', 'http://ht.szhuomei.com/Uploads/home_img/5a571bda27e20.png', '分享图片3');
INSERT INTO `web_game_config` VALUES ('27', 'share_img4', 'http://ht.szhuomei.com/Uploads/home_img/5a571bf8c0db3.png', '分享图片4');
INSERT INTO `web_game_config` VALUES ('28', 'share_img5', 'http://ht.szhuomei.com/Uploads/home_img/5a571bfab043f.png', '分享图片5');
INSERT INTO `web_game_config` VALUES ('29', 'BindPhoneSendMoney', '2000', '绑定手机');
INSERT INTO `web_game_config` VALUES ('30', 'share_domain', 'https://hh80hgfs.xhuomei.cn/index.php/index/index/download?id=1015', '分享域名');
INSERT INTO `web_game_config` VALUES ('31', 'service_web_url', 'http://www.szhmwl.com/customized/', '客服中心前往');
INSERT INTO `web_game_config` VALUES ('32', 'service_game_url', 'https://hh80hgfs.xhuomei.cn/index.php/index/index/download?id=1039', '客服中心下载');
INSERT INTO `web_game_config` VALUES ('33', 'user_register_content', '欢迎光临', '玩家注册发送邮件内容');

-- ----------------------------
-- Records of web_home_ad
-- ----------------------------

INSERT INTO `web_home_ad` VALUES ('3', 'logo', 'http://w.php.com/Uploads/home_img/5b7635746ea9b.jpg', '网站logo图片', '0', '1504519961');
INSERT INTO `web_home_ad` VALUES ('4', 'anzhuo', 'http://w.php.com/Uploads/home_img/5b76356b880ad.jpg', '下载二维码   安卓系统', '0', '1504522558');
INSERT INTO `web_home_ad` VALUES ('5', '轮播图1', 'http://w.php.com/Uploads/home_img/5b763560a4e85.jpg', '网站轮播图', '1', '1504522610');
INSERT INTO `web_home_ad` VALUES ('6', '轮播图2', 'http://w.php.com/Uploads/home_img/5b76355a251bc.jpg', '轮播图2', '1', '1504522675');
INSERT INTO `web_home_ad` VALUES ('7', '轮播图3', 'http://w.php.com/Uploads/home_img/5b763552302a3.jpg', '轮播图3', '1', '1504522689');
INSERT INTO `web_home_ad` VALUES ('8', 'qrcode', 'http://w.php.com/Uploads/home_img/5b763540bfaf4.png', '公众号二维码', '0', '1504680088');
-- ----------------------------
-- Records of web_home_menu
-- ----------------------------
INSERT INTO `web_home_menu` (`id`, `menu_name`, `link_url`) VALUES ('1', '首页', 'Index/index');
INSERT INTO `web_home_menu` (`id`, `menu_name`, `link_url`) VALUES ('2', '下载中心', 'Index/download');
INSERT INTO `web_home_menu` (`id`, `menu_name`, `link_url`) VALUES ('3', '新闻中心', 'Index/news');
INSERT INTO `web_home_menu` (`id`, `menu_name`, `link_url`) VALUES ('4', '联系我们', 'Index/lianxi');

-- ----------------------------
-- Records of web_home_config
-- ----------------------------
INSERT INTO `web_home_config` (`id`, `key`, `value`, `desc`, `create_time`) VALUES ('4', 'title', '皓天棋牌', '网站标题', '1504679233');
INSERT INTO `web_home_config` (`id`, `key`, `value`, `desc`, `create_time`) VALUES ('5', 'phone', '18025305658', '联系方式', '1504679252');
INSERT INTO `web_home_config` (`id`, `key`, `value`, `desc`, `create_time`) VALUES ('6', 'address', '深圳市宝安区海秀路19号国际西岸商务大厦801', '公司地址', '1504679272');
INSERT INTO `web_home_config` (`id`, `key`, `value`, `desc`, `create_time`) VALUES ('7', 'beian', 'Copyright 2012-2014 huomei, All Rights Reserved 宝城企业.火妹 版权所有 备案号：粤ICP备17067659号', '公司备案信息', '1504679295');
INSERT INTO `web_home_config` (`id`, `key`, `value`, `desc`, `create_time`) VALUES ('8', 'desc', '深圳市火妹网络科技有限公司，该公司是一家长期专注于定制开发特色棋牌游戏、手机棋牌游戏、捕鱼游戏等等的高新科技游戏软件开发公司，力求打造“专业化、国际化、产业化、品牌化”的高科技游戏软件产品，成为独具魅力的领跑者。 火妹集结了一群年轻而充满梦想与激情的游戏从业人员，团队核心成员均来自腾讯、中青宝、博雅互动等一线棋牌游戏开发公司，具有五年以上棋牌游戏研发经验，自公司成立以来，始终秉承“诚为本、技至精”的核心经营理念，不断钻研市场需求，不断打磨产品品质，与中手游、百度、91等众多知名互联网企业精诚合作，并取得了丰硕的成果。由火妹科技一手打造的房卡金币双模式棋牌游戏平台，铸就了大量网络棋牌游戏行业的成功案例。 在不断优化自身产品的同时，火妹科技将推出更多适应市场的运营级游戏产品和平台，凭借自身丰富的资源优势，成熟的经验优势，强大的技术优势，优质的服务优势，更好的为客户创造独特价值。', '公司介绍', '1504679361');
INSERT INTO `web_home_config` (`id`, `key`, `value`, `desc`, `create_time`) VALUES ('9', 'email', '2041302270@qq.com', '邮箱', '1504679958');
INSERT INTO `web_home_config` (`id`, `key`, `value`, `desc`, `create_time`) VALUES ('10', 'qq', '2041302270', 'qq', '1504679970');
INSERT INTO `web_home_config` (`id`, `key`, `value`, `desc`, `create_time`) VALUES ('3', 'abbreviation', '皓天', '简称', '1504679233');
-- ----------------------------
-- Records of web_packet_version_type
-- ----------------------------

INSERT INTO `web_packet_version_type` VALUES ('1', '普通版本', '1');
INSERT INTO `web_packet_version_type` VALUES ('2', '土豪版', '2');

-- ----------------------------
-- Records of web_packet_version
-- ----------------------------
INSERT INTO `web_packet_version` VALUES ('1', '1', '1', 'appStore', '1.0.6', 'https://fir.im/t761', '', '1', 'appStore');
INSERT INTO `web_packet_version` VALUES ('2', '1', '2', 'android', '1.3.9', 'http://haoyue.szhuomei.com/android-139.zip', '', '1', '平台包安卓1.3.9版本');
INSERT INTO `web_packet_version` VALUES ('3', '1', '3', 'ios签名包', '1.0.6', 'https://fir.im/t761', '', '1', 'ios签名包');
INSERT INTO `web_packet_version` VALUES ('4', '2', '1', '皓天棋牌', '7.0.2', 'http://haoyue.szhuomei.com/Lobby-201808111504.zip', '', '1', '大厅资源包');
INSERT INTO `web_packet_version` VALUES ('5', '2', '36610103', '斗地主', '7.0.0', 'http://haoyue.szhuomei.com/CSDDZ-201808101517.zip', '', '1', '斗地主');

-- ----------------------------
-- Records of web_packet_version_test
-- ----------------------------
INSERT INTO `web_packet_version_test` VALUES ('1', '1', '1', 'appStore', '7.0.2', 'http://haoyue.szhuomei.com/Lobby-201808111504.zip', '1.0.6', '1', 'appStore');
INSERT INTO `web_packet_version_test` VALUES ('2', '1', '2', 'android', '6.2.3', 'http://haoyue.szhuomei.com/Lobby-201807180930.zip', '', '1', '平台包安卓1.3.2版本');
INSERT INTO `web_packet_version_test` VALUES ('3', '1', '3', 'ios签名包', '1.0.0', 'http://www.baidu.com', '', '1', 'ios签名包');
INSERT INTO `web_packet_version_test` VALUES ('4', '2', '1', '皓天棋牌大厅', '7.0.2', 'http://haoyue.szhuomei.com/Lobby-201808111504.zip', '', '1', '大厅资源包');
INSERT INTO `web_packet_version_test` VALUES ('5', '2', '36610103', '斗地主', '7.0.0', 'http://haoyue.szhuomei.com/CSDDZ-201808101517.zip', '', '1', '斗地主');

-- ----------------------------
-- Records of web_pay_config
-- ----------------------------
INSERT INTO `web_pay_config` VALUES ('1', '苹果支付', 'wx624ff377019b69a1', '1487308352', '12345678123456781234567812345aab', '1', '12311', '12', '', '1', '0');
INSERT INTO `web_pay_config` VALUES ('2', '微信支付', 'wx624ff377019b69a1', '1487308352', '12345678123456781234567812345aab', '1', '', 'http://ht.szhmkeji.com/hm_ucenter/pay/hui_fu_bao/notify.php', '', '1', '0');
INSERT INTO `web_pay_config` VALUES ('3', '汇付宝', '2112482', '2112482', 'AEEC9E44A33B4380A02ADBE7', '1', '', 'http://ht.szhmkeji.com/hm_ucenter/pay/hui_fu_bao/notify.php', 'http://', '1', '1');
INSERT INTO `web_pay_config` VALUES ('4', '旺实富', '69018071165702', '69018071165702', '046909094053n7SiSWYr', '0', '', 'http://ht.szhmkeji.com/hm_ucenter/pay/wang_shi_fu/notify.php', 'http://', '1', '1');
INSERT INTO `web_pay_config` VALUES ('5', '简付', '180778708', '180778708', 'i4dlpqh8eza9i2kuy9oo6oaez6hw6dor', '0', '', 'http://ht.szhmkeji.com/hm_ucenter/pay/jian_fu/notify.php', 'http://ht.szhmkeji.com/hm_ucenter/pay/jian_fu/index.php', '1', '1');
INSERT INTO `web_pay_config` VALUES ('6', '鑫宝', '10068', '10068', 'c4ek3opkh9f8j52cr8qcgs9i2k48f6z2', '1', '', 'http://ht.szhmkeji.com/hm_ucenter/pay/xin_bao/notify.php', 'http://ht.szhmkeji.com/hm_ucenter/pay/xin_bao/index.php', '1', '1');
INSERT INTO `web_pay_config` VALUES ('7', '汇付宝直连', '2116566', '2116566', '7562ABCAAB3E432E881A2B7E', '0', '', 'http://ht.szhmkeji.com/hm_ucenter/pay/hui_fu_bao_zl/notify.php', 'http://', '1', '1');


-- ----------------------------
-- Records of web_pay_goods
-- ----------------------------
INSERT INTO `web_pay_goods` VALUES ('5', '金币', '10000', '1', '人民币', '100', '0', '0', null, '1', '10001', '1', '0', 'com.globIcon.10000', '1', null);
INSERT INTO `web_pay_goods` VALUES ('6', '金币', '36000', '1', '人民币', '300', '0', '0', null, '1', '10002', '0', '1', 'com.globIcon.36000', '2', null);
INSERT INTO `web_pay_goods` VALUES ('7', '金币', '94000', '1', '人民币', '800', '0', '0', null, '1', '10003', '0', '0', 'com.globIcon.94000', '3', null);
INSERT INTO `web_pay_goods` VALUES ('8', '金币', '142000', '1', '软妹币', '1200', '0', '0', null, '1', '10004', '0', '1', 'com.globIcon.142000', '4', null);
INSERT INTO `web_pay_goods` VALUES ('9', '金币', '216000', '1', '人民币', '1800', '0', '0', null, '1', '10005', '0', '1', 'com.globIcon.216000', '5', null);
INSERT INTO `web_pay_goods` VALUES ('10', '金币', '332000', '1', '人民币', '2800', '0', '0', null, '1', '10006', '0', '0', 'com.globIcon.332000', '6', null);
INSERT INTO `web_pay_goods` VALUES ('11', '金币', '596000', '1', '软妹币', '5000', '0', '0', null, '1', '10007', '0', '0', 'com.globIcon.596000', '7', null);
INSERT INTO `web_pay_goods` VALUES ('12', '金币', '1292000', '1', '软妹币', '10800', '0', '0', null, '1', '10008', '0', '0', 'com.globIcon.1292000', '8', null);
INSERT INTO `web_pay_goods` VALUES ('13', '钻石', '1', '2', '人民币', '100', '0', '0', null, '1', '10009', '0', '0', 'com.coupon.1', '1', null);
INSERT INTO `web_pay_goods` VALUES ('14', '钻石', '4', '2', '人民币', '300', '0', '0', null, '1', '10010', '0', '0', 'com.coupon.4', '2', null);
INSERT INTO `web_pay_goods` VALUES ('15', '钻石', '11', '2', '人民币', '800', '0', '0', null, '1', '10011', '0', '0', 'com.coupon.11', '3', null);
INSERT INTO `web_pay_goods` VALUES ('16', '钻石', '17', '2', '人民币', '1200', '0', '0', null, '1', '10012', '0', '0', 'com.coupon.17', '4', null);
INSERT INTO `web_pay_goods` VALUES ('17', '钻石', '26', '2', '人民币', '1800', '0', '0', null, '1', '10013', '0', '0', 'com.coupon.26', '5', null);
INSERT INTO `web_pay_goods` VALUES ('18', '钻石', '40', '2', '人民币', '2800', '0', '0', null, '1', '10014', '0', '0', 'com.coupon.40', '6', null);
INSERT INTO `web_pay_goods` VALUES ('19', '钻石', '75', '2', '人民币', '5000', '0', '0', null, '1', '10015', '0', '0', 'com.coupon.75', '7', null);
INSERT INTO `web_pay_goods` VALUES ('20', '钻石', '162', '2', '人民币', '10800', '0', '0', null, '1', '10016', '0', '0', 'com.coupon.162', '8', null);
INSERT INTO `web_pay_goods` VALUES ('21', '钻石', '1', '4', '金币', '2000000', '1', '0', '1504234004', '0', '10017', '1', '0', '', '1', null);
INSERT INTO `web_pay_goods` VALUES ('22', '十元话费券', '1', '4', '金币', '22000000', '1', '0', '1504237130', '0', '10018', '1', '0', '', '2', null);
INSERT INTO `web_pay_goods` VALUES ('23', '50元话费券', '1', '4', '金币', '102000000', '1', '0', '1504237202', '0', '10019', '1', '0', '', '3', null);
INSERT INTO `web_pay_goods` VALUES ('24', '100元话费券', '1', '4', '金币', '200000000', '1', '0', '1504237230', '0', '10020', '1', '0', '', '4', null);
INSERT INTO `web_pay_goods` VALUES ('25', '小米移动电源', '1', '4', '金币', '238000000', '1', '0', '1504237263', '0', '10021', '1', '0', '', '5', null);
INSERT INTO `web_pay_goods` VALUES ('26', '小米平衡车', '1', '4', '金币', null, '1', '0', '1504237298', '0', '10022', '1', '0', '', '6', null);
INSERT INTO `web_pay_goods` VALUES ('27', 'ipad', '1', '4', '金币', null, '1', '0', '1504237342', '0', '10023', '1', '0', '', '7', null);
INSERT INTO `web_pay_goods` VALUES ('28', 'iphone7', '1', '4', '金币', null, '1', '0', '1504237378', '0', '10024', '1', '0', '', '8', null);

-- ----------------------------
-- Records of web_server_info
-- ----------------------------
INSERT INTO `web_server_info` VALUES ('1', '1', '1');

-- ----------------------------
-- Records of web_sign_config
-- ----------------------------
INSERT INTO `web_sign_config` VALUES ('1', '1', '金币', '500', '1', '1');
INSERT INTO `web_sign_config` VALUES ('2', '2', '金币', '800', '1', '2');
INSERT INTO `web_sign_config` VALUES ('3', '3', '房卡', '2', '2', '3');
INSERT INTO `web_sign_config` VALUES ('4', '4', '金币', '1000', '1', '4');
INSERT INTO `web_sign_config` VALUES ('5', '5', '金币', '2000', '1', '5');
INSERT INTO `web_sign_config` VALUES ('6', '6', '房卡', '5', '2', '6');

-- ----------------------------
-- Records of web_socket_config
-- ----------------------------
INSERT INTO `web_socket_config` VALUES ('1', '172.18.163.18', '4015');

-- ----------------------------
-- Records of web_turntable_config
-- ----------------------------
INSERT INTO `web_turntable_config` VALUES ('1', '金币', '288', '20', '1');
INSERT INTO `web_turntable_config` VALUES ('2', '金币', '2', '10', '1');
INSERT INTO `web_turntable_config` VALUES ('3', '金币', '688', '7', '1');
INSERT INTO `web_turntable_config` VALUES ('4', '金币', '10', '6', '1');
INSERT INTO `web_turntable_config` VALUES ('5', '金币', '1888', '5', '1');
INSERT INTO `web_turntable_config` VALUES ('6', '金币', '50', '4', '1');
INSERT INTO `web_turntable_config` VALUES ('7', '金币', '5800', '1', '1');
INSERT INTO `web_turntable_config` VALUES ('8', '无', '0', '47', '0');