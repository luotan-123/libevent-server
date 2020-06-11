/*
Navicat MySQL Data Transfer

Source Server         : 156.241.5.157
Source Server Version : 50730
Source Host           : 156.241.5.157:3306
Source Database       : db_dblmgame

Target Server Type    : MYSQL
Target Server Version : 50730
File Encoding         : 65001

Date: 2020-06-11 10:05:45
*/

SET FOREIGN_KEY_CHECKS=0;

-- ----------------------------
-- Table structure for ag_agentdraw
-- ----------------------------
DROP TABLE IF EXISTS `ag_agentdraw`;
CREATE TABLE `ag_agentdraw` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `userid` int(11) NOT NULL,
  `nickname` varchar(30) NOT NULL DEFAULT '' COMMENT '昵称',
  `createdate` varchar(20) DEFAULT '' COMMENT '时间',
  `drawjetton` int(11) DEFAULT '0' COMMENT '提款金额',
  `drawnum` int(11) DEFAULT '0' COMMENT '到账金额',
  `bankacount` varchar(20) DEFAULT '' COMMENT '银行卡号',
  `bankname` varchar(30) DEFAULT '' COMMENT '银行名称',
  `bankpayee` varchar(20) DEFAULT '' COMMENT '银行卡姓名',
  `bankaddress` varchar(50) DEFAULT '' COMMENT '银行卡地址',
  `drawmtype` int(11) DEFAULT '0' COMMENT '1:提现到支付宝，2：提现到银行卡',
  `ratenum` float DEFAULT '0' COMMENT '提现抽取的手续费',
  `orderid` varchar(30) DEFAULT '' COMMENT '订单号',
  `state` int(11) DEFAULT '0' COMMENT '1：发起提款，2：审核中，3：成功，4：失败，5：驳回',
  `remark` varchar(100) DEFAULT '' COMMENT '备注',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for ag_apply
-- ----------------------------
DROP TABLE IF EXISTS `ag_apply`;
CREATE TABLE `ag_apply` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `userid` int(11) DEFAULT '0',
  `nickname` varchar(45) DEFAULT '',
  `rsdate` timestamp NULL DEFAULT CURRENT_TIMESTAMP,
  `applycode` varchar(45) DEFAULT '',
  `state` int(11) DEFAULT '0',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for ag_author
-- ----------------------------
DROP TABLE IF EXISTS `ag_author`;
CREATE TABLE `ag_author` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `binduserid` int(11) DEFAULT '0',
  `openid` varchar(45) CHARACTER SET latin1 DEFAULT '',
  `nickname` varchar(45) DEFAULT '',
  `unionid` varchar(45) CHARACTER SET latin1 DEFAULT '',
  `datemark` timestamp NULL DEFAULT CURRENT_TIMESTAMP,
  `wiringid` int(11) DEFAULT '0',
  `prelevel` int(11) DEFAULT '0',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for ag_drawmoney
-- ----------------------------
DROP TABLE IF EXISTS `ag_drawmoney`;
CREATE TABLE `ag_drawmoney` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `userid` int(11) DEFAULT '0',
  `nickname` varchar(45) DEFAULT '',
  `phone` varchar(45) DEFAULT '',
  `wechat` varchar(45) DEFAULT '',
  `drawnum` float DEFAULT '0',
  `state` int(11) DEFAULT '0' COMMENT '1：发起提款，2：审核中，3：成功，4：失败，5：驳回',
  `rsdate` timestamp NULL DEFAULT CURRENT_TIMESTAMP,
  `drawjetton` int(11) DEFAULT '0' COMMENT '金币数量',
  `julebuid` int(11) DEFAULT '0',
  `julebuname` varchar(45) DEFAULT '',
  `realname` varchar(128) DEFAULT '',
  `alipay` varchar(128) DEFAULT '',
  `bankacount` varchar(128) DEFAULT '',
  `bankname` varchar(128) DEFAULT '',
  `bankaddress` varchar(128) DEFAULT '',
  `orderid` varchar(128) DEFAULT '',
  `drawmtype` int(11) DEFAULT '0' COMMENT '1:提现到支付宝，2：提现到银行卡，3：提现到余额',
  `alipaypayee` varchar(128) DEFAULT '',
  `bankpayee` varchar(128) DEFAULT '',
  `taketype` int(11) DEFAULT '0' COMMENT '0：账号余额提现，1:佣金是提现，2：是直属佣金预提',
  `opttime` varchar(45) DEFAULT '' COMMENT '操作的时间',
  `remark` text COMMENT '备注：驳回的原因',
  `ratenum` float DEFAULT '0' COMMENT '提现抽取的手续费',
  `channel` varchar(60) DEFAULT '' COMMENT '渠道',
  `bankprovince` varchar(128) DEFAULT '' COMMENT '银行卡的省',
  `bankcity` varchar(128) DEFAULT '' COMMENT '银行卡的市',
  `bankidnumber` varchar(128) DEFAULT '' COMMENT '银行卡的身份证号码',
  `paytype` varchar(60) DEFAULT '' COMMENT '出款通道',
  `freefeecount` float DEFAULT '0' COMMENT '已减免的手续费',
  `prechannel` varchar(60) DEFAULT NULL,
  `payfreeamount` float DEFAULT '0' COMMENT '使用免费提现额度',
  `optid` int(11) DEFAULT NULL COMMENT '操作人',
  `risktype` int(11) DEFAULT '0' COMMENT '风控状态，0=等待处理，1=风控审核，2=主管风控，3=等待出款，4=财务出款',
  `optdate` datetime DEFAULT NULL COMMENT '操作时间',
  `optremark` varchar(255) DEFAULT NULL COMMENT '操作备注',
  `optname` varchar(50) DEFAULT NULL COMMENT '操作人名称',
  `optrisktype` int(11) DEFAULT '0' COMMENT '操作人执行时所在的风控状态',
  `drawtype` int(11) DEFAULT '0' COMMENT '0=客户提交，1=后台提交',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=545 DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for ag_paylist
-- ----------------------------
DROP TABLE IF EXISTS `ag_paylist`;
CREATE TABLE `ag_paylist` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `userid` int(11) DEFAULT '0' COMMENT '代理的ID',
  `payuserid` int(11) DEFAULT '0',
  `paynick` varchar(45) DEFAULT '0',
  `aglevel` int(11) DEFAULT '0' COMMENT '代理的等级：1:1级，2:2级，99：该用户是代理',
  `paymoney` int(11) DEFAULT '0' COMMENT '支付的数量',
  `getmoney` float DEFAULT '0' COMMENT '抽水的数量',
  `paydate` timestamp NULL DEFAULT CURRENT_TIMESTAMP COMMENT '支付日期',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for ag_paysoleagent
-- ----------------------------
DROP TABLE IF EXISTS `ag_paysoleagent`;
CREATE TABLE `ag_paysoleagent` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `userid` int(11) DEFAULT NULL COMMENT '代理的ID',
  `payuserid` int(11) DEFAULT NULL,
  `paynick` varchar(45) DEFAULT NULL,
  `aglevel` int(11) DEFAULT NULL COMMENT '代理的等级：1:1级，2:2级，99：该用户是代理',
  `paymoney` int(11) DEFAULT NULL COMMENT '支付的数量',
  `getmoney` float DEFAULT NULL COMMENT '抽水的数量',
  `paydate` timestamp NULL DEFAULT CURRENT_TIMESTAMP COMMENT '支付日期',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for ag_player
