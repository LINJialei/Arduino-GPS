clear all;
lat = 48.259506;
lon = 4.031537;
alt = 128.7;
B = 48.259376;
L = 4.031630;
H = 112.5;
[x,y,z]=lla2ecef(lat,lon,alt);
result=BLH_XYZ(B,L,H);