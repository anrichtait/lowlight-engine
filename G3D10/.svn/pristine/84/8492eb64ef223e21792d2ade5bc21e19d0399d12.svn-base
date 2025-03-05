/**
  \file G3D-base.lib/source/ParseSchematic.cpp

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/
#include "G3D-base/ParseSchematic.h"
#include "G3D-base/ParseError.h"
#include "G3D-base/BinaryInput.h"
#include "G3D-base/FileSystem.h"
#include "../../external/zlib.lib/include/zlib.h"
#include "../../external/zlib.lib/source/gzguts.h"


namespace G3D {

static ParseSchematic::SchematicPalette schematicPalette[256] = {
    // name                               read_color ralpha   txX,Y, emissive
    { /*   0 */ "Air",                    0x000000, 0.000f,   13,14, false},
    { /*   1 */ "Stone",                  0x7C7C7C, 1.000f,    1, 0, false},
    { /*   2 */ "Grass Block", /*output!*/0x8cbd57, 1.000f,    0, 0, false},
    { /*   3 */ "Dirt",                   0x8c6344, 1.000f,    2, 0, false},
    { /*   4 */ "Cobblestone",            0x828282, 1.000f,    0, 1, false},
    { /*   5 */ "Wood Planks",            0x9C8149, 1.000f,    4, 0, false},
    { /*   6 */ "Sapling",                0x7b9a29, 1.000f,   15, 0, false},
    { /*   7 */ "Bedrock",                0x565656, 1.000f,    1, 1, false},
    { /*   8 */ "Water",                  0x295dfe, 0.535f,   15,13, false},
    { /*   9 */ "Stationary Water",       0x295dfe, 0.535f,   15,13, false},
    { /*  10 */ "Lava",                   0xf56d00, 1.000f,   15,15, true},
    { /*  11 */ "Stationary Lava",        0xf56d00, 1.000f,   15,15, true},
    { /*  12 */ "Sand",                   0xDCD0A6, 1.000f,    2, 1, false},
    { /*  13 */ "Gravel",                 0x857b7b, 1.000f,    3, 1, false},
    { /*  14 */ "Gold Ore",               0xfcee4b, 1.000f,    0, 2, false},
    { /*  15 */ "Iron Ore",               0xbc9980, 1.000f,    1, 2, false},
    { /*  16 */ "Coal Ore",               0x343434, 1.000f,    2, 2, false},
    { /*  17 */ "Wood",                   0x695333, 1.000f,    5, 1, false},
    { /*  18 */ "Leaves",      /*output!*/0x6fac2c, 1.000f,    4, 3, false},
    { /*  19 */ "Sponge",                 0xD1D24E, 1.000f,    0, 3, false},
    { /*  20 */ "Glass",                  0xc0f6fe, 0.500f,    1, 3, false},
    { /*  21 */ "Lapis Lazuli Ore",       0x143880, 1.000f,    0,10, false},
    { /*  22 */ "Lapis Lazuli Block",     0x1b4ebb, 1.000f,    0, 9, false},
    { /*  23 */ "Dispenser",              0x6f6f6f, 1.000f,   14, 3, false},
    { /*  24 */ "Sandstone",              0xe0d8a6, 1.000f,    0,11, false},
    { /*  25 */ "Note Block",             0x342017, 1.000f,   10, 4, false},
    { /*  26 */ "Bed",                    0xff3333, 1.000f,    6, 8, false},
    { /*  27 */ "Powered Rail",           0xAB0301, 1.000f,    3,11, false},
    { /*  28 */ "Detector Rail",          0xCD5E58, 1.000f,    3,12, false},
    { /*  29 */ "Sticky Piston",          0x719e60, 1.000f,   12, 6, false},
    { /*  30 */ "Cobweb",                 0xeeeeee, 1.000f,   11, 0, false},
    { /*  31 */ "Grass",       /*output!*/0x8cbd57, 1.000f,    7, 2, false},
    { /*  32 */ "Dead Bush",              0x946428, 1.000f,    7, 3, false},
    { /*  33 */ "Piston",                 0x95774b, 1.000f,   12, 6, false},
    { /*  34 */ "Piston Head",            0x95774b, 1.000f,   11, 6, false},
    { /*  35 */ "Wool",                   0xEEEEEE, 1.000f,    0, 4, false},
    { /*  36 */ "Block moved By Piston",  0x000000, 1.000f,   11, 6, false},
    { /*  37 */ "Dandelion",              0xD3DD05, 1.000f,   13, 0, false},
    { /*  38 */ "Poppy",                  0xCE1A05, 1.000f,   12, 0, false},
    { /*  39 */ "Brown Mushroom",         0xc19171, 1.000f,   13, 1, false},
    { /*  40 */ "Red Mushroom",           0xfc5c5d, 1.000f,   12, 1, false},
    { /*  41 */ "Block of Gold",          0xfef74e, 1.000f,    7, 1, false},
    { /*  42 */ "Block of Iron",          0xeeeeee, 1.000f,    6, 1, false},
    { /*  43 */ "Double Stone Slab",      0xa6a6a6, 1.000f,    6, 0, false},
    { /*  44 */ "Stone Slab",             0xa5a5a5, 1.000f,    6, 0, false},
    { /*  45 */ "Bricks",                 0x985542, 1.000f,    7, 0, false},
    { /*  46 */ "TNT",                    0xdb441a, 1.000f,    9, 0, false},
    { /*  47 */ "Bookshelf",              0x795a39, 1.000f,    4, 0, false},
    { /*  48 */ "Moss Stone",             0x627162, 1.000f,    4, 2, false},
    { /*  49 */ "Obsidian",               0x1b1729, 1.000f,    5, 2, false},
    { /*  50 */ "Torch",                  0xfcfc00, 1.000f,    0, 5, true},
    { /*  51 */ "Fire",                   0xfca100, 1.000f,   15, 1, true},
    { /*  52 */ "Monster Spawner",        0x254254, 1.000f,    1, 4, false},
    { /*  53 */ "Oak Wood Stairs",        0x9e804f, 1.000f,    4, 0, false},
    { /*  54 */ "Chest",                  0xa06f23, 1.000f,    9, 1, false},
    { /*  55 */ "Redstone Wire",          0xd60000, 1.000f,    4,10, false},
    { /*  56 */ "Diamond Ore",            0x5decf5, 1.000f,    2, 3, false},
    { /*  57 */ "Block of Diamond",       0x7fe3df, 1.000f,    8, 1, false},
    { /*  58 */ "Crafting Table",         0x825432, 1.000f,   11, 2, false},
    { /*  59 */ "Wheat",                  0x766615, 1.000f,   15, 5, false},
    { /*  60 */ "Farmland",               0x40220b, 1.000f,    6, 5, false},
    { /*  61 */ "Furnace",                0x767677, 1.000f,   14, 3, false},
    { /*  62 */ "Burning Furnace",        0x777676, 1.000f,   14, 3, true},
    { /*  63 */ "Standing Sign",          0x9f814f, 1.000f,    4, 0, false},
    { /*  64 */ "Oak Door",               0x7e5d2d, 1.000f,    1, 5, false},
    { /*  65 */ "Ladder",                 0xaa8651, 1.000f,    3, 5, false},
    { /*  66 */ "Rail",                   0x686868, 1.000f,    0, 8, false},
    { /*  67 */ "Cobblestone Stairs",     0x818181, 1.000f,    0, 1, false},
    { /*  68 */ "Wall Sign",              0xa68a46, 1.000f,    4, 0, false},
    { /*  69 */ "Lever",                  0x8a6a3d, 1.000f,    0, 6, false},
    { /*  70 */ "Stone Pressure Plate",   0xa4a4a4, 1.000f,    1, 0, false},
    { /*  71 */ "Iron Door",              0xb2b2b2, 1.000f,    2, 5, false},
    { /*  72 */ "Wooden Pressure Plate",  0x9d7f4e, 1.000f,    4, 0, false},
    { /*  73 */ "Redstone Ore",           0x8f0303, 1.000f,    3, 3, false},
    { /*  74 */ "Glowing Redstone Ore",   0x900303, 1.000f,    3, 3, true},
    { /*  75 */ "Redstone Torch (inactive)",0x560000, 1.000f,  3, 7, false},
    { /*  76 */ "Redstone Torch (active)",  0xfd0000, 1.000f,  3, 6, true},
    { /*  77 */ "Stone Button",           0xacacac, 1.000f,    1, 0, false},
    { /*  78 */ "Snow (layer)",           0xf0fafa, 1.000f,    2, 4, false},
    { /*  79 */ "Ice",                    0x7dacfe, 0.613f,    3, 4, false},
    { /*  80 */ "Snow",                   0xf1fafa, 1.000f,    2, 4, false},
    { /*  81 */ "Cactus",                 0x0D6118, 1.000f,    5, 4, false},
    { /*  82 */ "Clay",                   0xa2a7b4, 1.000f,    8, 4, false},
    { /*  83 */ "Sugar Cane",             0x72944e, 1.000f,    9, 4, false},
    { /*  84 */ "Jukebox",                0x8a5a40, 1.000f,   11, 4, false},
    { /*  85 */ "Fence",                  0x9f814e, 1.000f,    4, 0, false},
    { /*  86 */ "Pumpkin",                0xc07615, 1.000f,    6, 6, false},
    { /*  87 */ "Netherrack",             0x723a38, 1.000f,    7, 6, false},
    { /*  88 */ "Soul Sand",              0x554134, 1.000f,    8, 6, false},
    { /*  89 */ "Glowstone",              0xf9d49c, 1.000f,    9, 6, true},
    { /*  90 */ "Nether Portal",          0x472272, 0.800f,   14, 0, true},
    { /*  91 */ "Jack o'Lantern",         0xe9b416, 1.000f,    6, 6, true},
    { /*  92 */ "Cake",                   0xfffdfd, 1.000f,    9, 7, false},
    { /*  93 */ "Redstone Repeater (off)",0x560000, 1.0345f,   3, 8, false},
    { /*  94 */ "Redstone Repeater (on)", 0xee5555, 1.0345f,   3, 9, false},
    { /*  95 */ "Stained Glass",          0xEFEFEF, 0.500f,    0,20, false},
    { /*  96 */ "Trapdoor",               0x886634, 1.000f,    4, 5, false},
    { /*  97 */ "Monster Egg",            0x787878, 1.000f,    1, 0, false},
    { /*  98 */ "Stone Bricks",           0x797979, 1.000f,    6, 3, false},
    { /*  99 */ "Brown Mushroom (block)", 0x654b39, 1.000f,   14, 7, false},
    { /* 100 */ "Red Mushroom (block)",   0xa91b19, 1.000f,   13, 7, false},
    { /* 101 */ "Iron Bars",              0xa3a4a4, 1.000f,    5, 5, false},
    { /* 102 */ "Glass Pane",             0xc0f6fe, 0.500f,    1, 3, false},
    { /* 103 */ "Melon",                  0xaead27, 1.000f,    9, 8, false},
    { /* 104 */ "Pumpkin Stem",           0xE1C71C, 1.000f,   14,11, false},
    { /* 105 */ "Melon Stem",             0xE1C71C, 1.000f,   15, 6, false},
    { /* 106 */ "Vines",                  0x76AB2F, 1.000f,   15, 8, false},
    { /* 107 */ "Fence Gate",             0xa88754, 1.000f,    0, 0, false},
    { /* 108 */ "Brick Stairs",           0xa0807b, 1.000f,    7, 0, false},
    { /* 109 */ "Stone Brick Stairs",     0x797979, 1.000f,    6, 3, false},
    { /* 110 */ "Mycelium",               0x685d69, 1.000f,   14, 4, false},
    { /* 111 */ "Lily Pad",               0x217F30, 1.000f,   12, 4, false},
    { /* 112 */ "Nether Brick",           0x32171c, 1.000f,    0,14, false},
    { /* 113 */ "Nether Brick Fence",     0x241316, 1.000f,    0,14, false},
    { /* 114 */ "Nether Brick Stairs",    0x32171c, 1.000f,    0,14, false},
    { /* 115 */ "Nether Wart",            0x81080a, 1.000f,    4,14, false},
    { /* 116 */ "Enchantment Table",      0xa6701a, 1.000f,    6,10, false},
    { /* 117 */ "Brewing Stand",          0x77692e, 1.000f,     0,0, false},
    { /* 118 */ "Cauldron",               0x3b3b3b, 1.000f,   10, 8, false},
    { /* 119 */ "End Portal",             0x0c0b0b, 0.7f,      8,11, true},
    { /* 120 */ "End Portal Frame",       0x3e6c60, 1.000f,   14, 9, false},
    { /* 121 */ "End Stone",              0xdadca6, 1.000f,   15,10, false},
    { /* 122 */ "Dragon Egg",             0x1b1729, 1.000f,    7,10, false},
    { /* 123 */ "Redstone Lamp (inactive)",0x9F6D4D, 1.000f,   3,13, false},
    { /* 124 */ "Redstone Lamp (active)", 0xf9d49c, 1.000f,    4,13, true},
    { /* 125 */ "Double Wooden Slab",     0x9f8150, 1.000f,    4, 0, false},
    { /* 126 */ "Wooden Slab",            0x9f8150, 1.000f,    4, 0, false},
    { /* 127 */ "Cocoa"      ,            0xBE742D, 1.000f,    8,10, false},
    { /* 128 */ "Sandstone Stairs",       0xe0d8a6, 1.000f,    0,11, false},
    { /* 129 */ "Emerald Ore",            0x900303, 1.000f,   11,10, false},
    { /* 130 */ "Ender Chest",            0x293A3C, 1.000f,   10,13, false},
    { /* 131 */ "Tripwire Hook",          0xC79F63, 1.000f,   12,10, false},
    { /* 132 */ "Tripwire",               0x000000, 1.000f,   13,10, false},
    { /* 133 */ "Block of Emerald",       0x53D778, 1.000f,   12,14, false},
    { /* 134 */ "Spruce Wood Stairs",     0x785836, 1.000f,    6,12, false},
    { /* 135 */ "Birch Wood Stairs",      0xD7C185, 1.000f,    6,13, false},
    { /* 136 */ "Jungle Wood Stairs",     0xB1805C, 1.000f,    7,12, false},
    { /* 137 */ "Command Block",          0xB28B79, 1.000f,   10,24, false},
    { /* 138 */ "Beacon",                 0x9CF2ED, 0.800f,   11,14, true},
    { /* 139 */ "Cobblestone Wall",       0x828282, 1.000f,    0, 1, false},
    { /* 140 */ "Flower Pot",             0x7C4536, 1.000f,   10,11, false},
    { /* 141 */ "Carrot",                 0x056B05, 1.000f,   11,12, false},
    { /* 142 */ "Potato",                 0x00C01B, 1.000f,   15,12, false},
    { /* 143 */ "Wooden Button",          0x9f8150, 1.000f,    4, 0, false},
    { /* 144 */ "Mob Head",               0xcacaca, 1.000f,    6, 6, false},
    { /* 145 */ "Anvil",                  0x404040, 1.000f,    7,13, false},
    { /* 146 */ "Trapped Chest",          0xa06f23, 1.000f,    9, 1, false},
    { /* 147 */ "Weighted Plate (Light)", 0xEFE140, 1.000f,    7, 1, false},
    { /* 148 */ "Weighted Plate (Heavy)", 0xD7D7D7, 1.000f,    6, 1, false},
    { /* 149 */ "Redstone Comparator",    0xC5BAAD, 1.000f,   14,14, false},
    { /* 150 */ "Redstone Comparator (old)",0xD1B5AA,1.000f,  15,14, false},
    { /* 151 */ "Daylight Sensor",        0xBBA890, 1.000f,    6,15, false},
    { /* 152 */ "Block of Redstone",      0xA81E09, 1.000f,   14,15, false},
    { /* 153 */ "Nether Quartz Ore",      0x7A5B57, 1.000f,    8,17, false},
    { /* 154 */ "Hopper",                 0x363636, 1.000f,   13,15, false},
    { /* 155 */ "Block of Quartz",        0xE0DDD7, 1.000f,    7,17, false},
    { /* 156 */ "Quartz Stairs",          0xE1DCD1, 1.000f,    7,17, false},
    { /* 157 */ "Activator Rail",         0x880300, 1.000f,   10,17, false},
    { /* 158 */ "Dropper",                0x6E6E6E, 1.000f,   14, 3, false},
    { /* 159 */ "Stained Clay",           0xCEAE9E, 1.000f,    0,16, false},
    { /* 160 */ "Stained Glass Pane",     0xEFEFEF, 0.500f,    0,20, false},
    { /* 161 */ "Leaves (Acacia/Dark Oak)",0x6fac2c,1.000f,   11,19, false},
    { /* 162 */ "Wood (Acacia/Dark Oak)", 0x766F64, 1.000f,   13,19, false},
    { /* 163 */ "Acacia Wood Stairs",     0xBA683B, 1.000f,    0,22, false},
    { /* 164 */ "Dark Oak Wood Stairs",   0x492F17, 1.000f,    1,22, false},
    { /* 165 */ "Slime Block",            0x787878, 0.500f,    3,22, false},
    { /* 166 */ "Barrier",                0xE30000, 0.000f,   14,25, false},
    { /* 167 */ "Iron Trapdoor",          0xC0C0C0, 1.000f,    2,22, false},
    { /* 168 */ "Prismarine",             0x5A9B95, 1.000f,   12,22, false},
    { /* 169 */ "Sea Lantern",            0xD3DBD3, 1.000f,   14,22, true},
    { /* 170 */ "Hay Bale",               0xB5970C, 1.000f,   10,15, false},
    { /* 171 */ "Carpet",                 0xEEEEEE, 1.000f,    0, 4, false},
    { /* 172 */ "Hardened Clay",          0x945A41, 1.000f,    0,17, false},
    { /* 173 */ "Block of Coal",          0x191919, 1.000f,   13,14, false},
    { /* 174 */ "Packed Ice",             0x7dacfe, 1.000f,   12,17, false},
    { /* 175 */ "Large Flowers",          0x8cbd57, 1.000f,    0,18, false},
    { /* 176 */ "Standing Banner",        0x9C9C9C, 1.000f,   10,23, false},
    { /* 177 */ "Wall Banner",            0x9C9C9C, 1.000f,   10,23, false},
    { /* 178 */ "Inverted Daylight Sensor",0xBBA890,1.000f,   13,22, false},
    { /* 179 */ "Red Sandstone",          0x964C19, 1.000f,   12,19, false},
    { /* 180 */ "Red Sandstone Stairs",   0x964C19, 1.000f,   12,19, false},
    { /* 181 */ "Double Red Sandstone Slab",0x964C19,1.000f,  12,19, false},
    { /* 182 */ "Red Sandstone Slab",     0x964C19, 1.000f,   12,19, false},
    { /* 183 */ "Spruce Fence Gate",      0x785836, 1.000f,    6,12, false},
    { /* 184 */ "Birch Fence Gate",       0xD7C185, 1.000f,    6,13, false},
    { /* 185 */ "Jungle Fence Gate",      0xB1805C, 1.000f,    7,12, false},
    { /* 186 */ "Dark Oak Fence Gate",    0x492F17, 1.000f,    1,22, false},
    { /* 187 */ "Acacia Fence Gate",      0xBA683B, 1.000f,    0,22, false},
    { /* 188 */ "Spruce Fence",           0x785836, 1.000f,    6,12, false},
    { /* 189 */ "Birch Fence",            0xD7C185, 1.000f,    6,13, false},
    { /* 190 */ "Jungle Fence",           0xB1805C, 1.000f,    7,12, false},
    { /* 191 */ "Dark Oak Fence",         0x492F17, 1.000f,    1,22, false},
    { /* 192 */ "Acacia Fence",           0xBA683B, 1.000f,    0,22, false},
    { /* 193 */ "Spruce Door",            0x7A5A36, 1.000f,    1,23, false},
    { /* 194 */ "Birch Door",             0xD6CA8C, 1.000f,    3,23, false},
    { /* 195 */ "Jungle Door",            0xB2825E, 1.000f,    5,23, false},
    { /* 196 */ "Acacia Door",            0xB16640, 1.000f,    7,23, false},
    { /* 197 */ "Dark Oak Door",          0x51341A, 1.000f,    9,23, false},
    { /* 198 */ "End Rod",                0xE0CFD0, 1.000f,   12,23, false},
    { /* 199 */ "Chorus Plant",           0x654765, 1.000f,   13,23, false},
    { /* 200 */ "Chorus Flower",          0x937E93, 1.000f,   14,23, false},
    { /* 201 */ "Purpur Block",           0xA77BA7, 1.000f,    0,24, false},
    { /* 202 */ "Purpur Pillar",          0xAC82AC, 1.000f,    2,24, false},
    { /* 203 */ "Purpur Stairs",          0xA77BA7, 1.000f,    0,24, false},
    { /* 204 */ "Purpur Double Slab",     0xA77BA7, 1.000f,    0,24, false},
    { /* 205 */ "Purpur Slab",            0xA77BA7, 1.000f,    0,24, false},
    { /* 206 */ "End Stone Bricks",       0xE2E8AC, 1.000f,    3,24, false},
    { /* 207 */ "Beetroot Seeds",         0x6D7F44, 1.000f,    7,24, false},
    { /* 208 */ "Grass Path",             0x977E48, 1.000f,    8,24, false},
    { /* 209 */ "End Gateway",            0x1A1828, 1.000f,    8,11, false},
    { /* 210 */ "Repeating Command Block",0x8577B2, 1.000f,   14,24, false},
    { /* 211 */ "Chain Command Block",    0x8AA59A, 1.000f,    2,25, false},
    { /* 212 */ "Frosted Ice",            0x81AFFF, 0.613f,    6,25, false},
    { /* 213 */ "Magma Block",            0x9D4E1D, 1.000f,    0,26, false},
    { /* 214 */ "Nether Wart Block",      0x770C0D, 1.000f,    1,26, false},
    { /* 215 */ "Red Nether Brick",       0x470709, 1.000f,    2,26, false},
    { /* 216 */ "Bone Block",             0xE1DDC9, 1.000f,    4,26, false},
    { /* 217 */ "Structure Void",         0xff0000, 0.000f,    1, 8, false},
    { /* 218 */ "Observer",               0x6E6E6E, 1.000f,   14, 6, false},
    { /* 219 */ "White Shulker Box",      0xDFDFDF, 1.000f,    0, 4, false},
    { /* 220 */ "Orange Shulker Box",     0xDB7E40, 1.000f,    0, 4, false},
    { /* 221 */ "Magenta Shulker Box",    0xB452BD, 1.000f,    0, 4, false},
    { /* 222 */ "Light Blue Shulker Box", 0x6D8BC9, 1.000f,    0, 4, false},
    { /* 223 */ "Yellow Shulker Box",     0xB2A728, 1.000f,    0, 4, false},
    { /* 224 */ "Lime Shulker Box",       0x42AF39, 1.000f,    0, 4, false},
    { /* 225 */ "Pink Shulker Box",       0xC9738B, 1.000f,    0, 4, false},
    { /* 226 */ "Gray Shulker Box",       0x414141, 1.000f,    0, 4, false},
    { /* 227 */ "Light Grey Shulker Box", 0x9CA2A2, 1.000f,    0, 4, false},
    { /* 228 */ "Cyan Shulker Box",       0x2F6F8A, 1.000f,    0, 4, false},
    { /* 229 */ "Purple Shulker Box",     0x7F3EB6, 1.000f,    0, 4, false},
    { /* 230 */ "Blue Shulker Box",       0x2E398E, 1.000f,    0, 4, false},
    { /* 231 */ "Brown Shulker Box",      0x50331F, 1.000f,    0, 4, false},
    { /* 232 */ "Green Shulker Box",      0x35471C, 1.000f,    0, 4, false},
    { /* 233 */ "Red Shulker Box",        0x983431, 1.000f,    0, 4, false},
    { /* 234 */ "Black Shulker Box",      0x1B1717, 1.000f,    0, 4, false},
    { /* 235 */ "Unknown Block",          0x565656, 1.000f,    1, 1, false},
    { /* 236 */ "Unknown Block",          0x565656, 1.000f,    1, 1, false},
    { /* 237 */ "Unknown Block",          0x565656, 1.000f,    1, 1, false},
    { /* 238 */ "Unknown Block",          0x565656, 1.000f,    1, 1, false},
    { /* 239 */ "Unknown Block",          0x565656, 1.000f,    1, 1, false},
    { /* 240 */ "Unknown Block",          0x565656, 1.000f,    1, 1, false},
    { /* 241 */ "Unknown Block",          0x565656, 1.000f,    1, 1, false},
    { /* 242 */ "Unknown Block",          0x565656, 1.000f,    1, 1, false},
    { /* 243 */ "Unknown Block",          0x565656, 1.000f,    1, 1, false},
    { /* 244 */ "Unknown Block",          0x565656, 1.000f,    1, 1, false},
    { /* 245 */ "Unknown Block",          0x565656, 1.000f,    1, 1, false},
    { /* 246 */ "Unknown Block",          0x565656, 1.000f,    1, 1, false},
    { /* 247 */ "Unknown Block",          0x565656, 1.000f,    1, 1, false},
    { /* 248 */ "Unknown Block",          0x565656, 1.000f,    1, 1, false},
    { /* 249 */ "Unknown Block",          0x565656, 1.000f,    1, 1, false},
    { /* 250 */ "Unknown Block",          0x565656, 1.000f,    1, 1, false},
    { /* 251 */ "Unknown Block",          0x565656, 1.000f,    1, 1, false},
    { /* 252 */ "Unknown Block",          0x565656, 1.000f,    1, 1, false},
    { /* 253 */ "Unknown Block",          0x565656, 1.000f,    1, 1, false},
    { /* 254 */ "Unknown Block",          0x565656, 1.000f,    1, 1, false},
    { /* 255 */ "Structure Block",        0x665E5F, 1.000f,   10,25, false}
};


