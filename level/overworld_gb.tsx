<?xml version="1.0" encoding="UTF-8"?>
<tileset version="1.2" tiledversion="1.3.3" name="overworld_gb" tilewidth="16" tileheight="16" tilecount="32" columns="8">
 <image source="../pix/overworld_a_gbc.png" trans="ff00ff" width="128" height="64"/>
 <terraintypes>
  <terrain name="grass" tile="0"/>
  <terrain name="tree" tile="13"/>
 </terraintypes>
 <tile id="0" terrain="0,0,0,0"/>
 <tile id="1" terrain="0,0,0,0" probability="0.01"/>
 <tile id="2" terrain="0,0,0,0" probability="0.02"/>
 <tile id="3" terrain="0,0,0,0" probability="0.1"/>
 <tile id="4" terrain="0,0,0,0" probability="0.05"/>
 <tile id="8" terrain="0,0,0,1"/>
 <tile id="9" terrain="0,0,1,0"/>
 <tile id="10" terrain="1,0,0,1"/>
 <tile id="11" terrain="0,1,1,0"/>
 <tile id="12" terrain="0,1,0,0"/>
 <tile id="13" terrain="1,0,0,0"/>
 <wangsets>
  <wangset name="path" tile="23">
   <wangedgecolor name="grass" color="#ff0000" tile="-1" probability="1"/>
   <wangedgecolor name="path" color="#00ff00" tile="-1" probability="1"/>
   <wangtile tileid="0" wangid="0x1010101"/>
   <wangtile tileid="1" wangid="0x1010101"/>
   <wangtile tileid="2" wangid="0x1010101"/>
   <wangtile tileid="3" wangid="0x1010101"/>
   <wangtile tileid="4" wangid="0x1010101"/>
   <wangtile tileid="20" wangid="0x1010101"/>
   <wangtile tileid="21" wangid="0x1020102"/>
   <wangtile tileid="22" wangid="0x2010201"/>
   <wangtile tileid="23" wangid="0x2020202"/>
  </wangset>
  <wangset name="tree" tile="12">
   <wangcornercolor name="grass" color="#ff0000" tile="-1" probability="1"/>
   <wangcornercolor name="tree" color="#00ff00" tile="-1" probability="1"/>
   <wangtile tileid="0" wangid="0x10101010"/>
   <wangtile tileid="1" wangid="0x10101010"/>
   <wangtile tileid="2" wangid="0x10101010"/>
   <wangtile tileid="3" wangid="0x10101010"/>
   <wangtile tileid="4" wangid="0x10101010"/>
   <wangtile tileid="8" wangid="0x10202020"/>
   <wangtile tileid="9" wangid="0x20202010"/>
   <wangtile tileid="10" wangid="0x20102010"/>
   <wangtile tileid="11" wangid="0x10201020"/>
   <wangtile tileid="12" wangid="0x20102020"/>
   <wangtile tileid="13" wangid="0x20201020"/>
  </wangset>
 </wangsets>
</tileset>
