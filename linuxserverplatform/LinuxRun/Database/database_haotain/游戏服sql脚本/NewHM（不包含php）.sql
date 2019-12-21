/*
Navicat MySQL Data Transfer

Source Server         : 192.168.198.121
Source Server Version : 50645
Source Host           : 192.168.198.121:3306
Source Database       : 皓天

Target Server Type    : MYSQL
Target Server Version : 50645
File Encoding         : 65001

Date: 2019-12-21 11:14:14
*/

SET FOREIGN_KEY_CHECKS=0;

-- ----------------------------
-- Table structure for bagattribute
-- ----------------------------
DROP TABLE IF EXISTS `bagattribute`;
CREATE TABLE `bagattribute` (
  `id` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `desc` varchar(25) DEFAULT '',
  `name` varchar(25) DEFAULT NULL COMMENT '字段名',
  `type` tinyint(2) DEFAULT '0' COMMENT '道具所属类型',
  `status` tinyint(2) DEFAULT '0' COMMENT '状态信息（可售等）',
  `price` int(11) DEFAULT '0' COMMENT '售价',
  `minMoney` int(11) DEFAULT '0' COMMENT '可以开出最小金币',
  `maxMoney` int(11) DEFAULT '0' COMMENT '可以开出的最大金币',
  `instruction` varchar(255) DEFAULT '' COMMENT '说明',
  PRIMARY KEY (`id`)
) ENGINE=MyISAM AUTO_INCREMENT=12 DEFAULT CHARSET=utf8;

-- ----------------------------
-- Records of bagattribute
-- ----------------------------
INSERT INTO `bagattribute` VALUES ('1', '冰冻技能', 'skillFrozen', '1', '0', '0', '0', '0', '可获得途径在打boss中捕获鱼时有概率掉落冰冻');
INSERT INTO `bagattribute` VALUES ('2', '锁定技能', 'skillLocking', '1', '0', '0', '0', '0', '可获得途径在打boss中捕获鱼时有概率掉瞄准');
INSERT INTO `bagattribute` VALUES ('3', '红包', 'redBag', '2', '0', '0', '100', '1000', '可获得途径在打boss中捕获红包可获得，红包中随机获得一定金币');
INSERT INTO `bagattribute` VALUES ('4', '1元话费卡', 'phoneBillCard1', '3', '0', '0', '0', '0', '可获得途径在打boss中捕获1元充值卡可获得，可用于兑换话费');
INSERT INTO `bagattribute` VALUES ('5', '5元话费卡', 'phoneBillCard5', '3', '0', '0', '0', '0', '可获得途径在打boss中捕获1元充值卡可获得，可用于兑换话费');
INSERT INTO `bagattribute` VALUES ('6', '黄金战甲', 'goldenArmor', '4', '1', '0', '0', '0', '');
INSERT INTO `bagattribute` VALUES ('7', '机甲先锋', 'mechatroPioneer', '4', '1', '20', '0', '0', '');
INSERT INTO `bagattribute` VALUES ('8', '深海大炮', 'deepSeaArtillery', '4', '1', '30', '0', '0', '');
INSERT INTO `bagattribute` VALUES ('9', '章鱼大炮', 'octopusCannon', '4', '1', '120', '0', '0', '');
INSERT INTO `bagattribute` VALUES ('10', '黄金圣龙', 'goldenDragon', '4', '1', '300', '0', '0', '');
INSERT INTO `bagattribute` VALUES ('11', '熔岩战甲', 'lavaArmor', '4', '1', '200', '0', '0', '');