-- ----------------------------
DROP TABLE IF EXISTS `ag_player`;
CREATE TABLE `ag_player` (
  `userid` int(11) NOT NULL,
  `nickname` varchar(45) DEFAULT '',
  `account` varchar(45) DEFAULT '' COMMENT '推广码',
  `password` varchar(45) DEFAULT '',
  `email` varchar(45) DEFAULT '',
  `phone` varchar(45) DEFAULT '',
  `wechat` varchar(45) DEFAULT '',
  `qq` varchar(45) DEFAULT '',
  `expandcode` varchar(45) DEFAULT '',
  `loginip` varchar(45) DEFAULT '',
  `rsdate` timestamp NULL DEFAULT CURRENT_TIMESTAMP,
  `lgdate` datetime DEFAULT '2000-01-01 01:00:01',
  `state` int(11) DEFAULT '0',
  `level` int(11) DEFAULT '0' COMMENT '等级是0的是玩家，99：总代理，1:1级代理，2：2级代理',
  `allmoney` float DEFAULT '0' COMMENT '收益总额',
  `takemoney` float DEFAULT '0' COMMENT '可以提现的总额',
  `logincount` int(11) DEFAULT '0',
  `moneyfee` int(11) DEFAULT '0' COMMENT '抽佣的总额，超过10元每天会计算佣金',
  `soleagent` int(11) DEFAULT '0' COMMENT '所属的总代',
  `agent1` int(11) DEFAULT '0' COMMENT '直接绑定的代理',
  `agent2` int(11) DEFAULT '0' COMMENT '绑定代理的上级代理',
  `agent3` int(11) DEFAULT '0' COMMENT '绑定代理的上上级代理',
  `agent4` int(11) DEFAULT '0' COMMENT '绑定代理的上上上级代理',
  `qrcodeurl` varchar(128) DEFAULT '',
  `timemark` datetime DEFAULT '2000-01-01 01:00:01',
  `moneymark` int(11) DEFAULT '0',
  `bankacount` varchar(45) DEFAULT '',
  `bankname` varchar(45) DEFAULT '',
  `bankaddress` varchar(128) DEFAULT '',
  `agent1count` int(11) DEFAULT '0',
  `agent2count` int(11) DEFAULT '0',
  `bankpayee` varchar(45) DEFAULT '',
  `aliaccount` varchar(45) DEFAULT '',
  `alipayee` varchar(45) DEFAULT '',
  `agent3count` int(11) DEFAULT '0',
  `getrate` float DEFAULT '0',
  `grade` int(11) DEFAULT '0',
  `wiringid` int(11) DEFAULT '0',
  `premoney` float DEFAULT '0' COMMENT '可以预支的金额',
  `extractedmoney` float DEFAULT '0',
  `fcrate` int(11) DEFAULT '0',
  `channel` varchar(60) DEFAULT '' COMMENT '渠道号',
  `bankprovince` varchar(128) DEFAULT '' COMMENT '银行卡的省',
  `bankcity` varchar(128) DEFAULT '' COMMENT '银行卡的市',
  `bankidnumber` varchar(128) DEFAULT '' COMMENT '银行卡的身份证号',
  `fcgotawardnum` int(11) DEFAULT '0' COMMENT '分成中已经分到的奖励',
  `dirfcgotawardnum` int(11) DEFAULT '0' COMMENT '直属分成奖励',
  `fcgotbalance` int(11) DEFAULT '0' COMMENT '分成中已经分到的奖励的余额',
  `bankpassword` varchar(45) DEFAULT '' COMMENT '资金密码',
  `prechannel` varchar(255) DEFAULT '' COMMENT '大渠道号',
  `idcardnumber` varchar(255) DEFAULT '' COMMENT '身份证号',
  `idcarda` varchar(255) DEFAULT '' COMMENT '身份证A面',
  `idcardb` varchar(255) DEFAULT '' COMMENT '身份证B面',
  `realname` varchar(255) DEFAULT '' COMMENT '真实姓名',
  PRIMARY KEY (`userid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for du_channel
-- ----------------------------
DROP TABLE IF EXISTS `du_channel`;
CREATE TABLE `du_channel` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `name` varchar(45) DEFAULT NULL,
  `channel` varchar(45) DEFAULT NULL,
  `description` varchar(45) DEFAULT NULL,
  `state` tinyint(3) DEFAULT NULL,
  `createdate` varchar(45) DEFAULT NULL,
  `lastdate` timestamp NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  `remark` varchar(125) DEFAULT NULL,
  `paytype` varchar(128) DEFAULT '' COMMENT '付费类型列表，1：微信支付，2：支付支付，3：ios支付，4：第三方SDK支付（调起第三方计费）',
  `modules` varchar(128) DEFAULT '' COMMENT '渠道对应开放模块列表',
  `introduce` varchar(256) DEFAULT '' COMMENT '对于房卡模式，这个是显示在登录界面的说明。',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=12 DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for du_channelversion
-- ----------------------------
DROP TABLE IF EXISTS `du_channelversion`;
CREATE TABLE `du_channelversion` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `channel` varchar(45) DEFAULT 'all',
  `version` int(11) DEFAULT NULL,
  `description` varchar(256) DEFAULT NULL,
  `isused` tinyint(3) DEFAULT '0',
  `createdate` varchar(45) DEFAULT NULL,
  `lastdate` timestamp NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  `downurl` varchar(256) DEFAULT NULL,
  `state` int(11) DEFAULT '0',
  `remark` varchar(256) DEFAULT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for du_downconf
-- ----------------------------
DROP TABLE IF EXISTS `du_downconf`;
CREATE TABLE `du_downconf` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `type` int(3) DEFAULT '0' COMMENT '类型（1.安卓  2.苹果）',
  `name` varchar(255) DEFAULT '' COMMENT '名称',
  `ip` varchar(40) DEFAULT '' COMMENT '绑定的ip地址',
  `p_url` varchar(255) DEFAULT '' COMMENT 'ios--plist下载域名地址',
  `p_title` varchar(40) DEFAULT '' COMMENT 'ios--plist标题',
  `createdate` varchar(50) DEFAULT '' COMMENT '创建时间',
  `file_src` varchar(50) DEFAULT '' COMMENT '文件保存路径',
  `bundle` varchar(20) DEFAULT '' COMMENT 'plist----bundle',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='游戏下载配置';

-- ----------------------------
-- Table structure for du_server
-- ----------------------------
DROP TABLE IF EXISTS `du_server`;
CREATE TABLE `du_server` (
  `serverid` int(11) NOT NULL AUTO_INCREMENT,
  `servername` varchar(45) DEFAULT NULL,
  `serverip` varchar(45) DEFAULT NULL,
  `gamename` varchar(45) DEFAULT NULL,
  `tcpport` int(11) DEFAULT '0',
  `httpport` int(11) DEFAULT '0',
  `udpport` int(11) DEFAULT '0',
  `nettype` tinyint(3) DEFAULT '0',
  `state` tinyint(3) DEFAULT '0',
  `createdate` datetime DEFAULT '2000-01-01 00:00:00',
  `lastdate` timestamp NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  `remark` varchar(127) DEFAULT NULL,
  `wsport` int(11) DEFAULT '0',
  `logonid` int(11) DEFAULT '1' COMMENT '登陆服id',
  `peoplecount` int(11) DEFAULT '0' COMMENT '登陆服人数',
  PRIMARY KEY (`serverid`),
  KEY `logonid` (`logonid`) USING BTREE
) ENGINE=InnoDB AUTO_INCREMENT=7 DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for du_version
-- ----------------------------
DROP TABLE IF EXISTS `du_version`;
CREATE TABLE `du_version` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `clientversion` int(11) DEFAULT NULL,
  `serverversion` int(11) DEFAULT NULL,
  `clientupdate` tinyint(3) DEFAULT '0',
  `serverupdate` tinyint(3) DEFAULT '0',
  `isused` tinyint(3) DEFAULT '0',
  `createdate` varchar(45) DEFAULT NULL,
  `lastdate` timestamp NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  `remark` varchar(256) DEFAULT NULL,
  `downurl` varchar(256) DEFAULT NULL,
  `allupdate` int(11) DEFAULT '0',
  `pkgsize` int(11) DEFAULT '0' COMMENT '包体大小',
  `filename` varchar(45) DEFAULT NULL,
  `channel` varchar(45) DEFAULT 'all',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for dy_activity
-- ----------------------------
DROP TABLE IF EXISTS `dy_activity`;
CREATE TABLE `dy_activity` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `activityid` int(11) DEFAULT NULL COMMENT '具体活动ID',
  `activitylabel` varchar(255) DEFAULT NULL COMMENT '活动标题，中文',
  `activityname` varchar(255) DEFAULT '' COMMENT '活动标题，英文',
  `activitydetails` text COMMENT '活动详情',
  `activitystate` int(11) DEFAULT '1' COMMENT '活动状态：1启动， 2停止，可能还有其他的',
  `displaystarttime` datetime DEFAULT NULL COMMENT '显示开始时间',
  `displayendtime` datetime DEFAULT NULL COMMENT '显示结束时间',
  `activitystarttime` datetime DEFAULT NULL COMMENT '活动开始时间',
  `activityendtime` datetime DEFAULT NULL COMMENT '活动结束时间',
  `sort` int(11) DEFAULT '0' COMMENT '排序，从小到大',
  `data` varchar(250) DEFAULT '' COMMENT '从活动类型获取的template',
  `times` int(11) DEFAULT '1' COMMENT '次数',
  `channel` varchar(255) DEFAULT '' COMMENT '渠道号',
  `activitypic` varchar(255) DEFAULT '' COMMENT '活动图片路径',
  `activityurl` varchar(255) DEFAULT '' COMMENT '活动网页地址',
  `prechannel` varchar(255) DEFAULT NULL COMMENT '大渠道',
  `propid` varchar(1024) DEFAULT '',
  `bannerpic` varchar(255) DEFAULT '' COMMENT '活动banner图',
  `quantity` varchar(255) DEFAULT '0' COMMENT '发放数量',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=38 DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for dy_activitytype
-- ----------------------------
DROP TABLE IF EXISTS `dy_activitytype`;
CREATE TABLE `dy_activitytype` (
  `id` int(11) NOT NULL AUTO_INCREMENT COMMENT '活动类型ID',
  `name` varchar(32) DEFAULT NULL COMMENT '活动类型名称，英文',
  `label` varchar(32) DEFAULT NULL COMMENT '活动类型标志，中文',
  `description` varchar(255) DEFAULT NULL COMMENT '活动类型描述',
  `template` text COMMENT '活动类型模板格式',
  `channel` varchar(50) DEFAULT '',
  `prechannel` varchar(50) DEFAULT '',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=114 DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for dy_admin
-- ----------------------------
DROP TABLE IF EXISTS `dy_admin`;
CREATE TABLE `dy_admin` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `admin_name` varchar(30) DEFAULT NULL COMMENT '后台管理员用户名',
  `admin_pass` char(64) DEFAULT NULL COMMENT '管理员密码',
  `admin_login` int(11) NOT NULL DEFAULT '0' COMMENT '登录次数',
  `admin_myname` varchar(100) DEFAULT NULL COMMENT '管理员名字',
  `admin_email` varchar(100) DEFAULT NULL COMMENT '管理员邮箱',
  `admin_oldip` varchar(20) NOT NULL,
  `admin_ip` varchar(20) DEFAULT NULL COMMENT '登录ip',
  `admin_rsdate` int(11) NOT NULL COMMENT '注册时间',
  `admin_olddate` int(11) NOT NULL,
  `admin_ok` int(11) DEFAULT '0' COMMENT '0不锁，1锁定',
  `admin_date` int(11) DEFAULT NULL COMMENT '登录日期',
  `admin_type` int(11) DEFAULT '1' COMMENT '管理员类型,0为超级，1为普通',
  `channel` varchar(40) DEFAULT '' COMMENT '渠道',
  `prechannel` varchar(40) DEFAULT '' COMMENT '大渠道',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=26 DEFAULT CHARSET=utf8 ROW_FORMAT=DYNAMIC COMMENT='后台管理员表';

-- ----------------------------
-- Table structure for dy_attentionrace
-- ----------------------------
DROP TABLE IF EXISTS `dy_attentionrace`;
CREATE TABLE `dy_attentionrace` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `raceid` varchar(50) DEFAULT NULL COMMENT '赛事id',
  `userid` int(11) DEFAULT NULL COMMENT '玩家id',
  `channel` varchar(40) DEFAULT NULL COMMENT '渠道',
  `prechannel` varchar(40) DEFAULT NULL COMMENT '大渠道',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for dy_authcode
-- ----------------------------
DROP TABLE IF EXISTS `dy_authcode`;
CREATE TABLE `dy_authcode` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `phonenum` varchar(45) DEFAULT '',
  `timemark` int(11) DEFAULT '0',
  `code` varchar(45) DEFAULT '',
  `state` int(11) DEFAULT '0',
  `channel` varchar(11) DEFAULT '' COMMENT '渠道',
  `prechannel` varchar(11) DEFAULT NULL COMMENT '大渠道',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=8193 DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for dy_broadcast
-- ----------------------------
DROP TABLE IF EXISTS `dy_broadcast`;
CREATE TABLE `dy_broadcast` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `senderid` int(11) DEFAULT NULL,
  `sender` varchar(45) DEFAULT NULL,
  `broadtype` int(11) DEFAULT NULL,
  `msg` varchar(256) DEFAULT NULL,
  `createdate` timestamp NULL DEFAULT CURRENT_TIMESTAMP,
  `broadtimes` int(11) DEFAULT '0',
  `validday` int(11) DEFAULT '0',
  `cycletype` tinyint(3) DEFAULT '0',
  `isexist` int(11) DEFAULT '0',
  `prechannel` varchar(50) DEFAULT '' COMMENT '大渠道',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=43 DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for dy_caipiaoconf
-- ----------------------------
DROP TABLE IF EXISTS `dy_caipiaoconf`;
CREATE TABLE `dy_caipiaoconf` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `gamekey` varchar(255) DEFAULT NULL COMMENT '彩种',
  `pourtype` int(11) DEFAULT NULL COMMENT '下注区域',
  `rate` int(11) DEFAULT '0' COMMENT '赔率 100 代表 1',
  `title` varchar(30) DEFAULT NULL COMMENT '标题',
  `name` varchar(30) DEFAULT NULL COMMENT '下注的name',
  `channel` varchar(40) DEFAULT NULL COMMENT '渠道',
  `prechannel` varchar(40) DEFAULT NULL COMMENT '大渠道',
  `gtype` varchar(20) DEFAULT NULL COMMENT '大标签',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=701 DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for dy_caipiaoinfo
-- ----------------------------
DROP TABLE IF EXISTS `dy_caipiaoinfo`;
CREATE TABLE `dy_caipiaoinfo` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `gamekey` varchar(255) DEFAULT NULL COMMENT '彩种',
  `gid` varchar(255) NOT NULL COMMENT '期号',
  `award` varchar(255) DEFAULT NULL COMMENT '开奖结果',
  `date` date DEFAULT NULL COMMENT '开奖日期',
  `opentime` timestamp NULL DEFAULT NULL,
  `nextOpenIssue` varchar(255) DEFAULT NULL COMMENT '下一期开奖期号',
  `nextOpenTime` timestamp NULL DEFAULT NULL ON UPDATE CURRENT_TIMESTAMP COMMENT '下一期开奖时间',
  `secondOpenIssue` varchar(255) DEFAULT NULL COMMENT '下下期开奖期号',
  `secondOpenTime` timestamp NULL DEFAULT NULL ON UPDATE CURRENT_TIMESTAMP COMMENT '下下期开奖时间',
  `state` int(11) DEFAULT '0' COMMENT '该期状态 0 未开奖 1 封盘 2 已开奖 未结算 3 已开奖 并结算',
  `createdate` timestamp NULL DEFAULT CURRENT_TIMESTAMP,
  `updatedate` timestamp NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  PRIMARY KEY (`id`),
  KEY `gid` (`gid`) USING BTREE,
  KEY `gamekey` (`gamekey`) USING BTREE
) ENGINE=MyISAM AUTO_INCREMENT=205282 DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for dy_caipiaopourinfo
-- ----------------------------
DROP TABLE IF EXISTS `dy_caipiaopourinfo`;
CREATE TABLE `dy_caipiaopourinfo` (
  `orderid` int(11) NOT NULL AUTO_INCREMENT COMMENT '订单id',
  `gid` varchar(32) NOT NULL COMMENT '开奖期号',
  `gamekey` int(11) NOT NULL COMMENT '彩种',
  `userid` varchar(50) NOT NULL,
  `pourtype` int(11) NOT NULL COMMENT '下注信息',
  `pourjetton` int(11) NOT NULL COMMENT '下注金额',
  `state` int(11) DEFAULT '0' COMMENT '0：生成 1：封盘锁定  2: 已结算	 3：玩家已撤单  4：系统撤单',
  `pourtime` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP COMMENT '下注时间',
  `counttime` timestamp NULL DEFAULT CURRENT_TIMESTAMP COMMENT '结算时间',
  `win` int(11) DEFAULT NULL COMMENT '盈利情况',
  `pourcontent` varchar(255) DEFAULT NULL COMMENT '下注内容',
  `fengpantime` timestamp NULL DEFAULT NULL COMMENT '封盘时间',
  `opentime` timestamp NULL DEFAULT NULL COMMENT '开奖时间 延后的',
  `createtime` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `updatetime` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  `channel` varchar(40) DEFAULT NULL COMMENT '渠道',
  `prechannel` varchar(40) DEFAULT NULL COMMENT '大渠道',
  `award` varchar(30) DEFAULT NULL COMMENT '开奖结果',
  `rate` int(11) DEFAULT '0' COMMENT '赔率 200 表示 2倍',
  `gtype` varchar(50) DEFAULT NULL COMMENT 'gtype 例：两面',
  PRIMARY KEY (`orderid`),
  KEY `gid` (`gid`) USING BTREE,
  KEY `userid` (`userid`) USING BTREE
) ENGINE=MyISAM AUTO_INCREMENT=763 DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for dy_convert
-- ----------------------------
DROP TABLE IF EXISTS `dy_convert`;
CREATE TABLE `dy_convert` (
  `userid` int(11) NOT NULL DEFAULT '0',
  `addname` varchar(45) DEFAULT '',
  `phone` varchar(45) DEFAULT '',
  `address1` varchar(256) DEFAULT '',
  `address2` varchar(256) DEFAULT '',
  `address3` varchar(256) DEFAULT '',
  `addselect` int(11) DEFAULT '0',
  PRIMARY KEY (`userid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for dy_drawconf
-- ----------------------------
DROP TABLE IF EXISTS `dy_drawconf`;
CREATE TABLE `dy_drawconf` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `drawid` int(11) NOT NULL DEFAULT '0',
  `drawtype` varchar(45) DEFAULT '',
  `channel` varchar(45) DEFAULT '',
  `name` varchar(45) DEFAULT '',
  `des` varchar(127) DEFAULT '',
  `wechat` int(11) DEFAULT '0',
  `alipay` int(11) DEFAULT '0',
  `bankcard` int(11) DEFAULT '0',
  `url` varchar(245) DEFAULT '',
  `state` int(11) DEFAULT '0',
  `feerate` int(11) DEFAULT '2' COMMENT '提现手续费，百分比，不支持小数位，默认 2 %',
  `minjetton` int(11) DEFAULT '0' COMMENT '最小提现额度',
  `userchannel` varchar(45) DEFAULT NULL COMMENT '渠道',
  `retain` int(11) DEFAULT '0' COMMENT '账户保留金币',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=4 DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for dy_exchangegifts
-- ----------------------------
DROP TABLE IF EXISTS `dy_exchangegifts`;
CREATE TABLE `dy_exchangegifts` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `userid` int(11) DEFAULT '0',
  `version` int(11) DEFAULT '0',
  `excode` varchar(45) DEFAULT '',
  `exdate` timestamp NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for dy_extrabanks
-- ----------------------------
DROP TABLE IF EXISTS `dy_extrabanks`;
CREATE TABLE `dy_extrabanks` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `userid` int(11) DEFAULT NULL COMMENT '玩家id',
  `bankacount` varchar(45) DEFAULT '' COMMENT '银行卡号',
  `bankname` varchar(45) DEFAULT '' COMMENT '银行名称',
  `bankaddress` varchar(128) DEFAULT '' COMMENT '银行地址',
  `bankprovince` varchar(128) DEFAULT '' COMMENT '银行卡的省',
  `bankcity` varchar(128) DEFAULT '' COMMENT '银行卡的市',
  `bankidnumber` varchar(128) DEFAULT '' COMMENT '银行卡的身份证号',
  `channel` varchar(50) DEFAULT '' COMMENT '渠道号',
  `prechannel` varchar(50) DEFAULT '' COMMENT '大渠道号',
  `bankpayee` varchar(45) DEFAULT '' COMMENT '收款人',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=43 DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for dy_footballorder
-- ----------------------------
DROP TABLE IF EXISTS `dy_footballorder`;
CREATE TABLE `dy_footballorder` (
  `orderid` bigint(11) NOT NULL AUTO_INCREMENT,
  `userid` int(11) DEFAULT '0' COMMENT '投注玩家id',
  `eventtype` varchar(50) DEFAULT NULL COMMENT '联赛类型',
  `subtype` int(11) DEFAULT NULL COMMENT '0:主场 1：客场 2：和',
  `yieltype` int(11) DEFAULT NULL COMMENT '类型 0:全场 1：半场',
  `typescore` varchar(30) DEFAULT NULL COMMENT '比分 格式"1-0" 其它是"99"',
  `yiel` varchar(10) DEFAULT NULL COMMENT '收益 "9.9"，获利率',
  `baoval` int(11) DEFAULT NULL COMMENT '0：不保 1: 保 2 :三星保',
  `tiyan` int(11) DEFAULT NULL COMMENT '0:否 1：是',
  `starttime` timestamp NULL DEFAULT NULL COMMENT '比赛开始时间',
  `endtime` datetime DEFAULT NULL COMMENT '比赛结束时间',
  `ordertime` timestamp NULL DEFAULT NULL COMMENT '下单时间',
  `orderstate` int(11) DEFAULT NULL COMMENT '订单状态 -1异常订单  0:未返利(新订单) 1：结算后亏损 2：结算后获利 3：返还本金（玩家撤单） 4：返还本金（赛事取消）',
  `fee` varchar(10) DEFAULT NULL COMMENT '手续费',
  `win` varchar(255) DEFAULT NULL COMMENT '预期收益',
  `hometeam` varchar(100) DEFAULT NULL COMMENT '主队',
  `awayteam` varchar(100) DEFAULT NULL COMMENT '客队',
  `homescore` int(11) DEFAULT NULL COMMENT '主队得分',
  `awayscore` int(11) DEFAULT NULL COMMENT '客队得分',
  `eventid` varchar(50) DEFAULT '' COMMENT '赛事id',
  `rebateId` varchar(50) DEFAULT '0' COMMENT '下注区域id',
  `pourjetton` varchar(32) DEFAULT '0' COMMENT '投注金额',
  `channel` varchar(32) DEFAULT '' COMMENT '渠道号',
  `repeatCount` int(11) DEFAULT '0' COMMENT '由于赛事重新结算导致的，重复计算',
  `pumpMoney` int(11) DEFAULT '0' COMMENT '这笔订单的抽水',
  `isBet` int(11) DEFAULT '0' COMMENT '0 猜比分 1 猜输赢',
  `wlpourtype` int(11) DEFAULT NULL COMMENT '输赢赛事下注区域  1：主, 2 平, 3 客',
  `raceid` varchar(50) DEFAULT NULL COMMENT '赛事id',
  `wlpourlist1` bigint(20) DEFAULT '0' COMMENT '猜胜负主胜下注额',
  `wlpourlist2` bigint(20) DEFAULT '0' COMMENT '猜胜负平局下注额',
  `wlpourlist3` bigint(20) DEFAULT '0' COMMENT '猜胜负客胜下注额',
  `prechannel` varchar(40) DEFAULT NULL COMMENT '大渠道',
  `ordertype` int(11) DEFAULT '0' COMMENT '注单类型 0真实订单 1体验金订单 2测试订单 3复活金订单',
  `schemeid` bigint(11) DEFAULT '0' COMMENT '方案id 大于0 跟投',
  `expertid` int(11) DEFAULT '0' COMMENT '专家id',
  `expert` varchar(50) DEFAULT NULL COMMENT '专家名字',
  `baobenmoney` bigint(20) DEFAULT '0' COMMENT '比分区域被设置最大金额1万元保本，A下注此区域10万元输了，A可以拿回1万元本金，其余金额归平台',
  `homelogo` varchar(255) DEFAULT NULL,
  `awaylogo` varchar(255) DEFAULT NULL,
  `cancletime` int(11) DEFAULT NULL COMMENT '时间戳',
  `planraceid` bigint(20) DEFAULT '0' COMMENT '跟投的方案赛事id',
  `isinsertorder` tinyint(4) DEFAULT '0' COMMENT '0:非插单 1插单',
  `cardid` varchar(50) DEFAULT NULL COMMENT '是否使用功能卡 空字符不用 其它通过接口获取功能相关信息',
  `ip` varchar(30) DEFAULT '未知' COMMENT 'IP地址',
  `usertype` int(11) DEFAULT '0' COMMENT '0 普通账户 1 内部测试账户 2 股东合伙人',
  `oprsys` varchar(100) DEFAULT '未知' COMMENT '操作平台',
  `canclefee` float DEFAULT '0' COMMENT '撤单手续费率 0.00',
  `counttime` datetime DEFAULT NULL COMMENT '订单结算时间',
  PRIMARY KEY (`orderid`),
  KEY `userid` (`userid`) USING BTREE,
  KEY `useridandexpertid` (`userid`,`expertid`) USING BTREE,
  KEY `planraceidanduserid` (`planraceid`,`userid`) USING BTREE,
  KEY `raceid` (`raceid`,`orderstate`) USING BTREE,
  KEY `lt_1` (`orderid`,`userid`,`orderstate`,`isBet`)
) ENGINE=InnoDB AUTO_INCREMENT=1006 DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for dy_friends
-- ----------------------------
DROP TABLE IF EXISTS `dy_friends`;
CREATE TABLE `dy_friends` (
  `userid` int(11) NOT NULL,
  `cratedate` timestamp NULL DEFAULT CURRENT_TIMESTAMP,
  `amount` int(11) DEFAULT '0',
  `friends` varchar(6000) DEFAULT '',
  PRIMARY KEY (`userid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for dy_gamelist
-- ----------------------------
DROP TABLE IF EXISTS `dy_gamelist`;
CREATE TABLE `dy_gamelist` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `gametype` int(11) DEFAULT '0',
  `tabletype` int(11) DEFAULT '0',
  `tableid` int(11) DEFAULT '0',
  `gamename` varchar(125) DEFAULT '',
  `tablename` varchar(125) DEFAULT '',
  `banklimit` int(11) DEFAULT '0' COMMENT '上庄限定',
  `enterlimit` int(11) DEFAULT '0' COMMENT '最小进入',
  `entermax` int(11) DEFAULT '0' COMMENT '最大进入，-1表示没有限制',
  `feerate` int(11) DEFAULT '0',
  `dayfee` int(11) DEFAULT '0',
  `weekfee` int(11) DEFAULT '0',
  `monthfee` int(11) DEFAULT '0',
  `allfee` int(11) DEFAULT '0',
  `isexist` int(11) DEFAULT '0',
  `robotswitch` int(11) DEFAULT '0' COMMENT '机器人开关 0 关 1 开',
  `robotnum` int(11) DEFAULT '0' COMMENT '机器人数量',
  `isrobotbanker` int(11) DEFAULT '0' COMMENT '机器人上庄 0 不上 1 上',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=27 DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for dy_gamestate
-- ----------------------------
DROP TABLE IF EXISTS `dy_gamestate`;
CREATE TABLE `dy_gamestate` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `icon` varchar(50) DEFAULT NULL COMMENT 'icon游戏值',
  `gametype` int(11) DEFAULT '0' COMMENT '游戏类型',
  `tabletype` int(11) DEFAULT '0' COMMENT '牌桌类型：初级，中级，高级，贵宾级',
  `state` int(11) DEFAULT '0' COMMENT '游戏开关：1 开启 2 关闭游戏运行 3关闭游戏也不允许',
  `taketime` int(11) DEFAULT '0' COMMENT '生效时间',
  `closegamelist` varchar(255) DEFAULT '[]' COMMENT '弃用',
  `gamename` varchar(45) DEFAULT '' COMMENT '游戏名字',
  `ip` varchar(45) DEFAULT '' COMMENT '游戏IP',
  `tcpport` int(11) DEFAULT '0' COMMENT 'tcp端口',
  `webport` int(11) DEFAULT '0' COMMENT 'webSocket端口',
  `abroadip` varchar(45) DEFAULT '' COMMENT '国外IP',
  `abroadtcpport` int(11) DEFAULT '0' COMMENT '国外tcp端口',
  `abroadwebport` int(11) DEFAULT '0' COMMENT '国外wb端口',
  `clientstate` int(11) DEFAULT '1' COMMENT '游戏前端的状态：1 正常 2 维护 3 火爆 4 冷清',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=91 DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for dy_goods
-- ----------------------------
DROP TABLE IF EXISTS `dy_goods`;
CREATE TABLE `dy_goods` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `userid` int(11) DEFAULT '0',
  `goodsid` int(11) DEFAULT '0',
  `amount` int(11) DEFAULT '0',
  `validity` int(11) DEFAULT '0' COMMENT '有效期：0是已经过期 9999是永久拥有',
  `createdate` datetime DEFAULT NULL,
  `iswear` tinyint(3) DEFAULT '0',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for dy_idcardverify
-- ----------------------------
DROP TABLE IF EXISTS `dy_idcardverify`;
CREATE TABLE `dy_idcardverify` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `userid` int(11) DEFAULT NULL COMMENT '用户id',
  `name` varchar(255) DEFAULT NULL COMMENT '用户名称',
  `idcard` varchar(255) DEFAULT NULL COMMENT '身份证号',
  `idcard_a` varchar(255) DEFAULT NULL COMMENT '身份证a',
  `idcard_b` varchar(255) DEFAULT NULL COMMENT '身份证b',
  `status` int(11) DEFAULT '0' COMMENT '0=未审核，1=审核中，2=已审核，3=已拒绝',
  `optid` int(11) DEFAULT NULL COMMENT '操作人id',
  `optname` varchar(255) DEFAULT NULL COMMENT '操作人名称',
  `created_at` datetime DEFAULT NULL COMMENT '创建时间',
  `channel` varchar(30) DEFAULT NULL,
  `prechannel` varchar(30) DEFAULT NULL,
  `update_at` datetime DEFAULT NULL COMMENT '操作时间',
  `remark` varchar(255) DEFAULT NULL COMMENT '备注',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=72 DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for dy_invite
-- ----------------------------
DROP TABLE IF EXISTS `dy_invite`;
CREATE TABLE `dy_invite` (
  `userid` int(11) NOT NULL DEFAULT '0' COMMENT '被邀请者的ID',
  `touserid` int(11) DEFAULT '0',
  `bindstate` int(11) DEFAULT '0',
  `binddate` varchar(32) CHARACTER SET latin1 DEFAULT '',
  PRIMARY KEY (`userid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='邀请码';

-- ----------------------------
-- Table structure for dy_legal
-- ----------------------------
DROP TABLE IF EXISTS `dy_legal`;
CREATE TABLE `dy_legal` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `legalkey` varchar(128) DEFAULT '' COMMENT '用于加密的字符串，由5个参数按照一定规则排列而成',
  `param1` varchar(128) DEFAULT '' COMMENT '参与加密的参数1',
  `param2` varchar(128) DEFAULT '' COMMENT '参与加密的参数2',
  `param3` varchar(128) DEFAULT '' COMMENT '参与加密的参数3',
  `param4` varchar(128) DEFAULT '' COMMENT '参与加密的参数4',
  `param5` varchar(128) DEFAULT '' COMMENT '参与加密的参数4',
  `legalsign` varchar(128) DEFAULT '' COMMENT '利用sha26取得的签名串',
  `craatedate` timestamp NULL DEFAULT CURRENT_TIMESTAMP,
  `state` int(11) DEFAULT '0' COMMENT '状态：0，没开始，1：已经开始，2：已经结束，可以查询',
  `cardlist` varchar(256) DEFAULT '' COMMENT '牌型列表',
  `checkurl` varchar(512) DEFAULT '',
  `tableid` int(11) DEFAULT '0',
  PRIMARY KEY (`id`),
  UNIQUE KEY `legalkey_UNIQUE` (`legalkey`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 ROW_FORMAT=DYNAMIC COMMENT='合法性校验';

-- ----------------------------
-- Table structure for dy_loginaward
-- ----------------------------
DROP TABLE IF EXISTS `dy_loginaward`;
CREATE TABLE `dy_loginaward` (
  `userid` int(11) NOT NULL,
  `onlineaward` varchar(129) DEFAULT '|' COMMENT '在线时长奖励',
  `continueaward` varchar(129) DEFAULT '|' COMMENT '连续登陆奖励',
  `signaward` varchar(512) DEFAULT '|' COMMENT '每天签到奖励',
  PRIMARY KEY (`userid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for dy_lottery_faker
-- ----------------------------
DROP TABLE IF EXISTS `dy_lottery_faker`;
CREATE TABLE `dy_lottery_faker` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `number_index` char(12) DEFAULT NULL,
  `prefix` char(4) DEFAULT NULL,
  `suffix` char(5) DEFAULT NULL COMMENT '后5五数字',
  `state` tinyint(1) DEFAULT '0' COMMENT '0=未修改，1=已修改',
  PRIMARY KEY (`id`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

-- ----------------------------
-- Table structure for dy_lottery_faker_base
-- ----------------------------
DROP TABLE IF EXISTS `dy_lottery_faker_base`;
CREATE TABLE `dy_lottery_faker_base` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `number_index` char(12) DEFAULT NULL,
  `prefix` varchar(255) DEFAULT NULL,
  `suffix` varchar(255) DEFAULT NULL COMMENT '后5五数字',
  `state` tinyint(1) DEFAULT '0' COMMENT '0=未修改，1=已修改',
  PRIMARY KEY (`id`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

-- ----------------------------
-- Table structure for dy_lottery_faker_copy1
-- ----------------------------
DROP TABLE IF EXISTS `dy_lottery_faker_copy1`;
CREATE TABLE `dy_lottery_faker_copy1` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `number_index` char(12) DEFAULT NULL,
  `prefix` char(4) DEFAULT NULL,
  `suffix` char(5) DEFAULT NULL COMMENT '后5五数字',
  `state` tinyint(1) DEFAULT '0' COMMENT '0=未修改，1=已修改',
  PRIMARY KEY (`id`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

-- ----------------------------
-- Table structure for dy_luckwheel
-- ----------------------------
DROP TABLE IF EXISTS `dy_luckwheel`;
CREATE TABLE `dy_luckwheel` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `lucknum` int(11) NOT NULL COMMENT '期号',
  `userid` int(11) DEFAULT NULL,
  `restate` int(11) DEFAULT NULL,
  `jettonlist` varchar(100) DEFAULT NULL,
  `rejetton` int(11) DEFAULT NULL,
  `achievement` varchar(20) DEFAULT NULL,
  `createdate` timestamp NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  `updatedate` timestamp NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  PRIMARY KEY (`id`,`lucknum`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 ROW_FORMAT=DYNAMIC;

-- ----------------------------
-- Table structure for dy_mail
-- ----------------------------
DROP TABLE IF EXISTS `dy_mail`;
CREATE TABLE `dy_mail` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `senderid` int(11) DEFAULT '0',
  `sender` varchar(45) DEFAULT '',
  `receiverid` int(11) DEFAULT '0',
  `receiver` varchar(45) DEFAULT '',
  `mailtype` tinyint(3) DEFAULT '0' COMMENT '邮件类型',
  `title` varchar(45) DEFAULT '',
  `content` varchar(512) DEFAULT '',
  `senddate` timestamp NULL DEFAULT CURRENT_TIMESTAMP,
  `markdate` timestamp NULL DEFAULT NULL,
  `mailstate` tinyint(2) DEFAULT '1' COMMENT '邮件状态:0：已经删除；1：未读；2：有附件未读；3：有附件已读；4：已读已领取。',
  `externdata` varchar(256) DEFAULT '',
  `remark` varchar(45) DEFAULT '',
  `validity` tinyint(3) DEFAULT '0' COMMENT '邮件的有效期，-1表示无期限。',
  `channel` varchar(20) DEFAULT '' COMMENT '渠道',
  PRIMARY KEY (`id`),
  KEY `senderid` (`senderid`),
  KEY `receiverid` (`receiverid`)
) ENGINE=InnoDB AUTO_INCREMENT=2616 DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for dy_mission
-- ----------------------------
DROP TABLE IF EXISTS `dy_mission`;
CREATE TABLE `dy_mission` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `userid` int(11) DEFAULT '0',
  `missionid` int(11) DEFAULT '0',
  `createdate` timestamp NULL DEFAULT CURRENT_TIMESTAMP,
  `finish` int(11) DEFAULT '0',
  `state` int(11) DEFAULT '0',
  PRIMARY KEY (`id`),
  KEY `userid` (`userid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for dy_notice
-- ----------------------------
DROP TABLE IF EXISTS `dy_notice`;
CREATE TABLE `dy_notice` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `title` varchar(255) COLLATE utf8mb4_unicode_ci DEFAULT '' COMMENT '标题',
  `status` int(11) DEFAULT '1' COMMENT '状态',
  `type` int(11) DEFAULT NULL COMMENT '类型',
  `createdate` varchar(40) COLLATE utf8mb4_unicode_ci DEFAULT '' COMMENT '创建时间',
  `descript` text COLLATE utf8mb4_unicode_ci,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=2 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- ----------------------------
-- Table structure for dy_order
-- ----------------------------
DROP TABLE IF EXISTS `dy_order`;
CREATE TABLE `dy_order` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `userid` int(11) DEFAULT '0',
  `buyid` int(11) DEFAULT '0',
  `timeid` int(11) DEFAULT '0',
  `status` tinyint(3) DEFAULT '0',
  `channel` varchar(45) DEFAULT NULL,
  `createtime` timestamp NULL DEFAULT CURRENT_TIMESTAMP,
  `sign` varchar(45) DEFAULT '' COMMENT '腾讯需要的字符字符串，在这里是MD5生成的字符串',
  `shoptype` int(11) DEFAULT '0' COMMENT '购买的入口类型：默认情况下是0和1是钻石，2是商店，为了区分不同的渠道而定制的',
  `paymoney` int(11) DEFAULT '0',
  `jettontype` int(11) DEFAULT '0' COMMENT '区分是哪里调起的 1 jetton 2 gamejetton 3 cpjetton',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=9781 DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for dy_order_offline
-- ----------------------------
DROP TABLE IF EXISTS `dy_order_offline`;
CREATE TABLE `dy_order_offline` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `userid` int(11) DEFAULT '0' COMMENT '充值人游戏ID',
  `nickname` varchar(45) DEFAULT '' COMMENT '充值人游戏昵称',
  `paytype` int(11) DEFAULT '0' COMMENT '1 支付宝 2 微信 3 银行卡 ',
  `payname` varchar(255) DEFAULT '' COMMENT '类型名字 如支付宝 微信',
  `payee` varchar(255) DEFAULT '' COMMENT '收款人名字',
  `account` varchar(255) DEFAULT '' COMMENT '收款账号',
  `bankername` varchar(255) DEFAULT '' COMMENT '银行名字(不是银行卡转账就是空的）',
  `receivablescode` varchar(255) DEFAULT '' COMMENT '收款二维码',
  `payer` varchar(255) DEFAULT '' COMMENT '付款人',
  `payvoucher` varchar(255) DEFAULT '' COMMENT '支付凭证',
  `payerremarks` varchar(1024) DEFAULT '' COMMENT '付款人备注',
  `state` int(11) DEFAULT '0' COMMENT '1：发起，2：审核中，3：成功，4：失败',
  `rsdate` timestamp NULL DEFAULT CURRENT_TIMESTAMP COMMENT '创建时间',
  `orderid` varchar(128) DEFAULT '' COMMENT '订单号',
  `opttime` datetime DEFAULT NULL COMMENT '操作的时间',
  `remark` varchar(255) DEFAULT '' COMMENT '操作人备注',
  `channel` varchar(60) DEFAULT '' COMMENT '渠道',
  `optid` int(11) DEFAULT NULL COMMENT '操作人id',
  `prechannel` varchar(60) DEFAULT '' COMMENT '大渠道号',
  `jetton` int(11) DEFAULT '0' COMMENT '充值金额',
  `jettontype` int(11) DEFAULT '0' COMMENT '区分哪里调的， 1 jetton 2 gamejetton 3 cpjetton',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=477 DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for dy_orderinfo
-- ----------------------------
DROP TABLE IF EXISTS `dy_orderinfo`;
CREATE TABLE `dy_orderinfo` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `userid` int(11) DEFAULT '0',
  `payid` int(11) DEFAULT '0',
  `orderid` int(11) DEFAULT '0',
  `channel` varchar(45) DEFAULT '0',
  `nonce_str` varchar(45) DEFAULT NULL,
  `time_start` varchar(45) DEFAULT NULL,
  `time_expire` varchar(45) DEFAULT NULL,
  `total_fee` int(11) DEFAULT '0',
  `state` int(11) DEFAULT '0' COMMENT '0:发起支付请求;1:收到支付回调，支付成功，-1：返回失败',
  `ctreatedate` timestamp NULL DEFAULT CURRENT_TIMESTAMP,
  `out_trade_no` varchar(45) DEFAULT NULL,
  `ch_ordderid` varchar(45) DEFAULT NULL COMMENT '渠道获得的id',
  `remark` varchar(45) DEFAULT NULL,
  `paytype` varchar(45) DEFAULT NULL COMMENT '支付宝，微信',
  `shoptype` varchar(45) DEFAULT '',
  `plat_order_id` varchar(128) DEFAULT '' COMMENT '平台订单号',
  `firstpay` int(11) DEFAULT '0' COMMENT '首充标记 1是   0 不是',
  `prechannel` varchar(255) DEFAULT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=1543 DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for dy_pay_shop_map
-- ----------------------------
DROP TABLE IF EXISTS `dy_pay_shop_map`;
CREATE TABLE `dy_pay_shop_map` (
  `pay_conf_id` int(11) NOT NULL COMMENT '支付通道id',
  `shop_conf_id` int(11) NOT NULL COMMENT '商品id'
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- ----------------------------
-- Table structure for dy_payconf
-- ----------------------------
DROP TABLE IF EXISTS `dy_payconf`;
CREATE TABLE `dy_payconf` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `payid` int(11) DEFAULT '0',
  `paytype` varchar(45) DEFAULT '',
  `channel` varchar(45) DEFAULT '',
  `name` varchar(45) DEFAULT '',
  `wechat` int(11) DEFAULT '0',
  `alipay` int(11) DEFAULT '0',
  `bankcard` int(11) DEFAULT '0',
  `des` varchar(127) DEFAULT '',
  `url` varchar(255) DEFAULT '' COMMENT '支付调起URL',
  `notifyurl` varchar(255) DEFAULT NULL COMMENT '支付回调URL',
  `appkey` varchar(255) DEFAULT NULL COMMENT '通道分配appkey',
  `appid` varchar(11) DEFAULT '0' COMMENT '通道分配商户appid',
  `appmark` varchar(255) DEFAULT NULL COMMENT '标识',
  `appencrymode` varchar(40) DEFAULT NULL COMMENT '通道加密方式',
  `state` varchar(45) DEFAULT '',
  `shoptype` int(11) DEFAULT '0',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=26 DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for dy_payrate
-- ----------------------------
DROP TABLE IF EXISTS `dy_payrate`;
CREATE TABLE `dy_payrate` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `payid` int(11) DEFAULT '0',
  `addtype` int(11) DEFAULT '0' COMMENT '加成的类型，：1：按照固定数量加成；2：按照固定比例加成',
  `addamount` int(11) DEFAULT '0' COMMENT '加成的数量',
  `adddes` int(11) DEFAULT '0' COMMENT '加成的描述',
  `addtime` datetime DEFAULT NULL COMMENT '加成的截止日期',
  `createtime` timestamp NULL DEFAULT CURRENT_TIMESTAMP,
  `isexist` int(11) DEFAULT '1',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for dy_player
-- ----------------------------
DROP TABLE IF EXISTS `dy_player`;
CREATE TABLE `dy_player` (
  `userid` int(11) NOT NULL AUTO_INCREMENT,
  `cid` varchar(45) NOT NULL COMMENT '设备唯一的标识',
  `account` varchar(255) NOT NULL COMMENT '账号，不对唯一性做检查',
  `password` varchar(45) NOT NULL,
  `nickname` varchar(45) DEFAULT NULL,
  `face_1` varchar(45) NOT NULL,
  `face_2` varchar(45) DEFAULT '',
  `face_3` varchar(45) DEFAULT '',
  `face_4` varchar(45) DEFAULT '',
  `regdate` timestamp NULL DEFAULT CURRENT_TIMESTAMP,
  `sex` int(11) DEFAULT '0',
  `age` int(11) DEFAULT '0',
  `email` varchar(45) DEFAULT '',
  `phonenum` varchar(45) DEFAULT '',
  `exp` int(11) DEFAULT '0',
  `level` int(11) DEFAULT '0',
  `viplevel` int(11) DEFAULT '0',
  `jetton` bigint(45) DEFAULT '0',
  `gold` int(11) DEFAULT '0',
  `money` int(11) DEFAULT '0',
  `channel` varchar(45) DEFAULT '',
  `province` varchar(45) DEFAULT '',
  `city` varchar(45) DEFAULT '',
  `popularity` int(11) DEFAULT '0',
  `idenstatus` int(11) DEFAULT '0',
  `bindtype` int(11) DEFAULT '0',
  `bindnick` varchar(45) DEFAULT '',
  `platformid` varchar(45) DEFAULT '' COMMENT 'android,ios',
  `imei` varchar(45) DEFAULT '',
  `devname` varchar(45) DEFAULT '',
  `macname` varchar(45) DEFAULT '',
  `mobiletype` int(11) DEFAULT '0',
  `lasttime` int(11) DEFAULT '0',
  `penulttime` int(11) DEFAULT '0',
  `silent` int(11) DEFAULT '0',
  `isban` int(11) DEFAULT '0',
  `isrobot` int(11) DEFAULT '0',
  `playtype` smallint(4) DEFAULT '1' COMMENT '1:超级保守型，需要很好的牌才跟或者all in；--2：保守型，会点牌技，一般好的牌就会跟了。--3：一般型，没有特别的，下牌全凭喜好；4:激进型，会点牌技，有点激进。5:超级激进型，只要拿到稍微好的牌，就会疯狂加注',
  `description` varchar(129) DEFAULT '',
  `blacklist` int(11) DEFAULT '0' COMMENT '列入黑名单',
  `propertyid` int(11) DEFAULT '0' COMMENT '佩戴物品的ID',
  `agencystatus` int(11) DEFAULT '0',
  `bank_password` varchar(32) DEFAULT NULL,
  `bank_jetton` bigint(40) DEFAULT '0',
  `ip` varchar(45) DEFAULT NULL,
  `sx_jetton` bigint(40) DEFAULT '0',
  `tyjetton` bigint(40) DEFAULT '0' COMMENT '体验金',
  `prechannel` varchar(255) DEFAULT '' COMMENT '大渠道号',
  `limitcount` int(11) DEFAULT '0' COMMENT '场次限制',
  `limitarch` bigint(40) DEFAULT '0' COMMENT '流水限制',
  `ipaddr` varchar(100) DEFAULT '未知' COMMENT 'IP地址所在地',
  `oprsys` varchar(100) DEFAULT '未知' COMMENT '操作平台',
  `idcardstatus` int(1) DEFAULT '0' COMMENT '0=未提交，1=审核中，2=已审核 3=审核失败',
  `gamejetton` int(11) DEFAULT '0' COMMENT '游戏钱包',
  `frozenjetton` int(11) DEFAULT '0' COMMENT '已冻结资金',
  `usertype` int(11) DEFAULT '0' COMMENT '0 普通账户 1 内部测试账户 2 股东合伙人',
  `realname` varchar(50) DEFAULT '' COMMENT '真实姓名',
  `cpjetton` bigint(20) DEFAULT '0' COMMENT '彩票币',
  `transfer` int(11) DEFAULT '0' COMMENT '0 不可转账 1 可以转账',
  PRIMARY KEY (`userid`)
) ENGINE=InnoDB AUTO_INCREMENT=102633 DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for dy_playerattentionrace
-- ----------------------------
DROP TABLE IF EXISTS `dy_playerattentionrace`;
CREATE TABLE `dy_playerattentionrace` (
  `id` int(11) NOT NULL,
  `raceid` varchar(50) DEFAULT NULL COMMENT '赛事id',
  `userid` int(11) DEFAULT NULL COMMENT '玩家id',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for dy_playermsg
-- ----------------------------
DROP TABLE IF EXISTS `dy_playermsg`;
CREATE TABLE `dy_playermsg` (
  `userid` int(11) NOT NULL,
  `nickname` varchar(45) DEFAULT '',
  `mdynickamount` smallint(5) DEFAULT '0' COMMENT '修改昵称的次数',
  `loginday` int(11) DEFAULT '1',
  `loginconday` int(11) DEFAULT '1',
  `maxloginconday` int(11) DEFAULT '1',
  `loginamount` int(11) DEFAULT '1',
  `onlinetime` int(11) DEFAULT '0',
  `maxjetton` int(11) DEFAULT '0',
  `payrmb` int(11) DEFAULT '0',
  `callpayrmb` int(11) DEFAULT '0',
  `paytime` int(11) DEFAULT '0',
  `callpaytime` int(11) DEFAULT '0',
  `maxmoney` int(11) DEFAULT '0',
  `dayrmb` int(11) DEFAULT '0',
  `daypaytime` int(11) DEFAULT '0',
  `lastlogin` timestamp NULL DEFAULT '1999-12-30 08:00:00',
  `lastpay` timestamp NULL DEFAULT '1999-12-30 08:00:00',
  PRIMARY KEY (`userid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for dy_pond
-- ----------------------------
DROP TABLE IF EXISTS `dy_pond`;
CREATE TABLE `dy_pond` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `gametype` int(11) DEFAULT '0' COMMENT '游戏类型',
  `tabletype` int(11) DEFAULT '0' COMMENT '牌桌类型：初级，中级，高级，贵宾级',
  `jiangjetton` bigint(20) DEFAULT '0',
  `caijetton` bigint(20) DEFAULT '0',
  `level_1` int(11) DEFAULT '0',
  `rate_1` int(11) DEFAULT '0',
  `level_2` int(11) DEFAULT '0',
  `rate_2` int(11) DEFAULT '0',
  `level_3` int(11) DEFAULT '0',
  `rate_3` int(11) DEFAULT '0',
  `level_4` int(11) DEFAULT '0',
  `rate_4` int(11) DEFAULT '0',
  `taxrate` int(11) DEFAULT '0',
  `taxnum` bigint(20) DEFAULT '0',
  `jiangremark` bigint(20) DEFAULT '0',
  `gradetype` int(11) DEFAULT '0',
  `taxday` bigint(20) DEFAULT '0',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=877 DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for dy_propinfo
-- ----------------------------
DROP TABLE IF EXISTS `dy_propinfo`;
CREATE TABLE `dy_propinfo` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `proptype` int(11) DEFAULT '0' COMMENT '1 保本 2 抽奖 3 体验金 4 添利 5 神偷 6红包',
  `propname` varchar(255) DEFAULT '' COMMENT '卡券名字',
  `propvalue` int(11) DEFAULT '0' COMMENT '道具面值',
  `proportion` int(11) DEFAULT '0' COMMENT '使用条件x%',
  `effectivetime` int(11) DEFAULT '0' COMMENT '有效时间',
  `remarks` varchar(255) DEFAULT NULL,
  `createtime` timestamp NULL DEFAULT NULL ON UPDATE CURRENT_TIMESTAMP,
  `createid` int(11) DEFAULT '0' COMMENT '创建者',
  `state` int(11) DEFAULT '0' COMMENT '0 创建 1 已发送 2 删除 ',
  `sendtype` int(11) DEFAULT '0' COMMENT '1 系统发放 2 活动发放 3 翻翻乐发放 4 任务发放 5 抽奖发放',
  `sendobject` int(11) DEFAULT '0' COMMENT '1 会员 2 代理 3 专家 4 新用户 5 指定ID',
  `senddata` varchar(255) DEFAULT '' COMMENT '根据发送对象的对应参数',
  `totalnum` int(11) DEFAULT '0' COMMENT '总数量',
  `receivenum` int(11) DEFAULT '0' COMMENT '领取数量',
  `surplusnum` int(11) DEFAULT '0' COMMENT '剩余数量',
  `usenum` int(11) DEFAULT '0' COMMENT '已使用数量',
  `optid` int(11) DEFAULT '0' COMMENT '操作人ID',
  `sendtime` timestamp NULL DEFAULT NULL ON UPDATE CURRENT_TIMESTAMP COMMENT '更新时间',
  `channel` varchar(255) DEFAULT '' COMMENT '渠道号',
  `prechannel` varchar(255) DEFAULT '' COMMENT '大渠道号',
  `sendprobability` int(11) DEFAULT '0' COMMENT '翻翻乐发放免费概率',
  `propid` varchar(255) DEFAULT '' COMMENT '道具ID',
  `sendpaidprobability` int(11) DEFAULT '0' COMMENT '翻翻乐消费概率',
  `prizeid` varchar(255) DEFAULT '' COMMENT '抽奖券自定中奖ID',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=831 DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for dy_proporder
-- ----------------------------
DROP TABLE IF EXISTS `dy_proporder`;
CREATE TABLE `dy_proporder` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `fatherid` varchar(255) DEFAULT '' COMMENT '所属的道具id',
  `selfid` varchar(255) DEFAULT '' COMMENT '领取的道具ID',
  `userid` int(11) DEFAULT '0' COMMENT '领取玩家ID',
  `nickname` varchar(255) DEFAULT '' COMMENT '昵称',
  `receivedate` timestamp NULL DEFAULT NULL COMMENT '领取时间',
  `state` int(11) DEFAULT '0' COMMENT '0 未使用， 1 已使用',
  `usedate` timestamp NULL DEFAULT NULL COMMENT '使用时间',
  `pourorderid` varchar(255) DEFAULT '0' COMMENT '使用的下注的订单',
  `channel` varchar(255) DEFAULT '' COMMENT '渠道号',
  `prechannel` varchar(255) DEFAULT '' COMMENT '大渠道号',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=1315 DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for dy_qpgameorder
-- ----------------------------
DROP TABLE IF EXISTS `dy_qpgameorder`;
CREATE TABLE `dy_qpgameorder` (
  `id` int(11) NOT NULL AUTO_INCREMENT COMMENT '订单id',
  `orderid` varchar(255) DEFAULT '',
  `gid` varchar(32) NOT NULL COMMENT '开奖期号',
  `gamekey` int(11) NOT NULL COMMENT '彩种',
  `gametype` int(11) DEFAULT '0' COMMENT '游戏类型',
  `userid` varchar(50) NOT NULL,
  `pourcount` int(11) NOT NULL COMMENT '所有区域下注总金额',
  `state` int(11) DEFAULT '0' COMMENT '0：生成 1: 已结算 2：玩家已撤单  3：系统撤单',
  `createtime` int(11) NOT NULL DEFAULT '0' COMMENT '下注时间',
  `counttime` int(11) DEFAULT '0' COMMENT '结算时间',
  `win` int(11) DEFAULT '0' COMMENT '盈利情况',
  `isRobot` int(11) DEFAULT '0' COMMENT '0 真人 1 机器人',
  `channel` varchar(255) DEFAULT '' COMMENT '渠道号',
  `prechannel` varchar(255) DEFAULT '' COMMENT '大渠道号',
  `isBanker` int(11) DEFAULT '0' COMMENT '0 无庄 1 有庄',
  `gamenumber` varchar(255) DEFAULT '' COMMENT '牌局编号',
  `freeze` int(11) DEFAULT '0' COMMENT '冻结资金',
  `tabletype` int(11) DEFAULT '0' COMMENT '牌桌类型',
  `pourdetails` varchar(1024) DEFAULT '' COMMENT '下注详情',
  `bankerorder` int(11) DEFAULT '0' COMMENT '0 下注订单， 1 庄家订单',
  PRIMARY KEY (`id`),
  KEY `orderid` (`orderid`) USING BTREE,
  KEY `gamenumber` (`gamenumber`) USING BTREE,
  KEY `userid` (`userid`) USING BTREE
) ENGINE=MyISAM AUTO_INCREMENT=1150129 DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for dy_raceconfig
-- ----------------------------
DROP TABLE IF EXISTS `dy_raceconfig`;
CREATE TABLE `dy_raceconfig` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `channel` varchar(45) DEFAULT '' COMMENT '渠道号',
  `prechannel` varchar(45) DEFAULT '' COMMENT '大渠道',
  `firstratio` int(11) DEFAULT '80' COMMENT '初始收益百分比',
  `starttime` int(11) DEFAULT '12' COMMENT '开赛时间，单位小时',
  `timeonce` int(11) DEFAULT '5' COMMENT '跳水间隔，单位分钟',
  `timetype` tinyint(4) DEFAULT '0' COMMENT '0:表示12小时内  1:12小时外',
  `nopourscoremin` decimal(11,5) DEFAULT '0.00200' COMMENT '非投注区间浮动下限',
  `nopourscoremax` decimal(11,5) DEFAULT '0.02000' COMMENT '非投注区间浮动上限',
  `upperlimit` decimal(11,5) DEFAULT '0.04000' COMMENT '上浮限制',
  `pourtouch` int(11) DEFAULT '500000' COMMENT '投注触发跳水',
  `pourscoremin` decimal(11,5) DEFAULT '0.00500' COMMENT '投注区间浮动下限，比如投注5000，收益下跌0.005',
  `pourscoremax` decimal(11,5) DEFAULT '0.02000' COMMENT '投注区间浮动上限，比如投注5000，收益下跌0.02',
  `closescore` decimal(11,5) DEFAULT '0.40000' COMMENT '关盘赔率，比如当收益率下跌到0.4的时候，关盘',
  `pourplanscoremin` decimal(11,5) DEFAULT '0.00100' COMMENT '跟投区间浮动下限，比如跟投5000，收益下跌0.001',
  `pourplanscoremax` decimal(11,5) DEFAULT '0.00500' COMMENT '跟投区间浮动上限，比如跟投5000，收益下跌0.005',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=9 DEFAULT CHARSET=utf8 COMMENT='跳水配置表';

-- ----------------------------
-- Table structure for dy_raceinfo
-- ----------------------------
DROP TABLE IF EXISTS `dy_raceinfo`;
CREATE TABLE `dy_raceinfo` (
  `id` varchar(20) NOT NULL DEFAULT '' COMMENT '赛事ID(此ID为第三方ID)',
  `category` varchar(50) NOT NULL COMMENT '类别',
  `start_time` datetime NOT NULL COMMENT '开赛时间(换算成东八区时间) 开赛之后不许再投注',
  `last_time` datetime DEFAULT NULL COMMENT '上次获取的开赛时间(东八区时间) 只有开赛时间变化，此字段才设值',
  `end_time` datetime DEFAULT NULL COMMENT '结束时间(换算成东八区时间)',
  `half_time` datetime DEFAULT NULL COMMENT '半场结束时间',
  `home_team` varchar(20) NOT NULL COMMENT '主队',
  `visit_team` varchar(20) NOT NULL COMMENT '客队',
  `create_time` datetime NOT NULL COMMENT '创建时间',
  `update_time` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  `shelves_status` int(1) NOT NULL DEFAULT '0' COMMENT '上架状态(0:未上架 1:已上架)',
  `race_status` int(1) NOT NULL DEFAULT '3' COMMENT '赛事状态:(0:取消(取消的赛事进行撤注操作) 1:正常进行中 2:已经结束(定时任务爬取赛事结果)3:未开始 4:赛事改期5赛事异常 6手动取消 7系统撤单 )',
  `is_valid` int(1) NOT NULL DEFAULT '1' COMMENT '是否有效，赛事取消或异常时值为0（0：无效 1：有效）',
  `win_team` varchar(20) DEFAULT NULL COMMENT '获胜队伍(如果平局 填写两个队的名称,以英文,隔开)',
  `win_result` varchar(20) DEFAULT NULL COMMENT '赛果eg: 1:2',
  `win_type` int(1) DEFAULT NULL COMMENT '赛果类型:(0:平局 1:主队胜 2:客队胜,3：暂无结果)',
  `half_result` varchar(20) DEFAULT NULL COMMENT '半场结果 eg(1:2)',
  `is_recommend` int(1) DEFAULT '0' COMMENT '是否推荐（0不推荐，1推荐）',
  `weight` decimal(10,2) DEFAULT '0.00',
  `open_status` int(1) DEFAULT '0' COMMENT '是否可以投注（0不可以，1可以）',
  `data_source` int(1) DEFAULT '1' COMMENT '数据来源(1.API接口 2.网络爬虫 3.手工录入 4.手动结算 5赛事回滚)',
  `again_settle` int(4) DEFAULT '0' COMMENT '全场结算状态(0未结算，1手动结算，2自动结算，3重新结算，4已取消，5赛事回滚)',
  `remark` varchar(1000) DEFAULT NULL COMMENT '备注信息',
  `half_settle` int(4) DEFAULT '0' COMMENT '半场结算状态(0未结算，1手动结算，2自动结算，3重新结算，4已取消，5赛事回滚)',
  `version_roll` varchar(2) DEFAULT '0' COMMENT '赛事回滚版本号',
  `version_again` varchar(2) DEFAULT '0' COMMENT '赛事重新结算版本号',
  `channel` varchar(32) DEFAULT '' COMMENT '渠道号',
  `is_bet` tinyint(4) DEFAULT '0' COMMENT '是否赌输赢，控制玩法',
  `fee` int(11) DEFAULT '10' COMMENT '赛事抽水 默认10%',
  `raceTotalBet` bigint(20) DEFAULT '100000' COMMENT '赛事最大可下注',
  `betfee` int(11) DEFAULT '10' COMMENT '猜输赢玩法的，抽水值',
  `homeTeamFace` varchar(255) DEFAULT '' COMMENT '主对头像',
  `visitTeamFace` varchar(255) DEFAULT '' COMMENT '客队头像',
  `prechannel` varchar(50) DEFAULT '0' COMMENT '大渠道',
  `totalBet` bigint(20) DEFAULT '0' COMMENT '总下注',
  `matchId` varchar(20) DEFAULT '' COMMENT '比赛id(与raceId不同,为新增的数据分析和文字直播需要) ,',
  `seasonId` varchar(20) DEFAULT '' COMMENT '联赛赛季ID',
  `homeId` varchar(20) DEFAULT '' COMMENT '主队id',
  `visitId` varchar(20) DEFAULT '' COMMENT 'visitId',
  `allwinmoney` bigint(20) DEFAULT '0' COMMENT '全场输赢',
  `halfwinmoney` bigint(20) DEFAULT '0' COMMENT '半场输赢',
  PRIMARY KEY (`id`),
  UNIQUE KEY `idx_id` (`id`) USING BTREE,
  KEY `idx_start_time` (`start_time`) USING BTREE,
  KEY `idx_race_status` (`race_status`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='赛事信息表';

-- ----------------------------
-- Table structure for dy_racerebateinfo
-- ----------------------------
DROP TABLE IF EXISTS `dy_racerebateinfo`;
CREATE TABLE `dy_racerebateinfo` (
  `id` varchar(40) NOT NULL COMMENT '返利id',
  `race_id` varchar(20) DEFAULT '0' COMMENT '比赛ID',
  `rule` varchar(2048) DEFAULT '' COMMENT '返利规则',
  `rule_type` int(11) DEFAULT '1' COMMENT '返利规则类型(1:波胆 参考betTypeEnum)',
  `create_time` datetime DEFAULT NULL COMMENT '创建时间',
  `update_time` datetime DEFAULT NULL,
  `valid_amount` decimal(10,2) DEFAULT '1000000.00' COMMENT '可下单量',
  `open_status` int(11) DEFAULT '0' COMMENT '是否可以投注(1:是 0:否)',
  `modifier` varchar(50) DEFAULT '' COMMENT '修改人',
  `channel` varchar(32) DEFAULT '' COMMENT '渠道号',
  `baotype` tinyint(4) DEFAULT '0' COMMENT '0：不保 1: 保 2 :三星保',
  `tiyan` tinyint(4) DEFAULT '0' COMMENT '是否可下体验金 0:否 1：是',
  `baobenMoney` int(11) DEFAULT '0' COMMENT '0-1比分被设置最大金额1万元保本，A下注此区域10万元输了，A可以拿回1万元本金，其余金额归平台',
  `rebateRatio` double DEFAULT '0' COMMENT '收益率',
  `totalBet` bigint(12) DEFAULT '0' COMMENT '真钱下单量',
  `prechannel` varchar(50) DEFAULT '0' COMMENT '大渠道',
  `tytotalBet` bigint(12) DEFAULT '0' COMMENT '体验金下单量',
  `guesswinmoney` decimal(12,2) DEFAULT '0.00' COMMENT '赛事预期输赢',
  `winmoney` decimal(12,2) DEFAULT '0.00' COMMENT '比分收益',
  PRIMARY KEY (`id`),
  KEY `race_id_idx` (`race_id`) USING BTREE,
  KEY `rule_type_idx` (`rule_type`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='赛事返利率信息表';

-- ----------------------------
-- Table structure for dy_rewardconfig
-- ----------------------------
DROP TABLE IF EXISTS `dy_rewardconfig`;
CREATE TABLE `dy_rewardconfig` (
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `channel` varchar(50) DEFAULT '' COMMENT '渠道号',
  `prechannel` varchar(50) DEFAULT '' COMMENT '大渠道',
  `tonghua` tinyint(2) DEFAULT '1' COMMENT '是否包含同花顺牛1：是 0：否',
  `tonghuarate` decimal(10,2) DEFAULT '8.00' COMMENT '同花顺占比',
  `wuxiao` tinyint(2) DEFAULT '1' COMMENT '是否包含五小牛1：是 0：否',
  `wuxiaorate` decimal(10,2) DEFAULT '6.00' COMMENT '五小牛占比',
  `zhadan` tinyint(2) DEFAULT '1' COMMENT '是否包含炸弹牛1：是 0：否',
  `zhadanrate` decimal(10,2) DEFAULT '2.00' COMMENT '炸弹牛占比',
  `hulu` tinyint(2) DEFAULT '0' COMMENT '是否包含葫芦牛1：是 0：否',
  `hulurate` decimal(10,2) DEFAULT '0.00' COMMENT '葫芦牛占比',
  `wuhua` tinyint(2) DEFAULT '0' COMMENT '是否包含葫芦牛1：是 0：否',
  `wuhuarate` decimal(10,2) DEFAULT '0.00' COMMENT '葫芦牛占比',
  `shunzi` tinyint(2) DEFAULT '0' COMMENT '是否包含顺子牛1：是 0：否',
  `shunzirate` decimal(10,2) DEFAULT '0.00' COMMENT '顺子牛占比',
  `gametype` int(11) DEFAULT '0' COMMENT '游戏类型',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=3 DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for dy_rewardpool
-- ----------------------------
DROP TABLE IF EXISTS `dy_rewardpool`;
CREATE TABLE `dy_rewardpool` (
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `gametype` int(11) NOT NULL DEFAULT '0' COMMENT '游戏类型',
  `gamename` varchar(64) DEFAULT '' COMMENT '游戏名字',
  `totalamount` bigint(20) DEFAULT '0' COMMENT '奖池总额',
  `useramount` bigint(20) DEFAULT '0' COMMENT '玩家贡献总额',
  `addamount` bigint(20) DEFAULT '0' COMMENT '添加总额',
  `giveoutamount` bigint(20) DEFAULT '0' COMMENT '发放总额',
  `conditiondes` varchar(256) DEFAULT '' COMMENT '条件描述，排序奖金占比',
  `modifier` varchar(50) DEFAULT '' COMMENT '修改人',
  `updatetime` bigint(20) DEFAULT '0' COMMENT '更新时间',
  `status` tinyint(4) DEFAULT '1' COMMENT '状态，0：关闭 1：开启',
  `channel` varchar(50) DEFAULT '' COMMENT '渠道号',
  `prechannel` varchar(50) DEFAULT '' COMMENT '大渠道',
  `useramountrobot` bigint(20) DEFAULT '0' COMMENT '机器人贡献总额',
  `giveoutamountrobot` bigint(20) DEFAULT '0' COMMENT '机器人发放总额',
  PRIMARY KEY (`id`),
  KEY `gametype` (`gametype`),
  KEY `gametypeandch` (`gametype`,`channel`)
) ENGINE=InnoDB AUTO_INCREMENT=9 DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for dy_roster
-- ----------------------------
DROP TABLE IF EXISTS `dy_roster`;
CREATE TABLE `dy_roster` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `userid` int(11) DEFAULT '0',
  `nickname` varchar(145) DEFAULT '',
  `createdate` timestamp NULL DEFAULT CURRENT_TIMESTAMP,
  `rostertype` int(11) DEFAULT '0' COMMENT '1:黑名单，2: 白名单',
  `starttime` int(11) DEFAULT '0' COMMENT '开始时间',
  `endtime` int(11) DEFAULT '0' COMMENT '结束时间',
  `gamelist` varchar(2048) DEFAULT '' COMMENT '游戏列表，json格式',
  `state` int(11) DEFAULT '0' COMMENT '状态',
  `gametype` int(11) DEFAULT '0' COMMENT '游戏类型',
  `tabletype` int(11) DEFAULT '0' COMMENT '牌桌类型',
  `optmark` float(255,0) DEFAULT '0' COMMENT '赢输总额标记，一旦达到这个额度。那么对应的控制就会失效',
  `ratemark` float(255,4) DEFAULT '0.0000' COMMENT '操作变量，对应大部分游戏，这个是概率，对于捕鱼游戏，这个是难度系数',
  `channel` varchar(60) DEFAULT '' COMMENT '渠道',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=22 DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for dy_roulette
-- ----------------------------
DROP TABLE IF EXISTS `dy_roulette`;
CREATE TABLE `dy_roulette` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `roulettename` varchar(45) DEFAULT NULL,
  `createdate` timestamp NULL DEFAULT CURRENT_TIMESTAMP,
  `amount` smallint(5) DEFAULT '0',
  `maxnum` smallint(5) DEFAULT '500',
  `state` tinyint(3) DEFAULT '0' COMMENT '活动的状态：0：没结束：1：已经结束',
  `winid` int(11) DEFAULT '0',
  `awardtime` datetime DEFAULT '2000-01-01 00:00:01',
  `winkey` varchar(45) DEFAULT '',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for dy_rouletteplayer
-- ----------------------------
DROP TABLE IF EXISTS `dy_rouletteplayer`;
CREATE TABLE `dy_rouletteplayer` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `userid` int(11) DEFAULT '0',
  `roulettename` varchar(45) DEFAULT NULL,
  `createdate` timestamp NULL DEFAULT CURRENT_TIMESTAMP,
  `cardid` varchar(45) DEFAULT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for dy_serverinfo
-- ----------------------------
DROP TABLE IF EXISTS `dy_serverinfo`;
CREATE TABLE `dy_serverinfo` (
  `serverid` int(11) NOT NULL AUTO_INCREMENT,
  `servername` varchar(45) DEFAULT NULL,
  `serverip` varchar(45) DEFAULT NULL,
  `gamename` varchar(45) DEFAULT NULL,
  `tcpport` int(11) DEFAULT NULL,
  `httpport` int(11) DEFAULT NULL,
  `udpport` int(11) DEFAULT NULL,
  `remark` varchar(45) DEFAULT NULL,
  `nettype` int(11) DEFAULT '0' COMMENT '内网还是外网',
  `state` int(11) DEFAULT NULL COMMENT '0是没开启，1是已经启用，-1是已经废弃',
  `createdate` datetime DEFAULT '2000-01-01 00:00:00',
  `lastdate` timestamp NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  `playercount` int(11) DEFAULT '0' COMMENT '用户总数',
  `playeronline` int(11) DEFAULT '0' COMMENT '用户在线人数',
  PRIMARY KEY (`serverid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for dy_servicelist
-- ----------------------------
DROP TABLE IF EXISTS `dy_servicelist`;
CREATE TABLE `dy_servicelist` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `processname` varchar(255) DEFAULT '' COMMENT '服务名字',
  `processid` int(11) DEFAULT '0' COMMENT '服务ID',
  `remarks` varchar(255) DEFAULT '' COMMENT '备注',
  `state` int(255) DEFAULT '0' COMMENT '0 关闭 1 开启',
  PRIMARY KEY (`id`)
) ENGINE=MyISAM AUTO_INCREMENT=37 DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for dy_sharecode
-- ----------------------------
DROP TABLE IF EXISTS `dy_sharecode`;
CREATE TABLE `dy_sharecode` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `userid` int(11) DEFAULT '0',
  `touserid` int(11) DEFAULT '0',
  `strcode` varchar(45) DEFAULT NULL,
  `state` int(11) DEFAULT '0' COMMENT '0:',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for dy_shopconf
-- ----------------------------
DROP TABLE IF EXISTS `dy_shopconf`;
CREATE TABLE `dy_shopconf` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `shopid` int(11) DEFAULT '0',
  `shoptype` varchar(127) DEFAULT '',
  `name` varchar(127) DEFAULT '',
  `des` varchar(255) DEFAULT '',
  `minmoney` int(11) DEFAULT '0',
  `maxmoney` int(11) DEFAULT '0',
  `iscustom` tinyint(2) DEFAULT '0' COMMENT '是否支持自定额数额大小，1：支持',
  `rebate` int(11) DEFAULT '0' COMMENT '赠送的折扣',
  `norm_1` int(11) DEFAULT '0',
  `norm_2` int(11) DEFAULT '0',
  `norm_3` int(11) DEFAULT '0',
  `norm_4` int(11) DEFAULT '0',
  `norm_5` int(11) DEFAULT '0',
  `norm_6` int(11) DEFAULT '0',
  `norm_7` int(11) DEFAULT '0',
  `norm_8` int(11) DEFAULT '0',
  `state` int(11) DEFAULT '0',
  `channel` varchar(60) DEFAULT '' COMMENT '渠道',
  `paytype` varchar(20) DEFAULT '' COMMENT '关联支付类型',
  `sort` int(11) DEFAULT '99' COMMENT '排序',
  `prechannel` varchar(255) DEFAULT NULL COMMENT '大渠道',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=78 DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for dy_shoprebate
-- ----------------------------
DROP TABLE IF EXISTS `dy_shoprebate`;
CREATE TABLE `dy_shoprebate` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `shopid` int(11) DEFAULT NULL,
  `shopname` varchar(45) DEFAULT NULL,
  `description` varchar(128) DEFAULT NULL,
  `startdate` timestamp NULL DEFAULT NULL COMMENT '起效日期',
  `enddate` timestamp NULL DEFAULT NULL COMMENT '结束日期',
  `validity` int(11) DEFAULT '0' COMMENT '有效期',
  `distype` int(11) DEFAULT NULL COMMENT '折扣类型：11：得到具体数量增加；12：得到按照百分比增加；21：价格降低具体数量；22：价格降低百分比',
  `discount` int(11) DEFAULT NULL,
  `isexist` tinyint(2) DEFAULT '1' COMMENT '1是存在，0是已经过期',
  `remark` varchar(45) DEFAULT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for dy_shopvip
-- ----------------------------
DROP TABLE IF EXISTS `dy_shopvip`;
CREATE TABLE `dy_shopvip` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `account` varchar(45) NOT NULL DEFAULT '' COMMENT '账号',
  `password` varchar(45) NOT NULL DEFAULT '' COMMENT '密码',
  `nickname` varchar(45) DEFAULT '',
  `wechat` varchar(45) DEFAULT '',
  `qq` varchar(45) DEFAULT '',
  `state` int(1) DEFAULT '0',
  `realname` varchar(45) DEFAULT '',
  `phonenumber` varchar(11) DEFAULT '',
  `bankname` varchar(45) DEFAULT '',
  `bankaddress` varchar(45) DEFAULT '',
  `bankaccount` varchar(45) DEFAULT '',
  `bankpayee` varchar(45) DEFAULT '',
  `money` int(11) DEFAULT '0' COMMENT '余额',
  `des` varchar(255) DEFAULT '',
  `isexist` int(11) DEFAULT '1',
  `remark` varchar(45) DEFAULT '',
  `logindate` varchar(45) DEFAULT '' COMMENT '最新登陆时间',
  `loginip` varchar(45) DEFAULT '' COMMENT '登陆ip',
  `channel` varchar(45) DEFAULT '' COMMENT '渠道',
  `kefuurl` char(255) DEFAULT '' COMMENT '客服链接',
  `prechannel` varchar(50) DEFAULT '',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=11 DEFAULT CHARSET=utf8 ROW_FORMAT=DYNAMIC;

-- ----------------------------
-- Table structure for dy_syscount
-- ----------------------------
DROP TABLE IF EXISTS `dy_syscount`;
CREATE TABLE `dy_syscount` (
  `id` int(11) NOT NULL,
  `description` varchar(45) DEFAULT NULL COMMENT '描述',
  `type` smallint(3) DEFAULT '0' COMMENT '货币的类型',
  `issue` bigint(20) DEFAULT '0' COMMENT '发行的数量',
  `recycle` bigint(20) DEFAULT '0' COMMENT '回收的货币',
  `count` bigint(20) DEFAULT '0' COMMENT '正在流通的货币',
  `curdate` timestamp NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for dy_task
-- ----------------------------
DROP TABLE IF EXISTS `dy_task`;
CREATE TABLE `dy_task` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `task_action` int(11) DEFAULT NULL COMMENT '任务动作：1=绑定手机号',
  `task_type` int(11) DEFAULT NULL COMMENT '任务类型， 1、新手任务2、日常任务3、普通任务（普通任务没有类型图标）',
  `banner_pic` varchar(255) DEFAULT NULL COMMENT 'banner图',
  `task_name` varchar(100) DEFAULT NULL COMMENT '任务标题',
  `content_pic` varchar(255) DEFAULT NULL COMMENT '任务内容描述图pic',
  `task_title` varchar(255) DEFAULT NULL COMMENT '绑定图片',
  `task_date_start` datetime DEFAULT NULL COMMENT '任务开始时间',
  `task_date_end` datetime DEFAULT NULL COMMENT '任务结束时间',
  `task_order` int(11) DEFAULT NULL COMMENT '排序，从小到大',
  `enabled` int(11) DEFAULT '1' COMMENT '1=上级，0=下架',
  `created_at` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP COMMENT '创建时间',
  `updated_at` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP COMMENT '修改时间',
  `prechannel` varchar(255) DEFAULT NULL,
  `task_content` varchar(255) DEFAULT '' COMMENT '任务内容描述',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=17 DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for dy_task_done
-- ----------------------------
DROP TABLE IF EXISTS `dy_task_done`;
CREATE TABLE `dy_task_done` (
  `task_id` int(11) NOT NULL COMMENT '完成任务id',
  `userid` int(11) NOT NULL COMMENT '完成的用户id',
  `status` int(11) DEFAULT '0' COMMENT '0=未完成，1=已完成，2=处理中，3=已领取',
  `attach` text COMMENT '任务用到的数据，json格式',
  `created_at` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP COMMENT '完成时间',
  `updated_at` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  `propidlist` varchar(255) DEFAULT NULL COMMENT '奖励卡id 多个有逗号分隔',
  PRIMARY KEY (`task_id`,`userid`),
  KEY `userid` (`userid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for dy_task_propinfo
-- ----------------------------
DROP TABLE IF EXISTS `dy_task_propinfo`;
CREATE TABLE `dy_task_propinfo` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `task_id` int(11) DEFAULT NULL COMMENT '任务id',
  `propid` varchar(255) DEFAULT NULL COMMENT '已创建出来发放的道具id',
  `quantity` int(11) DEFAULT '0' COMMENT '每人可以拥有的道具数量',
  `created_at` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `updated_at` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=63 DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for dy_tyrewardmgr
-- ----------------------------
DROP TABLE IF EXISTS `dy_tyrewardmgr`;
CREATE TABLE `dy_tyrewardmgr` (
  `userid` int(11) NOT NULL COMMENT '玩家id',
  `promotiontime` datetime DEFAULT NULL COMMENT '转正时间',
  `wintyjetton` bigint(20) DEFAULT '0' COMMENT '体验金盈利金额',
  `promotiontype` int(11) DEFAULT '0' COMMENT '转正类型 : 0 未转正  1 到期自动转正 2  手动转正',
  `channel` varchar(45) DEFAULT NULL COMMENT '渠道',
  `prechannel` varchar(45) DEFAULT NULL COMMENT '大渠道',
  `registertime` datetime DEFAULT NULL COMMENT '绑定手机时间或初次用手机登录的时间',
  PRIMARY KEY (`userid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for dy_unlimiteduser
-- ----------------------------
DROP TABLE IF EXISTS `dy_unlimiteduser`;
CREATE TABLE `dy_unlimiteduser` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `userid` int(11) DEFAULT '0' COMMENT '用户ID',
  `wiringid` int(11) DEFAULT '0' COMMENT '排线ID',
  `createdate` timestamp NULL DEFAULT CURRENT_TIMESTAMP COMMENT '加入日期',
  `prelevel` int(11) DEFAULT '0',
  `level` int(11) DEFAULT '0',
  `allperformance` bigint(22) DEFAULT '0' COMMENT '总业绩',
  `selfperformance` bigint(22) DEFAULT '0' COMMENT '个人业绩,个人业绩计入上级代理',
  `directperformance` bigint(22) DEFAULT '0' COMMENT '直属玩家业绩',
  `teamperformance` bigint(22) DEFAULT '0' COMMENT '团队业绩',
  `allincome` int(11) DEFAULT '0' COMMENT '总收入',
  `getincome` int(11) DEFAULT '0' COMMENT '可提现收入',
  `allpreincome` int(11) DEFAULT '0' COMMENT '可预提收入',
  `getpreincome` int(11) DEFAULT '0' COMMENT '已经预提金额',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

-- ----------------------------
-- Table structure for dy_unlimitedwiring
-- ----------------------------
DROP TABLE IF EXISTS `dy_unlimitedwiring`;
CREATE TABLE `dy_unlimitedwiring` (
  `id` int(11) NOT NULL AUTO_INCREMENT COMMENT '排线码ID',
  `name` varchar(145) DEFAULT '' COMMENT '排线名称',
  `ownerid` int(11) DEFAULT '0',
  `createdate` timestamp NULL DEFAULT CURRENT_TIMESTAMP,
  `currnum` int(11) DEFAULT '0' COMMENT '当前排线数额',
  `maxnum` int(11) DEFAULT '0' COMMENT '最大排线数额',
  `usernum` int(11) DEFAULT '0' COMMENT '有效排线数额',
  `qrcodeurl` varchar(256) CHARACTER SET latin1 DEFAULT '0',
  `isexist` int(11) DEFAULT '0' COMMENT '是否存在。1：存在，0：不存在',
  `achday` bigint(22) DEFAULT '0' COMMENT '今日团队流水',
  `achyesterday` bigint(22) DEFAULT '0' COMMENT '昨日团队流水',
  `achweek` bigint(22) DEFAULT '0' COMMENT '周团队流水',
  `achall` bigint(22) DEFAULT '0',
  `month` bigint(22) DEFAULT '0' COMMENT '月团队流水',
  `newuserday` int(11) DEFAULT '0',
  `activityday` int(11) DEFAULT '0',
  `timemark` varchar(45) DEFAULT '' COMMENT '时间，最后一个用户用户加入的时间',
  `newuserid` int(11) DEFAULT '0' COMMENT '最后一个用户加入的userid',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for dy_user
-- ----------------------------
DROP TABLE IF EXISTS `dy_user`;
CREATE TABLE `dy_user` (
  `userid` int(11) NOT NULL AUTO_INCREMENT,
  `username` varchar(45) DEFAULT NULL,
  `password` varchar(45) DEFAULT NULL,
  `phonenum` varchar(45) DEFAULT NULL,
  `channel` varchar(45) DEFAULT NULL,
  `tokenid` varchar(129) DEFAULT NULL,
  `createdate` datetime DEFAULT NULL,
  `email` varchar(129) DEFAULT NULL,
  `nickname` varchar(45) DEFAULT NULL,
  `sex` int(11) DEFAULT '0',
  `age` int(11) DEFAULT '0',
  `viplevel` int(11) DEFAULT '0',
  `coin` bigint(45) DEFAULT '0',
  `gold` bigint(45) DEFAULT '0',
  `money` int(11) DEFAULT '0',
  `face_1` varchar(129) DEFAULT NULL,
  `face_2` varchar(129) DEFAULT NULL,
  `face_3` varchar(129) DEFAULT NULL,
  `face_4` varchar(129) DEFAULT NULL,
  `lastip` varchar(45) DEFAULT NULL,
  `lasttime_1` int(11) DEFAULT '0',
  `lasttime_2` int(11) DEFAULT '0',
  `lasttime_3` int(11) DEFAULT '0',
  `loginnum` int(11) DEFAULT '0',
  `silent` int(11) DEFAULT '0',
  `isban` int(11) DEFAULT '0',
  `exp` int(11) DEFAULT '0',
  PRIMARY KEY (`userid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for dy_viprule
-- ----------------------------
DROP TABLE IF EXISTS `dy_viprule`;
CREATE TABLE `dy_viprule` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `viplevel` int(11) NOT NULL COMMENT 'vip 等级 0-9',
  `points` int(11) DEFAULT '0' COMMENT '积分',
  `keeppoints` int(11) DEFAULT '0' COMMENT '保级积分',
  `downpoints` int(11) DEFAULT '0' COMMENT '降级积分',
  `teampointslimit` int(11) DEFAULT '0' COMMENT '团队积分上限',
  `drawfee` decimal(10,4) DEFAULT '0.0000' COMMENT '提现手续费',
  `pourlimit` int(11) DEFAULT '0' COMMENT '单笔投注限额',
  `freedrawamount` int(11) DEFAULT '0' COMMENT '免费提现额度',
  `proplimit` varchar(255) DEFAULT NULL COMMENT '卡券使用限制 json格式 卡券类型和数量',
  `birthgitstatus` int(11) DEFAULT '0' COMMENT '生日礼包状态 0 不开启 1开启',
  `channel` varchar(40) DEFAULT NULL COMMENT '渠道',
  `prechannel` varchar(40) DEFAULT NULL COMMENT '大渠道',
  `birthprop` varchar(255) DEFAULT NULL COMMENT '生日礼包 json格式 卡券类型和数量',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=53 DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for dy_wxfriend
-- ----------------------------
DROP TABLE IF EXISTS `dy_wxfriend`;
CREATE TABLE `dy_wxfriend` (
  `userid` int(11) NOT NULL DEFAULT '0',
  `mysign` varchar(45) DEFAULT '' COMMENT '我自己的字符串',
  `touserid` int(11) DEFAULT '0',
  `tonickname` varchar(45) DEFAULT '',
  `tosign` int(11) DEFAULT '0' COMMENT '已经绑定的字符串',
  `bindstate` int(11) DEFAULT '0' COMMENT '已经绑定',
  `gotnum` int(11) DEFAULT '0' COMMENT '已经领取的房卡数',
  `canget` int(11) DEFAULT '0' COMMENT '可以领取的房卡数',
  `binddate` int(11) DEFAULT '1',
  PRIMARY KEY (`userid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for dy_yqsroomconfig
-- ----------------------------
DROP TABLE IF EXISTS `dy_yqsroomconfig`;
CREATE TABLE `dy_yqsroomconfig` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `tabletype` int(11) DEFAULT NULL,
  `conefficient` varchar(10) CHARACTER SET latin1 DEFAULT '1' COMMENT '房间难度系数',
  `controlmethod` int(11) DEFAULT '1' COMMENT '1：房间 和 个人难度系数调控  2：奖池波动调控',
  `ratenum` varchar(10) DEFAULT '0',
  `wavenum` int(11) DEFAULT '1000' COMMENT '波动',
  `channel` varchar(20) DEFAULT '' COMMENT '渠道',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=4 DEFAULT CHARSET=utf8 ROW_FORMAT=DYNAMIC;

-- ----------------------------
-- Table structure for dy_yqsuserconfig
-- ----------------------------
DROP TABLE IF EXISTS `dy_yqsuserconfig`;
CREATE TABLE `dy_yqsuserconfig` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `tabletype` int(11) DEFAULT NULL,
  `userid` int(11) DEFAULT NULL,
  `conefficient` varchar(10) DEFAULT NULL,
  `channel` varchar(20) DEFAULT '' COMMENT '渠道',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=2 DEFAULT CHARSET=latin1 ROW_FORMAT=DYNAMIC;

-- ----------------------------
-- Table structure for ex_applytocreateplan
-- ----------------------------
DROP TABLE IF EXISTS `ex_applytocreateplan`;
CREATE TABLE `ex_applytocreateplan` (
  `id` bigint(20) unsigned NOT NULL AUTO_INCREMENT,
  `planid` bigint(20) DEFAULT '0' COMMENT '申请发布的方案id',
  `expertid` int(11) DEFAULT '0' COMMENT '方案所属的专家',
  `title` varchar(64) DEFAULT '' COMMENT '方案标题',
  `planprice` int(11) DEFAULT '0' COMMENT '方案价格',
  `content` varchar(512) DEFAULT '' COMMENT '方案正文',
  `checkstatus` tinyint(4) DEFAULT '0' COMMENT '审核状态 0：没有发起审核   1：审核中   2：审核失败  3:审核成功',
  `remark` varchar(128) DEFAULT '' COMMENT '审核备注',
  `modifier` varchar(50) DEFAULT '' COMMENT '修改人',
  `channel` varchar(32) DEFAULT '' COMMENT '渠道号',
  `prechannel` varchar(32) DEFAULT '' COMMENT '大渠道',
  `time` bigint(20) DEFAULT '0' COMMENT '申请时间搓',
  PRIMARY KEY (`id`),
  KEY `checkstatus` (`checkstatus`),
  KEY `planid` (`planid`)
) ENGINE=InnoDB AUTO_INCREMENT=4215 DEFAULT CHARSET=utf8 COMMENT='方案发布审核';

-- ----------------------------
-- Table structure for ex_applytoexpert
-- ----------------------------
DROP TABLE IF EXISTS `ex_applytoexpert`;
CREATE TABLE `ex_applytoexpert` (
  `id` bigint(20) unsigned NOT NULL AUTO_INCREMENT,
  `userid` int(11) DEFAULT '0' COMMENT '申请成为专家的玩家id',
  `nickname` varchar(45) DEFAULT '',
  `face` varchar(45) NOT NULL DEFAULT '' COMMENT '头像',
  `style` tinyint(4) DEFAULT '0' COMMENT '1、保守型：≤1% 2、稳健型：≤3% 3、平衡型：≤5% 4、进取型：≤7% 5、激进型：＞7%',
  `checkstatus` tinyint(4) DEFAULT '0' COMMENT '审核状态 0：没有发起审核   1：审核中   2：审核失败  3:审核成功',
  `remark` varchar(128) DEFAULT '' COMMENT '审核备注',
  `modifier` varchar(50) DEFAULT '' COMMENT '修改人',
  `channel` varchar(32) DEFAULT '' COMMENT '渠道号',
  `prechannel` varchar(32) DEFAULT '' COMMENT '大渠道',
  `time` bigint(20) DEFAULT '0' COMMENT '申请时间搓',
  `content` varchar(512) DEFAULT '' COMMENT '专家的个人介绍',
  PRIMARY KEY (`id`),
  KEY `checkstatus` (`checkstatus`) USING BTREE,
  KEY `userid` (`userid`)
) ENGINE=InnoDB AUTO_INCREMENT=36 DEFAULT CHARSET=utf8 COMMENT='成为专家审核';

-- ----------------------------
-- Table structure for ex_buyplan
-- ----------------------------
DROP TABLE IF EXISTS `ex_buyplan`;
CREATE TABLE `ex_buyplan` (
  `id` bigint(20) unsigned NOT NULL AUTO_INCREMENT,
  `userid` int(11) DEFAULT '0' COMMENT '玩家id',
  `planid` bigint(20) DEFAULT '0' COMMENT '被购买的方案id',
  `time` bigint(20) DEFAULT '0' COMMENT '购买方案的时间搓',
  `expertid` int(11) DEFAULT '0' COMMENT '方案所属的专家',
  `planprice` int(11) DEFAULT '0' COMMENT '方案的价格',
  PRIMARY KEY (`id`),
  KEY `userid` (`userid`) USING BTREE,
  KEY `planid` (`planid`) USING BTREE
) ENGINE=InnoDB AUTO_INCREMENT=98 DEFAULT CHARSET=utf8 COMMENT='保存玩家购买过的方案';

-- ----------------------------
-- Table structure for ex_plan
-- ----------------------------
DROP TABLE IF EXISTS `ex_plan`;
CREATE TABLE `ex_plan` (
  `planid` bigint(20) NOT NULL DEFAULT '1' COMMENT '方案id',
  `expertid` int(11) DEFAULT '0' COMMENT '方案所属的专家id',
  `plantitle` varchar(64) DEFAULT '' COMMENT '方案标题',
  `content` varchar(512) DEFAULT '' COMMENT '推荐理由，就是方案的内容正文',
  `rsign` tinyint(4) DEFAULT '0' COMMENT 'r标签  R1:≤1% R2:1%＜& ≤3% R3:3%＜& ≤5% R4:5%＜& ≤7% R5:＞7% 根据⽤户选择的⽅案的收益率，给⽅案贴标签',
  `plantime` bigint(20) DEFAULT '0' COMMENT '方案投注时间（方案截止时间），如果有多场比赛，就是时间最先开始的那场，时间搓格式，单位秒',
  `guessprofit` float DEFAULT '0' COMMENT '预期收益，就是所有比分的平均收益',
  `checkstatus` tinyint(4) DEFAULT '0' COMMENT '审核状态 0：没有发起审核   1：审核中   2：审核失败  3:审核成功  4：方案下架',
  `remark` varchar(128) DEFAULT '' COMMENT '审核备注',
  `channel` varchar(45) DEFAULT '' COMMENT '渠道号',
  `time` bigint(20) DEFAULT '0' COMMENT '方案发布的时间',
  `planprice` int(11) DEFAULT '0' COMMENT '方案价格',
  `buycount` int(11) DEFAULT '0' COMMENT '购买方案人数',
  `ordercount` int(11) DEFAULT '0' COMMENT '投注人数',
  `allordermoney` bigint(11) DEFAULT '0' COMMENT '总投注金额',
  `allprofit` bigint(20) DEFAULT '0' COMMENT '总盈利额',
  `profitrate` float DEFAULT '0' COMMENT '盈利率',
  `score` bigint(11) DEFAULT '0' COMMENT '方案推荐分，后台编辑',
  `prechannel` varchar(45) DEFAULT '' COMMENT '大渠道',
  `passtime` bigint(20) DEFAULT '0' COMMENT '过审时间搓',
  `status` tinyint(3) DEFAULT '1' COMMENT '状态 1 上架 2 下架',
  PRIMARY KEY (`planid`),
  KEY `expertidandplantime` (`expertid`,`plantime`),
  KEY `exandstatus` (`expertid`,`checkstatus`),
  KEY `expertidandplantimeandid` (`planid`,`expertid`,`plantime`),
  KEY `planidexperidstatus` (`planid`,`expertid`,`checkstatus`),
  KEY `scoreandcheckstatusch` (`score`,`checkstatus`,`channel`,`time`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='存储方案的表';

-- ----------------------------
-- Table structure for ex_planconfig
-- ----------------------------
DROP TABLE IF EXISTS `ex_planconfig`;
CREATE TABLE `ex_planconfig` (
  `id` bigint(20) unsigned NOT NULL AUTO_INCREMENT,
  `plantitle` varchar(64) DEFAULT '' COMMENT '方案标题',
  `content` varchar(512) DEFAULT '' COMMENT '推荐理由，就是方案的内容正文',
  `planprice` int(11) DEFAULT '0' COMMENT '方案价格',
  `prechannel` varchar(20) DEFAULT '' COMMENT '渠道',
  `channel` varchar(45) DEFAULT '' COMMENT '渠道号',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=24 DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for ex_planorder
-- ----------------------------
DROP TABLE IF EXISTS `ex_planorder`;
CREATE TABLE `ex_planorder` (
  `id` bigint(20) unsigned NOT NULL AUTO_INCREMENT COMMENT '自增id',
  `planid` bigint(20) DEFAULT '0' COMMENT '方案id',
  `raceid` varchar(20) NOT NULL DEFAULT '0' COMMENT '比赛ID',
  `rebateid` varchar(40) DEFAULT '0' COMMENT '返利id',
  `starttime` bigint(20) DEFAULT '0' COMMENT '赛事开始时间，时间搓',
  `category` varchar(50) NOT NULL DEFAULT '' COMMENT '类别',
  `hometeam` varchar(20) NOT NULL DEFAULT '' COMMENT '主队',
  `visitteam` varchar(20) NOT NULL DEFAULT '' COMMENT '客队',
  `score` varchar(12) DEFAULT '' COMMENT '下注比分',
  `rebateRatio` double DEFAULT '0' COMMENT '收益率',
  `fee` int(11) DEFAULT '5' COMMENT '抽水值 5就是5%',
  `yieltype` tinyint(4) DEFAULT '0' COMMENT '类型 0:全场 1：半场',
  `time` bigint(20) DEFAULT '0' COMMENT '发布时间',
  `expertid` int(11) DEFAULT '0' COMMENT '方案所属的专家id',
  `checkstatus` tinyint(4) DEFAULT '0' COMMENT '审核状态 0：没有发起审核   1：审核中   2：审核失败  3:审核成功  4：方案下架',
  `raceresult` tinyint(4) DEFAULT '0' COMMENT '0：没有结果  1：胜利  2：失败',
  PRIMARY KEY (`id`),
  KEY `planid` (`planid`) USING HASH,
  KEY `raceid` (`raceid`),
  KEY `useridcheckresid` (`id`,`expertid`,`checkstatus`,`raceresult`),
  KEY `planidorder` (`planid`,`starttime`)
) ENGINE=InnoDB AUTO_INCREMENT=10161 DEFAULT CHARSET=utf8 COMMENT='方案投注信息表';

-- ----------------------------
-- Table structure for ex_player
-- ----------------------------
DROP TABLE IF EXISTS `ex_player`;
CREATE TABLE `ex_player` (
  `userid` int(11) NOT NULL DEFAULT '0' COMMENT '用户id',
  `nickname` varchar(64) DEFAULT '' COMMENT '用户昵称',
  `face` varchar(256) DEFAULT '' COMMENT '头像',
  `style` int(11) DEFAULT '1' COMMENT '1、保守型：≤1% 2、稳健型：≤3% 3、平衡型：≤5% 4、进取型：≤7% 5、激进型：＞7%',
  `level` int(11) DEFAULT '0' COMMENT '等级',
  `title` varchar(45) DEFAULT '' COMMENT '专家的头衔',
  `content` varchar(512) DEFAULT '' COMMENT '专家的个人介绍',
  `hitcount` tinyint(3) DEFAULT '0' COMMENT '近三场命中次数',
  `nearhitrate` float DEFAULT '0' COMMENT '近期命中率',
  `averagerate` float DEFAULT '0' COMMENT '平均赔率',
  `score` bigint(20) DEFAULT '0' COMMENT '推荐分，后台编辑',
  `fanscount` int(11) DEFAULT '0' COMMENT '粉丝数量',
  `concernedcount` int(11) DEFAULT '0' COMMENT '关注数量',
  `allincome` bigint(20) DEFAULT '0' COMMENT '总盈利',
  `allbuymoney` bigint(20) DEFAULT '0' COMMENT '累计购买金额',
  `allfollowmoney` bigint(20) DEFAULT '0' COMMENT '累计跟投金额',
  `followrate` float DEFAULT '0' COMMENT '跟投收益率',
  `channel` varchar(50) DEFAULT '' COMMENT '渠道号',
  `prechannel` varchar(50) DEFAULT '0' COMMENT '大渠道',
  `creattime` bigint(11) DEFAULT '0' COMMENT '添加时间 时间戳',
  `status` int(3) DEFAULT '1' COMMENT '状态 1 开启   2 禁用',
  `isauth` tinyint(3) DEFAULT '0' COMMENT '0 未认证的专家  1 已认证的专家',
  `commissionrate` float DEFAULT '0' COMMENT '佣金比例',
  `commissionincome` double DEFAULT '0' COMMENT '佣金收益',
  `weekcleartime` varchar(20) DEFAULT '' COMMENT '周统计清理时间',
  `fivehitcount` tinyint(3) DEFAULT '0' COMMENT '近五场命中次数',
  `sevenhitcount` tinyint(3) DEFAULT '0' COMMENT '近七场命中次数',
  `isrobot` tinyint(4) DEFAULT '0' COMMENT '0：非机器人专家  1：机器人专家',
  PRIMARY KEY (`userid`),
  KEY `scorech` (`score`,`channel`),
  KEY `nickname` (`nickname`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='专家表';

-- ----------------------------
-- Table structure for ex_pourplan
-- ----------------------------
DROP TABLE IF EXISTS `ex_pourplan`;
CREATE TABLE `ex_pourplan` (
  `id` bigint(20) unsigned NOT NULL AUTO_INCREMENT,
  `userid` int(11) DEFAULT '0' COMMENT '玩家id',
  `planid` bigint(20) DEFAULT '0' COMMENT '被跟投的方案id',
  `time` bigint(20) DEFAULT '0' COMMENT '跟投的时间搓',
  `expertid` int(11) DEFAULT '0' COMMENT '方案所属的专家',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=55 DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for ex_relationplan
-- ----------------------------
DROP TABLE IF EXISTS `ex_relationplan`;
CREATE TABLE `ex_relationplan` (
  `id` bigint(20) unsigned NOT NULL AUTO_INCREMENT,
  `userid` int(11) DEFAULT '0' COMMENT '玩家id',
  `planid` bigint(20) DEFAULT '0' COMMENT '被关注的方案id',
  `time` bigint(20) DEFAULT '0' COMMENT '关注方案的时间搓',
  PRIMARY KEY (`id`),
  KEY `userid` (`userid`) USING BTREE,
  KEY `planid` (`planid`) USING BTREE
) ENGINE=InnoDB AUTO_INCREMENT=48 DEFAULT CHARSET=utf8 COMMENT='保存玩家关注的方案';

-- ----------------------------
-- Table structure for ex_relationuser
-- ----------------------------
DROP TABLE IF EXISTS `ex_relationuser`;
CREATE TABLE `ex_relationuser` (
  `id` bigint(20) unsigned NOT NULL AUTO_INCREMENT,
  `userid` int(11) DEFAULT '0' COMMENT '玩家id',
  `beuserid` int(11) DEFAULT '0' COMMENT '被关注的玩家id',
  `time` bigint(20) DEFAULT '0' COMMENT '关注专家的时间搓',
  PRIMARY KEY (`id`),
  KEY `userid` (`userid`) USING HASH,
  KEY `beuserid` (`beuserid`) USING HASH
) ENGINE=InnoDB AUTO_INCREMENT=252 DEFAULT CHARSET=utf8 COMMENT='保存玩家粉丝表';

-- ----------------------------
-- Table structure for ex_sellplan
-- ----------------------------
DROP TABLE IF EXISTS `ex_sellplan`;
CREATE TABLE `ex_sellplan` (
  `id` bigint(20) unsigned NOT NULL AUTO_INCREMENT COMMENT '订单id',
  `expertid` int(11) DEFAULT '0' COMMENT '专家id',
  `purchaser` int(11) DEFAULT '0' COMMENT '购买方案的玩家id',
  `planid` bigint(20) DEFAULT '0' COMMENT '方案id',
  `plantitle` varchar(64) DEFAULT '' COMMENT '方案标题',
  `plantime` bigint(20) DEFAULT '0' COMMENT '开赛时间',
  `price` int(11) DEFAULT '0' COMMENT '方案单价',
  `time` bigint(20) DEFAULT '0' COMMENT '售卖时间',
  PRIMARY KEY (`id`),
  KEY `expertid` (`expertid`)
) ENGINE=InnoDB AUTO_INCREMENT=61 DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for ex_usernotice
-- ----------------------------
DROP TABLE IF EXISTS `ex_usernotice`;
CREATE TABLE `ex_usernotice` (
  `id` bigint(20) unsigned NOT NULL AUTO_INCREMENT,
  `userid` int(11) DEFAULT '0' COMMENT '玩家id',
  `beuserid` int(11) DEFAULT '0' COMMENT '被添加提醒的玩家id',
  `time` bigint(20) DEFAULT '0' COMMENT '添加通知的时间搓',
  PRIMARY KEY (`id`),
  KEY `userid` (`userid`) USING BTREE,
  KEY `beuserid` (`beuserid`) USING BTREE
) ENGINE=InnoDB AUTO_INCREMENT=37 DEFAULT CHARSET=utf8 COMMENT='添加提醒表';

-- ----------------------------
-- Table structure for log_activity_record
-- ----------------------------
DROP TABLE IF EXISTS `log_activity_record`;
CREATE TABLE `log_activity_record` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `userid` int(11) DEFAULT '0',
  `nickname` varchar(128) DEFAULT '',
  `createdate` timestamp NULL DEFAULT CURRENT_TIMESTAMP,
  `activitytype` int(11) DEFAULT '0' COMMENT '活动类型: 1为特殊牌型奖励活动 2为百人场连赢奖励活动',
  `gametype` int(11) DEFAULT '0' COMMENT '游戏类型',
  `tabletype` int(11) DEFAULT '0' COMMENT '桌子类型',
  `rewardjetton` int(11) DEFAULT '0' COMMENT '奖励金额',
  `remarks` varchar(255) DEFAULT '' COMMENT '备注',
  `activityid` int(11) DEFAULT '0' COMMENT '活动id',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for log_baobiao
-- ----------------------------
DROP TABLE IF EXISTS `log_baobiao`;
CREATE TABLE `log_baobiao` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `userid` int(11) DEFAULT '0',
  `nickname` varchar(45) DEFAULT '0',
  `wincount` int(11) DEFAULT '0',
  `paycount` int(11) DEFAULT '0',
  `pourcount` bigint(22) DEFAULT '0',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 ROW_FORMAT=DYNAMIC;

-- ----------------------------
-- Table structure for log_behavior
-- ----------------------------
DROP TABLE IF EXISTS `log_behavior`;
CREATE TABLE `log_behavior` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `userid` int(11) DEFAULT '0',
  `nickname` varchar(45) DEFAULT '',
  `level` varchar(45) DEFAULT 'info' COMMENT 'log的等级：debug，info，warning，error',
  `createdate` timestamp NULL DEFAULT NULL,
  `category` varchar(45) DEFAULT '',
  `tag` varchar(45) DEFAULT '',
  `gettype` varchar(45) DEFAULT '' COMMENT '获取到的物品的类型',
  `getamount` int(11) DEFAULT '0' COMMENT '获取到的数量',
  `costtype` varchar(45) DEFAULT '',
  `costamount` int(11) DEFAULT '0',
  `retcode` int(11) DEFAULT '0' COMMENT '错误码',
  `msg` varchar(256) DEFAULT '',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for log_convert
-- ----------------------------
DROP TABLE IF EXISTS `log_convert`;
CREATE TABLE `log_convert` (
  `id` int(11) NOT NULL DEFAULT '0',
  `userid` int(11) DEFAULT '0',
  `nickname` varchar(45) CHARACTER SET latin1 DEFAULT '',
  `convertid` int(11) DEFAULT '0',
  `convertname` varchar(45) CHARACTER SET latin1 DEFAULT '',
  `convertnum` varchar(45) CHARACTER SET latin1 DEFAULT '',
  `paytype` int(11) DEFAULT '0',
  `paynum` int(11) DEFAULT '0',
  `addname` varchar(45) CHARACTER SET latin1 DEFAULT '',
  `addphone` varchar(45) CHARACTER SET latin1 DEFAULT '',
  `address` varchar(256) CHARACTER SET latin1 DEFAULT '',
  `timemark` varchar(45) CHARACTER SET latin1 DEFAULT '',
  `state` int(11) DEFAULT '0',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for log_currencychangedrecord
-- ----------------------------
DROP TABLE IF EXISTS `log_currencychangedrecord`;
CREATE TABLE `log_currencychangedrecord` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `userid` int(11) NOT NULL,
  `currency_type` int(11) NOT NULL COMMENT '货币类型,0为money',
  `initial` bigint(45) NOT NULL,
  `amount` bigint(45) NOT NULL,
  `date` timestamp NULL DEFAULT CURRENT_TIMESTAMP,
  `opt_type` int(11) NOT NULL COMMENT '操作类型,0为代理赠送钻石,1为玩家收到代理赠送的钻石',
  `remark` varchar(256) DEFAULT NULL,
  `nickname` varchar(45) DEFAULT '',
  `fromuserid` int(11) DEFAULT '0',
  `fromnickname` varchar(45) DEFAULT '',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for log_dayuseraward
-- ----------------------------
DROP TABLE IF EXISTS `log_dayuseraward`;
CREATE TABLE `log_dayuseraward` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `userid` int(11) DEFAULT '0',
  `nickname` varchar(50) DEFAULT '' COMMENT '昵称',
  `diraward` int(11) DEFAULT '0' COMMENT '直属分成',
  `nodiraward` int(11) DEFAULT '0' COMMENT '团队分成',
  `createdate` varchar(20) DEFAULT '' COMMENT '时间',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for log_economy
-- ----------------------------
DROP TABLE IF EXISTS `log_economy`;
CREATE TABLE `log_economy` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `description` varchar(45) DEFAULT NULL,
  `type` tinyint(3) DEFAULT '0' COMMENT '货币类型',
  `category` smallint(5) DEFAULT '0' COMMENT '操作模块',
  `count` int(11) DEFAULT '0',
  `occurdate` timestamp NULL DEFAULT CURRENT_TIMESTAMP,
  `issue` bigint(20) DEFAULT '0' COMMENT '发放的总量',
  `recycle` bigint(20) DEFAULT '0' COMMENT '回收的总量',
  `currency` bigint(20) DEFAULT NULL COMMENT '剩余的总量',
  `remark` varchar(256) DEFAULT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for log_exception
-- ----------------------------
DROP TABLE IF EXISTS `log_exception`;
CREATE TABLE `log_exception` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `loglevel` varchar(45) DEFAULT NULL,
  `userid` int(11) DEFAULT NULL,
  `nickname` varchar(45) DEFAULT NULL,
  `tag` varchar(45) DEFAULT NULL,
  `category` varchar(45) DEFAULT NULL,
  `code` int(11) DEFAULT NULL,
  `msg` varchar(512) DEFAULT NULL,
  `createdate` timestamp NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for log_gameday
-- ----------------------------
DROP TABLE IF EXISTS `log_gameday`;
CREATE TABLE `log_gameday` (
  `datestr` varchar(45) NOT NULL DEFAULT '2001-01-01',
  `allfee` int(11) DEFAULT '0' COMMENT '总税收',
  `allach` int(11) DEFAULT '0' COMMENT '总下注额',
  `allwin` int(11) DEFAULT '0' COMMENT '总盈利额',
  `doudizhu` int(11) DEFAULT '0' COMMENT '斗地主每天税收',
  `longhuddou` int(11) DEFAULT '0' COMMENT '龙虎斗每天税收',
  `honghei` int(11) DEFAULT '0' COMMENT '红黑大战每天税收',
  `baijiale` int(11) DEFAULT '0' COMMENT '百家乐每天税收',
  `qiangzhuangniuniu` int(11) DEFAULT '0' COMMENT '抢庄牛牛每天税收',
  `bairenniuniu` int(11) DEFAULT '0' COMMENT '百人牛牛每天税收',
  `zhajinhua` int(11) DEFAULT '0' COMMENT '炸金花每天税收',
  `benchibaoma` int(11) DEFAULT '0' COMMENT '奔驰宝马每天税收',
  `shiziwanghuo` int(11) DEFAULT '0' COMMENT '狮子王国每天税收',
  `toubao` int(11) DEFAULT '0' COMMENT '骰宝每天税收',
  `ach_doudizhu` int(11) DEFAULT '0' COMMENT '斗地主的下注额',
  `ach_longhudou` int(11) DEFAULT '0' COMMENT '龙虎斗的下注额',
  `ach_honghei` int(11) DEFAULT '0' COMMENT '红黑的下注额',
  `ach_baijiale` int(11) DEFAULT '0' COMMENT '百家乐的下注额',
  `ach_qiangzhuangniuniu` int(11) DEFAULT '0' COMMENT '抢庄牛牛的下注额',
  `ach_bairenniuniu` int(11) DEFAULT '0' COMMENT '百人牛牛的下注额',
  `ach_zhajinhua` int(11) DEFAULT '0' COMMENT '炸金花的下注额',
  `ach_beichibaoma` int(11) DEFAULT '0' COMMENT '奔驰宝马的下注额',
  `ach_shiziwangguo` int(11) DEFAULT '0' COMMENT '狮子王国的下注额',
  `ach_toubao` int(11) DEFAULT '0' COMMENT '骰宝的下注额',
  `win_doudizhu` int(11) DEFAULT '0' COMMENT '系统在斗地主中赢钱',
  `win_longhudou` int(11) DEFAULT '0' COMMENT '系统在龙虎斗中赢钱',
  `win_honghei` int(11) DEFAULT '0' COMMENT '系统在红黑中赢钱',
  `win_baijiale` int(11) DEFAULT '0' COMMENT '系统在百家乐中赢钱',
  `win_qiangzhuangniuniu` int(11) DEFAULT '0' COMMENT '系统在抢庄牛牛中赢钱',
  `win_bairenniuniu` int(11) DEFAULT '0' COMMENT '系统在百人牛牛中赢钱',
  `win_zhajinhua` int(11) DEFAULT '0' COMMENT '系统在炸金花中赢钱',
  `win_beichibaoma` int(11) DEFAULT '0' COMMENT '系统在奔驰宝马中赢的前年',
  `win_shiziwangguo` int(11) DEFAULT '0' COMMENT '系统在狮子王国中赢的钱',
  `win_toubao` int(11) DEFAULT '0' COMMENT '系统在骰宝中赢的钱',
  `moneyinit` bigint(22) DEFAULT '0' COMMENT '每日初始值',
  `moneyin` int(11) DEFAULT '0' COMMENT '每日进项',
  `moneyout` int(11) DEFAULT '0' COMMENT '每日出项',
  PRIMARY KEY (`datestr`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for log_gamedetail
-- ----------------------------
DROP TABLE IF EXISTS `log_gamedetail`;
CREATE TABLE `log_gamedetail` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `createdate` varchar(45) DEFAULT 'CURRENT_TIMESTAMP',
  `gametype` varchar(128) DEFAULT '' COMMENT '游戏类型',
  `gamename` varchar(45) DEFAULT '',
  `tabletype` varchar(45) DEFAULT '' COMMENT '牌桌类型',
  `tablename` varchar(45) DEFAULT '',
  `tableid` int(11) DEFAULT '0',
  `result` varchar(512) DEFAULT '' COMMENT '开奖结果列表',
  `zhuanginfo` varchar(512) DEFAULT '' COMMENT '庄家信息',
  `winlist` varchar(4096) DEFAULT '' COMMENT '赢家列表',
  `loselist` varchar(4096) DEFAULT '',
  `whitelist` varchar(128) DEFAULT '' COMMENT '白名单列表',
  `blacklist` varchar(128) DEFAULT '' COMMENT '黑名单列表',
  `pondinfo` varchar(128) DEFAULT '' COMMENT '奖池信息',
  `syswin` int(11) DEFAULT '0' COMMENT '系统赢输',
  `feenum` int(11) DEFAULT '0' COMMENT '抽水数量',
  `addpondnum` int(11) DEFAULT '0' COMMENT '奖池增加值',
  `pondratenum` int(11) DEFAULT '0' COMMENT '奖池抽佣',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='记录每一个牌局的详细信息';

-- ----------------------------
-- Table structure for log_giftgold
-- ----------------------------
DROP TABLE IF EXISTS `log_giftgold`;
CREATE TABLE `log_giftgold` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `fromid` int(11) DEFAULT '0',
  `fromnick` varchar(128) DEFAULT '',
  `toid` int(11) DEFAULT '0',
  `tonick` varchar(128) DEFAULT '',
  `fromgoldbegin` int(11) DEFAULT '0',
  `fromgoldafter` int(11) DEFAULT '0',
  `togoldbegin` int(11) DEFAULT '0',
  `togoldafter` int(11) DEFAULT '0',
  `sendnum` int(11) DEFAULT '0',
  `state` int(11) DEFAULT '0',
  `createdate` timestamp NULL DEFAULT CURRENT_TIMESTAMP,
  `realnum` int(11) DEFAULT '0',
  `julebuid` int(11) DEFAULT '0',
  `msgtype` varchar(45) DEFAULT '',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for log_moneydetail
-- ----------------------------
DROP TABLE IF EXISTS `log_moneydetail`;
CREATE TABLE `log_moneydetail` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `userid` int(11) DEFAULT '0',
  `nickname` varchar(128) DEFAULT '',
  `createdate` timestamp NULL DEFAULT CURRENT_TIMESTAMP,
  `opttype` int(11) DEFAULT '0',
  `optmsg` varchar(45) DEFAULT '',
  `amount` int(11) DEFAULT '0' COMMENT '变化数量',
  `usermoney` int(11) DEFAULT '0' COMMENT '变化后金币',
  `remark` varchar(256) DEFAULT '' COMMENT '备注',
  `amount2` int(11) DEFAULT '0',
  `amount3` int(11) DEFAULT '0',
  `amount4` int(11) DEFAULT '0',
  `detail` varchar(5120) DEFAULT '',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for log_onlinedaily
-- ----------------------------
DROP TABLE IF EXISTS `log_onlinedaily`;
CREATE TABLE `log_onlinedaily` (
  `dateid` varchar(45) CHARACTER SET latin1 NOT NULL DEFAULT '',
  `alllist` varchar(45) CHARACTER SET latin1 DEFAULT '',
  `onlinelist` varchar(10000) CHARACTER SET latin1 DEFAULT '',
  `minulist` varchar(10000) CHARACTER SET latin1 DEFAULT '',
  PRIMARY KEY (`dateid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for log_player
-- ----------------------------
DROP TABLE IF EXISTS `log_player`;
CREATE TABLE `log_player` (
  `userid` int(11) NOT NULL,
  `nickname` varchar(45) DEFAULT NULL,
  `regdate` timestamp NULL DEFAULT CURRENT_TIMESTAMP,
  `lastdate` int(11) DEFAULT '0',
  `penultdate` int(11) DEFAULT '0',
  `loginday` int(11) DEFAULT '1' COMMENT '登录天数',
  `logincount` int(11) DEFAULT '1' COMMENT '登录次数',
  `onlinetime` int(11) DEFAULT '0' COMMENT '在线时长',
  `fpaydate` int(11) DEFAULT '0',
  `lpaydate` int(11) DEFAULT '0',
  `callpaycount` int(11) DEFAULT '0' COMMENT '调起付费总额',
  `callpaytimes` int(11) DEFAULT '0' COMMENT '调起付费次数',
  `paytimes` int(11) DEFAULT '0' COMMENT '付费次数',
  `loginconday` int(11) DEFAULT '0',
  `maxloginconday` int(11) DEFAULT '0',
  `maxjetton` int(11) DEFAULT '0',
  `paytoday` int(11) DEFAULT '0' COMMENT '今天付费',
  `payyesterday` int(11) DEFAULT '0' COMMENT '昨天付费',
  `payweek` int(11) DEFAULT '0' COMMENT '周付费',
  `paymonth` int(11) DEFAULT '0' COMMENT '月付费',
  `payall` int(11) DEFAULT '0',
  `wintoday` int(11) DEFAULT '0' COMMENT '今日盈利',
  `winyesterday` int(11) DEFAULT '0' COMMENT '昨日盈利',
  `winweek` int(11) DEFAULT '0' COMMENT '周盈利',
  `winlastweek` int(11) DEFAULT '0',
  `winmonth` int(11) DEFAULT '0' COMMENT '月盈利',
  `winlastmonth` int(11) DEFAULT '0',
  `winall` int(11) DEFAULT '0',
  `commisiontoday` int(11) DEFAULT '0',
  `commisionweek` int(11) DEFAULT '0',
  `commisionmonth` int(11) DEFAULT '0',
  `commisionall` int(11) DEFAULT '0',
  `feeday` int(11) DEFAULT '0',
  `feeweek` int(11) DEFAULT '0',
  `feemonth` int(11) DEFAULT '0',
  `feeall` int(11) DEFAULT '0',
  `achday` bigint(22) DEFAULT '0' COMMENT '今天流水',
  `achyesterday` bigint(22) DEFAULT '0' COMMENT '昨日流水',
  `achweek` bigint(22) DEFAULT '0' COMMENT '周流水',
  `achmonth` bigint(22) DEFAULT '0',
  `achall` bigint(22) DEFAULT '0' COMMENT '总流水',
  `teamusernum` int(11) DEFAULT '0' COMMENT '团队的人数',
  `newuserday` int(11) DEFAULT '0' COMMENT '在自己的推广团队中，表示每日新增',
  `activityday` int(11) DEFAULT '0' COMMENT '自己的团队中，表示每日活跃',
  `achteamday` bigint(22) DEFAULT '0' COMMENT '团队当日业绩',
  `achteamyesterday` bigint(22) DEFAULT '0',
  `achteamweek` bigint(22) DEFAULT '0' COMMENT '团队周总业绩',
  `achteammonth` bigint(22) DEFAULT '0' COMMENT '团队月总业绩',
  `achteamall` bigint(22) DEFAULT '0' COMMENT '个人团队全部总业绩',
  `drawlimit` bigint(22) DEFAULT '0',
  `taketoday` int(11) DEFAULT '0',
  `takeweek` int(11) DEFAULT '0',
  `takemonth` int(11) DEFAULT '0',
  `takeall` bigint(22) DEFAULT '0',
  PRIMARY KEY (`userid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for log_race
-- ----------------------------
DROP TABLE IF EXISTS `log_race`;
CREATE TABLE `log_race` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `gametype` int(11) DEFAULT '0',
  `tableid` int(11) DEFAULT '0',
  `gamename` varchar(129) DEFAULT '',
  `usernum` int(11) DEFAULT '0',
  `wintoday` int(11) DEFAULT '0',
  `winyesterday` int(11) DEFAULT '0',
  `winweek` int(11) DEFAULT '0',
  `winlastweek` int(11) DEFAULT '0',
  `winmonth` int(11) DEFAULT '0',
  `winlastmonth` int(11) DEFAULT '0',
  `winall` bigint(21) DEFAULT '0',
  `commisiontoday` int(11) DEFAULT '0',
  `commisionweek` int(11) DEFAULT '0',
  `commisionmonth` int(11) DEFAULT '0',
  `commisionall` bigint(21) DEFAULT '0',
  `state` int(11) DEFAULT '0',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for log_recharge
-- ----------------------------
DROP TABLE IF EXISTS `log_recharge`;
CREATE TABLE `log_recharge` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `userid` int(11) NOT NULL DEFAULT '0' COMMENT '用户id(被充值者)',
  `amount` decimal(11,2) NOT NULL COMMENT '充值金额',
  `type` int(11) NOT NULL DEFAULT '0' COMMENT '类型（0  用户充值  1 vip充值）',
  `recharge_id` int(11) NOT NULL DEFAULT '0' COMMENT '充值者id',
  `remark` varchar(255) DEFAULT '' COMMENT '备注',
  `add_time` int(11) DEFAULT '0' COMMENT '添加时间',
  `status` int(3) NOT NULL DEFAULT '1' COMMENT '状态（0 失败  1 成功）',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=DYNAMIC COMMENT='充值log';

-- ----------------------------
-- Table structure for log_retain
-- ----------------------------
DROP TABLE IF EXISTS `log_retain`;
CREATE TABLE `log_retain` (
  `userid` int(11) NOT NULL,
  `regdate` timestamp NULL DEFAULT CURRENT_TIMESTAMP,
  `loginday` int(11) DEFAULT '0',
  `day1` smallint(5) DEFAULT '1',
  `day2` smallint(5) DEFAULT '0',
  `day3` smallint(5) DEFAULT '0',
  `day4` smallint(5) DEFAULT '0',
  `day5` smallint(5) DEFAULT '0',
  `day6` smallint(5) DEFAULT '0',
  `day7` smallint(5) DEFAULT '0',
  `day8` smallint(5) DEFAULT '0',
  `day9` smallint(5) DEFAULT '0',
  `day10` smallint(5) DEFAULT '0',
  `day11` smallint(5) DEFAULT '0',
  `day12` smallint(5) DEFAULT '0',
  `day13` smallint(5) DEFAULT '0',
  `day14` smallint(5) DEFAULT '0',
  `day15` smallint(5) DEFAULT '0',
  `day16` smallint(5) DEFAULT '0',
  `day17` smallint(5) DEFAULT '0',
  `day18` smallint(5) DEFAULT '0',
  `day19` smallint(5) DEFAULT '0',
  `day20` smallint(5) DEFAULT '0',
  `day21` smallint(5) DEFAULT '0',
  `day22` smallint(5) DEFAULT '0',
  `day23` smallint(5) DEFAULT '0',
  `day24` smallint(5) DEFAULT '0',
  `day25` smallint(5) DEFAULT '0',
  `day26` smallint(5) DEFAULT '0',
  `day27` smallint(5) DEFAULT '0',
  `day28` smallint(5) DEFAULT '0',
  `day29` smallint(5) DEFAULT '0',
  `day30` smallint(5) DEFAULT '0',
  `day60` smallint(5) DEFAULT '0',
  `day90` smallint(5) DEFAULT '0',
  `day120` smallint(5) DEFAULT '0',
  `day150` smallint(5) DEFAULT '0',
  `day180` smallint(5) DEFAULT '0',
  `day210` smallint(5) DEFAULT '0',
  `day240` smallint(5) DEFAULT '0',
  `day270` smallint(5) DEFAULT '0',
  `day300` smallint(5) DEFAULT '0',
  `day330` smallint(5) DEFAULT '0',
  `day360` smallint(5) DEFAULT '0',
  `updatetime` int(11) DEFAULT '0',
  PRIMARY KEY (`userid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for log_sysdaily
-- ----------------------------
DROP TABLE IF EXISTS `log_sysdaily`;
CREATE TABLE `log_sysdaily` (
  `dateid` varchar(45) NOT NULL DEFAULT '2000-01-01',
  `activeplayer` int(11) DEFAULT '0',
  `logincount` int(11) DEFAULT '0',
  `validplayer` int(11) DEFAULT '0' COMMENT '有效玩家，每天登录进去，最少呆了15分钟的。',
  `newplayer` int(11) DEFAULT '0',
  `callpayplayer` int(11) DEFAULT '0' COMMENT '调起付费接口人数',
  `payplayer` int(11) DEFAULT '0' COMMENT '付费人数',
  `callpaytimes` int(11) DEFAULT '0' COMMENT '调起付费次数',
  `paytimes` int(11) DEFAULT '0' COMMENT '付费次数',
  `callpaycount` int(11) DEFAULT '0' COMMENT '调起付费接口总额',
  `paycount` int(11) DEFAULT '0' COMMENT '付费总额',
  `newpayplayer` int(11) DEFAULT '0' COMMENT '付费新玩家人数',
  `newpaycount` int(11) DEFAULT '0' COMMENT '付费新玩家总额',
  `remain2` int(11) DEFAULT '0',
  `remain3` int(11) DEFAULT '0',
  `remain7` int(11) DEFAULT '0',
  `remain15` int(11) DEFAULT '0',
  `remain30` int(11) DEFAULT '0',
  `commissionnum` int(11) DEFAULT '0' COMMENT '佣金发放总额',
  `allfee` int(11) DEFAULT '0' COMMENT '税收总和',
  `allach` bigint(22) DEFAULT '0' COMMENT '流水总和',
  `allwin` bigint(22) DEFAULT '0' COMMENT '系统盈利总和',
  `moneyinit` bigint(22) DEFAULT '0' COMMENT '每日初始值',
  `moneyin` bigint(22) DEFAULT '0' COMMENT '系统进项',
  `moneyout` bigint(22) DEFAULT '0' COMMENT '系统出款总额',
  `sysaward` int(11) DEFAULT '0' COMMENT '系统发放奖励总额',
  `withdrawnum` int(11) DEFAULT '0' COMMENT '已经体现总额',
  `playerjetton` bigint(22) DEFAULT '0' COMMENT '玩家金币库存',
  `trdpaycount` int(11) DEFAULT '0' COMMENT '第三方支付总额',
  `vippaycount` int(11) DEFAULT '0' COMMENT 'vip支付总额',
  `activityaward` int(11) DEFAULT '0' COMMENT '活动奖励总额',
  `betakemoney` int(11) DEFAULT '0' COMMENT '待提现总额',
  `robotjetton` bigint(22) DEFAULT '0' COMMENT '机器人库存',
  `freezejetton` bigint(22) DEFAULT '0' COMMENT '被冻结的库存',
  `spcjetton` bigint(22) DEFAULT '0' COMMENT '特殊库存',
  `alljetton` bigint(22) DEFAULT '0' COMMENT '全部库存',
  `allplayer` int(11) DEFAULT '0' COMMENT '玩家总数',
  `activeseven` int(11) DEFAULT '0' COMMENT '最近7日日活',
  `activethirty` int(11) DEFAULT '0' COMMENT '最近30天日活',
  `achdayall` bigint(22) DEFAULT '0' COMMENT '今日流水',
  `achweekall` bigint(22) DEFAULT '0' COMMENT '周流水',
  `syswinday` int(11) DEFAULT '0' COMMENT '系统今日盈利',
  `syswinweek` int(11) DEFAULT '0' COMMENT '系统周盈利',
  `syswinmonth` bigint(22) DEFAULT '0' COMMENT '系统月盈利',
  `feeday` int(11) DEFAULT '0' COMMENT '每日抽水',
  `feeweek` int(11) DEFAULT '0' COMMENT '每周抽水',
  `feemonth` int(11) DEFAULT '0' COMMENT '每月抽水',
  `commisionall` bigint(22) DEFAULT '0' COMMENT '用金总额',
  `commisiontake` bigint(22) DEFAULT '0' COMMENT '已经提现的佣金总额',
  `commisionweekpre` int(11) DEFAULT '0' COMMENT '一周已经预提部分',
  `commisionweek` int(11) DEFAULT '0' COMMENT '一周的佣金',
  `commisionday` int(11) DEFAULT '0' COMMENT '当天产生的佣金',
  `commisionunissued` int(11) DEFAULT '0' COMMENT '未发佣金',
  `maxonline` int(11) DEFAULT '0' COMMENT '最高同时在线',
  `moneyincompare` int(11) DEFAULT '0' COMMENT '入金日环比',
  `moneyoutcompare` int(11) DEFAULT '0' COMMENT '出金日环比',
  `jettoncompare` bigint(22) DEFAULT '0' COMMENT '库存的日环比',
  `deviatevalue` bigint(22) DEFAULT '0' COMMENT '偏离值',
  PRIMARY KEY (`dateid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for log_unlimitedagent
-- ----------------------------
DROP TABLE IF EXISTS `log_unlimitedagent`;
CREATE TABLE `log_unlimitedagent` (
  `userid` int(11) NOT NULL,
  `nickname` varchar(45) DEFAULT NULL,
  `regdate` timestamp NULL DEFAULT CURRENT_TIMESTAMP,
  `lastdate` int(11) DEFAULT '0',
  `penultdate` int(11) DEFAULT '0',
  `loginday` int(11) DEFAULT '1' COMMENT '登录天数',
  `logincount` int(11) DEFAULT '1' COMMENT '登录次数',
  `onlinetime` bigint(22) DEFAULT '0' COMMENT '在线时长',
  `fpaydate` int(11) DEFAULT '0',
  `lpaydate` int(11) DEFAULT '0',
  `callpaycount` bigint(22) DEFAULT '0' COMMENT '调起付费总额',
  `callpaytimes` bigint(22) DEFAULT '0' COMMENT '调起付费次数',
  `paytimes` bigint(22) DEFAULT '0' COMMENT '付费次数',
  `loginconday` bigint(22) DEFAULT '0',
  `maxloginconday` bigint(22) DEFAULT '0',
  `maxjetton` bigint(22) DEFAULT '0',
  `paytoday` bigint(22) DEFAULT '0' COMMENT '今天付费',
  `payyesterday` bigint(22) DEFAULT '0' COMMENT '昨天付费',
  `payweek` bigint(22) DEFAULT '0' COMMENT '周付费',
  `paymonth` bigint(22) DEFAULT '0' COMMENT '月付费',
  `payall` bigint(22) DEFAULT '0',
  `wintoday` bigint(22) DEFAULT '0' COMMENT '今日盈利',
  `winyesterday` bigint(22) DEFAULT '0' COMMENT '昨日盈利',
  `winweek` bigint(22) DEFAULT '0' COMMENT '周盈利',
  `winlastweek` bigint(22) DEFAULT '0',
  `winmonth` bigint(22) DEFAULT '0' COMMENT '月盈利',
  `winlastmonth` int(11) DEFAULT '0',
  `winall` bigint(22) DEFAULT '0',
  `commisiontoday` int(11) DEFAULT '0',
  `commisionweek` int(11) DEFAULT '0',
  `commisionmonth` int(11) DEFAULT '0',
  `commisionall` int(11) DEFAULT '0',
  `feeday` int(11) DEFAULT '0',
  `feeweek` int(11) DEFAULT '0',
  `feemonth` int(11) DEFAULT '0',
  `feeall` int(11) DEFAULT '0',
  `achday` bigint(22) DEFAULT '0' COMMENT '今天流水',
  `achyesterday` bigint(22) DEFAULT '0' COMMENT '昨日流水',
  `achweek` bigint(22) DEFAULT '0' COMMENT '周流水',
  `achmonth` bigint(22) DEFAULT '0',
  `achall` bigint(22) DEFAULT '0' COMMENT '总流水',
  `teamusernum` int(11) DEFAULT '0' COMMENT '团队的人数',
  `newuserday` int(11) DEFAULT '0' COMMENT '在自己的推广团队中，表示每日新增',
  `activityday` int(11) DEFAULT '0' COMMENT '自己的团队中，表示每日活跃',
  `achteamday` bigint(22) DEFAULT '0' COMMENT '团队当日业绩',
  `achteamyesterday` bigint(22) DEFAULT '0',
  `achteamweek` bigint(22) DEFAULT '0' COMMENT '团队周总业绩',
  `achteammonth` bigint(22) DEFAULT '0' COMMENT '团队月总业绩',
  `achteamall` bigint(22) DEFAULT '0' COMMENT '个人团队全部总业绩',
  `drawlimit` bigint(22) DEFAULT '0',
  `achteamdaydir` bigint(22) DEFAULT '0' COMMENT '直属业绩',
  `achteamweekdir` bigint(22) DEFAULT '0' COMMENT '每周直属业绩',
  `achteamdaynodir` bigint(22) DEFAULT '0' COMMENT '每天直属业绩，除开个人业绩',
  `achteamweeknodir` bigint(22) DEFAULT '0' COMMENT '每周直属业绩，除开个人业绩',
  `payteamweek` int(11) DEFAULT '0',
  `payteamlastweek` int(11) DEFAULT '0',
  `syswinteamweek` bigint(22) DEFAULT '0',
  `syswinteamlastweek` bigint(22) DEFAULT '0',
  `syswinteamweekdir` bigint(22) DEFAULT '0',
  `syswinteamweeknodir` bigint(22) DEFAULT '0',
  `taketeamweek` int(11) DEFAULT '0',
  PRIMARY KEY (`userid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for log_unlimiteddayaward
-- ----------------------------
DROP TABLE IF EXISTS `log_unlimiteddayaward`;
CREATE TABLE `log_unlimiteddayaward` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `userid` int(11) DEFAULT '0',
  `nickname` varchar(45) DEFAULT '',
  `wiringid` int(11) DEFAULT '0',
  `createdate` timestamp NULL DEFAULT CURRENT_TIMESTAMP,
  `achall` bigint(22) DEFAULT '0',
  `achteam` bigint(22) DEFAULT '0',
  `achdirect` bigint(22) DEFAULT '0',
  `achself` bigint(22) DEFAULT '0',
  `awardnum` int(11) DEFAULT '0' COMMENT '总收益',
  `awarddirectnum` int(11) DEFAULT '0' COMMENT '直属收益',
  `state` int(11) DEFAULT '0' COMMENT '1:通过，2：未通过：0：没处理',
  `diruserlist` varchar(1045) DEFAULT '',
  `teamuserlist` varchar(1045) DEFAULT '',
  `remark` varchar(45) DEFAULT '',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for log_unlimitedweekaward
-- ----------------------------
DROP TABLE IF EXISTS `log_unlimitedweekaward`;
CREATE TABLE `log_unlimitedweekaward` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `userid` int(11) DEFAULT '0',
  `nickname` varchar(45) DEFAULT '',
  `wiringid` int(11) DEFAULT '0',
  `createdate` timestamp NULL DEFAULT CURRENT_TIMESTAMP,
  `achall` bigint(22) DEFAULT '0',
  `achteam` bigint(22) DEFAULT '0',
  `achdirect` bigint(22) DEFAULT '0',
  `achself` bigint(22) DEFAULT '0',
  `awardnum` int(11) DEFAULT '0' COMMENT '总收益',
  `awarddirectnum` int(11) DEFAULT '0' COMMENT '直属收益',
  `state` int(11) DEFAULT '0' COMMENT '1:通过，2：未通过：0：没处理',
  `diruserlist` varchar(1045) DEFAULT '',
  `teamuserlist` varchar(1045) DEFAULT '',
  `gotmoney` int(11) DEFAULT '0',
  `remark` varchar(45) DEFAULT '',
  `realhair` int(11) DEFAULT '0',
  `payteamweek` int(11) DEFAULT '0' COMMENT '团队一周的付费总额',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for log_unlimitedweekaward_0503
-- ----------------------------
DROP TABLE IF EXISTS `log_unlimitedweekaward_0503`;
CREATE TABLE `log_unlimitedweekaward_0503` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `userid` int(11) DEFAULT '0',
  `nickname` varchar(45) DEFAULT '',
  `wiringid` int(11) DEFAULT '0',
  `createdate` timestamp NULL DEFAULT CURRENT_TIMESTAMP,
  `achall` bigint(22) DEFAULT '0',
  `achteam` bigint(22) DEFAULT '0',
  `achdirect` bigint(22) DEFAULT '0',
  `achself` bigint(22) DEFAULT '0',
  `awardnum` int(11) DEFAULT '0' COMMENT '总收益',
  `awarddirectnum` int(11) DEFAULT '0' COMMENT '直属收益',
  `state` int(11) DEFAULT '0' COMMENT '1:通过，2：未通过：0：没处理',
  `diruserlist` varchar(1045) DEFAULT '',
  `teamuserlist` varchar(1045) DEFAULT '',
  `gotmoney` int(11) DEFAULT '0',
  `remark` varchar(45) DEFAULT '',
  `realhair` int(11) DEFAULT '0',
  `numtemp` int(11) DEFAULT '0',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for log_usergamelist
-- ----------------------------
DROP TABLE IF EXISTS `log_usergamelist`;
CREATE TABLE `log_usergamelist` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `userid` int(11) DEFAULT '0',
  `nickname` varchar(45) DEFAULT '',
  `gametype` int(11) DEFAULT '0',
  `gamename` varchar(45) DEFAULT '',
  `createdate` timestamp NULL DEFAULT CURRENT_TIMESTAMP,
  `tabletype` int(11) DEFAULT '0',
  `tablename` varchar(45) DEFAULT '',
  `tableid` int(11) DEFAULT '0',
  `result` varchar(512) DEFAULT '' COMMENT '开奖结构',
  `pourjetton` int(11) DEFAULT '0' COMMENT '下注总额',
  `winjetton` int(11) DEFAULT '0' COMMENT '盈利总和',
  `feenum` int(11) DEFAULT '0' COMMENT '税收总额',
  `winlist` varchar(2048) DEFAULT '' COMMENT '压住在赢的区域列表',
  `loselist` varchar(2048) DEFAULT '' COMMENT '压住在输区域列表',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='玩家玩牌的记录';

-- ----------------------------
-- Table structure for log_zhuang
-- ----------------------------
DROP TABLE IF EXISTS `log_zhuang`;
CREATE TABLE `log_zhuang` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `userid` int(11) DEFAULT '0',
  `nickname` varchar(45) DEFAULT '',
  `createdate` timestamp NULL DEFAULT CURRENT_TIMESTAMP,
  `amount` int(11) DEFAULT NULL,
  `result` varchar(45) DEFAULT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for web_action_log
-- ----------------------------
DROP TABLE IF EXISTS `web_action_log`;
CREATE TABLE `web_action_log` (
  `id` int(255) NOT NULL AUTO_INCREMENT,
  `admin_id` int(11) DEFAULT NULL COMMENT '操作人id',
  `username` varchar(32) DEFAULT NULL COMMENT '当前操作人名称',
  `role` varchar(32) DEFAULT NULL COMMENT '当前角色',
  `controller` varchar(32) DEFAULT NULL COMMENT '控制器',
  `action` varchar(32) DEFAULT NULL COMMENT '方法',
  `params` text COMMENT '参数',
  `description` text COMMENT '说明',
  `created_at` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP COMMENT '操作时间',
  `channel` varchar(20) DEFAULT '' COMMENT '渠道',
  `prechannel` varchar(20) DEFAULT '' COMMENT '大渠道',
  `userid` int(11) DEFAULT '0' COMMENT '背操作人id，没有为0',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=1676 DEFAULT CHARSET=utf8mb4;

-- ----------------------------
-- Table structure for web_auth_group
-- ----------------------------
DROP TABLE IF EXISTS `web_auth_group`;
CREATE TABLE `web_auth_group` (
  `id` mediumint(8) unsigned NOT NULL AUTO_INCREMENT,
  `title` char(100) NOT NULL DEFAULT '',
  `status` tinyint(1) NOT NULL DEFAULT '1',
  `rules` text,
  `explain` varchar(255) DEFAULT NULL COMMENT '说明',
  `time` datetime DEFAULT NULL COMMENT '创建时间',
  PRIMARY KEY (`id`) USING BTREE
) ENGINE=InnoDB AUTO_INCREMENT=14 DEFAULT CHARSET=utf8 ROW_FORMAT=DYNAMIC COMMENT='权限组表';

-- ----------------------------
-- Table structure for web_auth_group_access
-- ----------------------------
DROP TABLE IF EXISTS `web_auth_group_access`;
CREATE TABLE `web_auth_group_access` (
  `uid` mediumint(8) unsigned NOT NULL,
  `group_id` mediumint(8) unsigned NOT NULL,
  UNIQUE KEY `uid_group_id` (`uid`,`group_id`) USING BTREE,
  KEY `uid` (`uid`) USING BTREE,
  KEY `group_id` (`group_id`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=DYNAMIC COMMENT='权限中间表';

-- ----------------------------
-- Table structure for web_auth_rule
-- ----------------------------
DROP TABLE IF EXISTS `web_auth_rule`;
CREATE TABLE `web_auth_rule` (
  `id` mediumint(8) unsigned NOT NULL AUTO_INCREMENT,
  `sort` int(8) NOT NULL DEFAULT '0' COMMENT '排序',
  `pid` int(8) NOT NULL DEFAULT '0' COMMENT '父级ID',
  `name` char(80) DEFAULT NULL,
  `title` char(20) NOT NULL DEFAULT '',
  `ico` varchar(255) DEFAULT '' COMMENT '菜单图标',
  `ismenu` tinyint(1) NOT NULL DEFAULT '0' COMMENT '是否菜单  0=不是  1=是',
  `status` tinyint(1) NOT NULL DEFAULT '1',
  `condition` char(100) NOT NULL DEFAULT '',
  `type` tinyint(1) NOT NULL DEFAULT '1' COMMENT '是否用condition进行验证，1为默认验证，0为不用condition验证',
  `describe` varchar(255) NOT NULL DEFAULT '' COMMENT '描述',
  `group` varchar(64) NOT NULL DEFAULT '',
  `group_label` varchar(64) NOT NULL DEFAULT '',
  PRIMARY KEY (`id`) USING BTREE,
  UNIQUE KEY `name` (`name`) USING BTREE
) ENGINE=InnoDB AUTO_INCREMENT=285 DEFAULT CHARSET=utf8 ROW_FORMAT=DYNAMIC COMMENT='权限规则表';

-- ----------------------------
-- Table structure for web_auth_special
-- ----------------------------
DROP TABLE IF EXISTS `web_auth_special`;
CREATE TABLE `web_auth_special` (
  `id` int(8) unsigned NOT NULL COMMENT '管理用户的UID',
  `title` char(100) NOT NULL DEFAULT '',
  `status` tinyint(1) NOT NULL DEFAULT '1',
  `rules` text NOT NULL,
  `explain` varchar(255) DEFAULT NULL COMMENT '说明',
  `time` datetime DEFAULT NULL COMMENT '创建时间',
  PRIMARY KEY (`id`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=DYNAMIC COMMENT='权限组表';

-- ----------------------------
-- Table structure for web_config
-- ----------------------------
DROP TABLE IF EXISTS `web_config`;
CREATE TABLE `web_config` (
  `channel` varchar(255) NOT NULL DEFAULT '' COMMENT '渠道',
  `key` varchar(255) NOT NULL,
  `value` text NOT NULL,
  `group` varchar(32) NOT NULL,
  `description` varchar(255) NOT NULL,
  PRIMARY KEY (`channel`,`key`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for web_dict
-- ----------------------------
DROP TABLE IF EXISTS `web_dict`;
CREATE TABLE `web_dict` (
  `id` int(11) NOT NULL AUTO_INCREMENT COMMENT '字典编码',
  `dict_name` varchar(100) DEFAULT NULL COMMENT '字典名称',
  `dict_category` varchar(100) NOT NULL COMMENT '字典KEY',
  `dict_value` varchar(255) NOT NULL,
  `reorder` int(4) DEFAULT NULL COMMENT '排序',
  `enable_status` int(4) DEFAULT '1' COMMENT '启用状态 启用-1、禁用-0',
  `creater` varchar(50) DEFAULT NULL COMMENT '创建人',
  `create_time` datetime DEFAULT NULL COMMENT '创建时间',
  `modifier` varchar(50) DEFAULT NULL COMMENT '修改人',
  `modify_time` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP COMMENT '更新时间',
  `remark` varchar(255) DEFAULT NULL COMMENT '备注',
  `themes` varchar(100) DEFAULT NULL COMMENT '所属主题',
  `dict_type` int(4) DEFAULT '1' COMMENT '字典类型(1管理后台，2操盘后台)',
  `channel` varchar(255) DEFAULT '' COMMENT '渠道',
  `prechannel` varchar(255) DEFAULT NULL COMMENT '大渠道号',
  `dict_group` varchar(255) DEFAULT '' COMMENT '字典分组',
  PRIMARY KEY (`id`) USING BTREE
) ENGINE=InnoDB AUTO_INCREMENT=139 DEFAULT CHARSET=utf8 COMMENT='数据字典表';

-- ----------------------------
-- Table structure for web_drawmoney_log
-- ----------------------------
DROP TABLE IF EXISTS `web_drawmoney_log`;
CREATE TABLE `web_drawmoney_log` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `admin_id` int(11) DEFAULT NULL COMMENT '操作人',
  `admin_name` varchar(255) DEFAULT NULL COMMENT '操作人名称',
  `draw_id` int(11) DEFAULT NULL COMMENT '订单id',
  `admin_remark` varchar(255) DEFAULT NULL COMMENT '订单内容',
  `risktype` int(11) DEFAULT NULL COMMENT '风控状态，0=等待处理，1=风控审核，2=主管风控，3=等待出款,4=系统出款',
  `created_at` datetime DEFAULT NULL COMMENT '操作时间',
  `opttype` int(11) DEFAULT NULL COMMENT '处理意见，1=通过，2=拒绝，3=下一个流程',
  `next_risktype` int(11) DEFAULT NULL COMMENT '风控状态，0=等待处理，1=风控审核，2=主管风控，3=等待出款,4=系统出款',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=288 DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for web_firstpay_order
-- ----------------------------
DROP TABLE IF EXISTS `web_firstpay_order`;
CREATE TABLE `web_firstpay_order` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `uid` int(11) NOT NULL COMMENT '用户id',
  `plat_order_id` varchar(255) NOT NULL COMMENT '平台订单号',
  `order_id` varchar(255) NOT NULL COMMENT '系统订单号',
  `out_order_id` varchar(255) NOT NULL COMMENT '三方订单号',
  `amount` varchar(255) NOT NULL DEFAULT '' COMMENT '金额',
  `createdate` timestamp NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP COMMENT '时间',
  `channel` varchar(50) DEFAULT NULL,
  `prechannel` varchar(50) DEFAULT NULL,
  `recharge_type` int(11) DEFAULT '1' COMMENT '1=前台充，2=后台充',
  PRIMARY KEY (`id`),
  UNIQUE KEY `uid` (`uid`)
) ENGINE=InnoDB AUTO_INCREMENT=2 DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for web_optorder_log
-- ----------------------------
DROP TABLE IF EXISTS `web_optorder_log`;
CREATE TABLE `web_optorder_log` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `optid` int(11) DEFAULT NULL COMMENT '操作人id',
  `optname` varchar(255) DEFAULT NULL COMMENT '操作人名称',
  `userid` int(11) DEFAULT NULL COMMENT '用户id',
  `orderid` varchar(32) DEFAULT NULL COMMENT '手动订单id',
  `amount` bigint(20) DEFAULT NULL COMMENT '金额',
  `fee` bigint(20) DEFAULT NULL COMMENT '手续费',
  `payaccount` varchar(255) DEFAULT NULL COMMENT '收款账号',
  `payee` varchar(255) DEFAULT NULL COMMENT '收款人',
  `created_at` datetime DEFAULT NULL COMMENT '创建时间',
  `channel` varchar(50) DEFAULT NULL,
  `prechannel` varchar(50) DEFAULT NULL,
  `paytype` int(11) DEFAULT NULL COMMENT '1=支付宝，3=银行卡',
  `remark` varchar(255) DEFAULT NULL COMMENT '手动订单的备注',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=13 DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for web_pics
-- ----------------------------
DROP TABLE IF EXISTS `web_pics`;
CREATE TABLE `web_pics` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `pic_type` varchar(255) DEFAULT NULL COMMENT '图片类型',
  `status` tinyint(1) DEFAULT '1' COMMENT '1=启用，0=不启用，默认启用=1',
  `pic_url` varchar(255) DEFAULT NULL COMMENT '图片地址',
  `description` varchar(255) DEFAULT NULL COMMENT '备注',
  `create_at` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  `channel` varchar(40) DEFAULT NULL COMMENT '渠道',
  `prechannel` varchar(40) DEFAULT '' COMMENT '大渠道',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=40 DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for web_session
-- ----------------------------
DROP TABLE IF EXISTS `web_session`;
CREATE TABLE `web_session` (
  `id` int(11) NOT NULL AUTO_INCREMENT COMMENT 'uid',
  `uid` int(11) NOT NULL COMMENT 'uid',
  `token` varchar(60) COLLATE utf8mb4_unicode_ci NOT NULL DEFAULT '',
  `time_out` int(11) NOT NULL COMMENT '登出时间',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- ----------------------------
-- Table structure for web_sites
-- ----------------------------
DROP TABLE IF EXISTS `web_sites`;
CREATE TABLE `web_sites` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `channel_id` int(11) DEFAULT NULL COMMENT '渠道ID',
  `class_name` varchar(32) DEFAULT NULL COMMENT '类目名称，英文',
  `class_label` varchar(32) DEFAULT NULL COMMENT '类目名称，中文',
  `class_domain` varchar(100) DEFAULT NULL COMMENT '类目主域名',
  `class_type` varchar(10) DEFAULT NULL COMMENT '类目类型：html=静态页，php=动态页',
  PRIMARY KEY (`id`) USING BTREE
) ENGINE=InnoDB AUTO_INCREMENT=144 DEFAULT CHARSET=utf8mb4;

-- ----------------------------
-- Table structure for web_withdraw_offline
-- ----------------------------
DROP TABLE IF EXISTS `web_withdraw_offline`;
CREATE TABLE `web_withdraw_offline` (
  `id` int(11) NOT NULL AUTO_INCREMENT COMMENT '平台收款ID',
  `user_id` int(11) DEFAULT NULL COMMENT '用户ID(对应用户表)',
  `band_name` varchar(20) DEFAULT NULL COMMENT '银行名称',
  `open_band` varchar(100) DEFAULT NULL COMMENT '开户行名称',
  `user_name` varchar(100) NOT NULL COMMENT '开户人姓名',
  `account_type` int(1) NOT NULL DEFAULT '0' COMMENT '账户类型(1:支付宝 2:微信 3:银行卡,4:线下支付宝 5线下微信 6线下银行卡)',
  `account_number` varchar(50) NOT NULL COMMENT '收款账号',
  `receivablescode` varchar(255) DEFAULT NULL COMMENT '二维码地址',
  `withdrawal_quota` int(8) DEFAULT '99999999' COMMENT '限额',
  `enable_status` int(1) DEFAULT '1' COMMENT '启用停用状态(0不启用，1启用，默认1)',
  `reorder` int(5) DEFAULT '0' COMMENT '排序',
  `creater` varchar(100) DEFAULT NULL COMMENT '创建人',
  `create_time` datetime DEFAULT NULL COMMENT '创建时间',
  `modifier` varchar(100) DEFAULT NULL COMMENT '修改人',
  `modifier_time` timestamp NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP COMMENT '更新时间',
  `channel` varchar(255) DEFAULT NULL COMMENT '渠道号',
  `prechannel` varchar(50) DEFAULT '' COMMENT '大渠道',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=36 DEFAULT CHARSET=utf8 COMMENT='收款信息配置';

-- ----------------------------
-- Table structure for web_withdrawconf
-- ----------------------------
DROP TABLE IF EXISTS `web_withdrawconf`;
CREATE TABLE `web_withdrawconf` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `payid` int(11) DEFAULT '0',
  `withdraw_type` varchar(45) DEFAULT '',
  `channel` varchar(45) DEFAULT '',
  `name` varchar(45) DEFAULT '',
  `wechat` int(11) DEFAULT '0',
  `alipay` int(11) DEFAULT '0',
  `bankcard` int(11) DEFAULT '0',
  `des` varchar(127) DEFAULT '',
  `url` varchar(255) DEFAULT '' COMMENT '支付调起URL',
  `notifyurl` varchar(255) DEFAULT NULL COMMENT '支付回调URL',
  `appkey` varchar(255) DEFAULT NULL COMMENT '通道分配appkey',
  `appid` varchar(255) DEFAULT '0' COMMENT '通道分配商户appid',
  `appmark` varchar(255) DEFAULT NULL COMMENT '标识',
  `appencrymode` varchar(40) DEFAULT NULL COMMENT '通道加密方式',
  `state` varchar(45) DEFAULT '',
  `shoptype` int(11) DEFAULT '0',
  PRIMARY KEY (`id`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
