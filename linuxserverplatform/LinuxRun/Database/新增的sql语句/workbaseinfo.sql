/*
Navicat MySQL Data Transfer

Source Server         : 192.168.198.171
Source Server Version : 50645
Source Host           : 192.168.198.171:3306
Source Database       : newframe

Target Server Type    : MYSQL
Target Server Version : 50645
File Encoding         : 65001

Date: 2020-05-29 18:21:53
*/

SET FOREIGN_KEY_CHECKS=0;

-- ----------------------------
-- Table structure for workbaseinfo
-- ----------------------------
DROP TABLE IF EXISTS `workbaseinfo`;
CREATE TABLE `workbaseinfo` (
  `workID` int(6) NOT NULL DEFAULT '0' COMMENT '逻辑服主id',
  `status` tinyint(2) DEFAULT '0' COMMENT '0：关闭状态，1：开启状态',
  `gateconnected` tinyint(2) DEFAULT '1' COMMENT '是否连接网关',
  PRIMARY KEY (`workID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Records of workbaseinfo
-- ----------------------------
INSERT INTO `workbaseinfo` VALUES ('1', '0', '1');