-- ----------------------------
-- Table structure for friendsgroupaccounts
-- ----------------------------
DROP TABLE IF EXISTS `friendsgroupaccounts`;
CREATE TABLE `friendsgroupaccounts` (
  `id` int(11) unsigned NOT NULL AUTO_INCREMENT COMMENT '主id',
  `friendsGroupID` int(11) DEFAULT '0' COMMENT '俱乐部id',
  `userID` int(11) DEFAULT '0' COMMENT '玩家id',
  `msgID` int(11) DEFAULT '0' COMMENT '消息id',
  `time` int(11) DEFAULT '0' COMMENT '生成时间',
  `roomID` int(11) DEFAULT '0' COMMENT '房间id',
  `realPlayCount` int(11) DEFAULT '0' COMMENT '实际游戏局数',
  `playCount` int(11) DEFAULT '0' COMMENT '最大游戏局数',
  `playMode` int(11) DEFAULT '0' COMMENT '游戏模式',
  `passwd` varchar(20) DEFAULT '' COMMENT '游戏房间号',
  `userInfoList` varchar(1024) DEFAULT '' COMMENT '战绩列表',
  `roomType` int(4) DEFAULT '0' COMMENT '房间类型',
  `srcType` int(4) DEFAULT '0' COMMENT '牌桌号码',
  `roomTipType` int(4) DEFAULT '0' COMMENT '抽水方式',
  `roomTipTypeNums` int(4) DEFAULT '0' COMMENT '抽水率',
  PRIMARY KEY (`id`),
  KEY `userID` (`userID`) USING BTREE,
  KEY `friendsGroupID` (`friendsGroupID`) USING BTREE,
  KEY `roomType` (`roomType`) USING BTREE,
  KEY `roomID` (`roomID`) USING BTREE,
  KEY `time` (`time`) USING BTREE
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- ----------------------------
-- Records of friendsgroupaccounts
-- ----------------------------

-- ----------------------------
-- Table structure for friendsgroupdesklistcost
-- ----------------------------
DROP TABLE IF EXISTS `friendsgroupdesklistcost`;
CREATE TABLE `friendsgroupdesklistcost` (
  `id` int(11) unsigned NOT NULL AUTO_INCREMENT COMMENT '主id',
  `friendsGroupID` int(11) DEFAULT '0' COMMENT '俱乐部id',
  `time` int(11) DEFAULT '0' COMMENT '生成时间',
  `userID` int(11) DEFAULT '0' COMMENT '玩家id',
  `costMoney` bigint(20) DEFAULT '0' COMMENT '消耗金币',
  `costJewels` int(11) DEFAULT '0' COMMENT '消耗钻石',
  `fireCoinRecharge` bigint(20) DEFAULT '0' COMMENT '火币充值',
  `fireCoinExchange` bigint(20) DEFAULT '0' COMMENT '火币兑换',
  `moneyPump` bigint(20) DEFAULT '0' COMMENT '金币抽水',
  `fireCoinPump` bigint(20) DEFAULT '0' COMMENT '火币抽水',
  `passwd` varchar(20) DEFAULT '' COMMENT '房间号码',
  `operationID` int(11) DEFAULT '0' COMMENT '操作人id',
  PRIMARY KEY (`id`),
  KEY `userID` (`userID`) USING BTREE,
  KEY `friendsGroupID` (`friendsGroupID`) USING BTREE,
  KEY `time` (`time`) USING BTREE
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- ----------------------------
-- Records of friendsgroupdesklistcost
-- ----------------------------

-- ----------------------------
-- Table structure for gamebaseinfo
-- ----------------------------
DROP TABLE IF EXISTS `gamebaseinfo`;
CREATE TABLE `gamebaseinfo` (
  `gameID` int(11) NOT NULL DEFAULT '0' COMMENT '游戏id',
  `name` varchar(64) DEFAULT '' COMMENT '游戏名字',
  `dllName` varchar(24) DEFAULT '' COMMENT '动态库名字',
  `deskPeople` int(11) DEFAULT '0' COMMENT '桌子人数',
  `watcherCount` int(11) DEFAULT '0' COMMENT '旁观者人数，canWatch=1的时候才有效',
  `canWatch` int(11) DEFAULT '0' COMMENT '这个游戏是否支持旁观',
  `canCombineDesk` tinyint(2) DEFAULT '0' COMMENT '0：匹配桌子模式，1：组桌模式',
  `multiPeopleGame` tinyint(2) DEFAULT '0' COMMENT '0：默认，1：标识2,3,4人是否公用同一个游戏id',
  PRIMARY KEY (`gameID`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- ----------------------------
-- Records of gamebaseinfo
-- ----------------------------
INSERT INTO `gamebaseinfo` VALUES ('10001000', '奔驰宝马', '10001000.DLL', '180', '180', '0', '0', '0');
INSERT INTO `gamebaseinfo` VALUES ('10000900', '豹子王', '10000900.DLL', '180', '180', '0', '0', '0');
INSERT INTO `gamebaseinfo` VALUES ('30000200', '百人牛牛', '30000200.DLL', '180', '180', '0', '0', '0');
INSERT INTO `gamebaseinfo` VALUES ('30000604', '二八杠', '30000604.DLL', '180', '180', '0', '0', '0');
INSERT INTO `gamebaseinfo` VALUES ('11000100', '飞禽走兽', '11000100.DLL', '180', '180', '0', '0', '0');
INSERT INTO `gamebaseinfo` VALUES ('11100200', '欢乐30秒', '11100200.DLL', '180', '180', '0', '0', '0');
INSERT INTO `gamebaseinfo` VALUES ('50000000', '李逵劈鱼', '50000000.DLL', '4', '4', '0', '0', '0');
INSERT INTO `gamebaseinfo` VALUES ('41100100', '百人牌九', '41100100.DLL', '180', '180', '0', '0', '0');
INSERT INTO `gamebaseinfo` VALUES ('40000100', '龙虎斗', '40000100.DLL', '180', '180', '0', '0', '0');
INSERT INTO `gamebaseinfo` VALUES ('40000000', '红包扫雷', '40000000.DLL', '180', '180', '0', '0', '0');
INSERT INTO `gamebaseinfo` VALUES ('50000001', 'BOSS来了', '50000001.DLL', '4', '4', '0', '0', '0');
INSERT INTO `gamebaseinfo` VALUES ('50000002', '深海捕鱼', '50000002.DLL', '4', '4', '0', '0', '0');
INSERT INTO `gamebaseinfo` VALUES ('10003000', '水浒传', '10003000.DLL', '1', '1', '0', '0', '0');
INSERT INTO `gamebaseinfo` VALUES ('10003001', '拉霸', '10003001.DLL', '1', '1', '0', '0', '0');
INSERT INTO `gamebaseinfo` VALUES ('10003002', '财神到', '10003002.DLL', '1', '1', '0', '0', '0');
INSERT INTO `gamebaseinfo` VALUES ('40000106', '百人水果机', '40000106.DLL', '180', '180', '0', '0', '0');
INSERT INTO `gamebaseinfo` VALUES ('11100604', '牌九', '11100604.DLL', '4', '4', '0', '0', '0');
INSERT INTO `gamebaseinfo` VALUES ('12101105', '炸金花', '12101105.DLL', '5', '5', '1', '0', '0');
INSERT INTO `gamebaseinfo` VALUES ('20170405', '21点', '20170405.DLL', '4', '4', '0', '0', '0');
INSERT INTO `gamebaseinfo` VALUES ('30000004', '跑得快', '30000004.DLL', '4', '4', '0', '0', '1');
INSERT INTO `gamebaseinfo` VALUES ('30000006', '十点半', '30000006.DLL', '6', '6', '0', '0', '1');
INSERT INTO `gamebaseinfo` VALUES ('30100008', '牛牛', '30100008.DLL', '8', '8', '1', '0', '0');
INSERT INTO `gamebaseinfo` VALUES ('30100009', '抢庄牛牛', '30100009.DLL', '8', '8', '1', '0', '0');
INSERT INTO `gamebaseinfo` VALUES ('30100108', '三公', '30100108.DLL', '8', '8', '1', '0', '0');
INSERT INTO `gamebaseinfo` VALUES ('36610103', '斗地主', '36610103.DLL', '3', '3', '0', '0', '0');
INSERT INTO `gamebaseinfo` VALUES ('40000107', '红黑', '40000107.DLL', '180', '180', '0', '0', '0');

-- ----------------------------
-- Table structure for logonbaseinfo
-- ----------------------------
DROP TABLE IF EXISTS `logonbaseinfo`;
CREATE TABLE `logonbaseinfo` (
  `logonID` int(6) NOT NULL DEFAULT '0' COMMENT '网关服主id',
  `ip` varchar(24) CHARACTER SET utf8 DEFAULT '' COMMENT '网关服ip地址',
  `intranetIP` varchar(24) CHARACTER SET utf8 DEFAULT '' COMMENT '内网ip',
  `port` int(11) DEFAULT '3015' COMMENT '网关服端口',
  `maxPeople` int(11) DEFAULT '3000' COMMENT '网关服TCP最多容纳的最大人数',
  `curPeople` int(11) DEFAULT '0' COMMENT '当前人数',
  `status` tinyint(2) DEFAULT '0' COMMENT '0：关闭状态，1：开启状态',
  `socketCount` int(11) DEFAULT '0' COMMENT 'socket数量',
  `webSocketPort` int(11) DEFAULT '3017' COMMENT 'websocket端口',
  `maxWebSocketPeople` int(11) DEFAULT '1000' COMMENT '网关服websocket最多容纳的最大人数',
  `curWebSocketPeople` int(11) DEFAULT '0' COMMENT '当前人数',
  `webSocketCount` int(11) DEFAULT '0' COMMENT 'socket数量',
  PRIMARY KEY (`logonID`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- ----------------------------
-- Records of logonbaseinfo
-- ----------------------------
INSERT INTO `logonbaseinfo` VALUES ('1', '14.215.33.38', '127.0.0.1', '3015', '1200', '0', '0', '0', '0', '3000', '0', '0');
INSERT INTO `logonbaseinfo` VALUES ('2', '14.215.33.38', '127.0.0.1', '3016', '1200', '0', '0', '0', '0', '3000', '0', '0');
INSERT INTO `logonbaseinfo` VALUES ('3', '14.215.33.38', '127.0.0.1', '3017', '1200', '0', '0', '0', '0', '3000', '0', '0');
INSERT INTO `logonbaseinfo` VALUES ('4', '14.215.33.38', '127.0.0.1', '3018', '1200', '0', '0', '0', '0', '3000', '0', '0');

-- ----------------------------
-- Table structure for otherconfig
-- ----------------------------
DROP TABLE IF EXISTS `otherconfig`;
CREATE TABLE `otherconfig` (
  `keyConfig` varchar(128) NOT NULL DEFAULT '' COMMENT '配置主键',
  `valueConfig` varchar(128) DEFAULT '' COMMENT '配置值',
  `describe` varchar(256) DEFAULT '' COMMENT '描述',
  PRIMARY KEY (`keyConfig`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- ----------------------------
-- Records of otherconfig
-- ----------------------------
INSERT INTO `otherconfig` VALUES ('supportTimesEveryDay', '2', '每天领取救济金次数');
INSERT INTO `otherconfig` VALUES ('supportMinLimitMoney', '2000', '低于多少金币领取救济金');
INSERT INTO `otherconfig` VALUES ('supportMoneyCount', '4000', '每次领取救济金的金币数量每次领取救济金的金币数量');
INSERT INTO `otherconfig` VALUES ('registerGiveMoney', '1000000', '注册送金币');
INSERT INTO `otherconfig` VALUES ('registerGiveJewels', '100', '注册送房卡');
INSERT INTO `otherconfig` VALUES ('logonGiveMoneyEveryDay', '0', '每日登录赠送金币数');
INSERT INTO `otherconfig` VALUES ('sendHornCostJewels', '5', '发送世界广播消耗房卡');
INSERT INTO `otherconfig` VALUES ('useMagicExpressCostMoney', '300', '发送魔法表情消耗金币数');
INSERT INTO `otherconfig` VALUES ('friendRewardMoney', '100', '用户打赏金币');
INSERT INTO `otherconfig` VALUES ('friendRewardMoneyCount', '5', '用户打赏金币次数');
INSERT INTO `otherconfig` VALUES ('buyingDeskCount', '5', '购买房卡次数');
INSERT INTO `otherconfig` VALUES ('sendGiftMyLimitMoney', '10000', '赠送要求自己最低金币数');
INSERT INTO `otherconfig` VALUES ('sendGiftMyLimitJewels', '100', '赠送要求自己最低房卡数');
INSERT INTO `otherconfig` VALUES ('sendGiftMinMoney', '100', '赠送最低金币数');
INSERT INTO `otherconfig` VALUES ('sendGiftMinJewels', '100', '赠送最低房卡数');
INSERT INTO `otherconfig` VALUES ('sendGiftRate', '0.01', '赠送系统扣除的税率');
INSERT INTO `otherconfig` VALUES ('bankMinSaveMoney', '10000', '银行保存最低金币数');
INSERT INTO `otherconfig` VALUES ('bankSaveMoneyMuti', '1000', '银行存钱必须是这个倍数');
INSERT INTO `otherconfig` VALUES ('bankMinTakeMoney', '10000', '银行最低取钱数');
INSERT INTO `otherconfig` VALUES ('bankTakeMoneyMuti', '1000', '银行取钱必须是这个倍数');
INSERT INTO `otherconfig` VALUES ('bankMinTransfer', '10000', '银行最低转账数');
INSERT INTO `otherconfig` VALUES ('bankTransferMuti', '1000', '银行转账必须是这个数的倍数');
INSERT INTO `otherconfig` VALUES ('groupCreateCount', '5', '俱乐部创建限制');
INSERT INTO `otherconfig` VALUES ('groupMemberCount', '150', '俱乐部人数限制');
INSERT INTO `otherconfig` VALUES ('groupJoinCount', '10', '加入俱乐部限制');
INSERT INTO `otherconfig` VALUES ('groupManageRoomCount', '5', '俱乐部管理房间人数限制');
INSERT INTO `otherconfig` VALUES ('groupRoomCount', '1', '俱乐部房间限制');
INSERT INTO `otherconfig` VALUES ('groupAllAlterNameCount', '3', '俱乐部更新ID总次数限制');
INSERT INTO `otherconfig` VALUES ('groupEveAlterNameCount', '1', '俱乐部每日更新ID次数限制');
INSERT INTO `otherconfig` VALUES ('friendTakeRewardMoneyCount', '10', '用户打赏金币次数');
INSERT INTO `otherconfig` VALUES ('groupTransferCount', '3', '俱乐部最多能授权管理员数量');
INSERT INTO `otherconfig` VALUES ('ftpIP', '192.168.1.39', 'ftp服务器ip');
INSERT INTO `otherconfig` VALUES ('ftpUser', 'ftpuser', 'ftp服务器账号');
INSERT INTO `otherconfig` VALUES ('ftpPasswd', '123456', 'ftp服务器密码');
INSERT INTO `otherconfig` VALUES ('byIsIPRegisterLimit', '0', '注册ip限制');
INSERT INTO `otherconfig` VALUES ('IPRegisterLimitCount', '20', '每个ip注册数量');
INSERT INTO `otherconfig` VALUES ('byIsDistributedTable', '0', '分布式表');
INSERT INTO `otherconfig` VALUES ('http', '192.168.1.39', 'http请求的域名');
INSERT INTO `otherconfig` VALUES ('byIsOneToOne', '0', '配置1表示本平台是1:1平台');

-- ----------------------------
-- Table structure for privatedeskconfig
-- ----------------------------
DROP TABLE IF EXISTS `privatedeskconfig`;
CREATE TABLE `privatedeskconfig` (
  `gameID` int(11) DEFAULT '0' COMMENT '游戏id',
  `count` int(11) DEFAULT '0' COMMENT '局数',
  `roomType` int(11) DEFAULT '0' COMMENT '房间类型',
  `costResType` int(11) DEFAULT '2' COMMENT '购买房间需要消耗的资源类型',
  `costNums` int(11) DEFAULT '0' COMMENT '消耗资源数量',
  `AAcostNums` int(11) DEFAULT '0' COMMENT 'AA支付消耗资源数量',
  `otherCostNums` int(11) DEFAULT '0' COMMENT '大赢家支付消耗资源数量',
  `peopleCount` int(11) DEFAULT '0' COMMENT '人数'
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- ----------------------------
-- Records of privatedeskconfig
-- ----------------------------
INSERT INTO `privatedeskconfig` VALUES ('30100008', '10', '1', '2', '3', '1', '0', '0');
INSERT INTO `privatedeskconfig` VALUES ('30100008', '20', '1', '2', '6', '2', '0', '0');
INSERT INTO `privatedeskconfig` VALUES ('30100108', '10', '1', '2', '3', '1', '0', '0');
INSERT INTO `privatedeskconfig` VALUES ('30100108', '20', '1', '2', '6', '2', '0', '0');
INSERT INTO `privatedeskconfig` VALUES ('30000004', '6', '1', '2', '3', '1', '0', '0');
INSERT INTO `privatedeskconfig` VALUES ('30000004', '12', '1', '2', '6', '2', '0', '0');
INSERT INTO `privatedeskconfig` VALUES ('12101105', '6', '1', '2', '3', '1', '0', '0');
INSERT INTO `privatedeskconfig` VALUES ('12101105', '12', '1', '2', '6', '2', '0', '0');
INSERT INTO `privatedeskconfig` VALUES ('30000404', '6', '1', '2', '3', '1', '0', '0');
INSERT INTO `privatedeskconfig` VALUES ('30000404', '12', '1', '2', '6', '2', '0', '0');
INSERT INTO `privatedeskconfig` VALUES ('37460003', '6', '1', '2', '3', '1', '0', '0');
INSERT INTO `privatedeskconfig` VALUES ('37460003', '12', '1', '2', '6', '2', '0', '0');
INSERT INTO `privatedeskconfig` VALUES ('36610103', '6', '1', '2', '3', '1', '0', '0');
INSERT INTO `privatedeskconfig` VALUES ('36610103', '12', '1', '2', '6', '2', '0', '0');
INSERT INTO `privatedeskconfig` VALUES ('30000006', '6', '1', '2', '3', '1', '0', '0');
INSERT INTO `privatedeskconfig` VALUES ('30000006', '12', '1', '2', '6', '2', '0', '0');
INSERT INTO `privatedeskconfig` VALUES ('11100604', '6', '1', '2', '3', '1', '0', '0');
INSERT INTO `privatedeskconfig` VALUES ('11100604', '12', '1', '2', '6', '2', '0', '0');
INSERT INTO `privatedeskconfig` VALUES ('20170405', '6', '1', '2', '3', '1', '0', '0');
INSERT INTO `privatedeskconfig` VALUES ('20170405', '12', '1', '2', '6', '2', '0', '0');
INSERT INTO `privatedeskconfig` VALUES ('20173124', '6', '1', '2', '3', '1', '0', '0');
INSERT INTO `privatedeskconfig` VALUES ('20173124', '12', '1', '2', '6', '2', '0', '0');
INSERT INTO `privatedeskconfig` VALUES ('20161010', '6', '1', '2', '3', '1', '0', '0');
INSERT INTO `privatedeskconfig` VALUES ('20161010', '12', '1', '2', '6', '2', '0', '0');
INSERT INTO `privatedeskconfig` VALUES ('23510004', '6', '1', '2', '3', '1', '0', '0');
INSERT INTO `privatedeskconfig` VALUES ('23510004', '12', '1', '2', '6', '2', '0', '0');
INSERT INTO `privatedeskconfig` VALUES ('26600004', '6', '1', '2', '3', '1', '0', '0');
INSERT INTO `privatedeskconfig` VALUES ('26600004', '12', '1', '2', '6', '2', '0', '0');
INSERT INTO `privatedeskconfig` VALUES ('26600003', '6', '1', '2', '3', '1', '0', '0');
INSERT INTO `privatedeskconfig` VALUES ('26600003', '12', '1', '2', '6', '2', '0', '0');
INSERT INTO `privatedeskconfig` VALUES ('20161004', '6', '1', '2', '3', '1', '0', '0');
INSERT INTO `privatedeskconfig` VALUES ('20161004', '12', '1', '2', '6', '2', '0', '0');
INSERT INTO `privatedeskconfig` VALUES ('30210005', '6', '1', '2', '3', '1', '0', '0');
INSERT INTO `privatedeskconfig` VALUES ('30210005', '12', '1', '2', '6', '2', '0', '0');
INSERT INTO `privatedeskconfig` VALUES ('37910005', '6', '1', '2', '3', '1', '0', '0');
INSERT INTO `privatedeskconfig` VALUES ('37910005', '12', '1', '2', '6', '2', '0', '0');
INSERT INTO `privatedeskconfig` VALUES ('30100008', '10', '2', '1', '3000', '100', '0', '0');
INSERT INTO `privatedeskconfig` VALUES ('30100008', '20', '2', '1', '6000', '300', '0', '0');
INSERT INTO `privatedeskconfig` VALUES ('30100108', '10', '2', '1', '3000', '100', '0', '0');
INSERT INTO `privatedeskconfig` VALUES ('30100108', '20', '2', '1', '6000', '300', '0', '0');
INSERT INTO `privatedeskconfig` VALUES ('30000004', '6', '2', '1', '3000', '100', '0', '0');
INSERT INTO `privatedeskconfig` VALUES ('30000004', '12', '2', '1', '6000', '300', '0', '0');
INSERT INTO `privatedeskconfig` VALUES ('12101105', '6', '2', '1', '3000', '100', '0', '0');
INSERT INTO `privatedeskconfig` VALUES ('12101105', '12', '2', '1', '6000', '300', '0', '0');
INSERT INTO `privatedeskconfig` VALUES ('30000404', '6', '2', '1', '3000', '100', '0', '0');
INSERT INTO `privatedeskconfig` VALUES ('30000404', '12', '2', '1', '6000', '300', '0', '0');
INSERT INTO `privatedeskconfig` VALUES ('37460003', '6', '2', '1', '3000', '100', '0', '0');
INSERT INTO `privatedeskconfig` VALUES ('37460003', '12', '2', '1', '6000', '300', '0', '0');
INSERT INTO `privatedeskconfig` VALUES ('36610103', '6', '2', '1', '3000', '100', '0', '0');
INSERT INTO `privatedeskconfig` VALUES ('36610103', '12', '2', '1', '6000', '300', '0', '0');
INSERT INTO `privatedeskconfig` VALUES ('30000006', '6', '2', '1', '3000', '100', '0', '0');
INSERT INTO `privatedeskconfig` VALUES ('30000006', '12', '2', '1', '6000', '300', '0', '0');
INSERT INTO `privatedeskconfig` VALUES ('11100604', '6', '2', '1', '3000', '100', '0', '0');
INSERT INTO `privatedeskconfig` VALUES ('11100604', '12', '2', '1', '6000', '300', '0', '0');
INSERT INTO `privatedeskconfig` VALUES ('20170405', '6', '2', '1', '3000', '100', '0', '0');
INSERT INTO `privatedeskconfig` VALUES ('20170405', '12', '2', '1', '6000', '300', '0', '0');
INSERT INTO `privatedeskconfig` VALUES ('20173124', '6', '2', '1', '3000', '100', '0', '0');
INSERT INTO `privatedeskconfig` VALUES ('20173124', '12', '2', '1', '6000', '300', '0', '0');
INSERT INTO `privatedeskconfig` VALUES ('20161010', '6', '2', '1', '3000', '100', '0', '0');
INSERT INTO `privatedeskconfig` VALUES ('20161010', '12', '2', '1', '6000', '300', '0', '0');
INSERT INTO `privatedeskconfig` VALUES ('23510004', '6', '2', '1', '3000', '100', '0', '0');
INSERT INTO `privatedeskconfig` VALUES ('23510004', '12', '2', '1', '6000', '300', '0', '0');
INSERT INTO `privatedeskconfig` VALUES ('26600004', '6', '2', '1', '3000', '100', '0', '0');
INSERT INTO `privatedeskconfig` VALUES ('26600004', '12', '2', '1', '6000', '300', '0', '0');
INSERT INTO `privatedeskconfig` VALUES ('26600003', '6', '2', '1', '3000', '100', '0', '0');
INSERT INTO `privatedeskconfig` VALUES ('26600003', '12', '2', '1', '6000', '300', '0', '0');
INSERT INTO `privatedeskconfig` VALUES ('20161004', '6', '2', '1', '3000', '100', '0', '0');
INSERT INTO `privatedeskconfig` VALUES ('20161004', '12', '2', '1', '6000', '300', '0', '0');
INSERT INTO `privatedeskconfig` VALUES ('30210005', '6', '2', '1', '3000', '100', '0', '0');
INSERT INTO `privatedeskconfig` VALUES ('30210005', '12', '2', '1', '6000', '300', '0', '0');
INSERT INTO `privatedeskconfig` VALUES ('37910005', '6', '2', '1', '3000', '100', '0', '0');
INSERT INTO `privatedeskconfig` VALUES ('37910005', '12', '2', '1', '6000', '300', '0', '0');
INSERT INTO `privatedeskconfig` VALUES ('30100008', '10', '3', '2', '3', '1', '0', '0');
INSERT INTO `privatedeskconfig` VALUES ('30100008', '20', '3', '2', '6', '2', '0', '0');
INSERT INTO `privatedeskconfig` VALUES ('30100108', '10', '3', '2', '3', '1', '0', '0');
INSERT INTO `privatedeskconfig` VALUES ('30100108', '20', '3', '2', '6', '2', '0', '0');
INSERT INTO `privatedeskconfig` VALUES ('30000004', '6', '3', '2', '3', '1', '0', '0');
INSERT INTO `privatedeskconfig` VALUES ('30000004', '12', '3', '2', '6', '2', '0', '0');
INSERT INTO `privatedeskconfig` VALUES ('12101105', '6', '3', '2', '3', '1', '0', '0');
INSERT INTO `privatedeskconfig` VALUES ('12101105', '12', '3', '2', '6', '2', '0', '0');
INSERT INTO `privatedeskconfig` VALUES ('30000404', '6', '3', '2', '3', '1', '0', '0');
INSERT INTO `privatedeskconfig` VALUES ('30000404', '12', '3', '2', '6', '2', '0', '0');
INSERT INTO `privatedeskconfig` VALUES ('37460003', '6', '3', '2', '3', '1', '0', '0');
INSERT INTO `privatedeskconfig` VALUES ('37460003', '12', '3', '2', '6', '2', '0', '0');
INSERT INTO `privatedeskconfig` VALUES ('36610103', '6', '3', '2', '3', '1', '0', '0');
INSERT INTO `privatedeskconfig` VALUES ('36610103', '12', '3', '2', '6', '2', '0', '0');
INSERT INTO `privatedeskconfig` VALUES ('30000006', '6', '3', '2', '3', '1', '0', '0');
INSERT INTO `privatedeskconfig` VALUES ('30000006', '12', '3', '2', '6', '2', '0', '0');
INSERT INTO `privatedeskconfig` VALUES ('11100604', '6', '3', '2', '3', '1', '0', '0');
INSERT INTO `privatedeskconfig` VALUES ('11100604', '12', '3', '2', '6', '2', '0', '0');
INSERT INTO `privatedeskconfig` VALUES ('20170405', '6', '3', '2', '3', '1', '0', '0');
INSERT INTO `privatedeskconfig` VALUES ('20170405', '12', '3', '2', '6', '2', '0', '0');
INSERT INTO `privatedeskconfig` VALUES ('20173124', '6', '3', '2', '3', '1', '0', '0');
INSERT INTO `privatedeskconfig` VALUES ('20173124', '12', '3', '2', '6', '2', '0', '0');
INSERT INTO `privatedeskconfig` VALUES ('20161010', '6', '3', '2', '3', '1', '0', '0');
INSERT INTO `privatedeskconfig` VALUES ('20161010', '12', '3', '2', '6', '2', '0', '0');
INSERT INTO `privatedeskconfig` VALUES ('23510004', '6', '3', '2', '3', '1', '0', '0');
INSERT INTO `privatedeskconfig` VALUES ('23510004', '12', '3', '2', '6', '2', '0', '0');
INSERT INTO `privatedeskconfig` VALUES ('26600004', '6', '3', '2', '3', '1', '0', '0');
INSERT INTO `privatedeskconfig` VALUES ('26600004', '12', '3', '2', '6', '2', '0', '0');
INSERT INTO `privatedeskconfig` VALUES ('26600003', '6', '3', '2', '3', '1', '0', '0');
INSERT INTO `privatedeskconfig` VALUES ('26600003', '12', '3', '2', '6', '2', '0', '0');
INSERT INTO `privatedeskconfig` VALUES ('20161004', '6', '3', '2', '3', '1', '0', '0');
INSERT INTO `privatedeskconfig` VALUES ('20161004', '12', '3', '2', '6', '2', '0', '0');
INSERT INTO `privatedeskconfig` VALUES ('30210005', '6', '3', '2', '3', '1', '0', '0');
INSERT INTO `privatedeskconfig` VALUES ('30210005', '12', '3', '2', '6', '2', '0', '0');
INSERT INTO `privatedeskconfig` VALUES ('37910005', '6', '3', '2', '3', '1', '0', '0');
INSERT INTO `privatedeskconfig` VALUES ('37910005', '12', '3', '2', '6', '2', '0', '0');
INSERT INTO `privatedeskconfig` VALUES ('37550102', '1', '2', '2', '0', '0', '0', '0');
INSERT INTO `privatedeskconfig` VALUES ('35160004', '6', '1', '2', '3', '1', '0', '0');
INSERT INTO `privatedeskconfig` VALUES ('35160004', '12', '1', '2', '6', '2', '0', '0');
INSERT INTO `privatedeskconfig` VALUES ('35160004', '6', '2', '1', '3000', '100', '0', '0');
INSERT INTO `privatedeskconfig` VALUES ('35160004', '12', '2', '1', '6000', '300', '0', '0');
INSERT INTO `privatedeskconfig` VALUES ('35160004', '6', '3', '2', '3', '1', '0', '0');
INSERT INTO `privatedeskconfig` VALUES ('35160004', '12', '3', '2', '6', '2', '0', '0');

-- ----------------------------
-- Table structure for redisbaseinfo
-- ----------------------------
DROP TABLE IF EXISTS `redisbaseinfo`;
CREATE TABLE `redisbaseinfo` (
  `redisTypeID` int(4) NOT NULL,
  `ip` varchar(128) CHARACTER SET utf8 DEFAULT '' COMMENT 'redis的ip',
  `port` int(11) DEFAULT '6379' COMMENT 'redis',
  `passwd` varchar(128) CHARACTER SET utf8 DEFAULT '' COMMENT 'redis密码，没有密码不配置',
  PRIMARY KEY (`redisTypeID`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- ----------------------------
-- Records of redisbaseinfo
-- ----------------------------
INSERT INTO `redisbaseinfo` VALUES ('1', '127.0.0.1', '6380', '');
INSERT INTO `redisbaseinfo` VALUES ('2', '127.0.0.1', '6381', '');

-- ----------------------------
-- Table structure for rewardspool
-- ----------------------------
DROP TABLE IF EXISTS `rewardspool`;
CREATE TABLE `rewardspool` (
  `roomID` int(11) NOT NULL DEFAULT '0' COMMENT '房间id',
  `poolMoney` bigint(20) DEFAULT '0' COMMENT '奖池',
  `gameWinMoney` bigint(20) DEFAULT '0' COMMENT '游戏输赢钱',
  `percentageWinMoney` bigint(20) DEFAULT '0' COMMENT '抽水获取金币数量',
  `otherWinMoney` bigint(20) DEFAULT '0' COMMENT '其它方式获得金币数量',
  `allGameWinMoney` bigint(20) DEFAULT '0' COMMENT '累计输赢金币数量',
  `allPercentageWinMoney` bigint(20) DEFAULT '0' COMMENT '累计抽水',
  `allOtherWinMoney` bigint(20) DEFAULT '0' COMMENT '累计其它方式赢钱',
  `platformCtrlType` tinyint(3) unsigned DEFAULT '0' COMMENT '0：根据platformCtrlPercent值进行控制，1：根据spotControlInfo进行多点控制',
  `platformCtrlPercent` int(11) DEFAULT '0' COMMENT '单点控制千分比，比如500：表示10局赢5局',
  `realPeopleFailPercent` int(11) DEFAULT '0' COMMENT '真人玩家输概率',
  `realPeopleWinPercent` int(11) DEFAULT '0' COMMENT '真人玩家赢概率',
  `detailInfo` varchar(1024) DEFAULT '' COMMENT '房间控制详情，比如豹子王出豹子概率。100,200,300',
  PRIMARY KEY (`roomID`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- ----------------------------
-- Records of rewardspool
-- ----------------------------

-- ----------------------------
-- Table structure for roombaseinfo
-- ----------------------------
DROP TABLE IF EXISTS `roombaseinfo`;
CREATE TABLE `roombaseinfo` (
  `roomID` int(11) NOT NULL,
  `gameID` int(11) DEFAULT '0',
  `name` varchar(48) DEFAULT '',
  `serviceName` varchar(48) DEFAULT '',
  `type` int(11) DEFAULT '0',
  `sort` int(11) DEFAULT '0',
  `deskCount` int(11) DEFAULT '1',
  `minPoint` int(11) DEFAULT '0',
  `maxPoint` int(11) DEFAULT '0',
  `basePoint` int(11) DEFAULT '1' COMMENT '底注',
  `gameBeginCostMoney` int(11) DEFAULT '0' COMMENT '开局消耗',
  `describe` varchar(64) DEFAULT '' COMMENT '房间说明',
  `roomSign` int(11) DEFAULT '0' COMMENT '房间标识',
  `robotCount` int(11) DEFAULT '0' COMMENT '每桌最多的机器人数量',
  `status` tinyint(2) DEFAULT '0' COMMENT '0：未启动，1：已经启动',
  `currPeopleCount` int(11) DEFAULT '0' COMMENT '当前房间人数',
  `level` tinyint(2) DEFAULT '0' COMMENT '房间等级，比如1：初级场，2：中级场',
  `configInfo` varchar(2048) DEFAULT '' COMMENT '房间配置信息，json格式保存',
  PRIMARY KEY (`roomID`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- ----------------------------
-- Records of roombaseinfo
-- ----------------------------
INSERT INTO `roombaseinfo` VALUES ('1', '30000200', '百人牛牛金币场', 'brnn', '0', '1', '1', '0', '0', '0', '0', '经典玩法', '0', '100', '1', '30', '1', '0');
INSERT INTO `roombaseinfo` VALUES ('2', '30000200', '百人牛牛银条场', 'brnn', '0', '1', '1', '10000', '0', '1', '0', '经典玩法', '0', '0', '0', '0', '2', '0');
INSERT INTO `roombaseinfo` VALUES ('3', '30000200', '百人牛牛金钻场', 'brnn', '0', '1', '1', '30000', '0', '1', '0', '经典玩法', '0', '0', '0', '0', '3', '0');
INSERT INTO `roombaseinfo` VALUES ('4', '11100200', '欢乐30秒金币场', 'hl30m', '0', '1', '1', '2000', '0', '1', '0', '经典玩法', '0', '0', '0', '0', '1', '0');
INSERT INTO `roombaseinfo` VALUES ('5', '11100200', '欢乐30秒银条场', 'hl30m', '0', '1', '1', '10000', '0', '1', '0', '经典玩法', '0', '0', '0', '0', '2', '0');
INSERT INTO `roombaseinfo` VALUES ('6', '11100200', '欢乐30秒金钻场', 'hl30m', '0', '1', '1', '30000', '0', '1', '0', '经典玩法', '0', '0', '0', '0', '3', '0');
INSERT INTO `roombaseinfo` VALUES ('7', '10000900', '豹子王金币场', 'bzw', '0', '1', '1', '2000', '0', '1', '0', '经典玩法', '0', '0', '0', '0', '1', '0');
INSERT INTO `roombaseinfo` VALUES ('8', '10000900', '豹子王银条场', 'bzw', '0', '1', '1', '10000', '0', '1', '0', '经典玩法', '0', '0', '0', '0', '2', '0');
INSERT INTO `roombaseinfo` VALUES ('9', '10000900', '豹子王金钻场', 'bzw', '0', '1', '1', '30000', '0', '1', '0', '经典玩法', '0', '0', '0', '0', '3', '0');
INSERT INTO `roombaseinfo` VALUES ('29', '30000604', '二八杠银条场', 'ebg', '0', '1', '1', '10000', '0', '1', '0', '经典玩法', '0', '0', '0', '0', '2', '0');
INSERT INTO `roombaseinfo` VALUES ('30', '30000604', '二八杠金钻场', 'ebg', '0', '1', '1', '30000', '0', '1', '0', '经典玩法', '0', '0', '0', '0', '3', '0');
INSERT INTO `roombaseinfo` VALUES ('10', '10001000', '奔驰宝马金币场', 'bcbm', '0', '1', '1', '2000', '0', '1', '0', '经典玩法', '0', '0', '0', '0', '1', '0');
INSERT INTO `roombaseinfo` VALUES ('11', '10001000', '奔驰宝马银条场', 'bcbm', '0', '1', '1', '10000', '0', '1', '0', '经典玩法', '0', '0', '0', '0', '2', '0');
INSERT INTO `roombaseinfo` VALUES ('12', '10001000', '奔驰宝马金钻场', 'bcbm', '0', '1', '1', '30000', '0', '1', '0', '经典玩法', '0', '0', '0', '0', '3', '0');
INSERT INTO `roombaseinfo` VALUES ('13', '11000100', '飞禽走兽金币场', 'fqzs', '0', '1', '1', '2000', '0', '1', '0', '经典玩法', '0', '0', '0', '0', '1', '0');
INSERT INTO `roombaseinfo` VALUES ('14', '11000100', '飞禽走兽银条场', 'fqzs', '0', '1', '1', '10000', '0', '1', '0', '经典玩法', '0', '0', '0', '0', '2', '0');
INSERT INTO `roombaseinfo` VALUES ('15', '11000100', '飞禽走兽金钻场', 'fqzs', '0', '1', '1', '30000', '0', '1', '0', '经典玩法', '0', '0', '0', '0', '3', '0');
INSERT INTO `roombaseinfo` VALUES ('16', '40000100', '龙虎斗金币场', 'lhd', '0', '1', '1', '2000', '0', '1', '0', '经典玩法', '0', '0', '0', '0', '1', '0');
INSERT INTO `roombaseinfo` VALUES ('17', '40000100', '龙虎斗银条场', 'lhd', '0', '1', '1', '10000', '0', '1', '0', '经典玩法', '0', '0', '0', '0', '2', '0');
INSERT INTO `roombaseinfo` VALUES ('18', '40000100', '龙虎斗金钻场', 'lhd', '0', '1', '1', '30000', '0', '1', '0', '经典玩法', '0', '0', '0', '0', '3', '0');
INSERT INTO `roombaseinfo` VALUES ('22', '50000000', '李逵劈鱼初级场', 'lkpy', '0', '2', '100', '2000', '0', '1', '0', '经典玩法', '0', '2', '0', '0', '1', '0');
INSERT INTO `roombaseinfo` VALUES ('23', '50000000', '李逵劈鱼中级场', 'lkpy', '0', '2', '100', '10000', '0', '1', '0', '经典玩法', '0', '2', '0', '0', '2', '0');
INSERT INTO `roombaseinfo` VALUES ('24', '50000000', '李逵劈鱼高级场', 'lkpy', '0', '2', '100', '30000', '0', '1', '0', '经典玩法', '0', '2', '0', '0', '3', '0');
INSERT INTO `roombaseinfo` VALUES ('25', '41100100', '百人牌九金币场', 'brpj', '0', '1', '1', '2000', '0', '1', '0', '经典玩法', '0', '0', '0', '0', '1', '0');
INSERT INTO `roombaseinfo` VALUES ('26', '41100100', '百人牌九银条场', 'brpj', '0', '1', '1', '10000', '0', '1', '0', '经典玩法', '0', '0', '0', '0', '2', '0');
INSERT INTO `roombaseinfo` VALUES ('19', '40000000', '红包扫雷初级场', 'hbsl', '0', '1', '1', '2000', '0', '1', '0', '经典玩法', '0', '0', '0', '0', '1', '0');
INSERT INTO `roombaseinfo` VALUES ('20', '40000000', '红包扫雷中级场', 'hbsl', '0', '1', '1', '10000', '0', '1', '0', '经典玩法', '0', '0', '0', '0', '2', '0');
INSERT INTO `roombaseinfo` VALUES ('21', '40000000', '红包扫雷高级场', 'hbsl', '0', '1', '1', '30000', '0', '1', '0', '经典玩法', '0', '0', '0', '0', '3', '0');
INSERT INTO `roombaseinfo` VALUES ('28', '30000604', '二八杠金币场', 'ebg', '0', '1', '1', '2000', '0', '1', '0', '经典玩法', '0', '20', '1', '31', '1', '0');
INSERT INTO `roombaseinfo` VALUES ('27', '41100100', '百人牌九金钻场', 'brpj', '0', '1', '1', '30000', '0', '1', '0', '经典玩法', '0', '0', '0', '0', '3', '0');
INSERT INTO `roombaseinfo` VALUES ('31', '50000001', 'BOSS来了初级场', 'bsll', '0', '2', '100', '2000', '0', '1', '0', '经典玩法', '0', '2', '0', '0', '1', '0');
INSERT INTO `roombaseinfo` VALUES ('32', '50000001', 'BOSS来了中级场', 'bsll', '0', '2', '100', '10000', '0', '1', '0', '经典玩法', '0', '2', '0', '0', '2', '0');
INSERT INTO `roombaseinfo` VALUES ('33', '50000001', 'BOSS来了高级场', 'bsll', '0', '2', '100', '30000', '0', '1', '0', '经典玩法', '0', '2', '0', '0', '3', '0');
INSERT INTO `roombaseinfo` VALUES ('34', '50000002', '深海捕鱼初级场', 'shby', '0', '2', '100', '2000', '0', '1', '0', '经典玩法', '0', '2', '0', '0', '1', '0');
INSERT INTO `roombaseinfo` VALUES ('35', '50000002', '深海捕鱼中级场', 'shby', '0', '2', '100', '10000', '0', '1', '0', '经典玩法', '0', '2', '0', '0', '2', '0');
INSERT INTO `roombaseinfo` VALUES ('36', '50000002', '深海捕鱼高级场', 'shby', '0', '2', '100', '30000', '0', '1', '0', '经典玩法', '0', '2', '0', '0', '3', '0');
INSERT INTO `roombaseinfo` VALUES ('37', '10003000', '水浒传', 'shz', '0', '2', '300', '2000', '0', '1', '0', '经典玩法', '0', '0', '0', '0', '1', '0');
INSERT INTO `roombaseinfo` VALUES ('38', '10003000', '水浒传', 'shz', '0', '2', '300', '10000', '0', '1', '0', '经典玩法', '0', '0', '0', '0', '2', '0');
INSERT INTO `roombaseinfo` VALUES ('39', '10003000', '水浒传', 'shz', '0', '2', '300', '30000', '0', '1', '0', '经典玩法', '0', '0', '0', '0', '3', '0');
INSERT INTO `roombaseinfo` VALUES ('40', '10003001', '拉霸', 'lb', '0', '2', '300', '2000', '0', '1', '0', '经典玩法', '0', '0', '0', '0', '1', '0');
INSERT INTO `roombaseinfo` VALUES ('41', '10003001', '拉霸', 'lb', '0', '2', '300', '10000', '0', '1', '0', '经典玩法', '0', '0', '0', '0', '2', '0');
INSERT INTO `roombaseinfo` VALUES ('42', '10003001', '拉霸', 'lb', '0', '2', '300', '30000', '0', '1', '0', '经典玩法', '0', '0', '0', '0', '3', '0');
INSERT INTO `roombaseinfo` VALUES ('43', '10003002', '财神到', 'csd', '0', '2', '300', '2000', '0', '1', '0', '经典玩法', '0', '0', '0', '0', '1', '0');
INSERT INTO `roombaseinfo` VALUES ('44', '10003002', '财神到', 'csd', '0', '2', '300', '10000', '0', '1', '0', '经典玩法', '0', '0', '0', '0', '2', '0');
INSERT INTO `roombaseinfo` VALUES ('45', '10003002', '财神到', 'csd', '0', '2', '300', '30000', '0', '1', '0', '经典玩法', '0', '0', '0', '0', '3', '0');
INSERT INTO `roombaseinfo` VALUES ('46', '40000106', '百人水果机金币场', 'brsgj', '0', '1', '300', '2000', '0', '1', '0', '经典玩法', '0', '0', '0', '0', '1', '0');
INSERT INTO `roombaseinfo` VALUES ('47', '40000106', '百人水果机银条场', 'brsgj', '0', '1', '300', '10000', '0', '1', '0', '经典玩法', '0', '0', '0', '0', '2', '0');
INSERT INTO `roombaseinfo` VALUES ('48', '40000106', '百人水果机金钻场', 'brsgj', '0', '1', '300', '30000', '0', '1', '0', '经典玩法', '0', '0', '0', '0', '3', '0');
INSERT INTO `roombaseinfo` VALUES ('49', '30100008', '通比牛牛1元场', 'njj', '0', '0', '200', '5000', '0', '100', '10', '通比牛牛', '5', '0', '0', '0', '1', '');
INSERT INTO `roombaseinfo` VALUES ('50', '30100008', '通比牛牛5元场', 'njj', '0', '0', '200', '20000', '0', '500', '10', '通比牛牛', '5', '0', '0', '0', '2', '');
INSERT INTO `roombaseinfo` VALUES ('51', '30100008', '通比牛牛10元场', 'njj', '0', '0', '200', '50000', '0', '1000', '10', '通比牛牛', '5', '0', '0', '0', '3', '');
INSERT INTO `roombaseinfo` VALUES ('52', '30100108', '三公1元场', 'sg', '0', '0', '200', '5000', '0', '100', '10', '通比三公', '5', '0', '0', '0', '1', '');
INSERT INTO `roombaseinfo` VALUES ('53', '30100108', '三公5元场', 'sg', '0', '0', '200', '20000', '0', '500', '10', '自由抢庄', '3', '0', '0', '0', '2', '');
INSERT INTO `roombaseinfo` VALUES ('54', '30100108', '三公10元场', 'sg', '0', '0', '200', '50000', '0', '1000', '10', '明牌抢庄', '4', '0', '0', '0', '3', '');
INSERT INTO `roombaseinfo` VALUES ('55', '30000004', '跑得快1元场', 'pdk', '0', '0', '200', '5000', '0', '100', '10', '经典玩法', '0', '0', '0', '0', '1', '');
INSERT INTO `roombaseinfo` VALUES ('56', '30000004', '跑得快5元场', 'pdk', '0', '0', '200', '20000', '0', '500', '10', '经典玩法', '0', '0', '0', '0', '2', '');
INSERT INTO `roombaseinfo` VALUES ('57', '30000004', '跑得快10元场', 'pdk', '0', '0', '200', '50000', '0', '1000', '10', '经典玩法', '0', '0', '0', '0', '3', '');
INSERT INTO `roombaseinfo` VALUES ('58', '12101105', '炸金花1元场', 'zhajinhua', '0', '0', '200', '5000', '0', '100', '10', '经典玩法', '0', '0', '0', '0', '1', '');
INSERT INTO `roombaseinfo` VALUES ('59', '12101105', '炸金花5元场', 'zhajinhua', '0', '0', '200', '20000', '0', '500', '10', '经典玩法', '0', '0', '0', '0', '2', '');
INSERT INTO `roombaseinfo` VALUES ('60', '12101105', '炸金花10元场', 'zhajinhua', '0', '0', '200', '50000', '0', '1000', '10', '经典玩法', '0', '0', '0', '0', '3', '');
INSERT INTO `roombaseinfo` VALUES ('61', '11100604', '牌九1元场', 'srpj', '0', '0', '200', '5000', '0', '100', '10', '经典玩法', '0', '0', '0', '0', '1', '');
INSERT INTO `roombaseinfo` VALUES ('62', '11100604', '牌九5元场', 'srpj', '0', '0', '200', '20000', '0', '500', '10', '经典玩法', '0', '0', '0', '0', '2', '');
INSERT INTO `roombaseinfo` VALUES ('63', '11100604', '牌九10元场', 'srpj', '0', '0', '200', '50000', '0', '1000', '10', '经典玩法', '0', '0', '0', '0', '3', '');
INSERT INTO `roombaseinfo` VALUES ('64', '20170405', '21点1元场', 'lyjk', '0', '0', '200', '5000', '0', '100', '10', '经典玩法', '0', '0', '0', '0', '1', '');
INSERT INTO `roombaseinfo` VALUES ('65', '20170405', '21点5元场', 'lyjk', '0', '0', '200', '20000', '0', '500', '10', '经典玩法', '0', '0', '0', '0', '2', '');
INSERT INTO `roombaseinfo` VALUES ('66', '20170405', '21点10元场', 'lyjk', '0', '0', '200', '50000', '0', '1000', '10', '经典玩法', '0', '0', '0', '0', '3', '');
INSERT INTO `roombaseinfo` VALUES ('67', '30000006', '十点半1元场', 'sdb', '0', '0', '200', '5000', '0', '100', '10', '经典玩法', '0', '0', '0', '0', '1', '');
INSERT INTO `roombaseinfo` VALUES ('68', '30000006', '十点半5元场', 'sdb', '0', '0', '200', '20000', '0', '500', '10', '经典玩法', '0', '0', '0', '0', '2', '');
INSERT INTO `roombaseinfo` VALUES ('69', '30000006', '十点半10元场', 'sdb', '0', '0', '200', '50000', '0', '1000', '10', '经典玩法', '0', '0', '0', '0', '3', '');
INSERT INTO `roombaseinfo` VALUES ('70', '30100009', '抢庄牛牛1元场', 'njj2', '0', '0', '200', '5000', '0', '100', '10', '明牌抢庄', '4', '0', '0', '0', '1', '');
INSERT INTO `roombaseinfo` VALUES ('71', '30100009', '抢庄牛牛5元场', 'njj2', '0', '0', '200', '20000', '0', '500', '10', '明牌抢庄', '4', '0', '0', '0', '2', '');
INSERT INTO `roombaseinfo` VALUES ('72', '30100009', '抢庄牛牛10元场', 'njj2', '0', '0', '200', '50000', '0', '1000', '10', '明牌抢庄', '4', '0', '0', '0', '3', '');
INSERT INTO `roombaseinfo` VALUES ('73', '36610103', '斗地主1元场', 'ddz', '0', '0', '200', '5000', '0', '100', '10', '不洗牌', '1', '0', '0', '0', '1', '');
INSERT INTO `roombaseinfo` VALUES ('74', '36610103', '斗地主5元场', 'ddz', '0', '0', '200', '20000', '0', '500', '10', '经典玩法', '0', '0', '0', '0', '2', '');
INSERT INTO `roombaseinfo` VALUES ('75', '36610103', '斗地主10元场', 'ddz', '0', '0', '200', '50000', '0', '1000', '10', '不洗牌', '1', '0', '0', '0', '3', '');
INSERT INTO `roombaseinfo` VALUES ('76', '40000107', '红黑金币场', 'hh', '0', '1', '1', '2000', '0', '1', '0', '经典玩法', '0', '0', '0', '0', '1', '0');
INSERT INTO `roombaseinfo` VALUES ('77', '40000107', '红黑银条场', 'hh', '0', '1', '1', '10000', '0', '1', '0', '经典玩法', '0', '0', '0', '0', '2', '0');
INSERT INTO `roombaseinfo` VALUES ('78', '40000107', '红黑金钻场', 'hh', '0', '1', '1', '30000', '0', '1', '0', '经典玩法', '0', '0', '0', '0', '3', '0');

-- ----------------------------
-- Table structure for statistics_firecoinchange
-- ----------------------------
DROP TABLE IF EXISTS `statistics_firecoinchange`;
CREATE TABLE `statistics_firecoinchange` (
  `id` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `userID` int(11) DEFAULT '0' COMMENT '玩家id',
  `time` int(11) DEFAULT '0' COMMENT '时间',
  `fireCoin` bigint(20) DEFAULT '0' COMMENT '玩家变化之后的火币',
  `changeFireCoin` bigint(20) DEFAULT '0' COMMENT '变化火币值',
  `reason` int(11) DEFAULT '0' COMMENT '变化原因',
  `roomID` int(11) DEFAULT '0' COMMENT '房间id',
  `friendsGroupID` int(11) DEFAULT '0' COMMENT '俱乐部id',
  `rateFireCoin` int(11) DEFAULT '0' COMMENT '抽水火币',
  `status` tinyint(1) DEFAULT '0',
  PRIMARY KEY (`id`),
  KEY `userID` (`userID`) USING BTREE,
  KEY `friendsGroupID` (`friendsGroupID`),
  KEY `roomID` (`roomID`),
  KEY `reason` (`reason`),
  KEY `time` (`time`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- ----------------------------
-- Records of statistics_firecoinchange
-- ----------------------------

-- ----------------------------
-- Table structure for statistics_gamerecordinfo
-- ----------------------------
DROP TABLE IF EXISTS `statistics_gamerecordinfo`;
CREATE TABLE `statistics_gamerecordinfo` (
  `id` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `passwd` varchar(20) DEFAULT '' COMMENT '房间号',
  `deskIdx` int(11) DEFAULT '-1',
  `roomID` int(11) DEFAULT '0',
  `createTime` int(11) DEFAULT '0',
  `beginTime` int(11) DEFAULT '0',
  `finishedTime` int(11) DEFAULT '0',
  `userIDList` varchar(1024) DEFAULT '' COMMENT '战绩列表',
  PRIMARY KEY (`id`),
  KEY `roomID` (`roomID`) USING BTREE,
  KEY `userIDList` (`userIDList`(255)) USING BTREE,
  KEY `beginTime` (`beginTime`),
  KEY `finishedTime` (`finishedTime`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- ----------------------------
-- Records of statistics_gamerecordinfo
-- ----------------------------

-- ----------------------------
-- Table structure for statistics_jewelschange
-- ----------------------------
DROP TABLE IF EXISTS `statistics_jewelschange`;
CREATE TABLE `statistics_jewelschange` (
  `id` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `userID` int(11) DEFAULT '0',
  `time` int(11) DEFAULT '0',
  `jewels` int(11) DEFAULT '0',
  `changeJewels` int(11) DEFAULT '0',
  `reason` int(11) DEFAULT '0',
  `roomID` int(11) DEFAULT '0',
  `friendsGroupID` int(11) DEFAULT '0',
  `rateJewels` int(11) DEFAULT '0' COMMENT '钻石抽水',
  `status` tinyint(1) DEFAULT '0' COMMENT '计算代理提成使用',
  PRIMARY KEY (`id`),
  KEY `userID` (`userID`) USING BTREE,
  KEY `time` (`time`) USING BTREE,
  KEY `status` (`status`) USING BTREE,
  KEY `reason` (`reason`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- ----------------------------
-- Records of statistics_jewelschange
-- ----------------------------

-- ----------------------------
-- Table structure for statistics_logonandlogout
-- ----------------------------
DROP TABLE IF EXISTS `statistics_logonandlogout`;
CREATE TABLE `statistics_logonandlogout` (
  `id` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `userID` int(11) DEFAULT '0',
  `type` int(11) DEFAULT '0' COMMENT '1：大厅登录，2：大厅登出，3：金币场登录',
  `time` int(11) DEFAULT '0' COMMENT '操作时间',
  `ip` varchar(24) DEFAULT '',
  `platformType` int(11) DEFAULT '0' COMMENT '登陆平台类型。如果登录金币场，记录的是roomID',
  `macAddr` varchar(64) DEFAULT '' COMMENT '物理地址',
  PRIMARY KEY (`id`),
  KEY `userID` (`userID`) USING BTREE,
  KEY `type` (`type`),
  KEY `time` (`time`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- ----------------------------
-- Records of statistics_logonandlogout
-- ----------------------------

-- ----------------------------
-- Table structure for statistics_moneychange
-- ----------------------------
DROP TABLE IF EXISTS `statistics_moneychange`;
CREATE TABLE `statistics_moneychange` (
  `id` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `userID` int(11) DEFAULT '0' COMMENT '金币变化用户',
  `time` int(11) DEFAULT '0' COMMENT '时间',
  `money` bigint(20) DEFAULT '0' COMMENT '变化之后的金币',
  `changeMoney` bigint(20) DEFAULT '0' COMMENT '变化金币',
  `reason` int(11) DEFAULT '0' COMMENT '变化原因',
  `roomID` int(11) DEFAULT '0' COMMENT '房间id',
  `friendsGroupID` int(11) DEFAULT '0',
  `rateMoney` bigint(20) DEFAULT '0' COMMENT '抽水金币',
  `status` tinyint(1) DEFAULT '0',
  PRIMARY KEY (`id`),
  KEY `userID` (`userID`) USING BTREE,
  KEY `status` (`status`) USING BTREE,
  KEY `time` (`time`) USING BTREE,
  KEY `reason` (`reason`) USING BTREE
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- ----------------------------
-- Records of statistics_moneychange
-- ----------------------------

-- ----------------------------
-- Table structure for statistics_rewardspool
-- ----------------------------
DROP TABLE IF EXISTS `statistics_rewardspool`;
CREATE TABLE `statistics_rewardspool` (
  `id` int(12) unsigned NOT NULL AUTO_INCREMENT,
  `time` bigint(20) DEFAULT '0' COMMENT '生成时间',
  `roomID` int(11) DEFAULT '0' COMMENT '房间id',
  `poolMoney` bigint(20) DEFAULT '0' COMMENT '奖池',
  `gameWinMoney` bigint(20) DEFAULT '0' COMMENT '游戏输赢钱',
  `percentageWinMoney` bigint(20) DEFAULT '0' COMMENT '抽水获取金币数量',
  `otherWinMoney` bigint(20) DEFAULT '0' COMMENT '其它方式获得金币数量',
  PRIMARY KEY (`id`),
  KEY `roomID` (`roomID`) USING BTREE
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- ----------------------------
-- Records of statistics_rewardspool
-- ----------------------------

-- ----------------------------
-- Table structure for statistics_roomwincount
-- ----------------------------
DROP TABLE IF EXISTS `statistics_roomwincount`;
CREATE TABLE `statistics_roomwincount` (
  `id` bigint(20) unsigned NOT NULL AUTO_INCREMENT COMMENT '自增id',
  `time` bigint(20) DEFAULT '0' COMMENT '时间',
  `roomID` int(11) DEFAULT '0' COMMENT '房间id',
  `gameID` int(11) DEFAULT '0' COMMENT '游戏id',
  `runGameCount` int(11) DEFAULT '0' COMMENT '游戏运行局数',
  `winMoney` bigint(20) DEFAULT '0' COMMENT '系统输赢钱',
  `taxMoney` bigint(20) DEFAULT '0' COMMENT '系统抽水赢钱',
  `friendsGroupID` int(11) DEFAULT '0' COMMENT '俱乐部id',
  `platformCtrlPercent` int(11) DEFAULT '0' COMMENT '当前控制百分比',
  `ctrlParamInfo` varchar(128) DEFAULT '' COMMENT '控制信息',
  PRIMARY KEY (`id`),
  KEY `timeandroomID` (`time`,`roomID`) USING BTREE,
  KEY `roomID` (`roomID`),
  KEY `time` (`time`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- ----------------------------
-- Records of statistics_roomwincount
-- ----------------------------

-- ----------------------------
-- Table structure for userbag
-- ----------------------------
DROP TABLE IF EXISTS `userbag`;
CREATE TABLE `userbag` (
  `userID` int(11) NOT NULL DEFAULT '0' COMMENT '玩家id',
  `skillFrozen` int(11) DEFAULT '0' COMMENT '冰冻技能',
  `skillLocking` int(11) DEFAULT '0' COMMENT '技能锁定',
  `redBag` int(11) DEFAULT '0' COMMENT '红包',
  `phoneBillCard1` int(11) DEFAULT '0' COMMENT '话费卡1元',
  `phoneBillCard5` int(11) DEFAULT '0' COMMENT '话费卡5元',
  `goldenArmor` int(11) DEFAULT '0' COMMENT '黄金战甲',
  `mechatroPioneer` int(11) DEFAULT '0' COMMENT '机甲先锋',
  `deepSeaArtillery` int(11) DEFAULT '0' COMMENT '深海大炮',
  `octopusCannon` int(11) DEFAULT '0' COMMENT '章鱼大炮',
  `goldenDragon` int(11) DEFAULT '0' COMMENT '黄金圣龙',
  `lavaArmor` int(11) DEFAULT '0' COMMENT '熔岩战甲',
  PRIMARY KEY (`userID`),
  KEY `userID` (`userID`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- ----------------------------
-- Records of userbag
-- ----------------------------

-- ----------------------------
-- Table structure for userinfo
-- ----------------------------
DROP TABLE IF EXISTS `userinfo`;
CREATE TABLE `userinfo` (
  `userID` int(11) NOT NULL,
  `account` varchar(64) DEFAULT '' COMMENT '账号',
  `passwd` varchar(64) DEFAULT '' COMMENT '密码',
  `name` varchar(64) DEFAULT '' COMMENT '玩家名字',
  `phone` varchar(24) DEFAULT '' COMMENT '电话号码',
  `sex` tinyint(2) DEFAULT '1' COMMENT '性别',
  `mail` varchar(64) DEFAULT '' COMMENT '邮件',
  `money` bigint(20) DEFAULT '0' COMMENT '玩家金币',
  `bankMoney` bigint(20) DEFAULT '0' COMMENT '银行金币',
  `bankPasswd` varchar(20) DEFAULT '' COMMENT '银行密码',
  `jewels` int(11) DEFAULT '0' COMMENT '钻石数量',
  `roomID` int(11) DEFAULT '0' COMMENT '玩家所有房间的roomID',
  `deskIdx` int(11) DEFAULT '-1' COMMENT '玩家所在的桌子索引',
  `logonIP` varchar(24) DEFAULT '' COMMENT '登陆ip',
  `headURL` varchar(256) DEFAULT '' COMMENT '头像',
  `winCount` int(11) DEFAULT '0' COMMENT '胜局数',
  `macAddr` varchar(64) DEFAULT '' COMMENT '物理地址',
  `token` varchar(64) DEFAULT '' COMMENT '链接token',
  `isVirtual` tinyint(2) DEFAULT '0' COMMENT '是否是机器人',
  `status` int(11) DEFAULT '0' COMMENT '标识玩家身份，1超端',
  `reqSupportTimes` int(11) DEFAULT '0' COMMENT '领取救济金次数',
  `lastReqSupportDate` int(11) DEFAULT '0' COMMENT '上次领取救济金时间',
  `registerTime` int(11) DEFAULT '0' COMMENT '注册时间',
  `registerIP` varchar(24) DEFAULT '' COMMENT '注册ip',
  `registerType` tinyint(2) DEFAULT '1' COMMENT '注册类型',
  `buyingDeskCount` int(11) DEFAULT '0' COMMENT '已经购买的桌子数量，不包括俱乐部牌桌',
  `lastCrossDayTime` int(11) DEFAULT '0' COMMENT '上次登录时间',
  `totalGameCount` int(11) DEFAULT '0' COMMENT '总共游戏局数',
  `sealFinishTime` int(11) DEFAULT '0' COMMENT '封号时间，-1永久封号，0正常',
  `wechat` varchar(24) DEFAULT '' COMMENT '玩家微信',
  `phonePasswd` varchar(64) DEFAULT '' COMMENT '手机登陆密码',
  `accountInfo` varchar(64) DEFAULT '' COMMENT '微信号信息',
  `totalGameWinCount` int(11) DEFAULT '0' COMMENT '累计赢局数',
  `Lng` varchar(12) DEFAULT '' COMMENT '经度',
  `Lat` varchar(12) DEFAULT '' COMMENT '纬度',
  `address` varchar(64) DEFAULT '' COMMENT '地址',
  `lastCalcOnlineToTime` bigint(20) DEFAULT '0' COMMENT '上次计算在线时长的时间',
  `allOnlineToTime` bigint(20) DEFAULT '0' COMMENT '总共在线时长',
  `IsOnline` tinyint(2) DEFAULT '0' COMMENT '0：不在线，1：在线',
  `motto` varchar(128) DEFAULT '' COMMENT '玩家的个性签名',
  `xianliao` varchar(64) DEFAULT '' COMMENT '闲聊唯一信息',
  `controlParam` int(11) DEFAULT '-1' COMMENT '个人控制权重，部分游戏使用',
  `ModifyInformationCount` int(11) DEFAULT '0' COMMENT 'sd',
  `combineMatchID` bigint(20) DEFAULT '0' COMMENT '比赛场组合',
  `matchStatus` tinyint(4) DEFAULT '0' COMMENT '比赛场状态',
  `curMatchRank` int(11) DEFAULT '0' COMMENT '比赛场排名',
  `matchType` tinyint(2) DEFAULT '0' COMMENT '比赛类型,1:定时赛，2：实时赛',
  PRIMARY KEY (`userID`),
  KEY `isVirtual` (`isVirtual`),
  KEY `roomID` (`roomID`),
  KEY `status` (`status`),
  KEY `registerTime` (`registerTime`),
  KEY `IsOnline` (`IsOnline`),
  KEY `registerIP` (`registerIP`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- ----------------------------
-- Records of userinfo
-- ----------------------------