Color4unorm8 ParseSchematic::schematicBlockColor(uint8 blockIndex) {
    const Color3 c = Color3::fromARGB(schematicPalette[blockIndex].color);
    // Gamma correct
    return Color4unorm8(Color3unorm8(c * c), 
        // use emissive in alpha
        schematicPalette[blockIndex].emissive ? unorm8::fromBits(255) : unorm8::fromBits(0));
        //unorm8(schematicPalette[blockIndex].alpha));
}


void ParseSchematic::parse(const String& filename) {
    const int64 size = FileSystem::size(filename);

    // Schematic files are gzipped, so we open and read them using gz functions from zlib
    gzFile file = gzopen(filename.c_str(), "rb");

    // Hack to make sure we have enough space; max out at 2 GB
    const size_t estimatedDecompressedSize = std::min(size_t(size) * 1000, size_t(1) * 1024 * 1024 * 1024);
//    alwaysAssertM(estimatedDecompressedSize < 0xEFFFFFFF, "Estimated size overflow");
    const size_t gzInternalBufferSize = 1024 * 1024;
    gzbuffer(file, uint32(gzInternalBufferSize) ); 

    void* buffer = System::malloc(estimatedDecompressedSize);

    const size_t actualBytesRead = gzread(file, buffer, int(estimatedDecompressedSize)); //TODO: revert
    gzclose(file);
    {
        BinaryInput bi((uint8*)buffer, actualBytesRead, G3D_BIG_ENDIAN, false, false);    
        parseSchematicTag(bi);
    }
    System::free(buffer);
    buffer = nullptr;

}


