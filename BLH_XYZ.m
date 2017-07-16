function Result=BLH_XYZ(B,L,H)  
%transform the (B L H) into (X Y Z) under the WGS84 coordinate system 
%by bofeng Li 25/8/2006 
 
AE_WGS84=6378137.0;       % Major axis of the WGS84 ellipsoid 
Alfa_WGS84=298.257223563; %1 over flattening  of the WGS 84 ellipsoid 
 
e2=0.00669437999013; 
 
N=AE_WGS84/sqrt(1.0-e2*sin(B)*sin(B)); 
TmpV=N+H; 
Result.X=TmpV*cos(B)*cos(L); 
Result.Y=TmpV*cos(B)*sin(L); 
Result.Z=(N*(1-e2)+H)*sin(B); 
 
return 