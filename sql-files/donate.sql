
CREATE TABLE IF NOT EXISTS `donate` (
  `id` int(6) unsigned NOT NULL AUTO_INCREMENT,
  `userid` varchar(23) NOT NULL,
  `account_id` int(11) NOT NULL,
  `amount` int(4) unsigned NOT NULL,
  `status` tinyint(1) unsigned NOT NULL,
  `added_time` datetime NOT NULL,
  `claim_time` datetime NOT NULL,
  `referenceNo` varchar(15) NOT NULL,
  `gbpReferenceNo` varchar(250) NOT NULL,
  PRIMARY KEY (`id`),
  KEY `account_id` (`account_id`)
) ENGINE=MyISAM AUTO_INCREMENT=1 DEFAULT CHARSET=utf8mb3;

