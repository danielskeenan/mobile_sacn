import {useContext} from "react";
import AppContext from "../Context";
import {LEVEL_PERCENT_TABLE} from "../constants";

export enum LevelDisplayMode {
    DECIMAL = "decimal",
    HEX = "hex",
    PERCENT = "percent",
}

const percentStringLookupTable = new Map([
    [0, (LEVEL_PERCENT_TABLE.get(0) as number).toString().padStart(2, "0")],
    [1, (LEVEL_PERCENT_TABLE.get(1) as number).toString().padStart(2, "0")],
    [2, (LEVEL_PERCENT_TABLE.get(2) as number).toString().padStart(2, "0")],
    [3, (LEVEL_PERCENT_TABLE.get(3) as number).toString().padStart(2, "0")],
    [4, (LEVEL_PERCENT_TABLE.get(4) as number).toString().padStart(2, "0")],
    [5, (LEVEL_PERCENT_TABLE.get(5) as number).toString().padStart(2, "0")],
    [6, (LEVEL_PERCENT_TABLE.get(6) as number).toString().padStart(2, "0")],
    [7, (LEVEL_PERCENT_TABLE.get(7) as number).toString().padStart(2, "0")],
    [8, (LEVEL_PERCENT_TABLE.get(8) as number).toString().padStart(2, "0")],
    [9, (LEVEL_PERCENT_TABLE.get(9) as number).toString().padStart(2, "0")],
    [10, (LEVEL_PERCENT_TABLE.get(10) as number).toString().padStart(2, "0")],
    [11, (LEVEL_PERCENT_TABLE.get(11) as number).toString().padStart(2, "0")],
    [12, (LEVEL_PERCENT_TABLE.get(12) as number).toString().padStart(2, "0")],
    [13, (LEVEL_PERCENT_TABLE.get(13) as number).toString().padStart(2, "0")],
    [14, (LEVEL_PERCENT_TABLE.get(14) as number).toString().padStart(2, "0")],
    [15, (LEVEL_PERCENT_TABLE.get(15) as number).toString().padStart(2, "0")],
    [16, (LEVEL_PERCENT_TABLE.get(16) as number).toString().padStart(2, "0")],
    [17, (LEVEL_PERCENT_TABLE.get(17) as number).toString().padStart(2, "0")],
    [18, (LEVEL_PERCENT_TABLE.get(18) as number).toString().padStart(2, "0")],
    [19, (LEVEL_PERCENT_TABLE.get(19) as number).toString().padStart(2, "0")],
    [20, (LEVEL_PERCENT_TABLE.get(20) as number).toString().padStart(2, "0")],
    [21, (LEVEL_PERCENT_TABLE.get(21) as number).toString().padStart(2, "0")],
    [22, (LEVEL_PERCENT_TABLE.get(22) as number).toString().padStart(2, "0")],
    [23, (LEVEL_PERCENT_TABLE.get(23) as number).toString().padStart(2, "0")],
    [24, (LEVEL_PERCENT_TABLE.get(24) as number).toString().padStart(2, "0")],
    [25, (LEVEL_PERCENT_TABLE.get(25) as number).toString().padStart(2, "0")],
    [26, (LEVEL_PERCENT_TABLE.get(26) as number).toString().padStart(2, "0")],
    [27, (LEVEL_PERCENT_TABLE.get(27) as number).toString().padStart(2, "0")],
    [28, (LEVEL_PERCENT_TABLE.get(28) as number).toString().padStart(2, "0")],
    [29, (LEVEL_PERCENT_TABLE.get(29) as number).toString().padStart(2, "0")],
    [30, (LEVEL_PERCENT_TABLE.get(30) as number).toString().padStart(2, "0")],
    [31, (LEVEL_PERCENT_TABLE.get(31) as number).toString().padStart(2, "0")],
    [32, (LEVEL_PERCENT_TABLE.get(32) as number).toString().padStart(2, "0")],
    [33, (LEVEL_PERCENT_TABLE.get(33) as number).toString().padStart(2, "0")],
    [34, (LEVEL_PERCENT_TABLE.get(34) as number).toString().padStart(2, "0")],
    [35, (LEVEL_PERCENT_TABLE.get(35) as number).toString().padStart(2, "0")],
    [36, (LEVEL_PERCENT_TABLE.get(36) as number).toString().padStart(2, "0")],
    [37, (LEVEL_PERCENT_TABLE.get(37) as number).toString().padStart(2, "0")],
    [38, (LEVEL_PERCENT_TABLE.get(38) as number).toString().padStart(2, "0")],
    [39, (LEVEL_PERCENT_TABLE.get(39) as number).toString().padStart(2, "0")],
    [40, (LEVEL_PERCENT_TABLE.get(40) as number).toString().padStart(2, "0")],
    [41, (LEVEL_PERCENT_TABLE.get(41) as number).toString().padStart(2, "0")],
    [42, (LEVEL_PERCENT_TABLE.get(42) as number).toString().padStart(2, "0")],
    [43, (LEVEL_PERCENT_TABLE.get(43) as number).toString().padStart(2, "0")],
    [44, (LEVEL_PERCENT_TABLE.get(44) as number).toString().padStart(2, "0")],
    [45, (LEVEL_PERCENT_TABLE.get(45) as number).toString().padStart(2, "0")],
    [46, (LEVEL_PERCENT_TABLE.get(46) as number).toString().padStart(2, "0")],
    [47, (LEVEL_PERCENT_TABLE.get(47) as number).toString().padStart(2, "0")],
    [48, (LEVEL_PERCENT_TABLE.get(48) as number).toString().padStart(2, "0")],
    [49, (LEVEL_PERCENT_TABLE.get(49) as number).toString().padStart(2, "0")],
    [50, (LEVEL_PERCENT_TABLE.get(50) as number).toString().padStart(2, "0")],
    [51, (LEVEL_PERCENT_TABLE.get(51) as number).toString().padStart(2, "0")],
    [52, (LEVEL_PERCENT_TABLE.get(52) as number).toString().padStart(2, "0")],
    [53, (LEVEL_PERCENT_TABLE.get(53) as number).toString().padStart(2, "0")],
    [54, (LEVEL_PERCENT_TABLE.get(54) as number).toString().padStart(2, "0")],
    [55, (LEVEL_PERCENT_TABLE.get(55) as number).toString().padStart(2, "0")],
    [56, (LEVEL_PERCENT_TABLE.get(56) as number).toString().padStart(2, "0")],
    [57, (LEVEL_PERCENT_TABLE.get(57) as number).toString().padStart(2, "0")],
    [58, (LEVEL_PERCENT_TABLE.get(58) as number).toString().padStart(2, "0")],
    [59, (LEVEL_PERCENT_TABLE.get(59) as number).toString().padStart(2, "0")],
    [60, (LEVEL_PERCENT_TABLE.get(60) as number).toString().padStart(2, "0")],
    [61, (LEVEL_PERCENT_TABLE.get(61) as number).toString().padStart(2, "0")],
    [62, (LEVEL_PERCENT_TABLE.get(62) as number).toString().padStart(2, "0")],
    [63, (LEVEL_PERCENT_TABLE.get(63) as number).toString().padStart(2, "0")],
    [64, (LEVEL_PERCENT_TABLE.get(64) as number).toString().padStart(2, "0")],
    [65, (LEVEL_PERCENT_TABLE.get(65) as number).toString().padStart(2, "0")],
    [66, (LEVEL_PERCENT_TABLE.get(66) as number).toString().padStart(2, "0")],
    [67, (LEVEL_PERCENT_TABLE.get(67) as number).toString().padStart(2, "0")],
    [68, (LEVEL_PERCENT_TABLE.get(68) as number).toString().padStart(2, "0")],
    [69, (LEVEL_PERCENT_TABLE.get(69) as number).toString().padStart(2, "0")],
    [70, (LEVEL_PERCENT_TABLE.get(70) as number).toString().padStart(2, "0")],
    [71, (LEVEL_PERCENT_TABLE.get(71) as number).toString().padStart(2, "0")],
    [72, (LEVEL_PERCENT_TABLE.get(72) as number).toString().padStart(2, "0")],
    [73, (LEVEL_PERCENT_TABLE.get(73) as number).toString().padStart(2, "0")],
    [74, (LEVEL_PERCENT_TABLE.get(74) as number).toString().padStart(2, "0")],
    [75, (LEVEL_PERCENT_TABLE.get(75) as number).toString().padStart(2, "0")],
    [76, (LEVEL_PERCENT_TABLE.get(76) as number).toString().padStart(2, "0")],
    [77, (LEVEL_PERCENT_TABLE.get(77) as number).toString().padStart(2, "0")],
    [78, (LEVEL_PERCENT_TABLE.get(78) as number).toString().padStart(2, "0")],
    [79, (LEVEL_PERCENT_TABLE.get(79) as number).toString().padStart(2, "0")],
    [80, (LEVEL_PERCENT_TABLE.get(80) as number).toString().padStart(2, "0")],
    [81, (LEVEL_PERCENT_TABLE.get(81) as number).toString().padStart(2, "0")],
    [82, (LEVEL_PERCENT_TABLE.get(82) as number).toString().padStart(2, "0")],
    [83, (LEVEL_PERCENT_TABLE.get(83) as number).toString().padStart(2, "0")],
    [84, (LEVEL_PERCENT_TABLE.get(84) as number).toString().padStart(2, "0")],
    [85, (LEVEL_PERCENT_TABLE.get(85) as number).toString().padStart(2, "0")],
    [86, (LEVEL_PERCENT_TABLE.get(86) as number).toString().padStart(2, "0")],
    [87, (LEVEL_PERCENT_TABLE.get(87) as number).toString().padStart(2, "0")],
    [88, (LEVEL_PERCENT_TABLE.get(88) as number).toString().padStart(2, "0")],
    [89, (LEVEL_PERCENT_TABLE.get(89) as number).toString().padStart(2, "0")],
    [90, (LEVEL_PERCENT_TABLE.get(90) as number).toString().padStart(2, "0")],
    [91, (LEVEL_PERCENT_TABLE.get(91) as number).toString().padStart(2, "0")],
    [92, (LEVEL_PERCENT_TABLE.get(92) as number).toString().padStart(2, "0")],
    [93, (LEVEL_PERCENT_TABLE.get(93) as number).toString().padStart(2, "0")],
    [94, (LEVEL_PERCENT_TABLE.get(94) as number).toString().padStart(2, "0")],
    [95, (LEVEL_PERCENT_TABLE.get(95) as number).toString().padStart(2, "0")],
    [96, (LEVEL_PERCENT_TABLE.get(96) as number).toString().padStart(2, "0")],
    [97, (LEVEL_PERCENT_TABLE.get(97) as number).toString().padStart(2, "0")],
    [98, (LEVEL_PERCENT_TABLE.get(98) as number).toString().padStart(2, "0")],
    [99, (LEVEL_PERCENT_TABLE.get(99) as number).toString().padStart(2, "0")],
    [100, (LEVEL_PERCENT_TABLE.get(100) as number).toString().padStart(2, "0")],
    [101, (LEVEL_PERCENT_TABLE.get(101) as number).toString().padStart(2, "0")],
    [102, (LEVEL_PERCENT_TABLE.get(102) as number).toString().padStart(2, "0")],
    [103, (LEVEL_PERCENT_TABLE.get(103) as number).toString().padStart(2, "0")],
    [104, (LEVEL_PERCENT_TABLE.get(104) as number).toString().padStart(2, "0")],
    [105, (LEVEL_PERCENT_TABLE.get(105) as number).toString().padStart(2, "0")],
    [106, (LEVEL_PERCENT_TABLE.get(106) as number).toString().padStart(2, "0")],
    [107, (LEVEL_PERCENT_TABLE.get(107) as number).toString().padStart(2, "0")],
    [108, (LEVEL_PERCENT_TABLE.get(108) as number).toString().padStart(2, "0")],
    [109, (LEVEL_PERCENT_TABLE.get(109) as number).toString().padStart(2, "0")],
    [110, (LEVEL_PERCENT_TABLE.get(110) as number).toString().padStart(2, "0")],
    [111, (LEVEL_PERCENT_TABLE.get(111) as number).toString().padStart(2, "0")],
    [112, (LEVEL_PERCENT_TABLE.get(112) as number).toString().padStart(2, "0")],
    [113, (LEVEL_PERCENT_TABLE.get(113) as number).toString().padStart(2, "0")],
    [114, (LEVEL_PERCENT_TABLE.get(114) as number).toString().padStart(2, "0")],
    [115, (LEVEL_PERCENT_TABLE.get(115) as number).toString().padStart(2, "0")],
    [116, (LEVEL_PERCENT_TABLE.get(116) as number).toString().padStart(2, "0")],
    [117, (LEVEL_PERCENT_TABLE.get(117) as number).toString().padStart(2, "0")],
    [118, (LEVEL_PERCENT_TABLE.get(118) as number).toString().padStart(2, "0")],
    [119, (LEVEL_PERCENT_TABLE.get(119) as number).toString().padStart(2, "0")],
    [120, (LEVEL_PERCENT_TABLE.get(120) as number).toString().padStart(2, "0")],
    [121, (LEVEL_PERCENT_TABLE.get(121) as number).toString().padStart(2, "0")],
    [122, (LEVEL_PERCENT_TABLE.get(122) as number).toString().padStart(2, "0")],
    [123, (LEVEL_PERCENT_TABLE.get(123) as number).toString().padStart(2, "0")],
    [124, (LEVEL_PERCENT_TABLE.get(124) as number).toString().padStart(2, "0")],
    [125, (LEVEL_PERCENT_TABLE.get(125) as number).toString().padStart(2, "0")],
    [126, (LEVEL_PERCENT_TABLE.get(126) as number).toString().padStart(2, "0")],
    [127, (LEVEL_PERCENT_TABLE.get(127) as number).toString().padStart(2, "0")],
    [128, (LEVEL_PERCENT_TABLE.get(128) as number).toString().padStart(2, "0")],
    [129, (LEVEL_PERCENT_TABLE.get(129) as number).toString().padStart(2, "0")],
    [130, (LEVEL_PERCENT_TABLE.get(130) as number).toString().padStart(2, "0")],
    [131, (LEVEL_PERCENT_TABLE.get(131) as number).toString().padStart(2, "0")],
    [132, (LEVEL_PERCENT_TABLE.get(132) as number).toString().padStart(2, "0")],
    [133, (LEVEL_PERCENT_TABLE.get(133) as number).toString().padStart(2, "0")],
    [134, (LEVEL_PERCENT_TABLE.get(134) as number).toString().padStart(2, "0")],
    [135, (LEVEL_PERCENT_TABLE.get(135) as number).toString().padStart(2, "0")],
    [136, (LEVEL_PERCENT_TABLE.get(136) as number).toString().padStart(2, "0")],
    [137, (LEVEL_PERCENT_TABLE.get(137) as number).toString().padStart(2, "0")],
    [138, (LEVEL_PERCENT_TABLE.get(138) as number).toString().padStart(2, "0")],
    [139, (LEVEL_PERCENT_TABLE.get(139) as number).toString().padStart(2, "0")],
    [140, (LEVEL_PERCENT_TABLE.get(140) as number).toString().padStart(2, "0")],
    [141, (LEVEL_PERCENT_TABLE.get(141) as number).toString().padStart(2, "0")],
    [142, (LEVEL_PERCENT_TABLE.get(142) as number).toString().padStart(2, "0")],
    [143, (LEVEL_PERCENT_TABLE.get(143) as number).toString().padStart(2, "0")],
    [144, (LEVEL_PERCENT_TABLE.get(144) as number).toString().padStart(2, "0")],
    [145, (LEVEL_PERCENT_TABLE.get(145) as number).toString().padStart(2, "0")],
    [146, (LEVEL_PERCENT_TABLE.get(146) as number).toString().padStart(2, "0")],
    [147, (LEVEL_PERCENT_TABLE.get(147) as number).toString().padStart(2, "0")],
    [148, (LEVEL_PERCENT_TABLE.get(148) as number).toString().padStart(2, "0")],
    [149, (LEVEL_PERCENT_TABLE.get(149) as number).toString().padStart(2, "0")],
    [150, (LEVEL_PERCENT_TABLE.get(150) as number).toString().padStart(2, "0")],
    [151, (LEVEL_PERCENT_TABLE.get(151) as number).toString().padStart(2, "0")],
    [152, (LEVEL_PERCENT_TABLE.get(152) as number).toString().padStart(2, "0")],
    [153, (LEVEL_PERCENT_TABLE.get(153) as number).toString().padStart(2, "0")],
    [154, (LEVEL_PERCENT_TABLE.get(154) as number).toString().padStart(2, "0")],
    [155, (LEVEL_PERCENT_TABLE.get(155) as number).toString().padStart(2, "0")],
    [156, (LEVEL_PERCENT_TABLE.get(156) as number).toString().padStart(2, "0")],
    [157, (LEVEL_PERCENT_TABLE.get(157) as number).toString().padStart(2, "0")],
    [158, (LEVEL_PERCENT_TABLE.get(158) as number).toString().padStart(2, "0")],
    [159, (LEVEL_PERCENT_TABLE.get(159) as number).toString().padStart(2, "0")],
    [160, (LEVEL_PERCENT_TABLE.get(160) as number).toString().padStart(2, "0")],
    [161, (LEVEL_PERCENT_TABLE.get(161) as number).toString().padStart(2, "0")],
    [162, (LEVEL_PERCENT_TABLE.get(162) as number).toString().padStart(2, "0")],
    [163, (LEVEL_PERCENT_TABLE.get(163) as number).toString().padStart(2, "0")],
    [164, (LEVEL_PERCENT_TABLE.get(164) as number).toString().padStart(2, "0")],
    [165, (LEVEL_PERCENT_TABLE.get(165) as number).toString().padStart(2, "0")],
    [166, (LEVEL_PERCENT_TABLE.get(166) as number).toString().padStart(2, "0")],
    [167, (LEVEL_PERCENT_TABLE.get(167) as number).toString().padStart(2, "0")],
    [168, (LEVEL_PERCENT_TABLE.get(168) as number).toString().padStart(2, "0")],
    [169, (LEVEL_PERCENT_TABLE.get(169) as number).toString().padStart(2, "0")],
    [170, (LEVEL_PERCENT_TABLE.get(170) as number).toString().padStart(2, "0")],
    [171, (LEVEL_PERCENT_TABLE.get(171) as number).toString().padStart(2, "0")],
    [172, (LEVEL_PERCENT_TABLE.get(172) as number).toString().padStart(2, "0")],
    [173, (LEVEL_PERCENT_TABLE.get(173) as number).toString().padStart(2, "0")],
    [174, (LEVEL_PERCENT_TABLE.get(174) as number).toString().padStart(2, "0")],
    [175, (LEVEL_PERCENT_TABLE.get(175) as number).toString().padStart(2, "0")],
    [176, (LEVEL_PERCENT_TABLE.get(176) as number).toString().padStart(2, "0")],
    [177, (LEVEL_PERCENT_TABLE.get(177) as number).toString().padStart(2, "0")],
    [178, (LEVEL_PERCENT_TABLE.get(178) as number).toString().padStart(2, "0")],
    [179, (LEVEL_PERCENT_TABLE.get(179) as number).toString().padStart(2, "0")],
    [180, (LEVEL_PERCENT_TABLE.get(180) as number).toString().padStart(2, "0")],
    [181, (LEVEL_PERCENT_TABLE.get(181) as number).toString().padStart(2, "0")],
    [182, (LEVEL_PERCENT_TABLE.get(182) as number).toString().padStart(2, "0")],
    [183, (LEVEL_PERCENT_TABLE.get(183) as number).toString().padStart(2, "0")],
    [184, (LEVEL_PERCENT_TABLE.get(184) as number).toString().padStart(2, "0")],
    [185, (LEVEL_PERCENT_TABLE.get(185) as number).toString().padStart(2, "0")],
    [186, (LEVEL_PERCENT_TABLE.get(186) as number).toString().padStart(2, "0")],
    [187, (LEVEL_PERCENT_TABLE.get(187) as number).toString().padStart(2, "0")],
    [188, (LEVEL_PERCENT_TABLE.get(188) as number).toString().padStart(2, "0")],
    [189, (LEVEL_PERCENT_TABLE.get(189) as number).toString().padStart(2, "0")],
    [190, (LEVEL_PERCENT_TABLE.get(190) as number).toString().padStart(2, "0")],
    [191, (LEVEL_PERCENT_TABLE.get(191) as number).toString().padStart(2, "0")],
    [192, (LEVEL_PERCENT_TABLE.get(192) as number).toString().padStart(2, "0")],
    [193, (LEVEL_PERCENT_TABLE.get(193) as number).toString().padStart(2, "0")],
    [194, (LEVEL_PERCENT_TABLE.get(194) as number).toString().padStart(2, "0")],
    [195, (LEVEL_PERCENT_TABLE.get(195) as number).toString().padStart(2, "0")],
    [196, (LEVEL_PERCENT_TABLE.get(196) as number).toString().padStart(2, "0")],
    [197, (LEVEL_PERCENT_TABLE.get(197) as number).toString().padStart(2, "0")],
    [198, (LEVEL_PERCENT_TABLE.get(198) as number).toString().padStart(2, "0")],
    [199, (LEVEL_PERCENT_TABLE.get(199) as number).toString().padStart(2, "0")],
    [200, (LEVEL_PERCENT_TABLE.get(200) as number).toString().padStart(2, "0")],
    [201, (LEVEL_PERCENT_TABLE.get(201) as number).toString().padStart(2, "0")],
    [202, (LEVEL_PERCENT_TABLE.get(202) as number).toString().padStart(2, "0")],
    [203, (LEVEL_PERCENT_TABLE.get(203) as number).toString().padStart(2, "0")],
    [204, (LEVEL_PERCENT_TABLE.get(204) as number).toString().padStart(2, "0")],
    [205, (LEVEL_PERCENT_TABLE.get(205) as number).toString().padStart(2, "0")],
    [206, (LEVEL_PERCENT_TABLE.get(206) as number).toString().padStart(2, "0")],
    [207, (LEVEL_PERCENT_TABLE.get(207) as number).toString().padStart(2, "0")],
    [208, (LEVEL_PERCENT_TABLE.get(208) as number).toString().padStart(2, "0")],
    [209, (LEVEL_PERCENT_TABLE.get(209) as number).toString().padStart(2, "0")],
    [210, (LEVEL_PERCENT_TABLE.get(210) as number).toString().padStart(2, "0")],
    [211, (LEVEL_PERCENT_TABLE.get(211) as number).toString().padStart(2, "0")],
    [212, (LEVEL_PERCENT_TABLE.get(212) as number).toString().padStart(2, "0")],
    [213, (LEVEL_PERCENT_TABLE.get(213) as number).toString().padStart(2, "0")],
    [214, (LEVEL_PERCENT_TABLE.get(214) as number).toString().padStart(2, "0")],
    [215, (LEVEL_PERCENT_TABLE.get(215) as number).toString().padStart(2, "0")],
    [216, (LEVEL_PERCENT_TABLE.get(216) as number).toString().padStart(2, "0")],
    [217, (LEVEL_PERCENT_TABLE.get(217) as number).toString().padStart(2, "0")],
    [218, (LEVEL_PERCENT_TABLE.get(218) as number).toString().padStart(2, "0")],
    [219, (LEVEL_PERCENT_TABLE.get(219) as number).toString().padStart(2, "0")],
    [220, (LEVEL_PERCENT_TABLE.get(220) as number).toString().padStart(2, "0")],
    [221, (LEVEL_PERCENT_TABLE.get(221) as number).toString().padStart(2, "0")],
    [222, (LEVEL_PERCENT_TABLE.get(222) as number).toString().padStart(2, "0")],
    [223, (LEVEL_PERCENT_TABLE.get(223) as number).toString().padStart(2, "0")],
    [224, (LEVEL_PERCENT_TABLE.get(224) as number).toString().padStart(2, "0")],
    [225, (LEVEL_PERCENT_TABLE.get(225) as number).toString().padStart(2, "0")],
    [226, (LEVEL_PERCENT_TABLE.get(226) as number).toString().padStart(2, "0")],
    [227, (LEVEL_PERCENT_TABLE.get(227) as number).toString().padStart(2, "0")],
    [228, (LEVEL_PERCENT_TABLE.get(228) as number).toString().padStart(2, "0")],
    [229, (LEVEL_PERCENT_TABLE.get(229) as number).toString().padStart(2, "0")],
    [230, (LEVEL_PERCENT_TABLE.get(230) as number).toString().padStart(2, "0")],
    [231, (LEVEL_PERCENT_TABLE.get(231) as number).toString().padStart(2, "0")],
    [232, (LEVEL_PERCENT_TABLE.get(232) as number).toString().padStart(2, "0")],
    [233, (LEVEL_PERCENT_TABLE.get(233) as number).toString().padStart(2, "0")],
    [234, (LEVEL_PERCENT_TABLE.get(234) as number).toString().padStart(2, "0")],
    [235, (LEVEL_PERCENT_TABLE.get(235) as number).toString().padStart(2, "0")],
    [236, (LEVEL_PERCENT_TABLE.get(236) as number).toString().padStart(2, "0")],
    [237, (LEVEL_PERCENT_TABLE.get(237) as number).toString().padStart(2, "0")],
    [238, (LEVEL_PERCENT_TABLE.get(238) as number).toString().padStart(2, "0")],
    [239, (LEVEL_PERCENT_TABLE.get(239) as number).toString().padStart(2, "0")],
    [240, (LEVEL_PERCENT_TABLE.get(240) as number).toString().padStart(2, "0")],
    [241, (LEVEL_PERCENT_TABLE.get(241) as number).toString().padStart(2, "0")],
    [242, (LEVEL_PERCENT_TABLE.get(242) as number).toString().padStart(2, "0")],
    [243, (LEVEL_PERCENT_TABLE.get(243) as number).toString().padStart(2, "0")],
    [244, (LEVEL_PERCENT_TABLE.get(244) as number).toString().padStart(2, "0")],
    [245, (LEVEL_PERCENT_TABLE.get(245) as number).toString().padStart(2, "0")],
    [246, (LEVEL_PERCENT_TABLE.get(246) as number).toString().padStart(2, "0")],
    [247, (LEVEL_PERCENT_TABLE.get(247) as number).toString().padStart(2, "0")],
    [248, (LEVEL_PERCENT_TABLE.get(248) as number).toString().padStart(2, "0")],
    [249, (LEVEL_PERCENT_TABLE.get(249) as number).toString().padStart(2, "0")],
    [250, (LEVEL_PERCENT_TABLE.get(250) as number).toString().padStart(2, "0")],
    [251, (LEVEL_PERCENT_TABLE.get(251) as number).toString().padStart(2, "0")],
    [252, (LEVEL_PERCENT_TABLE.get(252) as number).toString().padStart(2, "0")],
    [253, (LEVEL_PERCENT_TABLE.get(253) as number).toString().padStart(2, "0")],
    [254, (LEVEL_PERCENT_TABLE.get(254) as number).toString().padStart(2, "0")],
    [255, "FF"],
]);

interface LevelDisplayProps {
    level: number;
    displayMode?: LevelDisplayMode;
}

export default function LevelDisplay(props: LevelDisplayProps) {
    const {level, displayMode} = {
        ...props, ...{
            displayMode: useContext(AppContext).levelDisplayMode,
        },
    };

    return (
        <span className="msacn-level">{levelDisplayString(level, displayMode)}</span>
    );
}

export function levelDisplayString(level: number, displayMode?: LevelDisplayMode) {
    if (level === undefined) {
        return "";
    }
    switch (displayMode) {
        case LevelDisplayMode.DECIMAL:
            return level.toString(10).padStart(3, "0");
        case LevelDisplayMode.HEX:
            return level.toString(16).toUpperCase().padStart(2, "0");
        case LevelDisplayMode.PERCENT:
            const percent = percentStringLookupTable.get(level) ?? "??";
            return `${percent}%`;
    }
    return "??";
}
