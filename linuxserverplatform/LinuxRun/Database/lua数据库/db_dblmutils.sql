/*
Navicat MySQL Data Transfer

Source Server         : 156.241.5.157
Source Server Version : 50730
Source Host           : 156.241.5.157:3306
Source Database       : db_dblmutils

Target Server Type    : MYSQL
Target Server Version : 50730
File Encoding         : 65001

Date: 2020-06-11 10:04:02
*/

SET FOREIGN_KEY_CHECKS=0;

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
  `prechannel` varchar(20) DEFAULT '' COMMENT '大渠道',
  `channel` varchar(20) DEFAULT '' COMMENT '渠道号',
  `activityname` varchar(20) DEFAULT '' COMMENT '活动标题，英文',
  `propid` varchar(255) DEFAULT '0' COMMENT '道具ID',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=33 DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for log_alarm
-- ----------------------------
DROP TABLE IF EXISTS `log_alarm`;
CREATE TABLE `log_alarm` (
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT COMMENT '自增属性',
  `ranktype` tinyint(3) DEFAULT '1' COMMENT '排行榜类型，1:1个小时排行榜，3:3个小时排行榜，，以此类推',
  `wintype` tinyint(2) DEFAULT '1' COMMENT '输赢类型，1：赢排行，2：输排行',
  `alarmlevel` tinyint(2) DEFAULT '1' COMMENT '警告等级，1：notice，2：warning',
  `userid` int(11) DEFAULT '0' COMMENT '玩家id',
  `nickname` varchar(45) DEFAULT NULL COMMENT '名字',
  `winmoney` bigint(20) DEFAULT '0' COMMENT '输赢钱',
  `payamountdaily` int(11) DEFAULT '0' COMMENT '今日充值总额',
  `winamountdaily` int(11) DEFAULT '0' COMMENT '今日输赢总和',
  `payamountweek` int(11) DEFAULT '0' COMMENT '本周充值总额',
  `winamountweek` int(11) DEFAULT '0' COMMENT '本周输赢总和',
  `payamount` int(11) DEFAULT '0' COMMENT '总充值',
  `winamount` int(11) DEFAULT '0' COMMENT '总输赢',
  `payamountyesterday` int(11) DEFAULT '0' COMMENT '昨天总充值',
  `winamountyesterday` int(11) DEFAULT '0' COMMENT '昨天总输赢',
  `channel` varchar(60) DEFAULT '' COMMENT '渠道',
  `prechannel` varchar(60) DEFAULT NULL,
  PRIMARY KEY (`id`),
  KEY `rankwin` (`ranktype`,`wintype`) USING BTREE,
  KEY `rankwinnotice` (`ranktype`,`wintype`,`alarmlevel`),
  KEY `ranknotice` (`ranktype`,`alarmlevel`),
  KEY `rank` (`ranktype`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=DYNAMIC;

-- ----------------------------
-- Table structure for log_announcement
-- ----------------------------
DROP TABLE IF EXISTS `log_announcement`;
CREATE TABLE `log_announcement` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `title` varchar(255) DEFAULT NULL COMMENT '标题',
  `content` mediumtext COMMENT '内容',
  `sendtime` datetime DEFAULT CURRENT_TIMESTAMP COMMENT '定时发送时间，日期格式，立刻发送=null，服务器更新',
  `sendid` int(11) DEFAULT '1' COMMENT '发送者id，系统id=1，固定值',
  `receiveid` varchar(255) DEFAULT NULL COMMENT '接收者 0=表示所有人，多个人，用逗号分隔',
  `sendstate` int(11) NOT NULL DEFAULT '0' COMMENT '发送状态 0未发送（可编辑） 1：已发送（不可编辑）',
  `updatetime` datetime DEFAULT NULL ON UPDATE CURRENT_TIMESTAMP COMMENT '更新时间',
  `newtype` int(11) DEFAULT NULL COMMENT '公共类型，1=通知，2=活动，3=公告，4=赛事',
  `channel` varchar(255) DEFAULT NULL COMMENT '渠道',
  `prechannel` varchar(255) DEFAULT NULL COMMENT '大渠道',
  PRIMARY KEY (`id`)
) ENGINE=MyISAM AUTO_INCREMENT=11997 DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for log_award
-- ----------------------------
DROP TABLE IF EXISTS `log_award`;
CREATE TABLE `log_award` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `userid` int(11) NOT NULL,
  `dateid` varchar(20) DEFAULT NULL COMMENT '时间(周)',
  `nickname` varchar(50) DEFAULT '' COMMENT '昵称',
  `diraward` int(13) DEFAULT '0' COMMENT '直属分成',
  `nodiraward` int(13) DEFAULT '0' COMMENT '团队分成',
  `state` int(3) DEFAULT '0' COMMENT '状态(0.未发放,1.已发放)',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

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
  `channel` varchar(20) DEFAULT '' COMMENT '渠道',
  `prechannel` varchar(20) DEFAULT '',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=48456 DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for log_betdaily
-- ----------------------------
DROP TABLE IF EXISTS `log_betdaily`;
CREATE TABLE `log_betdaily` (
  `dateid` date NOT NULL COMMENT '日期',
  `bet` bigint(20) DEFAULT NULL COMMENT '普通投注量',
  `tybet` bigint(20) DEFAULT NULL COMMENT '体验金投注量',
  `channel` varchar(50) DEFAULT NULL,
  `prechannel` varchar(50) NOT NULL,
  PRIMARY KEY (`dateid`,`prechannel`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for log_commisionagent
-- ----------------------------
DROP TABLE IF EXISTS `log_commisionagent`;
CREATE TABLE `log_commisionagent` (
  `id` bigint(20) unsigned NOT NULL AUTO_INCREMENT COMMENT '自增id',
  `time` bigint(20) DEFAULT '0' COMMENT '时间搓',
  `userid` int(11) DEFAULT '0' COMMENT '活动佣金玩家',
  `type` tinyint(4) DEFAULT '0' COMMENT '0：个人收益  1：团队收益',
  `level` int(11) DEFAULT '0' COMMENT '贡献级别，0：自己贡献自己，n：收益者与贡献者相差级别',
  `producerid` int(11) DEFAULT '0' COMMENT '抽水玩家',
  `pumpcount` int(11) DEFAULT '0' COMMENT '抽水数量',
  `amount` int(11) DEFAULT '0' COMMENT '得到的佣金',
  `diruserid` int(11) DEFAULT '0' COMMENT '收益玩家的直属下级',
  `channel` varchar(32) DEFAULT '' COMMENT '渠道号',
  PRIMARY KEY (`id`),
  KEY `userid` (`userid`),
  KEY `useridtime` (`time`,`userid`)
) ENGINE=InnoDB AUTO_INCREMENT=1774 DEFAULT CHARSET=utf8;

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
  `channel` varchar(20) DEFAULT '' COMMENT '渠道',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=274 DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for log_dayaward
-- ----------------------------
DROP TABLE IF EXISTS `log_dayaward`;
CREATE TABLE `log_dayaward` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `userid` int(11) DEFAULT NULL,
  `nickname` varchar(50) DEFAULT '' COMMENT '昵称',
  `payamount` int(11) DEFAULT '0' COMMENT '充值总额',
  `jetton` int(11) DEFAULT '0' COMMENT '库存',
  `takeamount` int(11) DEFAULT '0' COMMENT '提现总额',
  `profitnum` int(11) DEFAULT '0' COMMENT '盈利',
  `createdate` varchar(20) DEFAULT '' COMMENT '时间',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for log_daylevelaward
-- ----------------------------
DROP TABLE IF EXISTS `log_daylevelaward`;
CREATE TABLE `log_daylevelaward` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `userid` int(11) DEFAULT '0',
  `datestr` varchar(45) DEFAULT NULL,
  `awardall` float DEFAULT '0' COMMENT '奖励总额',
  `ach1` bigint(21) DEFAULT '0' COMMENT '一级流水总额',
  `award1` float DEFAULT '0' COMMENT '一级分润总额',
  `ach2` bigint(21) DEFAULT '0' COMMENT '二级流水总额',
  `award2` float DEFAULT '0' COMMENT '二级分润总额',
  `ach3` bigint(21) DEFAULT '0' COMMENT '三级流水总额',
  `award3` float DEFAULT '0' COMMENT '三级分润总额',
  `ach4` bigint(21) DEFAULT '0' COMMENT '四级流水总额',
  `award4` float DEFAULT '0' COMMENT '四级分润总额',
  `ach5` bigint(21) DEFAULT '0' COMMENT '五级流水总额',
  `award5` float DEFAULT '0' COMMENT '五级分润总额',
  `ach6` bigint(21) DEFAULT '0' COMMENT '六级流水总额',
  `award6` float DEFAULT '0' COMMENT '六级分润总额',
  `createdate` timestamp NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`id`),
  KEY `index2` (`userid`),
  KEY `index3` (`datestr`)
) ENGINE=InnoDB AUTO_INCREMENT=2130 DEFAULT CHARSET=utf8mb4;

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
  `state` int(11) DEFAULT '0' COMMENT '状态 0  未发放  1  已发放',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for log_discounttrans
-- ----------------------------
DROP TABLE IF EXISTS `log_discounttrans`;
CREATE TABLE `log_discounttrans` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `userid` int(11) DEFAULT NULL COMMENT '用户',
  `orderid` varchar(20) DEFAULT NULL COMMENT '订单号',
  `amount` bigint(64) DEFAULT NULL COMMENT '赠送金币',
  `arch` bigint(64) DEFAULT NULL COMMENT '流水',
  `remark` varchar(255) DEFAULT NULL COMMENT '备注',
  `created_at` datetime DEFAULT NULL COMMENT '创建时间',
  `optid` int(11) DEFAULT NULL COMMENT '操作人id',
  `optname` varchar(40) DEFAULT NULL COMMENT '操作人名称',
  `distype` tinyint(1) DEFAULT NULL COMMENT '优惠类型 1 系统优惠 2 优秀代理奖 3 异常资金',
  `count` int(11) DEFAULT NULL COMMENT '场次',
  `channel` varchar(20) DEFAULT NULL,
  `prechannel` varchar(20) DEFAULT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=57 DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for log_draw
-- ----------------------------
DROP TABLE IF EXISTS `log_draw`;
CREATE TABLE `log_draw` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `realname` varchar(50) DEFAULT '' COMMENT '真实名称',
  `userid` int(11) DEFAULT NULL COMMENT '用户id',
  `prechannel` varchar(50) DEFAULT '',
  `channel` varchar(50) DEFAULT '',
  `created_at` datetime DEFAULT NULL COMMENT '创建时间',
  `amount` bigint(20) DEFAULT '0' COMMENT '金额',
  `bankname` varchar(255) DEFAULT '' COMMENT '银行名称，当payway=3时使用',
  `bankcard` varchar(30) DEFAULT '' COMMENT '银行卡号，当payway=3时使用',
  `bankman` varchar(50) DEFAULT '' COMMENT '收款人，当payway=3时使用',
  `paytype` varchar(255) NOT NULL COMMENT '交易类型，1=vip，2=在线出款，3=线下出款，4=系统优惠，5=异常资金',
  `status` int(11) NOT NULL DEFAULT '0' COMMENT '交易状态，0=处理中，1=成功，2=失败',
  `source` varchar(100) DEFAULT '' COMMENT '格式：IP/设备，如 202.112.123.11/PC',
  `optremark` varchar(255) DEFAULT '' COMMENT '审核人审核备注',
  `orderid` varchar(50) DEFAULT NULL COMMENT '订单号',
  `updated_at` datetime DEFAULT NULL COMMENT '修改时间',
  `optid` int(11) DEFAULT NULL COMMENT '审核人id',
  `optname` varchar(50) DEFAULT '' COMMENT '审核人名称',
  `opttime` datetime DEFAULT NULL COMMENT '审核时间',
  `payway` int(11) DEFAULT NULL COMMENT '支付渠道，1=支付宝，2=微信，3银行卡，4=人工充值',
  `pay_account` varchar(50) DEFAULT NULL COMMENT '支付账号，当payway=1或者2时使用',
  `pay_name` varchar(50) DEFAULT NULL COMMENT '支付名称，当payway=1或者2时使用',
  `remark` varchar(255) DEFAULT '' COMMENT '备注',
  `optstatus` int(11) DEFAULT '1' COMMENT '审核状态，1=成功，0=失败',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=123 DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for log_drawlottoryrecords
-- ----------------------------
DROP TABLE IF EXISTS `log_drawlottoryrecords`;
CREATE TABLE `log_drawlottoryrecords` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `userid` int(11) DEFAULT NULL,
  `drawtime` datetime DEFAULT NULL COMMENT '抽奖时间',
  `category` int(11) DEFAULT '0' COMMENT '类别 0 翻翻乐 1抽奖券',
  `prizeid` int(11) DEFAULT NULL COMMENT '奖品id',
  `prizecount` int(11) DEFAULT '0' COMMENT '奖品数量',
  `payjetton` int(11) DEFAULT '0' COMMENT '花费金额 0 免费',
  `prizedesc` varchar(100) DEFAULT NULL COMMENT '奖品描述',
  `channel` varchar(30) DEFAULT NULL COMMENT '渠道',
  `prechannel` varchar(40) DEFAULT NULL COMMENT '大渠道',
  `state` int(11) DEFAULT '0' COMMENT '中奖状态 0 未中奖 1中奖',
  `propid` varchar(50) DEFAULT NULL,
  `proptype` int(255) DEFAULT NULL COMMENT '道具类型',
  `propname` varchar(50) DEFAULT NULL COMMENT '道具名字',
  `propvalue` int(11) DEFAULT '0' COMMENT '道具面值',
  `conditions` int(11) DEFAULT '0' COMMENT '百分比',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=569 DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for log_expertdata
-- ----------------------------
DROP TABLE IF EXISTS `log_expertdata`;
CREATE TABLE `log_expertdata` (
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `channel` varchar(45) DEFAULT '' COMMENT '渠道号',
  `prechannel` varchar(45) DEFAULT '' COMMENT '大渠道',
  `plannums` bigint(20) DEFAULT '0' COMMENT '专家方案数量',
  `buynums` bigint(20) DEFAULT '0' COMMENT '购买人数',
  `buymoney` bigint(20) DEFAULT '0' COMMENT '购买总金额',
  `follownums` bigint(20) DEFAULT '0' COMMENT '跟投人数',
  `followmoney` bigint(20) DEFAULT '0' COMMENT '跟投金额',
  `winmoney` bigint(20) DEFAULT '0' COMMENT '跟投盈亏',
  `createtime` bigint(20) DEFAULT '0' COMMENT '生成记录时间',
  PRIMARY KEY (`id`),
  KEY `channel` (`channel`),
  KEY `prechannel` (`prechannel`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for log_expertdatadaily
-- ----------------------------
DROP TABLE IF EXISTS `log_expertdatadaily`;
CREATE TABLE `log_expertdatadaily` (
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `channel` varchar(45) DEFAULT '' COMMENT '渠道号',
  `prechannel` varchar(45) DEFAULT '' COMMENT '大渠道',
  `plannums` bigint(20) DEFAULT '0' COMMENT '专家方案数量',
  `buynums` bigint(20) DEFAULT '0' COMMENT '购买人数',
  `buymoney` bigint(20) DEFAULT '0' COMMENT '购买总金额',
  `follownums` bigint(20) DEFAULT '0' COMMENT '跟投人数',
  `followmoney` bigint(20) DEFAULT '0' COMMENT '跟投金额',
  `winmoney` bigint(20) DEFAULT '0' COMMENT '跟投盈亏',
  `dateid` varchar(45) DEFAULT '' COMMENT '时间id',
  `createtime` bigint(20) DEFAULT '0' COMMENT '生成记录时间',
  PRIMARY KEY (`id`),
  KEY `channel` (`channel`) USING BTREE,
  KEY `prechannel` (`prechannel`) USING BTREE,
  KEY `dateid` (`dateid`),
  KEY `chtime` (`channel`,`dateid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for log_fbanalysis
-- ----------------------------
DROP TABLE IF EXISTS `log_fbanalysis`;
CREATE TABLE `log_fbanalysis` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `dateid` date DEFAULT NULL,
  `channel` varchar(50) DEFAULT NULL,
  `prechannel` varchar(50) DEFAULT NULL,
  `registercount` int(11) DEFAULT NULL COMMENT '当⽇注册⼈数',
  `activecount` int(11) DEFAULT NULL COMMENT '活跃数',
  `rechargetotal` bigint(20) DEFAULT NULL COMMENT '当⽇充值⾦额',
  `drawtotal` bigint(20) DEFAULT NULL COMMENT '当⽇提现⾦额',
  `drawfeetotal` bigint(20) DEFAULT NULL COMMENT '当日提款手续费',
  `unfinishtotal` bigint(20) DEFAULT NULL COMMENT '从⽤户投注开始，到未结算到⽤户的钱包中的钱，都叫未结算，仅限现 ⾦钱包，体验⾦不算',
  `moneytotal` bigint(20) DEFAULT NULL COMMENT '平台用户当前所有账户中的余额',
  `bettotal` bigint(20) DEFAULT NULL COMMENT '当⽇投注⾦额（现⾦钱包）',
  `recharge1count` bigint(20) DEFAULT NULL COMMENT '当⽇⽤户第⼀次充值的⾦额',
  `recharge2count` bigint(20) DEFAULT NULL COMMENT '当日用户第二次充值的金额',
  `recharge3count` bigint(20) DEFAULT NULL COMMENT '当日用户第三次充值的金额',
  `rechargegt100count` bigint(20) DEFAULT NULL COMMENT '当⽇充值超过≥100元的充值订单',
  `rechargegt1000count` bigint(20) DEFAULT NULL COMMENT '当日充值超过≥1000元的充值订单',
  `rechargegt10000count` bigint(20) DEFAULT NULL COMMENT '当日充值超过≥10000元的充值订单',
  `mastercount` int(11) DEFAULT NULL COMMENT '当⽇认证通过的专家⼈数',
  `plancount` int(11) DEFAULT NULL COMMENT '当⽇专家发布⽅案的个数',
  `buyplancount` int(11) DEFAULT NULL COMMENT '当⽇购买⽅案的⼈数',
  `buyplantotal` bigint(20) DEFAULT NULL COMMENT '当⽇购买⽅案的⾦额',
  `flowplancount` int(11) DEFAULT NULL COMMENT '当⽇跟投⼈数',
  `flowplantotal` bigint(20) DEFAULT NULL COMMENT '当⽇跟投⾦额',
  `incomerate` int(11) DEFAULT NULL COMMENT '当⽇已经结算的赛事的收益率，（派彩⾦额-投注⾦额）/投注⾦额=收 益率，正数是⽤户赚钱，负数则是平台赚钱',
  `allincomerate` int(11) DEFAULT NULL COMMENT '总收益率，参见incomerate字段',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for log_firstpay
-- ----------------------------
DROP TABLE IF EXISTS `log_firstpay`;
CREATE TABLE `log_firstpay` (
  `userid` int(11) NOT NULL COMMENT '用户id做主键',
  `money` bigint(20) DEFAULT NULL COMMENT '充值金额',
  `rechargetype` int(11) DEFAULT NULL COMMENT '1=后台人工充，2=前台用户充',
  `orderid` varchar(50) DEFAULT NULL COMMENT '关联订单号',
  `create_at` datetime DEFAULT NULL COMMENT '创建时间',
  `channel` varchar(50) DEFAULT NULL,
  `prechannel` varchar(50) DEFAULT NULL,
  PRIMARY KEY (`userid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for log_flipprofitreport
-- ----------------------------
DROP TABLE IF EXISTS `log_flipprofitreport`;
CREATE TABLE `log_flipprofitreport` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `profitid` varchar(60) DEFAULT NULL COMMENT '收益报告id',
  `userid` int(11) DEFAULT NULL COMMENT '玩家id',
  `win` bigint(20) DEFAULT '0' COMMENT '本次报告全部收益',
  `freeflipcount` int(11) DEFAULT '0' COMMENT '本次报告免费翻翻乐次数',
  `orderidlist` text COMMENT '订单id list',
  `status` int(11) DEFAULT '0' COMMENT '状态 0 未推送 1 已推送',
  `channel` varchar(30) DEFAULT NULL COMMENT '渠道',
  `prechannel` varchar(40) DEFAULT NULL COMMENT '大渠道',
  `createtime` timestamp NULL DEFAULT NULL ON UPDATE CURRENT_TIMESTAMP,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=276 DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for log_frozenjettondetail
-- ----------------------------
DROP TABLE IF EXISTS `log_frozenjettondetail`;
CREATE TABLE `log_frozenjettondetail` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `userid` int(11) DEFAULT '0',
  `nickname` varchar(128) DEFAULT '',
  `createdate` timestamp NULL DEFAULT CURRENT_TIMESTAMP,
  `gametype` int(11) DEFAULT '0',
  `amount` int(11) DEFAULT '0' COMMENT '变化数量',
  `usermoney` int(11) DEFAULT '0' COMMENT '变化后金币',
  `channel` varchar(45) DEFAULT '',
  `prechannel` varchar(45) DEFAULT '' COMMENT '大渠道号',
  `gamenumber` varchar(45) DEFAULT '' COMMENT '牌局编号',
  PRIMARY KEY (`id`),
  KEY `index2` (`userid`)
) ENGINE=InnoDB AUTO_INCREMENT=367540 DEFAULT CHARSET=utf8;

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
  `result` varchar(4096) DEFAULT '' COMMENT '开奖结果列表',
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
  `gamenumber` varchar(45) DEFAULT '' COMMENT '牌局编号',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='记录每一个牌局的详细信息';

-- ----------------------------
-- Table structure for log_gamejettondetail
-- ----------------------------
DROP TABLE IF EXISTS `log_gamejettondetail`;
CREATE TABLE `log_gamejettondetail` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `userid` int(11) DEFAULT '0',
  `nickname` varchar(128) DEFAULT '',
  `createdate` timestamp NULL DEFAULT CURRENT_TIMESTAMP,
  `opttype` int(11) DEFAULT '0',
  `optmsg` varchar(45) DEFAULT '',
  `amount` int(11) DEFAULT '0' COMMENT '变化数量',
  `usermoney` int(11) DEFAULT '0' COMMENT '变化后金币',
  `remark` varchar(256) DEFAULT '' COMMENT '备注',
  `detail` mediumtext,
  `channel` varchar(45) DEFAULT '',
  `prechannel` varchar(45) DEFAULT '' COMMENT '大渠道号',
  `achamount` int(11) DEFAULT '0' COMMENT '流水',
  `gamenumber` varchar(45) DEFAULT '' COMMENT '牌局编号',
  `season` int(11) DEFAULT NULL COMMENT '金币变化的原因',
  PRIMARY KEY (`id`),
  KEY `index2` (`userid`)
) ENGINE=InnoDB AUTO_INCREMENT=367559 DEFAULT CHARSET=utf8;

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
  `usermoney` int(11) DEFAULT '0' COMMENT '现金钱包',
  `remark` varchar(256) DEFAULT '' COMMENT '备注',
  `detail` mediumtext,
  `channel` varchar(45) DEFAULT '',
  `achamount` int(11) DEFAULT '0' COMMENT '流水',
  `gamenumber` varchar(45) DEFAULT '' COMMENT '牌局编号',
  `season` int(11) DEFAULT NULL COMMENT '金币变化的原因',
  `prechannel` varchar(50) DEFAULT '' COMMENT '大渠道',
  `orderid` varchar(50) DEFAULT NULL COMMENT '订单id',
  `tyjetton` int(11) DEFAULT '0' COMMENT '体验金钱包',
  `gamejetton` int(11) DEFAULT '0' COMMENT '游戏钱包',
  `cpjetton` bigint(20) DEFAULT '0' COMMENT '彩票币',
  PRIMARY KEY (`id`),
  KEY `index2` (`userid`)
) ENGINE=InnoDB AUTO_INCREMENT=477966 DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for log_onlinedaily
-- ----------------------------
DROP TABLE IF EXISTS `log_onlinedaily`;
CREATE TABLE `log_onlinedaily` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `dateid` varchar(45) CHARACTER SET latin1 NOT NULL DEFAULT '',
  `channel` varchar(45) DEFAULT '',
  `alllist` varchar(45) CHARACTER SET latin1 DEFAULT '',
  `onlinelist` varchar(10000) CHARACTER SET latin1 DEFAULT '',
  `minulist` varchar(10000) CHARACTER SET latin1 DEFAULT '',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=361 DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for log_pay
-- ----------------------------
DROP TABLE IF EXISTS `log_pay`;
CREATE TABLE `log_pay` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `userid` int(11) DEFAULT NULL COMMENT '用户id',
  `realname` varchar(255) DEFAULT NULL COMMENT '真实姓名',
  `amount` bigint(20) DEFAULT '0' COMMENT '金额',
  `paytype` int(11) DEFAULT NULL COMMENT '交易类型，1=vip，2=在线充值，3=线下充值，4=系统优惠，5=异常资金',
  `payway` int(11) DEFAULT NULL COMMENT '支付渠道，1=支付宝，2=微信，3银行卡，4=人工充值',
  `status` int(11) DEFAULT '0' COMMENT '交易状态，0=处理中，1=成功，2=失败',
  `bankcard` varchar(30) DEFAULT '' COMMENT '银行卡号，当payway=3时使用',
  `bankman` varchar(30) DEFAULT '' COMMENT '持卡人，当payway=3时使用',
  `bankname` varchar(30) DEFAULT '' COMMENT '银行名称，当payway=3时使用',
  `remark` varchar(255) DEFAULT '' COMMENT '备注',
  `orderid` varchar(50) DEFAULT '' COMMENT '订单号',
  `created_at` datetime DEFAULT NULL COMMENT '创建时间',
  `updated_at` datetime DEFAULT NULL COMMENT '修改时间',
  `channel` varchar(50) DEFAULT '',
  `prechannel` varchar(50) DEFAULT '',
  `optid` int(11) DEFAULT '0' COMMENT '审核人id',
  `optname` varchar(50) DEFAULT '' COMMENT '审核人每次',
  `optremark` varchar(255) DEFAULT '' COMMENT '审核人备注',
  `opttime` datetime DEFAULT NULL COMMENT '审核时间',
  `optstatus` int(11) DEFAULT '1' COMMENT '审核状态，1=成功，0=失败',
  `pay_account` varchar(50) DEFAULT NULL COMMENT '支付账号，当payway=1或者2时使用',
  `pay_name` varchar(50) DEFAULT NULL COMMENT '支付名称，当payway=1或者2时使用',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=269 DEFAULT CHARSET=utf8;

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
  `payamount` int(11) DEFAULT '0' COMMENT '总充值',
  `onlinepayamonut` bigint(22) DEFAULT '0',
  `vippayamount` bigint(22) DEFAULT '0',
  `winamount` int(11) DEFAULT '0' COMMENT '赢钱数量',
  `commisionamount` int(11) DEFAULT '0' COMMENT '总收益',
  `feeamount` int(11) DEFAULT '0' COMMENT '个人总抽水',
  `achamount` bigint(22) DEFAULT '0' COMMENT '总投注',
  `teamusernum` int(11) DEFAULT '0' COMMENT '团队的人数',
  `teamuserdirnum` int(11) DEFAULT '0' COMMENT '直属玩家数',
  `newuserday` int(11) DEFAULT '0' COMMENT '在自己的推广团队中，表示每日新增',
  `activityday` int(11) DEFAULT '0' COMMENT '自己的团队中，表示每日活跃',
  `achteamamount` bigint(22) DEFAULT '0' COMMENT '个人团队全部总业绩',
  `drawlimit` int(11) DEFAULT '0',
  `takeamount` bigint(22) DEFAULT '0',
  `channel` varchar(32) DEFAULT '',
  `achteamdiramount` bigint(22) DEFAULT '0',
  `payteamamount` bigint(22) DEFAULT '0',
  `payteamdiramount` bigint(22) DEFAULT '0',
  `commisiondiramount` bigint(22) DEFAULT '0' COMMENT '直属佣金',
  `teampumpamount` float DEFAULT '0' COMMENT '玩家团队对上级的贡献（抽水模式代理中）',
  `dirpumpamount` float DEFAULT '0' COMMENT '玩家对上级的贡献（抽水模式代理中）',
  `zhanjiamount` bigint(22) DEFAULT '0' COMMENT '个人总战绩',
  `zhanjiteamamount` bigint(22) DEFAULT '0' COMMENT '个人团队全部总战绩',
  `zhanjiteamdiramount` bigint(22) DEFAULT '0' COMMENT '个人直属总战绩',
  `wucommisionamount` int(11) DEFAULT '0' COMMENT '个人收益',
  `userincome` int(11) DEFAULT '0' COMMENT '个人收益',
  `teamincome` int(11) DEFAULT '0' COMMENT '团队收益',
  `tywinamount` int(11) DEFAULT '0' COMMENT '体验金盈利',
  `maxpaynum` int(11) DEFAULT '0' COMMENT '玩家单笔充值中的最大金额',
  `playcount` int(11) DEFAULT '0' COMMENT '已玩场次',
  `prechannel` varchar(20) DEFAULT '' COMMENT '大渠道',
  `ordercount` bigint(20) DEFAULT '0' COMMENT '普通注单',
  `tyordercount` bigint(20) DEFAULT '0' COMMENT '体验金注单',
  `typourtotaljetton` bigint(20) DEFAULT '0' COMMENT '体验总投注额',
  `tywincount` bigint(20) DEFAULT '0' COMMENT '体验金总收益，只算赢得部分',
  `incomerate` float DEFAULT '0' COMMENT '个人收益率',
  `winscount` int(11) DEFAULT '0' COMMENT '连胜场次',
  `planrebateRatio` float DEFAULT '0' COMMENT '专家的派彩金额',
  `palnordercount` int(11) DEFAULT '0' COMMENT '专家方案订单总数',
  `transferpayamount` bigint(22) DEFAULT '0' COMMENT '汇款转账充值数量',
  `achgameamount` bigint(22) DEFAULT '0' COMMENT '游戏模块个人流水',
  `achgameteamamount` bigint(22) DEFAULT '0' COMMENT '游戏模块团总队流水',
  `achgameteamdiramount` bigint(22) DEFAULT '0' COMMENT '游戏模块直属流水',
  `sysgive` bigint(20) DEFAULT '0' COMMENT '赠金',
  `sysdiscountamount` bigint(20) DEFAULT '0' COMMENT '系统优惠交易',
  `sysdiffamount` bigint(20) DEFAULT '0' COMMENT '异常资金',
  `gamecount` bigint(20) DEFAULT '0' COMMENT '游戏总场次',
  `gamewin` bigint(20) DEFAULT '0' COMMENT '游戏总盈亏',
  `racebet` bigint(20) DEFAULT '0' COMMENT '赛事正常投注总额',
  `racetybet` bigint(20) DEFAULT '0' COMMENT '赛事体验金投注总额',
  `unsettlebet` bigint(20) DEFAULT '0' COMMENT '赛事未结算总额',
  `unsettletybet` bigint(20) DEFAULT '0' COMMENT '赛事体验金未结算总额',
  `gamefee` bigint(20) DEFAULT '0' COMMENT '游戏总抽水',
  `gamejettonin` bigint(20) DEFAULT '0' COMMENT '游戏钱包总转入',
  `gamejettonout` bigint(20) DEFAULT '0' COMMENT '游戏钱包总转出',
  `taketimes` bigint(20) DEFAULT '0' COMMENT '提现次数',
  `cpwinamount` bigint(20) DEFAULT '0' COMMENT '彩票收益（只算赢的部分）',
  `cpwin` bigint(20) DEFAULT '0' COMMENT '彩票总盈利 净胜值',
  PRIMARY KEY (`userid`),
  KEY `userid` (`userid`),
  KEY `inomech` (`channel`,`incomerate`),
  KEY `wincch` (`channel`,`winscount`),
  KEY `winch` (`zhanjiamount`,`channel`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for log_player_risk
-- ----------------------------
DROP TABLE IF EXISTS `log_player_risk`;
CREATE TABLE `log_player_risk` (
  `userid` int(11) NOT NULL,
  `change_phone` int(11) DEFAULT '0' COMMENT '1 修改手机号',
  `change_pass` int(11) DEFAULT '0' COMMENT ' 2：登录密码 ',
  `change_money_pass` int(11) DEFAULT '0' COMMENT '3：资金密码 ',
  `change_alipay` int(11) DEFAULT '0' COMMENT '4：收款方式:支付宝 ',
  `change_bank` int(11) DEFAULT '0' COMMENT '5：收款方式:银行卡',
  PRIMARY KEY (`userid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for log_playerdaily
-- ----------------------------
DROP TABLE IF EXISTS `log_playerdaily`;
CREATE TABLE `log_playerdaily` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `userid` int(11) NOT NULL DEFAULT '0',
  `dateid` varchar(45) DEFAULT '' COMMENT '时间id',
  `nickname` varchar(45) DEFAULT NULL,
  `channel` varchar(32) DEFAULT '',
  `regdate` timestamp NULL DEFAULT CURRENT_TIMESTAMP,
  `logincount` int(11) DEFAULT '1' COMMENT '登录次数',
  `onlinetime` int(11) DEFAULT '0' COMMENT '在线时长',
  `callpaycount` int(11) DEFAULT '0' COMMENT '调起付费总额',
  `callpaytimes` int(11) DEFAULT '0' COMMENT '调起付费次数',
  `paytimes` int(11) DEFAULT '0' COMMENT '付费次数',
  `payamount` int(11) DEFAULT '0',
  `onlinepaytimes` int(11) DEFAULT '0',
  `onlinepayamount` int(11) DEFAULT '0',
  `vippaytimes` int(11) DEFAULT '0' COMMENT 'vip充值次数',
  `vippayamount` int(11) DEFAULT '0',
  `winamount` int(11) DEFAULT '0',
  `commisionamount` int(11) DEFAULT '0',
  `feeamount` int(11) DEFAULT '0' COMMENT '个人总抽水',
  `achamount` bigint(22) DEFAULT '0' COMMENT '自己玩的流水',
  `teamusernum` int(11) DEFAULT '0' COMMENT '团队的人数',
  `newuserday` int(11) DEFAULT '0' COMMENT '在自己的推广团队中，表示每日新增',
  `activityday` int(11) DEFAULT '0' COMMENT '自己的团队中，表示每日活跃',
  `achteamamount` bigint(22) DEFAULT '0' COMMENT '个人团队全部总业绩',
  `takeamount` bigint(22) DEFAULT '0',
  `achteamdiramount` bigint(22) DEFAULT '0' COMMENT '每日团队直属流水',
  `payteamamount` int(11) DEFAULT '0' COMMENT '团队付费',
  `payteamdiramount` int(11) DEFAULT '0' COMMENT '直属团队付费',
  `teampumpamount` float DEFAULT '0' COMMENT '玩家团队对上级的贡献（抽水模式代理中）',
  `dirpumpamount` float DEFAULT '0' COMMENT '玩家对上级的贡献（抽水模式代理中）',
  `dirusernum` int(11) DEFAULT '0' COMMENT '直属人数',
  `teampayusernum` int(11) DEFAULT '0' COMMENT '团队充值人数',
  `dirpayusernum` int(11) DEFAULT '0' COMMENT '直属充值人数',
  `teamincome` bigint(20) DEFAULT '0' COMMENT '总收益',
  `dirincome` bigint(20) DEFAULT '0' COMMENT '直属收益',
  `kucun` bigint(20) DEFAULT '0' COMMENT '今日库存',
  `zhanjiamount` bigint(22) DEFAULT '0' COMMENT '个人总战绩',
  `zhanjiteamamount` bigint(22) DEFAULT '0' COMMENT '个人团队全部总战绩',
  `zhanjiteamdiramount` bigint(22) DEFAULT '0' COMMENT '个人直属总战绩',
  `wucommisionamount` int(11) DEFAULT '0' COMMENT '个人收益',
  `tywinamount` int(11) DEFAULT '0' COMMENT '体验金盈利',
  `playcount` int(11) DEFAULT '0' COMMENT '已玩场次',
  `ordercount` bigint(20) DEFAULT '0' COMMENT '普通注单',
  `prechannel` varchar(50) DEFAULT NULL,
  `tyordercount` bigint(20) DEFAULT '0' COMMENT '体验金注单',
  `typourtotaljetton` bigint(20) DEFAULT '0' COMMENT '体验总投注额',
  `tywincount` bigint(20) DEFAULT '0' COMMENT '体验金总收益，只算赢得部分',
  `counttime` bigint(20) DEFAULT '0' COMMENT '周结算佣金发放时间',
  `activeusercount` int(11) DEFAULT '0' COMMENT '活跃用户数量',
  `incomerate` int(11) DEFAULT '0' COMMENT '收益率 10就是10%',
  `userincome` int(11) DEFAULT '0' COMMENT '个人收益',
  `planrebateRatio` float DEFAULT '0' COMMENT '专家的派彩金额',
  `palnordercount` int(11) DEFAULT '0' COMMENT '专家方案订单总数',
  `transferpayamount` bigint(22) DEFAULT '0' COMMENT '汇款转账充值数量',
  `commisiondiramount` bigint(22) DEFAULT '0' COMMENT '直属佣金',
  `achgameamount` bigint(22) DEFAULT '0' COMMENT '游戏模块个人流水',
  `achgameteamamount` bigint(22) DEFAULT '0' COMMENT '游戏模块团总队流水',
  `achgameteamdiramount` bigint(22) DEFAULT '0' COMMENT '游戏模块直属流水',
  `sysgive` bigint(20) DEFAULT '0' COMMENT '赠金',
  `sysdiscountamount` bigint(20) DEFAULT '0' COMMENT '系统优惠交易',
  `sysdiffamount` bigint(20) DEFAULT '0' COMMENT '异常资金',
  `gamecount` bigint(20) DEFAULT '0' COMMENT '游戏总场次',
  `gamewin` bigint(20) DEFAULT '0' COMMENT '游戏总盈亏',
  `racebet` bigint(20) DEFAULT '0' COMMENT '赛事正常投注总额',
  `racetybet` bigint(20) DEFAULT '0' COMMENT '赛事体验金投注总额',
  `unsettlebet` bigint(20) DEFAULT '0' COMMENT '赛事未结算总额',
  `unsettletybet` bigint(20) DEFAULT '0' COMMENT '赛事体验金未结算总额',
  `gamefee` bigint(20) DEFAULT '0' COMMENT '游戏总抽水',
  `gamejettonin` bigint(20) DEFAULT '0' COMMENT '游戏钱包总转入',
  `gamejettonout` bigint(20) DEFAULT '0' COMMENT '游戏钱包总转出',
  `taketimes` bigint(20) DEFAULT '0' COMMENT '提现次数',
  `cpwinamount` bigint(20) DEFAULT '0' COMMENT '彩票收益（只算赢的部分）',
  `cpwin` bigint(20) DEFAULT '0' COMMENT '彩票总盈利 净胜值',
  PRIMARY KEY (`id`),
  KEY `dateid` (`dateid`) USING BTREE,
  KEY `dateuserid` (`userid`,`dateid`) USING BTREE,
  KEY `userid` (`userid`) USING BTREE
) ENGINE=InnoDB AUTO_INCREMENT=11378 DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for log_playermonth
-- ----------------------------
DROP TABLE IF EXISTS `log_playermonth`;
CREATE TABLE `log_playermonth` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `userid` int(11) NOT NULL DEFAULT '0',
  `dateid` varchar(45) DEFAULT '' COMMENT '时间id',
  `nickname` varchar(45) DEFAULT '',
  `channel` varchar(32) DEFAULT '',
  `regdate` timestamp NULL DEFAULT CURRENT_TIMESTAMP,
  `logincount` int(11) DEFAULT '1' COMMENT '登录次数',
  `onlinetime` int(11) unsigned DEFAULT '0' COMMENT '在线时长',
  `callpaycount` int(11) DEFAULT '0' COMMENT '调起付费总额',
  `callpaytimes` int(11) DEFAULT '0' COMMENT '调起付费次数',
  `paytimes` int(11) DEFAULT '0' COMMENT '付费次数',
  `payamount` int(11) DEFAULT '0',
  `onlinepaytimes` int(11) DEFAULT '0',
  `onlinepayamount` int(11) DEFAULT '0',
  `vippayamount` int(11) DEFAULT '0',
  `winamount` int(11) DEFAULT '0',
  `commisionamount` int(11) DEFAULT '0',
  `feeamount` int(11) DEFAULT '0' COMMENT '个人总抽水',
  `achamount` bigint(22) DEFAULT '0' COMMENT '自己玩的流水',
  `teamusernum` int(11) DEFAULT '0' COMMENT '团队的人数',
  `newuserday` int(11) DEFAULT '0' COMMENT '在自己的推广团队中，表示每日新增',
  `activityday` int(11) DEFAULT '0' COMMENT '自己的团队中，表示每日活跃',
  `achteamamount` bigint(22) DEFAULT '0' COMMENT '个人团队全部总业绩',
  `takeamount` bigint(22) DEFAULT '0',
  `achteamdiramount` bigint(22) DEFAULT '0' COMMENT '每日团队直属流水',
  `payteamamount` int(11) DEFAULT '0' COMMENT '团队付费',
  `payteamdiramount` int(11) DEFAULT '0' COMMENT '直属团队付费',
  `teampumpamount` float DEFAULT '0' COMMENT '玩家团队对上级的贡献（抽水模式代理中）',
  `dirpumpamount` float DEFAULT '0' COMMENT '玩家对上级的贡献（抽水模式代理中）',
  `zhanjiamount` bigint(22) DEFAULT '0' COMMENT '个人总战绩',
  `zhanjiteamamount` bigint(22) DEFAULT '0' COMMENT '个人团队全部总战绩',
  `zhanjiteamdiramount` bigint(22) DEFAULT '0' COMMENT '个人直属总战绩',
  `wucommisionamount` int(11) DEFAULT '0' COMMENT '个人收益',
  `tywinamount` int(11) DEFAULT '0' COMMENT '体验金盈利',
  `playcount` int(11) DEFAULT '0' COMMENT '已玩场次',
  `ordercount` bigint(20) DEFAULT '0' COMMENT '普通注单',
  `prechannel` varchar(50) DEFAULT NULL,
  `tyordercount` bigint(20) DEFAULT '0' COMMENT '体验金注单',
  `typourtotaljetton` bigint(20) DEFAULT '0' COMMENT '体验总投注额',
  `tywincount` bigint(20) DEFAULT '0' COMMENT '体验金总收益，只算赢得部分',
  `planrebateRatio` float DEFAULT '0' COMMENT '专家的派彩金额',
  `palnordercount` int(11) DEFAULT '0' COMMENT '专家方案订单总数',
  `transferpayamount` bigint(22) DEFAULT '0' COMMENT '汇款转账充值数量',
  `commisiondiramount` bigint(22) DEFAULT '0' COMMENT '直属佣金',
  `achgameamount` bigint(22) DEFAULT '0' COMMENT '游戏模块个人流水',
  `achgameteamamount` bigint(22) DEFAULT '0' COMMENT '游戏模块团总队流水',
  `achgameteamdiramount` bigint(22) DEFAULT '0' COMMENT '游戏模块直属流水',
  `sysgive` bigint(20) DEFAULT '0' COMMENT '赠金',
  `sysdiscountamount` bigint(20) DEFAULT '0' COMMENT '系统优惠交易',
  `sysdiffamount` bigint(20) DEFAULT '0' COMMENT '异常资金',
  `gamecount` bigint(20) DEFAULT '0' COMMENT '游戏总场次',
  `gamewin` bigint(20) DEFAULT '0' COMMENT '游戏总盈亏',
  `racebet` bigint(20) DEFAULT '0' COMMENT '赛事正常投注总额',
  `racetybet` bigint(20) DEFAULT '0' COMMENT '赛事体验金投注总额',
  `unsettlebet` bigint(20) DEFAULT '0' COMMENT '赛事未结算总额',
  `unsettletybet` bigint(20) DEFAULT '0' COMMENT '赛事体验金未结算总额',
  `gamefee` bigint(20) DEFAULT '0' COMMENT '游戏总抽水',
  `gamejettonin` bigint(20) DEFAULT '0' COMMENT '游戏钱包总转入',
  `gamejettonout` bigint(20) DEFAULT '0' COMMENT '游戏钱包总转出',
  `taketimes` bigint(20) DEFAULT '0' COMMENT '提现次数',
  `cpwinamount` bigint(20) DEFAULT '0' COMMENT '彩票收益（只算赢的部分）',
  `cpwin` bigint(20) DEFAULT '0' COMMENT '彩票总盈利 净胜值',
  PRIMARY KEY (`id`),
  KEY `dateid` (`dateid`) USING BTREE,
  KEY `dateuserid` (`userid`,`dateid`) USING BTREE,
  KEY `userid` (`userid`) USING BTREE
) ENGINE=InnoDB AUTO_INCREMENT=6511 DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for log_playerweek
-- ----------------------------
DROP TABLE IF EXISTS `log_playerweek`;
CREATE TABLE `log_playerweek` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `userid` int(11) NOT NULL DEFAULT '0',
  `dateid` varchar(45) DEFAULT '' COMMENT '时间id',
  `nickname` varchar(45) DEFAULT NULL,
  `channel` varchar(32) DEFAULT '',
  `regdate` timestamp NULL DEFAULT CURRENT_TIMESTAMP,
  `logincount` int(11) DEFAULT '1' COMMENT '登录次数',
  `onlinetime` int(11) DEFAULT '0' COMMENT '在线时长',
  `callpaycount` int(11) DEFAULT '0' COMMENT '调起付费总额',
  `callpaytimes` int(11) DEFAULT '0' COMMENT '调起付费次数',
  `paytimes` int(11) DEFAULT '0' COMMENT '付费次数',
  `payamount` int(11) DEFAULT '0',
  `onlinepaytimes` int(11) DEFAULT '0',
  `onlinepayamount` int(11) DEFAULT '0',
  `vippaytimes` int(11) DEFAULT '0',
  `vippayamount` int(11) DEFAULT '0',
  `winamount` int(11) DEFAULT '0',
  `commisionamount` int(11) DEFAULT '0',
  `feeamount` int(11) DEFAULT '0' COMMENT '个人总抽水',
  `achamount` bigint(22) DEFAULT '0' COMMENT '自己玩的流水',
  `teamusernum` int(11) DEFAULT '0' COMMENT '团队的人数',
  `newuserday` int(11) DEFAULT '0' COMMENT '在自己的推广团队中，表示每日新增',
  `activityday` int(11) DEFAULT '0' COMMENT '自己的团队中，表示每日活跃',
  `achteamamount` bigint(22) DEFAULT '0' COMMENT '个人团队全部总业绩',
  `takeamount` bigint(22) DEFAULT '0',
  `achteamdiramount` bigint(22) DEFAULT '0' COMMENT '每日团队直属流水',
  `payteamamount` int(11) DEFAULT '0' COMMENT '团队付费',
  `payteamdiramount` int(11) DEFAULT '0' COMMENT '直属团队付费',
  `teampumpamount` float DEFAULT '0' COMMENT '玩家团队对上级的贡献（抽水模式代理中）',
  `dirpumpamount` float DEFAULT '0' COMMENT '玩家对上级的贡献（抽水模式代理中）',
  `zhanjiamount` bigint(22) DEFAULT '0' COMMENT '个人总战绩',
  `zhanjiteamamount` bigint(22) DEFAULT '0' COMMENT '个人团队全部总战绩',
  `zhanjiteamdiramount` bigint(22) DEFAULT '0' COMMENT '个人直属总战绩',
  `wucommisionamount` int(11) DEFAULT '0' COMMENT '个人收益',
  `counttime` bigint(20) DEFAULT '0' COMMENT '周结算佣金发放时间',
  `activeusercount` int(11) DEFAULT '0' COMMENT '活跃用户数量',
  `incomerate` int(11) DEFAULT '0' COMMENT '收益率 10就是10%',
  `userincome` int(11) DEFAULT '0' COMMENT '个人收益',
  `teamincome` int(11) DEFAULT '0' COMMENT '团队收益',
  `tywinamount` int(11) DEFAULT '0' COMMENT '体验金盈利',
  `playcount` int(11) DEFAULT '0' COMMENT '已玩场次',
  `ordercount` bigint(20) DEFAULT '0' COMMENT '普通注单',
  `prechannel` varchar(50) DEFAULT NULL,
  `tyordercount` bigint(20) DEFAULT '0' COMMENT '体验金注单',
  `typourtotaljetton` bigint(20) DEFAULT '0' COMMENT '体验总投注额',
  `tywincount` bigint(20) DEFAULT '0' COMMENT '体验金总收益，只算赢得部分',
  `planrebateRatio` float DEFAULT '0' COMMENT '专家的派彩金额',
  `palnordercount` int(11) DEFAULT '0' COMMENT '专家方案订单总数',
  `transferpayamount` bigint(22) DEFAULT '0' COMMENT '汇款转账充值数量',
  `commisiondiramount` bigint(22) DEFAULT '0' COMMENT '直属佣金',
  `achgameamount` bigint(22) DEFAULT '0' COMMENT '游戏模块个人流水',
  `achgameteamamount` bigint(22) DEFAULT '0' COMMENT '游戏模块团总队流水',
  `achgameteamdiramount` bigint(22) DEFAULT '0' COMMENT '游戏模块直属流水',
  `sysgive` bigint(20) DEFAULT '0' COMMENT '赠金',
  `sysdiscountamount` bigint(20) DEFAULT '0' COMMENT '系统优惠交易',
  `sysdiffamount` bigint(20) DEFAULT '0' COMMENT '异常资金',
  `gamecount` bigint(20) DEFAULT '0' COMMENT '游戏总场次',
  `gamewin` bigint(20) DEFAULT '0' COMMENT '游戏总盈亏',
  `racebet` bigint(20) DEFAULT '0' COMMENT '赛事正常投注总额',
  `racetybet` bigint(20) DEFAULT '0' COMMENT '赛事体验金投注总额',
  `unsettlebet` bigint(20) DEFAULT '0' COMMENT '赛事未结算总额',
  `unsettletybet` bigint(20) DEFAULT '0' COMMENT '赛事体验金未结算总额',
  `gamefee` bigint(20) DEFAULT '0' COMMENT '游戏总抽水',
  `gamejettonin` bigint(20) DEFAULT '0' COMMENT '游戏钱包总转入',
  `gamejettonout` bigint(20) DEFAULT '0' COMMENT '游戏钱包总转出',
  `taketimes` bigint(20) DEFAULT '0' COMMENT '提现次数',
  `cpwinamount` bigint(20) DEFAULT '0' COMMENT '彩票收益（只算赢的部分）',
  `cpwin` bigint(20) DEFAULT '0' COMMENT '彩票总盈利 净胜值',
  PRIMARY KEY (`id`),
  KEY `dateid` (`dateid`) USING BTREE,
  KEY `dateuserid` (`userid`,`dateid`) USING BTREE,
  KEY `userid` (`userid`)
) ENGINE=InnoDB AUTO_INCREMENT=7564 DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for log_qpgamerecord
-- ----------------------------
DROP TABLE IF EXISTS `log_qpgamerecord`;
CREATE TABLE `log_qpgamerecord` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `gamekey` varchar(255) DEFAULT NULL COMMENT '彩种',
  `gametype` int(11) DEFAULT '0' COMMENT '游戏类型',
  `gid` varchar(255) DEFAULT NULL COMMENT '期号',
  `award` varchar(255) DEFAULT NULL COMMENT '开奖结果',
  `data` varchar(2048) DEFAULT '' COMMENT '牌局内容',
  `date` timestamp NULL DEFAULT CURRENT_TIMESTAMP COMMENT '开奖日期',
  `updatedate` timestamp NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  `gamenumber` varchar(255) DEFAULT '' COMMENT '牌局编号',
  `state` int(11) DEFAULT '0' COMMENT '0 未结算， 1 已结算',
  `tabletype` int(11) DEFAULT '0' COMMENT '牌桌类型',
  PRIMARY KEY (`id`),
  KEY `gamenumber` (`gamenumber`) USING BTREE
) ENGINE=MyISAM AUTO_INCREMENT=752789 DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for log_racedaily
-- ----------------------------
DROP TABLE IF EXISTS `log_racedaily`;
CREATE TABLE `log_racedaily` (
  `racename` varchar(50) NOT NULL DEFAULT '' COMMENT '联赛名称',
  `bet` bigint(20) DEFAULT '0' COMMENT '普通投注量',
  `tybet` bigint(20) DEFAULT '0' COMMENT '体验价投注量',
  `tybetcount` int(11) DEFAULT NULL COMMENT '体验投注数',
  `betcount` int(11) DEFAULT '0' COMMENT '普通注单数',
  `channel` varchar(50) DEFAULT '',
  `prechannel` varchar(50) NOT NULL DEFAULT '',
  `income` bigint(20) DEFAULT '0' COMMENT '盈亏总额',
  `dateid` date NOT NULL,
  PRIMARY KEY (`racename`,`prechannel`,`dateid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for log_racewin
-- ----------------------------
DROP TABLE IF EXISTS `log_racewin`;
CREATE TABLE `log_racewin` (
  `raceid` varchar(32) NOT NULL DEFAULT '' COMMENT '赛事id',
  `racename` varchar(128) DEFAULT '' COMMENT '赛事名字',
  `starttime` bigint(20) DEFAULT '0' COMMENT '赛事开始时间',
  `ordercount` int(11) DEFAULT '0' COMMENT '总订单数',
  `pourcount` bigint(20) DEFAULT '0' COMMENT '总投注额',
  `wincount` bigint(20) DEFAULT '0' COMMENT '盈利数量',
  `fee` int(11) DEFAULT '0' COMMENT '总手续费',
  PRIMARY KEY (`raceid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='赛事输赢，下单统计表';

-- ----------------------------
-- Table structure for log_recharge
-- ----------------------------
DROP TABLE IF EXISTS `log_recharge`;
CREATE TABLE `log_recharge` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `userid` int(11) NOT NULL DEFAULT '0' COMMENT '用户id(被充值者)',
  `amount` decimal(11,2) NOT NULL COMMENT '充值金额',
  `type` int(11) NOT NULL DEFAULT '0' COMMENT '类型【（0 用户充值  1 后台vip充值 2 赠送 3 提现扣款  4 普通扣款）被遗弃】5-系统赠送，6-优秀代理，7-充值补发，8-异常资金',
  `recharge_id` int(11) NOT NULL DEFAULT '0' COMMENT '充值者id',
  `remark` varchar(255) DEFAULT '' COMMENT '备注',
  `add_time` int(11) DEFAULT '0' COMMENT '添加时间',
  `status` int(3) NOT NULL DEFAULT '0' COMMENT '状态（0 待处理  1 成功 2 失败）',
  `orderid` varchar(40) DEFAULT '0' COMMENT '订单号',
  `channel` varchar(20) DEFAULT '' COMMENT '渠道',
  `prechannel` varchar(20) DEFAULT '',
  `optid` int(11) DEFAULT '0',
  `optname` varchar(50) DEFAULT '',
  `optremark` varchar(50) DEFAULT '',
  `opttime` datetime DEFAULT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=975 DEFAULT CHARSET=utf8 ROW_FORMAT=DYNAMIC COMMENT='VIP充值log';

-- ----------------------------
-- Table structure for log_records
-- ----------------------------
DROP TABLE IF EXISTS `log_records`;
CREATE TABLE `log_records` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `userid` int(11) DEFAULT NULL,
  `opttype` int(11) DEFAULT NULL COMMENT '1 收入 2 支出',
  `amount` bigint(20) DEFAULT '0' COMMENT '交易金额',
  `opttime` datetime DEFAULT NULL COMMENT '操作时间',
  `remark` varchar(100) DEFAULT NULL,
  `orderid` varchar(50) DEFAULT NULL,
  `msg` varchar(100) DEFAULT NULL,
  PRIMARY KEY (`id`),
  KEY `userid` (`userid`) USING BTREE
) ENGINE=InnoDB AUTO_INCREMENT=6644 DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for log_retain
-- ----------------------------
DROP TABLE IF EXISTS `log_retain`;
CREATE TABLE `log_retain` (
  `userid` int(11) NOT NULL,
  `regdate` timestamp NULL DEFAULT CURRENT_TIMESTAMP,
  `loginday` int(11) DEFAULT '0',
  `channel` varchar(11) DEFAULT '',
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
-- Table structure for log_rewarddate
-- ----------------------------
DROP TABLE IF EXISTS `log_rewarddate`;
CREATE TABLE `log_rewarddate` (
  `id` bigint(20) unsigned NOT NULL AUTO_INCREMENT,
  `time` bigint(20) DEFAULT '0' COMMENT '时间搓',
  `maxrate` decimal(10,2) DEFAULT '0.00' COMMENT '最高中奖比例，比如7表示 7%比例',
  `number` varchar(30) DEFAULT '0' COMMENT '期号',
  `allreward` bigint(20) DEFAULT '0' COMMENT '本期总共发放金额',
  `maxwinface` varchar(255) DEFAULT '' COMMENT '大赢家头像',
  `nickname` varchar(45) DEFAULT '' COMMENT '大赢家名字',
  `maxwin` int(11) DEFAULT '0' COMMENT '大赢家赢的钱',
  `rewardinfo` varchar(256) DEFAULT '' COMMENT '中奖额外信息，1,23,25,26,25,27,6,8|1,23,25,26,25,27,6,8',
  `gametype` int(11) DEFAULT '0' COMMENT '游戏类型，7700代表百人牛牛',
  `remainmoney` bigint(20) DEFAULT '0' COMMENT '奖池余额',
  PRIMARY KEY (`id`),
  KEY `gameandid` (`id`,`gametype`)
) ENGINE=InnoDB AUTO_INCREMENT=1825 DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for log_rewarduser
-- ----------------------------
DROP TABLE IF EXISTS `log_rewarduser`;
CREATE TABLE `log_rewarduser` (
  `id` bigint(20) unsigned NOT NULL AUTO_INCREMENT,
  `userid` int(11) DEFAULT '0' COMMENT '玩家id',
  `time` bigint(20) DEFAULT '0' COMMENT '时间搓',
  `number` varchar(30) DEFAULT '0' COMMENT '期号',
  `winmoney` bigint(20) DEFAULT '0' COMMENT '赢钱数量',
  `cardtype` varchar(64) DEFAULT '' COMMENT '中奖牌型，如果有多个，1,1,2 这样隔开 ',
  `gametype` int(11) DEFAULT '0' COMMENT '游戏类型，7700代表百人牛牛',
  `rewardinfo` varchar(256) DEFAULT '' COMMENT '中奖额外信息，1,23,25,26,25,27,6,8|1,23,25,26,25,27,6,8',
  PRIMARY KEY (`id`),
  KEY `useridandid` (`userid`,`id`,`gametype`) USING BTREE,
  KEY `useridandnum` (`userid`,`number`,`gametype`) USING BTREE
) ENGINE=InnoDB AUTO_INCREMENT=21312 DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for log_riskaction
-- ----------------------------
DROP TABLE IF EXISTS `log_riskaction`;
CREATE TABLE `log_riskaction` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `userid` int(11) DEFAULT NULL COMMENT '被操作人',
  `action` int(1) DEFAULT NULL COMMENT '1 修改手机号 2：登录密码 3：资金密码 4：收款方式:支付宝 5：收款方式:银行卡',
  `logintype` varchar(20) DEFAULT NULL COMMENT '登录方式',
  `actionip` varchar(20) DEFAULT NULL COMMENT '登录ip',
  `opttime` datetime DEFAULT NULL COMMENT '操作时间',
  `optname` varchar(255) DEFAULT NULL COMMENT '操作人名称',
  `optid` int(11) DEFAULT NULL COMMENT '操作人',
  `remark` varchar(255) DEFAULT NULL COMMENT '备注',
  `content` varchar(255) DEFAULT NULL COMMENT '操作内容描述',
  `channel` varchar(50) DEFAULT NULL,
  `prechannel` varchar(50) DEFAULT NULL,
  PRIMARY KEY (`id`),
  KEY `userid` (`userid`)
) ENGINE=InnoDB AUTO_INCREMENT=374 DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for log_serverinfo
-- ----------------------------
DROP TABLE IF EXISTS `log_serverinfo`;
CREATE TABLE `log_serverinfo` (
  `id` bigint(20) unsigned NOT NULL AUTO_INCREMENT,
  `time` varchar(24) DEFAULT '' COMMENT '上报的时间',
  `processName` varchar(64) DEFAULT '' COMMENT '进程名字',
  `TotalCPU` int(11) DEFAULT '0' COMMENT 'CPU总数',
  `RateCPU` int(11) DEFAULT '0' COMMENT 'CPU使用率',
  `TotalMem` int(11) DEFAULT '0' COMMENT '内存总量',
  `RemainMem` int(11) DEFAULT '0' COMMENT '可使用内存总量',
  `controllerMaxThread` int(11) DEFAULT '0' COMMENT 'controller最大线程数',
  `controllerFreeThread` int(11) DEFAULT '0' COMMENT 'controller空闲线程数',
  `controllerListNums` int(11) DEFAULT '0' COMMENT 'controller待处理事务总数',
  `workerMaxThread` int(11) DEFAULT '0' COMMENT 'worker最大线程数',
  `workerFreeThread` int(11) DEFAULT '0' COMMENT 'worker空闲线程数',
  `workerListNums` int(11) DEFAULT '0' COMMENT 'worker待处理事务总数',
  `timestamp` bigint(20) DEFAULT '0' COMMENT '时间戳',
  PRIMARY KEY (`id`),
  KEY `processName` (`processName`) USING BTREE,
  KEY `timeandname` (`timestamp`,`processName`) USING BTREE
) ENGINE=InnoDB AUTO_INCREMENT=2154497 DEFAULT CHARSET=utf8 ROW_FORMAT=DYNAMIC;

-- ----------------------------
-- Table structure for log_specialmoney
-- ----------------------------
DROP TABLE IF EXISTS `log_specialmoney`;
CREATE TABLE `log_specialmoney` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `createdate` varchar(20) DEFAULT '' COMMENT '日期',
  `jetton` int(20) DEFAULT '0' COMMENT '剩余库存',
  `payamount` int(20) DEFAULT '0' COMMENT '总充值',
  `takeamount` int(20) DEFAULT '0' COMMENT '总提现',
  `settlementamount` int(20) DEFAULT '0' COMMENT '结算金额',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for log_specialplayer
-- ----------------------------
DROP TABLE IF EXISTS `log_specialplayer`;
CREATE TABLE `log_specialplayer` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `userid` int(11) DEFAULT NULL COMMENT '用户id',
  `nickname` varchar(100) DEFAULT '' COMMENT '昵称',
  `agent1` int(20) DEFAULT NULL COMMENT '上级id',
  `createdate` timestamp NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP COMMENT '加入时间',
  `state` int(11) DEFAULT '1' COMMENT '状态 1、开启  0、关闭',
  `parent_id` int(11) DEFAULT '0' COMMENT '父级id',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=DYNAMIC;

-- ----------------------------
-- Table structure for log_specialwin
-- ----------------------------
DROP TABLE IF EXISTS `log_specialwin`;
CREATE TABLE `log_specialwin` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `userid` int(11) DEFAULT '0' COMMENT '时间',
  `createdate` varchar(100) DEFAULT '' COMMENT '时间',
  `winall` varchar(20) DEFAULT '' COMMENT '盈利',
  `jetton` int(15) DEFAULT '0' COMMENT '当日库存',
  `payamount` int(20) DEFAULT '0' COMMENT '当日总充值',
  `drawamount` int(20) DEFAULT '0' COMMENT '当日总提现',
  `takeamount` int(20) DEFAULT '0' COMMENT '当日应返金额',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for log_syschannel
-- ----------------------------
DROP TABLE IF EXISTS `log_syschannel`;
CREATE TABLE `log_syschannel` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `dateid` varchar(45) DEFAULT '',
  `channel` varchar(45) DEFAULT '',
  `playernum` int(11) DEFAULT '0' COMMENT '玩家数量',
  `newplayernum` int(11) DEFAULT '0' COMMENT '新增用户数',
  `activeplayernum` int(11) DEFAULT '0' COMMENT '活跃用户数',
  `playerpaynum` int(11) DEFAULT '0' COMMENT '付费人数',
  `moneyin` bigint(22) DEFAULT '0' COMMENT '系统进项',
  `moneyout` bigint(22) DEFAULT '0' COMMENT '系统出款总额',
  `playerjetton` bigint(22) DEFAULT '0' COMMENT '玩家金币库存',
  `paytimes` int(11) DEFAULT '0' COMMENT '付费次数',
  `paycount` bigint(22) DEFAULT '0' COMMENT '付费总额',
  `onlinepaytimes` int(11) DEFAULT '0',
  `onlinepaycount` bigint(22) DEFAULT '0' COMMENT '在线支付总额',
  `vippaytimes` int(11) DEFAULT '0' COMMENT '流水总和',
  `vippaycount` bigint(22) DEFAULT '0' COMMENT '系统盈利总和',
  `registeraward` bigint(22) DEFAULT '0' COMMENT '注册放奖励总额',
  `dailyaward` bigint(22) DEFAULT '0' COMMENT '每天登入的救济金',
  `activityaward` bigint(22) DEFAULT '0' COMMENT '活动奖励',
  `sysgive` bigint(22) DEFAULT '0' COMMENT '系统赠送总额',
  `achamount` bigint(22) DEFAULT '0' COMMENT '流水总额',
  `feeamount` bigint(22) DEFAULT '0' COMMENT '税收总额',
  `commisionamount` bigint(22) DEFAULT '0' COMMENT '佣金总额',
  `commisiontake` bigint(22) DEFAULT '0' COMMENT '佣金提现',
  `commisionchange` bigint(22) DEFAULT '0' COMMENT '佣金转入账户',
  `syswinamount` bigint(22) DEFAULT '0' COMMENT '系统盈利总额',
  `takeamount` bigint(22) DEFAULT '0' COMMENT '提现总额',
  `betakeamount` bigint(22) DEFAULT '0' COMMENT '待提现总额',
  `robotjetton` bigint(22) DEFAULT '0' COMMENT '机器人库存',
  `freezejetton` bigint(22) DEFAULT '0' COMMENT '被冻结库存',
  `moneyincompare` bigint(22) DEFAULT '0' COMMENT '入金环比',
  `moneyoutcompare` bigint(22) DEFAULT '0' COMMENT '出金环比',
  `jettoncompare` bigint(22) DEFAULT '0' COMMENT '库存环比',
  `deviatevalue` bigint(22) DEFAULT '0',
  `backstagegive` bigint(22) DEFAULT '0' COMMENT '后台赠送',
  `backstagededuct` bigint(22) DEFAULT '0' COMMENT '后台扣款',
  `prechannel` varchar(50) DEFAULT NULL,
  `tywinamount` bigint(20) DEFAULT '0' COMMENT '体验金盈利',
  `wincount` bigint(20) DEFAULT '0' COMMENT '普通总收益 只算赢的部分',
  `tywincount` bigint(20) DEFAULT '0' COMMENT '体验金盈利  只算赢的部分',
  `transferpaytimes` int(11) DEFAULT '0' COMMENT '汇款转账次数',
  `transferpaycount` bigint(20) DEFAULT '0' COMMENT '汇款转账充值总额',
  `ratenum` bigint(20) DEFAULT '0' COMMENT '提现手续费',
  `racebet` bigint(20) DEFAULT '0' COMMENT '赛事正常投注总额',
  `racetybet` bigint(20) DEFAULT '0' COMMENT '赛事体验金投注总额',
  `unsettlebet` bigint(20) DEFAULT '0' COMMENT '赛事未结算总额',
  `unsettletybet` bigint(20) DEFAULT '0' COMMENT '赛事体验金未结算总额',
  `gamebet` bigint(20) DEFAULT '0' COMMENT '游戏投注总额',
  `gametype` int(11) DEFAULT '0' COMMENT '游戏类型（游戏使用）',
  `sysdiscountamount` bigint(20) DEFAULT '0' COMMENT '系统优惠交易',
  `sysdiffamount` bigint(20) DEFAULT '0' COMMENT '异常资金',
  `lossjetton` double(20,4) DEFAULT '0.0000' COMMENT '系统损耗',
  `gamewin` bigint(20) DEFAULT '0' COMMENT '游戏总盈亏',
  `gameamount` bigint(20) DEFAULT '0' COMMENT '游戏总流水',
  `taketimes` bigint(20) DEFAULT '0' COMMENT '提现次数',
  `newplaynum` bigint(20) DEFAULT '0' COMMENT '有玩游戏和下注足球的玩家数量',
  `newbindphonenum` bigint(20) DEFAULT '0' COMMENT '绑定手机的数量',
  `newrealnamenum` bigint(20) DEFAULT '0' COMMENT '这天注册的玩家，实名的数量',
  `newplayermatchnum` bigint(20) DEFAULT '0' COMMENT '有下注足球的玩家数量',
  `newplayergamenum` bigint(20) DEFAULT '0' COMMENT '有玩游戏的玩家数量',
  `newexpertnum` bigint(20) DEFAULT '0' COMMENT '成为专家的玩家数量',
  `gamefeeamount` bigint(20) DEFAULT '0' COMMENT '游戏的税收',
  `commisionplayernum` bigint(20) DEFAULT '0' COMMENT '分佣的人数',
  `cpwinamount` bigint(20) DEFAULT '0' COMMENT '彩票收益（只算赢的部分）',
  `cpwin` bigint(20) DEFAULT '0' COMMENT '彩票总盈利 净胜值',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=18 DEFAULT CHARSET=utf8 COMMENT='统计渠道的总表';

-- ----------------------------
-- Table structure for log_sysdaily
-- ----------------------------
DROP TABLE IF EXISTS `log_sysdaily`;
CREATE TABLE `log_sysdaily` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `dateid` varchar(45) DEFAULT '',
  `channel` varchar(45) DEFAULT '',
  `playernum` int(11) DEFAULT '0' COMMENT '玩家数量',
  `newplayernum` int(11) DEFAULT '0' COMMENT '新增用户数',
  `activeplayernum` int(11) DEFAULT '0' COMMENT '活跃用户数',
  `playerpaynum` int(11) DEFAULT '0' COMMENT '付费人数',
  `moneyin` bigint(22) DEFAULT '0' COMMENT '系统进项',
  `moneyout` bigint(22) DEFAULT '0' COMMENT '系统出款总额',
  `playerjetton` bigint(22) DEFAULT '0' COMMENT '玩家金币库存',
  `paytimes` int(11) DEFAULT '0' COMMENT '付费次数',
  `paycount` bigint(22) DEFAULT '0' COMMENT '付费总额',
  `onlinepaytimes` int(11) DEFAULT '0',
  `onlinepaycount` bigint(22) DEFAULT '0' COMMENT '在线支付总额',
  `vippaytimes` int(11) DEFAULT '0' COMMENT 'vip充值次数',
  `vippaycount` bigint(22) DEFAULT '0' COMMENT 'VIP充值金额',
  `registeraward` bigint(22) DEFAULT '0' COMMENT '注册放奖励总额',
  `dailyaward` bigint(22) DEFAULT '0' COMMENT '每天登入的救济金',
  `activityaward` bigint(22) DEFAULT '0' COMMENT '活动奖励',
  `sysgive` bigint(22) DEFAULT '0' COMMENT '系统赠送总额',
  `achamount` bigint(22) DEFAULT '0' COMMENT '流水总额',
  `feeamount` bigint(22) DEFAULT '0' COMMENT '税收总额',
  `commisionamount` bigint(22) DEFAULT '0' COMMENT '佣金总额',
  `commisiontake` bigint(22) DEFAULT '0' COMMENT '佣金提现',
  `commisionchange` bigint(22) DEFAULT '0' COMMENT '佣金转入账户',
  `syswinamount` bigint(22) DEFAULT '0' COMMENT '系统盈利总额',
  `takeamount` bigint(22) DEFAULT '0' COMMENT '提现总额',
  `betakeamount` bigint(22) DEFAULT '0' COMMENT '待提现总额',
  `robotjetton` bigint(22) DEFAULT '0' COMMENT '机器人库存',
  `freezejetton` bigint(22) DEFAULT '0' COMMENT '被冻结库存',
  `moneyincompare` bigint(22) DEFAULT '0' COMMENT '入金环比',
  `moneyoutcompare` bigint(22) DEFAULT '0' COMMENT '出金环比',
  `jettoncompare` bigint(22) DEFAULT '0' COMMENT '库存环比',
  `deviatevalue` bigint(22) DEFAULT '0',
  `onlinetime` bigint(22) DEFAULT '0' COMMENT '全部人的在线时长',
  `maxonline` int(11) DEFAULT '0',
  `backstagegive` bigint(22) DEFAULT '0' COMMENT '后台赠送',
  `backstagededuct` bigint(22) DEFAULT '0' COMMENT '后台扣款',
  `prechannel` varchar(50) DEFAULT NULL,
  `tywinamount` bigint(20) DEFAULT '0' COMMENT '体验金盈利',
  `wincount` bigint(20) DEFAULT '0' COMMENT '普通总收益 只算赢的部分',
  `tywincount` bigint(20) DEFAULT '0' COMMENT '体验金盈利  只算赢的部分',
  `transferpaytimes` int(11) DEFAULT '0' COMMENT '汇款转账次数',
  `transferpaycount` bigint(20) DEFAULT '0' COMMENT '汇款转账充值总额',
  `ratenum` bigint(20) DEFAULT '0' COMMENT '提现手续费',
  `racebet` bigint(20) DEFAULT '0' COMMENT '赛事正常投注总额',
  `racetybet` bigint(20) DEFAULT '0' COMMENT '赛事体验金投注总额',
  `unsettlebet` bigint(20) DEFAULT '0' COMMENT '赛事未结算总额',
  `unsettletybet` bigint(20) DEFAULT '0' COMMENT '赛事体验金未结算总额',
  `gamebet` bigint(20) DEFAULT '0' COMMENT '游戏投注总额',
  `gametype` int(11) DEFAULT '0' COMMENT '游戏类型（游戏使用）',
  `sysdiscountamount` bigint(20) DEFAULT '0' COMMENT '系统优惠交易',
  `sysdiffamount` bigint(20) DEFAULT '0' COMMENT '异常资金',
  `gamewin` bigint(20) DEFAULT '0' COMMENT '游戏总盈亏',
  `gameamount` bigint(20) DEFAULT '0' COMMENT '游戏总流水',
  `taketimes` bigint(20) DEFAULT '0' COMMENT '提现次数',
  `newplaynum` bigint(20) DEFAULT '0' COMMENT '这天注册的玩家，有玩游戏和下注足球的玩家数量',
  `newbindphonenum` bigint(20) DEFAULT '0' COMMENT '这天注册的玩家，绑定手机的数量',
  `newrealnamenum` bigint(20) DEFAULT '0' COMMENT '这天注册的玩家，实名的数量',
  `newplayermatchnum` bigint(20) DEFAULT '0' COMMENT '这天注册的玩家，有下注足球的玩家数量',
  `newplayergamenum` bigint(20) DEFAULT '0' COMMENT '这天注册的玩家，有玩游戏的玩家数量',
  `newexpertnum` bigint(20) DEFAULT '0' COMMENT '这天注册的玩家，成为专家的玩家数量',
  `gamefeeamount` bigint(20) DEFAULT '0' COMMENT '游戏的税收',
  `commisionplayernum` bigint(20) DEFAULT '0' COMMENT '分佣的人数',
  `cpwinamount` bigint(20) DEFAULT '0' COMMENT '彩票收益（只算赢的部分）',
  `cpwin` bigint(20) DEFAULT '0' COMMENT '彩票总盈利 净胜值',
  PRIMARY KEY (`id`),
  KEY `index2` (`dateid`)
) ENGINE=InnoDB AUTO_INCREMENT=701 DEFAULT CHARSET=utf8 COMMENT='统计渠道的总表';

-- ----------------------------
-- Table structure for log_sysmonth
-- ----------------------------
DROP TABLE IF EXISTS `log_sysmonth`;
CREATE TABLE `log_sysmonth` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `dateid` varchar(45) DEFAULT '',
  `channel` varchar(45) DEFAULT '',
  `playernum` int(11) DEFAULT '0' COMMENT '玩家数量',
  `newplayernum` int(11) DEFAULT '0' COMMENT '新增用户数',
  `activeplayernum` int(11) DEFAULT '0' COMMENT '活跃用户数',
  `playerpaynum` int(11) DEFAULT '0' COMMENT '付费人数',
  `moneyin` bigint(22) DEFAULT '0' COMMENT '系统进项',
  `moneyout` bigint(22) DEFAULT '0' COMMENT '系统出款总额',
  `playerjetton` bigint(22) DEFAULT '0' COMMENT '玩家金币库存',
  `paytimes` int(11) DEFAULT '0' COMMENT '付费次数',
  `paycount` bigint(22) DEFAULT '0' COMMENT '付费总额',
  `onlinepaytimes` int(11) DEFAULT '0',
  `onlinepaycount` bigint(22) DEFAULT '0' COMMENT '在线支付总额',
  `vippaytimes` int(11) DEFAULT '0' COMMENT '流水总和',
  `vippaycount` bigint(22) DEFAULT '0' COMMENT '系统盈利总和',
  `registeraward` bigint(22) DEFAULT '0' COMMENT '注册放奖励总额',
  `dailyaward` bigint(22) DEFAULT '0' COMMENT '每天登入的救济金',
  `activityaward` bigint(22) DEFAULT '0' COMMENT '活动奖励',
  `sysgive` bigint(22) DEFAULT '0' COMMENT '系统赠送总额',
  `achamount` bigint(22) DEFAULT '0' COMMENT '流水总额',
  `feeamount` bigint(22) DEFAULT '0' COMMENT '税收总额',
  `commisionamount` bigint(22) DEFAULT '0' COMMENT '佣金总额',
  `commisiontake` bigint(22) DEFAULT '0' COMMENT '佣金提现',
  `commisionchange` bigint(22) DEFAULT '0' COMMENT '佣金转入账户',
  `syswinamount` bigint(22) DEFAULT '0' COMMENT '系统盈利总额',
  `takeamount` bigint(22) DEFAULT '0' COMMENT '提现总额',
  `betakeamount` bigint(22) DEFAULT '0' COMMENT '待提现总额',
  `robotjetton` bigint(22) DEFAULT '0' COMMENT '机器人库存',
  `freezejetton` bigint(22) DEFAULT '0' COMMENT '被冻结库存',
  `moneyincompare` bigint(22) DEFAULT '0' COMMENT '入金环比',
  `moneyoutcompare` bigint(22) DEFAULT '0' COMMENT '出金环比',
  `jettoncompare` bigint(22) DEFAULT '0' COMMENT '库存环比',
  `deviatevalue` bigint(22) DEFAULT '0',
  `onlinetime` bigint(22) DEFAULT '0' COMMENT '全部人的在线时长',
  `backstagegive` bigint(22) DEFAULT '0' COMMENT '后台赠送',
  `backstagededuct` bigint(22) DEFAULT '0' COMMENT '后台扣款',
  `prechannel` varchar(50) DEFAULT NULL,
  `tywinamount` bigint(20) DEFAULT '0' COMMENT '体验金盈利',
  `wincount` bigint(20) DEFAULT '0' COMMENT '普通总收益 只算赢的部分',
  `tywincount` bigint(20) DEFAULT '0' COMMENT '体验金盈利  只算赢的部分',
  `transferpaytimes` int(11) DEFAULT '0' COMMENT '汇款转账次数',
  `transferpaycount` bigint(20) DEFAULT '0' COMMENT '汇款转账充值总额',
  `ratenum` bigint(20) DEFAULT '0' COMMENT '提现手续费',
  `racebet` bigint(20) DEFAULT '0' COMMENT '赛事正常投注总额',
  `racetybet` bigint(20) DEFAULT '0' COMMENT '赛事体验金投注总额',
  `unsettlebet` bigint(20) DEFAULT '0' COMMENT '赛事未结算总额',
  `unsettletybet` bigint(20) DEFAULT '0' COMMENT '赛事体验金未结算总额',
  `gamebet` bigint(20) DEFAULT '0' COMMENT '游戏投注总额',
  `gametype` int(11) DEFAULT '0' COMMENT '游戏类型（游戏使用）',
  `sysdiscountamount` bigint(20) DEFAULT '0' COMMENT '系统优惠交易',
  `sysdiffamount` bigint(20) DEFAULT '0' COMMENT '异常资金',
  `gamewin` bigint(20) DEFAULT '0' COMMENT '游戏总盈亏',
  `gameamount` bigint(20) DEFAULT '0' COMMENT '游戏总流水',
  `taketimes` bigint(20) DEFAULT '0' COMMENT '提现次数',
  `newplaynum` bigint(20) DEFAULT '0' COMMENT '这月注册的玩家，有玩游戏和下注足球的玩家数量',
  `newbindphonenum` bigint(20) DEFAULT '0' COMMENT '这月注册的玩家，绑定手机的数量',
  `newrealnamenum` bigint(20) DEFAULT '0' COMMENT '这月注册的玩家，实名的数量',
  `newplayermatchnum` bigint(20) DEFAULT '0' COMMENT '这月注册的玩家，有下注足球的玩家数量',
  `newplayergamenum` bigint(20) DEFAULT '0' COMMENT '这月注册的玩家，有玩游戏的玩家数量',
  `newexpertnum` bigint(20) DEFAULT '0' COMMENT '这月注册的玩家，成为专家的玩家数量',
  `gamefeeamount` bigint(20) DEFAULT '0' COMMENT '游戏的税收',
  `commisionplayernum` bigint(20) DEFAULT '0' COMMENT '分佣的人数',
  `cpwinamount` bigint(20) DEFAULT '0' COMMENT '彩票收益（只算赢的部分）',
  `cpwin` bigint(20) DEFAULT '0' COMMENT '彩票总盈利 净胜值',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=45 DEFAULT CHARSET=utf8 COMMENT='统计渠道的总表';

-- ----------------------------
-- Table structure for log_sysweek
-- ----------------------------
DROP TABLE IF EXISTS `log_sysweek`;
CREATE TABLE `log_sysweek` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `dateid` varchar(45) DEFAULT '',
  `channel` varchar(45) DEFAULT '',
  `playernum` int(11) DEFAULT '0' COMMENT '玩家数量',
  `newplayernum` int(11) DEFAULT '0' COMMENT '新增用户数',
  `activeplayernum` int(11) DEFAULT '0' COMMENT '活跃用户数',
  `playerpaynum` int(11) DEFAULT '0' COMMENT '付费人数',
  `moneyin` bigint(22) DEFAULT '0' COMMENT '系统进项',
  `moneyout` bigint(22) DEFAULT '0' COMMENT '系统出款总额',
  `playerjetton` bigint(22) DEFAULT '0' COMMENT '玩家金币库存',
  `paytimes` int(11) DEFAULT '0' COMMENT '付费次数',
  `paycount` bigint(22) DEFAULT '0' COMMENT '付费总额',
  `onlinepaytimes` int(11) DEFAULT '0',
  `onlinepaycount` bigint(22) DEFAULT '0' COMMENT '在线支付总额',
  `vippaytimes` int(11) DEFAULT '0' COMMENT '流水总和',
  `vippaycount` bigint(22) DEFAULT '0' COMMENT '系统盈利总和',
  `registeraward` bigint(22) DEFAULT '0' COMMENT '注册放奖励总额',
  `dailyaward` bigint(22) DEFAULT '0' COMMENT '每天登入的救济金',
  `activityaward` bigint(22) DEFAULT '0' COMMENT '活动奖励',
  `sysgive` bigint(22) DEFAULT '0' COMMENT '系统赠送总额',
  `achamount` bigint(22) DEFAULT '0' COMMENT '流水总额',
  `feeamount` bigint(22) DEFAULT '0' COMMENT '税收总额',
  `commisionamount` bigint(22) DEFAULT '0' COMMENT '佣金总额',
  `commisiontake` bigint(22) DEFAULT '0' COMMENT '佣金提现',
  `commisionchange` bigint(22) DEFAULT '0' COMMENT '佣金转入账户',
  `syswinamount` bigint(22) DEFAULT '0' COMMENT '系统盈利总额',
  `takeamount` bigint(22) DEFAULT '0' COMMENT '提现总额',
  `betakeamount` bigint(22) DEFAULT '0' COMMENT '待提现总额',
  `robotjetton` bigint(22) DEFAULT '0' COMMENT '机器人库存',
  `freezejetton` bigint(22) DEFAULT '0' COMMENT '被冻结库存',
  `moneyincompare` bigint(22) DEFAULT '0' COMMENT '入金环比',
  `moneyoutcompare` bigint(22) DEFAULT '0' COMMENT '出金环比',
  `jettoncompare` bigint(22) DEFAULT '0' COMMENT '库存环比',
  `deviatevalue` bigint(22) DEFAULT '0',
  `onlinetime` bigint(22) DEFAULT '0' COMMENT '全部人的在线时长',
  `backstagegive` bigint(22) DEFAULT '0' COMMENT '后台赠送',
  `backstagededuct` bigint(22) DEFAULT '0' COMMENT '后台扣款',
  `prechannel` varchar(50) DEFAULT NULL,
  `tywinamount` bigint(20) DEFAULT '0' COMMENT '体验金盈利',
  `wincount` bigint(20) DEFAULT '0' COMMENT '普通总收益 只算赢的部分',
  `tywincount` bigint(20) DEFAULT '0' COMMENT '体验金盈利  只算赢的部分',
  `transferpaytimes` int(11) DEFAULT '0' COMMENT '汇款转账次数',
  `transferpaycount` bigint(20) DEFAULT '0' COMMENT '汇款转账充值总额',
  `ratenum` bigint(20) DEFAULT '0' COMMENT '提现手续费',
  `racebet` bigint(20) DEFAULT '0' COMMENT '赛事正常投注总额',
  `racetybet` bigint(20) DEFAULT '0' COMMENT '赛事体验金投注总额',
  `unsettlebet` bigint(20) DEFAULT '0' COMMENT '赛事未结算总额',
  `unsettletybet` bigint(20) DEFAULT '0' COMMENT '赛事体验金未结算总额',
  `gamebet` bigint(20) DEFAULT '0' COMMENT '游戏投注总额',
  `gametype` int(11) DEFAULT '0' COMMENT '游戏类型（游戏使用）',
  `sysdiscountamount` bigint(20) DEFAULT '0' COMMENT '系统优惠交易',
  `sysdiffamount` bigint(20) DEFAULT '0' COMMENT '异常资金',
  `gamewin` bigint(20) DEFAULT '0' COMMENT '游戏总盈亏',
  `gameamount` bigint(20) DEFAULT '0' COMMENT '游戏总流水',
  `taketimes` bigint(20) DEFAULT '0' COMMENT '提现次数',
  `newplaynum` bigint(20) DEFAULT '0' COMMENT '这周注册的玩家，有玩游戏和下注足球的玩家数量',
  `newbindphonenum` bigint(20) DEFAULT '0' COMMENT '这周注册的玩家，绑定手机的数量',
  `newrealnamenum` bigint(20) DEFAULT '0' COMMENT '这周注册的玩家，实名的数量',
  `newplayermatchnum` bigint(20) DEFAULT '0' COMMENT '这周注册的玩家，有下注足球的玩家数量',
  `newplayergamenum` bigint(20) DEFAULT '0' COMMENT '这周注册的玩家，有玩游戏的玩家数量',
  `newexpertnum` bigint(20) DEFAULT '0' COMMENT '这周注册的玩家，成为专家的玩家数量',
  `gamefeeamount` bigint(20) DEFAULT '0' COMMENT '游戏的税收',
  `commisionplayernum` bigint(20) DEFAULT '0' COMMENT '分佣的人数',
  `cpwinamount` bigint(20) DEFAULT '0' COMMENT '彩票收益（只算赢的部分）',
  `cpwin` bigint(20) DEFAULT '0' COMMENT '彩票总盈利 净胜值',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=122 DEFAULT CHARSET=utf8 COMMENT='统计渠道的总表';

-- ----------------------------
-- Table structure for log_turntable_record
-- ----------------------------
DROP TABLE IF EXISTS `log_turntable_record`;
CREATE TABLE `log_turntable_record` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `userid` int(11) DEFAULT '0',
  `nickname` varchar(128) DEFAULT '',
  `createdate` timestamp NULL DEFAULT CURRENT_TIMESTAMP,
  `usepropid` varchar(255) DEFAULT '0' COMMENT '使用的道具ID',
  `iswin` int(11) DEFAULT '0' COMMENT '是否中奖 1 中奖 0 没中奖',
  `prizepropid` varchar(255) DEFAULT '0' COMMENT '奖品道具ID',
  `proptype` int(11) DEFAULT '0' COMMENT '道具类型',
  `propname` varchar(255) DEFAULT '' COMMENT '道具名字',
  `propvalue` int(11) DEFAULT '0' COMMENT '道具面值',
  `proportion` int(11) DEFAULT '0' COMMENT '道具使用百分比',
  `effectivetime` int(11) DEFAULT '0' COMMENT '有效时间',
  `channel` varchar(20) DEFAULT '' COMMENT '渠道',
  `prechannel` varchar(255) DEFAULT '' COMMENT '大渠道',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=76 DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for log_tymgrrecord
-- ----------------------------
DROP TABLE IF EXISTS `log_tymgrrecord`;
CREATE TABLE `log_tymgrrecord` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `userid` int(11) DEFAULT NULL COMMENT '玩家id',
  `name` varchar(50) DEFAULT NULL COMMENT '玩家真实姓名',
  `nickname` varchar(100) DEFAULT NULL COMMENT '昵称',
  `phone` bigint(11) DEFAULT NULL COMMENT '电话',
  `registertime` datetime DEFAULT NULL COMMENT '注册时间',
  `promotiontime` datetime DEFAULT NULL COMMENT '转正时间',
  `wintyjetton` bigint(20) DEFAULT '0' COMMENT '体验金盈利金额',
  `tyjetton` bigint(20) DEFAULT '0' COMMENT '体验金额度',
  `jetton` bigint(20) DEFAULT '0' COMMENT '存款余额',
  `arch` bigint(20) DEFAULT '0' COMMENT '流水',
  `sendrewardstate` int(11) DEFAULT '0' COMMENT '奖励发放状态 0 未发放 1 已发放 2 已失效',
  `promotiontype` int(11) DEFAULT '0' COMMENT '转正类型 : 0 未转正  1 到期自动转正 2  手动转正',
  `logtype` int(11) DEFAULT '0' COMMENT '日志类型 0 转正类型 1 奖励发送',
  `channel` varchar(40) DEFAULT NULL COMMENT '渠道',
  `prechannel` varchar(40) DEFAULT NULL COMMENT '大渠道',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=26 DEFAULT CHARSET=utf8;

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
  `gotmoney` float DEFAULT '0',
  `realhair` float DEFAULT '0',
  `datestr` varchar(45) DEFAULT '',
  `payteam` int(11) DEFAULT '0' COMMENT '代理充值',
  `paydir` int(11) DEFAULT '0' COMMENT '直属充值',
  PRIMARY KEY (`id`),
  KEY `userid` (`userid`),
  KEY `datestr` (`datestr`)
) ENGINE=InnoDB AUTO_INCREMENT=3932 DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for log_useraction
-- ----------------------------
DROP TABLE IF EXISTS `log_useraction`;
CREATE TABLE `log_useraction` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `userid` int(11) DEFAULT NULL,
  `plat` varchar(20) DEFAULT NULL COMMENT '平台',
  `ip` varchar(255) DEFAULT NULL COMMENT 'ip',
  `lastaddr` varchar(50) DEFAULT NULL COMMENT '上次登录地点',
  `create_at` datetime DEFAULT NULL COMMENT '操作时间',
  `remark` varchar(255) DEFAULT NULL COMMENT '备注',
  `channel` varchar(50) DEFAULT NULL,
  `prechannel` varchar(50) DEFAULT NULL,
  `opttype` int(11) DEFAULT NULL COMMENT '操作类型，1=登录，2=登出，3=修改密码，4=信息修改',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=24585 DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for log_userannouncement
-- ----------------------------
DROP TABLE IF EXISTS `log_userannouncement`;
CREATE TABLE `log_userannouncement` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `userid` int(11) DEFAULT NULL,
  `newsid` int(11) DEFAULT NULL COMMENT '对应log_announcement中的id',
  `newsstate` int(11) DEFAULT '0' COMMENT '0: 未读，1:已读',
  `newstype` int(11) DEFAULT '1' COMMENT '1=通知，2=活动，3=公告，4=赛事',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=370977 DEFAULT CHARSET=utf8;

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
) ENGINE=InnoDB AUTO_INCREMENT=30390 DEFAULT CHARSET=utf8 COMMENT='玩家玩牌的记录';

-- ----------------------------
-- Table structure for log_vippointrecords
-- ----------------------------
DROP TABLE IF EXISTS `log_vippointrecords`;
CREATE TABLE `log_vippointrecords` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `userid` int(11) DEFAULT NULL COMMENT '玩家id',
  `points` float(11,2) DEFAULT '0.00' COMMENT '积分',
  `pointstype` int(11) DEFAULT '0' COMMENT '获取积分类型 0=充值  1=流水奖励（个人） 2=拉新奖励  3=团队充值奖励  4=团队流水  5= 降级扣除积分',
  `inviteuserid` int(11) DEFAULT '0' COMMENT '邀请的玩家的id',
  `createtime` timestamp NULL DEFAULT NULL,
  `channel` varchar(40) DEFAULT NULL,
  `prechannel` varchar(40) DEFAULT NULL,
  `datemonth` varchar(30) DEFAULT NULL COMMENT '年月',
  PRIMARY KEY (`id`),
  KEY `datemonth` (`datemonth`) USING BTREE,
  KEY `userid` (`userid`) USING BTREE
) ENGINE=InnoDB AUTO_INCREMENT=1441 DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for log_weekaward
-- ----------------------------
DROP TABLE IF EXISTS `log_weekaward`;
CREATE TABLE `log_weekaward` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `dateid` varchar(45) DEFAULT '' COMMENT '结算日期',
  `userid` int(11) DEFAULT '0',
  `nickname` varchar(129) DEFAULT '',
  `payamount` int(11) DEFAULT '0' COMMENT '付费总额',
  `jetton` bigint(22) DEFAULT '0' COMMENT '携带金币',
  `profitnum` bigint(22) DEFAULT '0' COMMENT '盈利总额',
  `state` int(11) DEFAULT '0' COMMENT '状态',
  `takeamount` int(11) DEFAULT '0' COMMENT '提现总额',
  `createdate` varchar(30) DEFAULT '' COMMENT '创建时间',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for log_wrjcuserrecord
-- ----------------------------
DROP TABLE IF EXISTS `log_wrjcuserrecord`;
CREATE TABLE `log_wrjcuserrecord` (
  `id` bigint(20) unsigned NOT NULL AUTO_INCREMENT,
  `userid` int(11) DEFAULT '0' COMMENT '玩家id',
  `number` varchar(255) DEFAULT '' COMMENT '期号',
  `time` bigint(20) DEFAULT '0' COMMENT '时间搓',
  `selfwin` bigint(20) DEFAULT '0' COMMENT '赢钱',
  `award` varchar(48) DEFAULT '' COMMENT '开奖结果',
  `myslf` varchar(255) DEFAULT '' COMMENT '自己的下注',
  `allarea` varchar(255) DEFAULT '' COMMENT '所有区域',
  `counttype` tinyint(4) DEFAULT '0' COMMENT '0 正常结算 1 取不到开奖结果',
  PRIMARY KEY (`id`),
  KEY `idanduserid` (`id`,`userid`),
  KEY `userid` (`userid`)
) ENGINE=InnoDB AUTO_INCREMENT=417 DEFAULT CHARSET=utf8;

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
