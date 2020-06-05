/*
Navicat MySQL Data Transfer

Source Server         : 192.168.198.171
Source Server Version : 80020
Source Host           : 192.168.198.171:3306
Source Database       : newframe

Target Server Type    : MYSQL
Target Server Version : 80020
File Encoding         : 65001

Date: 2020-06-05 17:13:36
*/

SET FOREIGN_KEY_CHECKS=0;

-- ----------------------------
-- Table structure for logonbaseinfo
-- ----------------------------
DROP TABLE IF EXISTS `logonbaseinfo`;
CREATE TABLE `logonbaseinfo` (
  `logonID` int NOT NULL DEFAULT '0' COMMENT '网关服主id',
  `ip` varchar(24) CHARACTER SET utf8 COLLATE utf8_general_ci DEFAULT '' COMMENT '网关服ip地址',
  `intranetIP` varchar(24) CHARACTER SET utf8 COLLATE utf8_general_ci DEFAULT '' COMMENT '内网ip',
  `port` int DEFAULT '3015' COMMENT '网关服端口',
  `webSocketPort` int DEFAULT '3017' COMMENT 'websocket端口',
  `maxPeople` int DEFAULT '3000' COMMENT '网关服最多容纳的最大人数',
  `maxWebSocketPeople` int DEFAULT '2000' COMMENT '网关服websocket最多容纳的最大人数',
  `status` tinyint DEFAULT '0' COMMENT '0：关闭状态，1：开启状态',
  `curPeople` int DEFAULT '0' COMMENT '当前人数',
  `curWebSocketPeople` int DEFAULT '0' COMMENT '当前人数',
  `socketCount` int DEFAULT '0' COMMENT 'socket数量',
  `webSocketCount` int DEFAULT '0' COMMENT 'socket数量',
  PRIMARY KEY (`logonID`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- ----------------------------
-- Records of logonbaseinfo
-- ----------------------------
INSERT INTO `logonbaseinfo` VALUES ('1', '192.168.198.171', '127.0.0.1', '3016', '3017', '3000', '2000', '0', '0', '0', '107', '0');