void ParseSchematic::parseSchematicTag(BinaryInput& bi, int processTagType) {

    // http://minecraft.gamepedia.com/Schematic_file_format (Schematic file format spec)
    // http://minecraft.gamepedia.com/NBT_format (NBT format spec)

    if (! bi.hasMore()) return;

    uint8 tagID = 0;
    String name;

    if (processTagType != -1) {
        tagID = uint8(processTagType);
    } else {
        tagID = bi.readUInt8();

        // tag_END has no name, so we disregard it and keep reading
        if (tagID == 0) { 
            return;
        }

        const uint16 name_len = bi.readUInt16();
        name = bi.readFixedLengthString(name_len);
    }

    switch (tagID) {
    case 0:
        return;

    case 1:
        bi.readInt8();
        break;
        
    case 2: {
        const int16 dimension = bi.readInt16();
        if (name == "Width") {
            size.x = dimension;
        } else if (name == "Height") {
            size.y = dimension;
        } else if (name == "Length") {
            size.z = dimension;
        }
    }
    break;

    case 3:
        bi.readInt32();
        break;

    case 4:
        bi.readInt64();
        break;

    case 5:
        bi.readFloat32();
        break;

    case 6:
        bi.readFloat64();
        break;

    case 7: {
        const int32 size = bi.readInt32();
        if (name == "Blocks") {
            blockID.resize(size);
            bi.readBytes(blockID.getCArray(), size);
        } else if (name == "Data") {
            blockData.resize(size);
            bi.readBytes(blockData.getCArray(), size);
        } else {
            bi.skip(size);
        }
    }
    break;
    
    case 8: {
        const int16 str_len = bi.readInt16();
        if (name == "Materials"){
            materials = bi.readFixedLengthString(str_len);
        } else {
            bi.skip(str_len);
        }
    }
    break;

    case 9: {
        const uint8 tag_ID = bi.readUInt8();
        const int32 size = bi.readInt32();

        // Parse the tag without the preprocessing
        for (int i = 0; i < size; ++i) {
            parseSchematicTag(bi, tag_ID);
        }
    }
    break;

    case 10: // Schematic, entities, tile entities
        parseSchematicTag(bi);
        break;

    case 11: {// tag_int_arry
        const int32 size = bi.readInt32();
        bi.skip(size * 4);
    } break;
        
    default:
        alwaysAssertM(false, format("Error, invalid tag %u read from file.", tagID));
    }

    // Always parse the next tag if there's more to parse
    if (processTagType == -1) {
        parseSchematicTag(bi);
    }
}


void ParseSchematic::getSparseVoxelTable(BlockIDVoxels& voxel) const {
    Point3int16 v;
    int i = 0;
    // Assume 16 voxels per column
    voxel.clear(size.x * size.z * 16);

    for (v.y = 0; v.y < size.y; ++v.y) {
        for (v.z = 0; v.z < size.z; ++v.z) {
            for (v.x = 0; v.x < size.x; ++v.x, ++i) {
                const uint8 id  = blockID[i];
                // Remove transparent blocks
                if (id != 0) {
                    voxel[v] = id;
                }
            }
        }
    }
}


void ParseSchematic::getSparseVoxelTable(ColorVoxels& voxel) const {
    Point3int16 v;
    int i = 0;
    voxel.clear(size.x * size.z * 16);
    debugAssert(blockID.size() == size.x * size.y * size.z);
    for (v.y = 0; v.y < size.y; ++v.y) {
        for (v.z = 0; v.z < size.z; ++v.z) {
            for (v.x = 0; v.x < size.x; ++v.x, ++i) {
                const uint8 id  = blockID[i];
                // Skip air blocks
                if (id != 0) {
                    voxel[v] = schematicBlockColor(id);
                }
            }
        }
    }
}
} // G3D
